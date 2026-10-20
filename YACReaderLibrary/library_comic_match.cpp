#include "library_comic_match.h"

#include "QsLog.h"
#include "comic_db.h"
#include "db_helper.h"
#include "descriptive_comic_metadata.h"
#include "placeholder.h"
#include "yacreader_global.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

using namespace YACReader;

namespace {

const char *const entryTables[] = { "reading_list_entry", "label_entry" };

bool isUnknown(const QVariant &value)
{
    return value.isNull() || value.toString().trimmed().isEmpty();
}

// the empty fields of `comic` that `missing` knows
DescriptiveMetadataPatch fillPatch(const DescriptiveComicMetadata &comic, const DescriptiveComicMetadata &missing)
{
    DescriptiveMetadataPatch patch;
    for (const auto &column : DescriptiveComicMetadata::columns()) {
        if (isUnknown(comic.*column.member) && !isUnknown(missing.*column.member))
            patch.set(column.member, missing.*column.member);
    }
    return patch;
}

// the comic_info, only when a comic file uses it
std::optional<ComicInfo> loadComicWithFile(ComicInfoId id, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("SELECT 1 FROM comic WHERE comicInfoId = :id LIMIT 1");
    query.bindValue(":id", id.value());
    if (!query.exec() || !query.next())
        return std::nullopt;
    return DBHelper::loadComicInfo(id, db);
}

bool exec(QSqlQuery &query, const char *what)
{
    if (query.exec())
        return true;
    QLOG_ERROR() << "Unable to" << what << ":" << query.lastError().text();
    return false;
}

// the label entries of `column` = `id` whose label already has the comic
bool removeLabelRepeats(const char *column, qulonglong id, ComicInfoId comicInfoId, QSqlDatabase &db)
{
    QSqlQuery remove(db);
    remove.prepare(QString("DELETE FROM label_entry WHERE %1 = :id "
                           "AND EXISTS (SELECT 1 FROM label_entry other WHERE other.label_id = label_entry.label_id AND other.comic_info_id = :comic_info_id)")
                           .arg(column));
    remove.bindValue(":id", id);
    remove.bindValue(":comic_info_id", comicInfoId.value());
    return exec(remove, "remove label entries that repeat a comic");
}

QString idList(const QList<qulonglong> &ids)
{
    QStringList texts;
    for (const auto id : ids)
        texts.append(QString::number(id));
    return texts.join(',');
}

// the placeholder's cover becomes the comic's custom cover
bool copyPlaceholderCover(const QString &coverKey, ComicInfo &info, const QString &libraryDataPath, QSqlDatabase &db, ComicMatchFiles &files)
{
    const auto source = LibraryPaths::placeholderCoverPathFromDataPath(libraryDataPath, coverKey);
    const auto destination = LibraryPaths::coverPathFromLibraryDataPath(libraryDataPath, info.hash);
    const auto size = QImageReader(source).size();
    if (!size.isValid() || !QDir().mkpath(QFileInfo(destination).absolutePath()) || !QFile::copy(source, destination)) {
        QLOG_ERROR() << "Unable to copy the cover of a missing comic to" << destination;
        return false;
    }
    files.writtenCoverPaths.append(destination);

    info.coverPage = QVariant();
    info.originalCoverSize = QStringLiteral("%1x%2").arg(size.width()).arg(size.height());
    info.coverSizeRatio = static_cast<double>(size.width()) / size.height();
    info.lastTimeCoverSet = QDateTime::currentSecsSinceEpoch();
    info.usesExternalCover = true;
    return DBHelper::updateComicInfoCover(info, db);
}

}

