#include "missing_comic_server_helper.h"

#include "data_base_management.h"
#include "db_helper.h"
#include "yacreader_libraries.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QImage>
#include <QSqlQuery>

static void initializeMissingCoverResource()
{
    Q_INIT_RESOURCE(missing_comic_cover);
}

namespace {
// Keep wire ids positive, exactly representable by JSON clients, and separate
// from ordinary SQLite row ids. The low bit distinguishes labels from lists.
constexpr qulonglong placeholderBase = qulonglong(1) << 52;
constexpr qulonglong placeholderLimit = qulonglong(1) << 53;

ComicDB notFound()
{
    ComicDB comic;
    comic.id = 0;
    comic.info.id = 0;
    return comic;
}
}

bool MissingComicServerHelper::isPlaceholderId(qulonglong id)
{
    return id >= placeholderBase && id < placeholderLimit;
}

qulonglong MissingComicServerHelper::placeholderId(qulonglong entryId, bool label)
{
    if (entryId == 0 || entryId >= placeholderBase / 2)
        return 0;
    return placeholderBase + entryId * 2 + (label ? 1 : 0);
}

QString MissingComicServerHelper::description()
{
    return QStringLiteral("This comic is missing from your collection.");
}

QByteArray MissingComicServerHelper::cover()
{
    static const QByteArray bytes = [] {
        initializeMissingCoverResource();
        const QImage image(QStringLiteral(":/images/defaultCover.png"));
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "JPG");
        return data;
    }();
    return bytes;
}

ComicDB MissingComicServerHelper::placeholder(qulonglong libraryId, const ComicDB &entry, bool label)
{
    if (entry.id != 0)
        return entry;
    ComicDB comic = entry;
    comic.id = placeholderId(entry.parentId, label);
    if (comic.id == 0)
        return notFound();
    comic.info.id = comic.id;
    comic.info.existOnDb = true;
    QString name = entry.info.series.toString().trimmed();
    if (name.isEmpty())
        name = entry.info.title.toString().trimmed();
    comic.info.title = name.isEmpty() ? QStringLiteral("[MISSING]") : QStringLiteral("[MISSING] %1").arg(name);
    QString number = entry.info.number.toString().trimmed();
    if (!number.isEmpty()) {
        if (!number.startsWith(QLatin1Char('#')))
            number.prepend(QLatin1Char('#'));
        if (!name.isEmpty())
            name += QLatin1Char(' ');
        name += number;
    }
    // These names also travel through the colon-delimited legacy import format.
    for (auto &character : name) {
        if (character.unicode() < 32 || QStringLiteral("/\\:*?\"<>|").contains(character))
            character = QLatin1Char('-');
    }
    comic.name = name.isEmpty() ? QStringLiteral("[MISSING].cbz") : QStringLiteral("[MISSING] %1.cbz").arg(name);
    comic.path = QLatin1Char('/') + comic.name;
    comic.info.synopsis = description();
    comic.info.numPages = 1;
    comic.info.currentPage = 0;
    comic.info.coverPage = 0;
    comic.info.read = false;
    comic.info.hasBeenOpened = false;
    comic.info.type = QVariant::fromValue(YACReader::FileType::Comic);
    const QImage image = QImage::fromData(cover());
    comic.info.coverSizeRatio = image.height() ? double(image.width()) / image.height() : 0.7;
    comic.info.originalCoverSize = QStringLiteral("%1x%2").arg(image.width()).arg(image.height());
    const auto key = QStringLiteral("yacreader-missing:%1:%2").arg(libraryId).arg(comic.id).toUtf8();
    const auto digest = QString::fromLatin1(QCryptographicHash::hash(key, QCryptographicHash::Sha1).toHex());
    // Encode a marker and wire id in a normal 40-character hex hash so the legacy cover
    // route can resolve it. Never advertise the real comic's retained hash:
    // importing a placeholder must not make the client think it has that comic.
    comic.info.hash = QStringLiteral("facade00%1").arg(comic.id, 14, 16, QLatin1Char('0')) + digest.mid(22) + QString::number(cover().size());
    return comic;
}

qulonglong MissingComicServerHelper::idFromHash(const QString &hash)
{
    if (hash.size() <= 40 || !hash.startsWith(QStringLiteral("facade00")))
        return 0;
    bool valid = false;
    const auto id = hash.mid(8, 14).toULongLong(&valid, 16);
    return valid && isPlaceholderId(id) ? id : 0;
}

ComicDB MissingComicServerHelper::resolve(qulonglong libraryId, qulonglong comicId, QSqlDatabase &db)
{
    if (!isPlaceholderId(comicId))
        return notFound();
    const bool label = (comicId & 1) != 0;
    const auto entryId = (comicId - placeholderBase) / 2;
    const auto table = label ? QStringLiteral("label_entry") : QStringLiteral("reading_list_entry");
    QSqlQuery query(db);
    query.prepare(QStringLiteral("SELECT e.* FROM %1 e LEFT JOIN comic c ON c.id = e.comic_id "
                                 "WHERE e.id = :id AND c.id IS NULL")
                          .arg(table));
    query.bindValue(":id", entryId);
    if (!query.exec() || !query.next())
        return notFound();
    ComicDB entry = notFound();
    entry.parentId = entryId;
    entry.info.title = query.value("title");
    entry.info.number = query.value("number");
    entry.info.series = query.value("series");
    entry.info.volume = query.value("volume");
    entry.info.date = query.value("date");
    entry.info.storyArc = query.value("story_arc");
    return placeholder(libraryId, entry, label);
}

ComicDB MissingComicServerHelper::resolve(qulonglong libraryId, qulonglong comicId)
{
    if (!isPlaceholderId(comicId))
        return DBHelper::getComicInfo(libraryId, comicId);
    const auto libraries = DBHelper::getLibraries();
    if (!libraries.contains(libraryId))
        return notFound();
    ComicDB comic = notFound();
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        comic = resolve(libraryId, comicId, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return comic;
}