bool matchPlaceholderToComic(PlaceholderId placeholderId, ComicInfoId comicInfoId, const QString &libraryDataPath, QSqlDatabase &db, ComicMatchFiles &files)
{
    const auto placeholder = DBHelper::loadPlaceholder(placeholderId, db);
    auto info = loadComicWithFile(comicInfoId, db);
    if (!placeholder || !info)
        return false;

    if (!DBHelper::updateComicInfoMetadata(comicInfoId, fillPatch(*info, *placeholder), db))
        return false;

    if (placeholder->coverKey) {
        if (!QFile::exists(LibraryPaths::coverPathFromLibraryDataPath(libraryDataPath, info->hash)) &&
            !copyPlaceholderCover(*placeholder->coverKey, *info, libraryDataPath, db, files))
            return false;
        files.placeholderCoverKeys.append(*placeholder->coverKey);
    }

    if (!removeLabelRepeats("placeholder_id", placeholderId.value(), comicInfoId, db))
        return false;
    for (const auto table : entryTables) {
        QSqlQuery move(db);
        move.prepare(QString("UPDATE %1 SET comic_info_id = :comic_info_id, placeholder_id = NULL WHERE placeholder_id = :placeholder_id").arg(table));
        move.bindValue(":comic_info_id", comicInfoId.value());
        move.bindValue(":placeholder_id", placeholderId.value());
        if (!exec(move, "give a comic to the entries of a placeholder"))
            return false;
    }

    QSqlQuery remove(db);
    remove.prepare("DELETE FROM placeholder WHERE id = :id");
    remove.bindValue(":id", placeholderId.value());
    return exec(remove, "delete a matched placeholder") && remove.numRowsAffected() == 1;
}

bool matchDeletedComicToComic(ComicInfoId deletedComicInfoId, ComicInfoId comicInfoId, QSqlDatabase &db, ComicMatchFiles &files)
{
    if (deletedComicInfoId == comicInfoId)
        return false;

    const auto deleted = DBHelper::loadComicInfo(deletedComicInfoId, db);
    const auto info = loadComicWithFile(comicInfoId, db);
    if (!deleted || !info || loadComicWithFile(deletedComicInfoId, db))
        return false;

    if (!DBHelper::updateComicInfoMetadata(comicInfoId, fillPatch(*info, *deleted), db))
        return false;

    const bool takeReadState = !info->read && !info->hasBeenOpened;
    const bool takeRating = info->rating <= 0;
    QSqlQuery state(db);
    state.prepare("UPDATE comic_info SET read = :read, hasBeenOpened = :hasBeenOpened, rating = :rating, lastTimeOpened = :lastTimeOpened WHERE id = :id");
    state.bindValue(":read", takeReadState ? deleted->read : info->read);
    state.bindValue(":hasBeenOpened", takeReadState ? deleted->hasBeenOpened : info->hasBeenOpened);
    state.bindValue(":rating", takeRating ? deleted->rating : info->rating);
    state.bindValue(":lastTimeOpened", deleted->lastTimeOpened.toLongLong() > info->lastTimeOpened.toLongLong() ? deleted->lastTimeOpened : info->lastTimeOpened);
    state.bindValue(":id", comicInfoId.value());
    if (!exec(state, "merge the reading state of a deleted comic"))
        return false;

    if (!removeLabelRepeats("comic_info_id", deletedComicInfoId.value(), comicInfoId, db))
        return false;
    // the last known file was of the deleted comic
    for (const auto table : entryTables) {
        QSqlQuery move(db);
        move.prepare(QString("UPDATE %1 SET comic_info_id = :comic_info_id, last_known_name = NULL, last_known_path = NULL "
                             "WHERE comic_info_id = :deleted_comic_info_id")
                             .arg(table));
        move.bindValue(":comic_info_id", comicInfoId.value());
        move.bindValue(":deleted_comic_info_id", deletedComicInfoId.value());
        if (!exec(move, "give a comic to the entries of a deleted comic"))
            return false;
    }

    QSqlQuery remove(db);
    remove.prepare("DELETE FROM comic_info WHERE id = :id");
    remove.bindValue(":id", deletedComicInfoId.value());
    if (!exec(remove, "delete a matched deleted comic") || remove.numRowsAffected() != 1)
        return false;

    files.comicCoverHashes.append(deleted->hash);
    return true;
}

void finishComicMatchFiles(const QString &libraryDataPath, const ComicMatchFiles &files, bool committed)
{
    if (!committed) {
        for (const auto &path : files.writtenCoverPaths)
            QFile::remove(path);
        return;
    }

    DBHelper::removePlaceholderCovers(libraryDataPath, files.placeholderCoverKeys);
    for (const auto &hash : files.comicCoverHashes)
        QFile::remove(LibraryPaths::coverPathFromLibraryDataPath(libraryDataPath, hash));
}

bool giveComicToEntry(ListItemId entryId, ComicInfoId comicInfoId, QSqlDatabase &db)
{
    if (entryId.listKind == ListKind::Label) {
        QSqlQuery remove(db);
        remove.prepare("DELETE FROM label_entry WHERE id = :id "
                       "AND EXISTS (SELECT 1 FROM label_entry other WHERE other.label_id = label_entry.label_id AND other.comic_info_id = :comic_info_id AND other.id <> :id)");
        remove.bindValue(":id", entryId.id);
        remove.bindValue(":comic_info_id", comicInfoId.value());
        if (!exec(remove, "remove a label entry that repeats a comic"))
            return false;
        if (remove.numRowsAffected() == 1)
            return true;
    }

    QSqlQuery update(db);
    update.prepare(QString("UPDATE %1 SET comic_info_id = :comic_info_id, placeholder_id = NULL, last_known_name = NULL, last_known_path = NULL WHERE id = :id")
                           .arg(entryId.listKind == ListKind::Label ? "label_entry" : "reading_list_entry"));
    update.bindValue(":comic_info_id", comicInfoId.value());
    update.bindValue(":id", entryId.id);
    return exec(update, "give a comic to an entry") && update.numRowsAffected() == 1;
}

std::optional<QStringList> otherListsWithEntriesOf(const QList<MissingComic> &missing, ListKind listKind, const QSet<qulonglong> &listIds, QSqlDatabase &db)
{
    QList<qulonglong> placeholderIds;
    QList<qulonglong> comicInfoIds;
    for (const auto &value : missing) {
        if (const auto *id = std::get_if<PlaceholderId>(&value))
            placeholderIds.append(id->value());
        else
            comicInfoIds.append(std::get<ComicInfoId>(value).value());
    }
    if (missing.isEmpty())
        return QStringList();

    QStringList refersTo;
    if (!placeholderIds.isEmpty())
        refersTo.append("e.placeholder_id IN (" + idList(placeholderIds) + ")");
    if (!comicInfoIds.isEmpty())
        refersTo.append("e.comic_info_id IN (" + idList(comicInfoIds) + ")");
    const auto excluded = idList(QList<qulonglong>(listIds.cbegin(), listIds.cend()));
    const auto readingListsExcluded = listKind == ListKind::ReadingList && !excluded.isEmpty() ? excluded : QStringLiteral("0");
    const auto labelsExcluded = listKind == ListKind::Label && !excluded.isEmpty() ? excluded : QStringLiteral("0");

    QSqlQuery query(db);
    query.setForwardOnly(true);
    if (!query.exec(QString("SELECT name FROM ("
                            "SELECT rl.name AS name FROM reading_list rl WHERE rl.id NOT IN (%2) "
                            "AND EXISTS (SELECT 1 FROM reading_list_entry e WHERE e.reading_list_id = rl.id AND (%1)) "
                            "UNION ALL "
                            "SELECT l.name AS name FROM label l WHERE l.id NOT IN (%3) "
                            "AND EXISTS (SELECT 1 FROM label_entry e WHERE e.label_id = l.id AND (%1))) "
                            "ORDER BY name COLLATE NOCASE")
                            .arg(refersTo.join(" OR "), readingListsExcluded, labelsExcluded))) {
        QLOG_ERROR() << "Unable to find the other lists of missing comics:" << query.lastError().text();
        return std::nullopt;
    }

    QStringList names;
    while (query.next())
        names.append(query.value(0).toString());
    return names;
}
