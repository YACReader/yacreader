#include "data_base_management.h"
#include "db_helper.h"
#include "library_creator.h"
#include "xml_info_library_scanner.h"
#include "yacreader_global.h"

#include <QBuffer>
#include <QColor>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QRegularExpression>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTemporaryDir>
#include <QtTest>

#include <functional>
#include <memory>

using namespace YACReader;

namespace {

quint32 crc32(const QByteArray &data)
{
    quint32 crc = 0xFFFFFFFF;
    for (const auto byte : data) {
        crc ^= static_cast<quint8>(byte);
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : crc >> 1;
        }
    }
    return ~crc;
}

// A ZIP archive with stored (uncompressed) entries.
QByteArray storedZip(const QList<QPair<QString, QByteArray>> &entries)
{
    QByteArray archive;
    QByteArray centralDirectory;
    QDataStream out(&archive, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    QDataStream central(&centralDirectory, QIODevice::WriteOnly);
    central.setByteOrder(QDataStream::LittleEndian);

    for (const auto &[name, data] : entries) {
        const auto fileName = name.toUtf8();
        const auto crc = crc32(data);
        const auto offset = static_cast<quint32>(archive.size());

        out << quint32(0x04034b50) << quint16(20) << quint16(0) << quint16(0) << quint16(0) << quint16(0x21)
            << crc << quint32(data.size()) << quint32(data.size()) << quint16(fileName.size()) << quint16(0);
        out.writeRawData(fileName.constData(), fileName.size());
        out.writeRawData(data.constData(), data.size());

        central << quint32(0x02014b50) << quint16(20) << quint16(20) << quint16(0) << quint16(0) << quint16(0) << quint16(0x21)
                << crc << quint32(data.size()) << quint32(data.size()) << quint16(fileName.size()) << quint16(0) << quint16(0)
                << quint16(0) << quint16(0) << quint32(0) << offset;
        central.writeRawData(fileName.constData(), fileName.size());
    }

    const auto centralOffset = static_cast<quint32>(archive.size());
    out.writeRawData(centralDirectory.constData(), centralDirectory.size());
    out << quint32(0x06054b50) << quint16(0) << quint16(0) << quint16(entries.size()) << quint16(entries.size())
        << quint32(centralDirectory.size()) << centralOffset << quint16(0);

    return archive;
}

QByteArray png(const QColor &color)
{
    QImage image(32, 48, QImage::Format_RGB32);
    image.fill(color);
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return data;
}

}

class LibraryScanTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void createAddsFoldersAndComics();
    void updateMergesDiskAndLibrary();
    void canceledUpdateRollsBackAndReleasesLock();
    void xmlRescanHonorsLocksAndCancellation();
    void folderXmlRescanUsesDatabaseDescendants();
    void folderContentLoadersReturnSortedValues();
    void replacementDecision();
    void cleanupKeepsTheComicInfoOfEntries();
    void replacementMovesEntriesToTheNewComicInfo();
    void replacementWithKnownContentKeepsThatComicInfo();
    void unreadableReplacementKeepsTheComic();
    void replacementKeepsEntriesWhileAnotherFileHasTheOldContent();
    void updateDeletesTheMissingComicsOfRemovedFolders();
    void folderUpdateDeletesTheMissingComicsOfRemovedFolders();
    void updateGivesANewFileItsMissingComic();
    void folderUpdateGivesANewFileItsMissingComic();

private:
    std::unique_ptr<QTemporaryDir> root;
    QString source;

    bool writeComic(const QString &relativePath, const QColor &color, int padding = 0);
    bool removePath(const QString &relativePath);
    bool scan(bool create);
    bool scanFolder(const QString &relativePath);
    // the placeholders 5 (cover key5, in /c/d), 6 (in /c/d and /b), and 7 (cover key7, in /c/d and a reading list)
    bool addMissingComics();
    bool withDatabase(const std::function<bool(QSqlDatabase &)> &function);
    QStringList rows(const QString &sql);
    QString value(const QString &sql);
    bool addToListAndLabel(const QString &comicInfoId);
    QStringList entries();
};

void LibraryScanTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    source = QDir::cleanPath(root->filePath("library"));
    QVERIFY(QDir().mkpath(source));

    QVERIFY(writeComic("a.cbz", Qt::red));
    QVERIFY(writeComic("b/b1.cbz", Qt::green));
    QVERIFY(writeComic("b/b2.cbz", Qt::blue));
    QVERIFY(writeComic("b.cbz", Qt::yellow));
    QVERIFY(writeComic("c/d/d1.cbz", Qt::cyan));
    QVERIFY(writeComic("z.cbz", Qt::magenta));
    QVERIFY(scan(true));
}

void LibraryScanTest::cleanup()
{
    root.reset();
}

bool LibraryScanTest::writeComic(const QString &relativePath, const QColor &color, int padding)
{
    QList<QPair<QString, QByteArray>> entries = { { "page.png", png(color) } };
    if (padding > 0) {
        entries.append({ "padding.txt", QByteArray(padding, 'x') });
    }

    const auto path = source + "/" + relativePath;
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) && file.write(storedZip(entries)) > 0;
}

bool LibraryScanTest::removePath(const QString &relativePath)
{
    const QFileInfo info(source + "/" + relativePath);
    return info.isDir() ? QDir(info.absoluteFilePath()).removeRecursively() : QFile::remove(info.absoluteFilePath());
}

bool LibraryScanTest::scan(bool create)
{
    QSettings settings(root->filePath("settings.ini"), QSettings::IniFormat);
    settings.setValue(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, false);
    settings.sync();

    LibraryCreator creator(&settings);
    bool failed = false;
    connect(&creator, &LibraryCreator::failedCreatingDB, this, [&failed](const QString &error) { qWarning() << error; failed = true; }, Qt::DirectConnection);
    connect(&creator, &LibraryCreator::failedOpeningDB, this, [&failed](const QString &error) { qWarning() << error; failed = true; }, Qt::DirectConnection);

    if (create) {
        creator.createLibrary(source, LibraryPaths::libraryDataPath(source));
    } else {
        creator.updateLibrary(source, LibraryPaths::libraryDataPath(source));
    }
    creator.start();
    return creator.wait(120000) && !failed;
}

bool LibraryScanTest::scanFolder(const QString &relativePath)
{
    QSettings settings(root->filePath("settings.ini"), QSettings::IniFormat);
    settings.setValue(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, false);
    settings.sync();

    LibraryCreator creator(&settings);
    bool failed = false;
    connect(&creator, &LibraryCreator::failedOpeningDB, this, [&failed](const QString &error) { qWarning() << error; failed = true; }, Qt::DirectConnection);

    const auto folderId = value(QString("SELECT id FROM folder WHERE path = '/%1'").arg(relativePath)).toULongLong();
    creator.updateFolder(source, LibraryPaths::libraryDataPath(source), source + "/" + relativePath, folderId);
    creator.start();
    return folderId != 0 && creator.wait(120000) && !failed;
}

bool LibraryScanTest::addMissingComics()
{
    for (const auto key : { "key5", "key7" }) {
        const auto path = LibraryPaths::placeholderCoverPathFromDataPath(LibraryPaths::libraryDataPath(source), key);
        QDir().mkpath(QFileInfo(path).absolutePath());
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly) || file.write(png(Qt::gray)) <= 0)
            return false;
    }

    return withDatabase([&](QSqlDatabase &db) {
        QSqlQuery query(db);
        return query.exec("INSERT INTO placeholder (id, series, number, coverKey) VALUES (5, 'D', '2', 'key5'), (6, 'D', '3', NULL), (7, 'D', '4', 'key7')") &&
                query.exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) "
                           "SELECT id, 5 FROM folder WHERE path = '/c/d' UNION ALL SELECT id, 6 FROM folder WHERE path IN ('/c/d', '/b') "
                           "UNION ALL SELECT id, 7 FROM folder WHERE path = '/c/d'") &&
                query.exec("INSERT INTO reading_list (id, name) VALUES (1, 'List')") &&
                query.exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 0, 7)");
    });
}

bool LibraryScanTest::withDatabase(const std::function<bool(QSqlDatabase &)> &function)
{
    const QString connectionName = "library_scan_test";
    bool success = false;
    {
        auto db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(LibraryPaths::libraryDatabasePath(source));
        success = db.open() && function(db);
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return success;
}

// Each row as its values joined with '|'.
QStringList LibraryScanTest::rows(const QString &sql)
{
    QStringList result;
    withDatabase([&sql, &result](QSqlDatabase &db) {
        QSqlQuery query(db);
        if (!query.exec(sql)) {
            qWarning() << query.lastError().text() << sql;
            return false;
        }
        while (query.next()) {
            QStringList values;
            for (int i = 0; i < query.record().count(); i++) {
                values.append(query.value(i).isNull() ? "NULL" : query.value(i).toString());
            }
            result.append(values.join('|'));
        }
        return true;
    });
    return result;
}

QString LibraryScanTest::value(const QString &sql)
{
    const auto result = rows(sql);
    return result.isEmpty() ? QString() : result.first();
}

bool LibraryScanTest::addToListAndLabel(const QString &comicInfoId)
{
    return withDatabase([&](QSqlDatabase &db) {
        QSqlQuery query(db);
        return query.exec("INSERT INTO reading_list (id, name) VALUES (1, 'List')") &&
                query.exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Tag', 'red', 0)") &&
                DBHelper::appendComicsToReadingList(ReadingListId(1), { ComicInfoId(comicInfoId.toULongLong()) }, db) &&
                DBHelper::appendComicsToLabel(LabelId(1), { ComicInfoId(comicInfoId.toULongLong()) }, db);
    });
}

// "table|id|comic_info_id" of every entry
QStringList LibraryScanTest::entries()
{
    return rows("SELECT 'list', id, comic_info_id FROM reading_list_entry UNION ALL SELECT 'label', id, comic_info_id FROM label_entry ORDER BY 1, 2");
}

static const QString comicsWithFolders = "SELECT c.path, f.path FROM comic c INNER JOIN folder f ON c.parentId = f.id ORDER BY c.path";
static const QString folders = "SELECT path, numChildren FROM folder WHERE id <> 1 ORDER BY path";

void LibraryScanTest::createAddsFoldersAndComics()
{
    QCOMPARE(rows(comicsWithFolders), QStringList({
                                              "/a.cbz|/",
                                              "/b.cbz|/",
                                              "/b/b1.cbz|/b",
                                              "/b/b2.cbz|/b",
                                              "/c/d/d1.cbz|/c/d",
                                              "/z.cbz|/",
                                      }));
    QCOMPARE(rows(folders), QStringList({ "/b|2", "/c|1", "/c/d|1" }));
}

void LibraryScanTest::updateMergesDiskAndLibrary()
{
    const auto keptComics = rows("SELECT id, comicInfoId, path FROM comic WHERE path IN ('/b.cbz', '/b/b1.cbz', '/b/b2.cbz') ORDER BY path");
    const auto keptFolder = value("SELECT id FROM folder WHERE path = '/b'");
    const auto replacedInfo = value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'");
    QCOMPARE(keptComics.size(), 3);

    QVERIFY(removePath("z.cbz"));
    QVERIFY(removePath("c"));
    QVERIFY(writeComic("b/b0.cbz", Qt::white));
    QVERIFY(writeComic("e/e1.cbz", Qt::black));
    QVERIFY(writeComic("a.cbz", Qt::red, 4096));
    QVERIFY(scan(false));

    QCOMPARE(rows(comicsWithFolders), QStringList({
                                              "/a.cbz|/",
                                              "/b.cbz|/",
                                              "/b/b0.cbz|/b",
                                              "/b/b1.cbz|/b",
                                              "/b/b2.cbz|/b",
                                              "/e/e1.cbz|/e",
                                      }));
    QCOMPARE(rows(folders), QStringList({ "/b|3", "/e|1" }));

    QCOMPARE(rows("SELECT id, comicInfoId, path FROM comic WHERE path IN ('/b.cbz', '/b/b1.cbz', '/b/b2.cbz') ORDER BY path"), keptComics);
    QCOMPARE(value("SELECT id FROM folder WHERE path = '/b'"), keptFolder);

    const auto newInfo = value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'");
    QVERIFY(!newInfo.isEmpty());
    QVERIFY(newInfo != replacedInfo);
    QCOMPARE(value("SELECT numPages FROM comic_info WHERE id = " + newInfo), QString("1"));
}

void LibraryScanTest::canceledUpdateRollsBackAndReleasesLock()
{
    const auto before = rows("SELECT fileName FROM comic ORDER BY fileName");
    QVERIFY(writeComic("new.cbz", Qt::gray));
    QSettings settings(root->filePath("cancel-settings.ini"), QSettings::IniFormat);
    settings.setValue(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, false);
    LibraryCreator creator(&settings);
    int updated = 0;
    connect(&creator, &LibraryCreator::updated, &creator, [&] { ++updated; }, Qt::DirectConnection);
    connect(&creator, &LibraryCreator::comicAdded, &creator, [&] { creator.cancel(); }, Qt::DirectConnection);
    creator.updateLibrary(source, LibraryPaths::libraryDataPath(source));
    creator.start();
    QVERIFY(creator.wait(30000));
    QCOMPARE(updated, 0);
    QCOMPARE(rows("SELECT fileName FROM comic ORDER BY fileName"), before);
    {
        LibraryMaintenanceLock next(source);
        QVERIFY(next.tryLock());
    }
    QVERIFY(scan(false));
    QVERIFY(value("SELECT id FROM comic WHERE fileName = 'new.cbz'") != "");
}

void LibraryScanTest::xmlRescanHonorsLocksAndCancellation()
{
    QFile file(source + "/a.cbz");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(file.write(storedZip({ { "page.png", png(Qt::red) }, { "ComicInfo.xml", "<ComicInfo><Title>Imported title</Title></ComicInfo>" } })) > 0);
    file.close();
    QVERIFY(withDatabase([](QSqlDatabase &db) {
        return QSqlQuery(db).exec("UPDATE comic_info SET title = NULL WHERE id IN (SELECT comicInfoId FROM comic WHERE fileName = 'a.cbz')");
    }));
    XMLInfoLibraryScanner scanner;
    {
        LibraryMaintenanceLock owner(source);
        QVERIFY(owner.tryLock());
        scanner.scanLibrary(source, LibraryPaths::libraryDataPath(source));
        QVERIFY(scanner.wait(30000));
        QCOMPARE(scanner.maintenanceFailure().error, QLockFile::LockFailedError);
        QVERIFY(!scanner.completedSuccessfully());
    }
    const auto cancel = connect(&scanner, &XMLInfoLibraryScanner::comicScanned, &scanner, [&] { scanner.cancel(); }, Qt::DirectConnection);
    scanner.scanLibrary(source, LibraryPaths::libraryDataPath(source));
    QVERIFY(scanner.wait(30000));
    QVERIFY(scanner.wasCanceled());
    QVERIFY(!scanner.completedSuccessfully());
    QCOMPARE(value("SELECT COALESCE(title, '') FROM comic_info WHERE id IN (SELECT comicInfoId FROM comic WHERE fileName = 'a.cbz')"), QString());
    disconnect(cancel);
    scanner.scanLibrary(source, LibraryPaths::libraryDataPath(source));
    QVERIFY(scanner.wait(30000));
    QVERIFY2(scanner.completedSuccessfully(), qPrintable(scanner.errorString()));
    QCOMPARE(value("SELECT title FROM comic_info WHERE id IN (SELECT comicInfoId FROM comic WHERE fileName = 'a.cbz')"), QString("Imported title"));
    LibraryMaintenanceLock next(source);
    QVERIFY(next.tryLock());
}

void LibraryScanTest::folderXmlRescanUsesDatabaseDescendants()
{
    for (const auto &path : { QString("a.cbz"), QString("b/b1.cbz") }) {
        QFile file(source + "/" + path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QVERIFY(file.write(storedZip({ { "page.png", png(Qt::red) }, { "ComicInfo.xml", "<ComicInfo><Title>Folder import</Title></ComicInfo>" } })) > 0);
    }
    QVERIFY(withDatabase([](QSqlDatabase &db) {
        return QSqlQuery(db).exec("UPDATE comic_info SET title = NULL");
    }));
    XMLInfoLibraryScanner scanner;
    scanner.scanFolder(source, LibraryPaths::libraryDataPath(source), source + "/b", { });
    QVERIFY(scanner.wait(30000));
    QVERIFY2(scanner.completedSuccessfully(), qPrintable(scanner.errorString()));
    QCOMPARE(value("SELECT title FROM comic_info WHERE id IN (SELECT comicInfoId FROM comic WHERE fileName = 'b1.cbz')"), QString("Folder import"));
    QCOMPARE(value("SELECT COALESCE(title, '') FROM comic_info WHERE id IN (SELECT comicInfoId FROM comic WHERE fileName = 'a.cbz')"), QString());
    scanner.scanFolder(source, LibraryPaths::libraryDataPath(source), source, { });
    QVERIFY(scanner.wait(30000));
    QVERIFY2(scanner.completedSuccessfully(), qPrintable(scanner.errorString()));
    QCOMPARE(value("SELECT title FROM comic_info WHERE id IN (SELECT comicInfoId FROM comic WHERE fileName = 'a.cbz')"), QString("Folder import"));
}

void LibraryScanTest::folderContentLoadersReturnSortedValues()
{
    const auto folderB = value("SELECT id FROM folder WHERE path = '/b'").toULongLong();

    QList<Folder> subfolders;
    QList<ComicDB> comics;
    QVERIFY(withDatabase([&](QSqlDatabase &db) {
        subfolders = DBHelper::getFoldersFromParent(1, db, true);
        comics = DBHelper::getComicsFromParent(folderB, db, true);
        return true;
    }));

    QStringList folderNames;
    for (const auto &folder : subfolders)
        folderNames.append(folder.name);
    QCOMPARE(folderNames, QStringList({ "b", "c" }));

    QStringList comicNames;
    for (const auto &comic : comics) {
        comicNames.append(comic.file.fileName);
        QCOMPARE(comic.file.folderId, FolderId(folderB));
        QVERIFY(comic.info.id != ComicInfoId());
        QVERIFY(!comic.info.hash.isEmpty());
    }
    QCOMPARE(comicNames, QStringList({ "b1.cbz", "b2.cbz" }));
}

void LibraryScanTest::replacementDecision()
{
    QCOMPARE(classifyReplaceComic(ComicInfoId(1), std::nullopt, false), ReplaceComicResult::NotInserted);
    QCOMPARE(classifyReplaceComic(ComicInfoId(1), ComicInfoId(1), true), ReplaceComicResult::SameComicInfo);
    QCOMPARE(classifyReplaceComic(ComicInfoId(1), ComicInfoId(2), false), ReplaceComicResult::NewComicInfo);
    QCOMPARE(classifyReplaceComic(ComicInfoId(1), ComicInfoId(2), true), ReplaceComicResult::KnownComicInfo);
}

void LibraryScanTest::cleanupKeepsTheComicInfoOfEntries()
{
    const auto listedInfo = value("SELECT comicInfoId FROM comic WHERE path = '/z.cbz'");
    const auto unlistedInfo = value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'");
    const auto listedCover = LibraryPaths::coverPathFromLibraryDataPath(LibraryPaths::libraryDataPath(source), value("SELECT hash FROM comic_info WHERE id = " + listedInfo));
    const auto unlistedCover = LibraryPaths::coverPathFromLibraryDataPath(LibraryPaths::libraryDataPath(source), value("SELECT hash FROM comic_info WHERE id = " + unlistedInfo));
    QVERIFY(QFile::exists(listedCover));
    QVERIFY(QFile::exists(unlistedCover));
    QVERIFY(withDatabase([&](QSqlDatabase &db) {
        QSqlQuery query(db);
        return query.exec("INSERT INTO reading_list (id, name) VALUES (1, 'List')") &&
                DBHelper::appendComicsToReadingList(ReadingListId(1), { ComicInfoId(listedInfo.toULongLong()) }, db);
    }));

    QVERIFY(removePath("z.cbz"));
    QVERIFY(removePath("a.cbz"));
    QVERIFY(scan(false));

    QCOMPARE(value("SELECT COUNT(*) FROM comic_info WHERE id = " + listedInfo), QString("1"));
    QCOMPARE(value("SELECT last_known_path FROM reading_list_entry WHERE comic_info_id = " + listedInfo), QString("/z.cbz"));
    QVERIFY(QFile::exists(listedCover));
    QCOMPARE(value("SELECT COUNT(*) FROM comic_info WHERE id = " + unlistedInfo), QString("0"));
    QVERIFY(!QFile::exists(unlistedCover));
}

void LibraryScanTest::replacementMovesEntriesToTheNewComicInfo()
{
    const auto oldInfo = value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'");
    QVERIFY(rows("UPDATE comic_info SET title = 'Kept title' WHERE id = " + oldInfo).isEmpty());
    QVERIFY(addToListAndLabel(oldInfo));
    const auto oldEntries = entries();

    QVERIFY(writeComic("a.cbz", Qt::red, 4096));
    QVERIFY(scan(false));

    const auto newInfo = value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'");
    QVERIFY(newInfo != oldInfo);
    QCOMPARE(entries(), QStringList(oldEntries).replaceInStrings(QRegularExpression("\\|" + oldInfo + "$"), "|" + newInfo));
    QCOMPARE(value("SELECT title FROM comic_info WHERE id = " + newInfo), QString("Kept title"));
    QCOMPARE(value("SELECT COUNT(*) FROM comic_info WHERE id = " + oldInfo), QString("0"));
}

void LibraryScanTest::replacementWithKnownContentKeepsThatComicInfo()
{
    QVERIFY(writeComic("e.cbz", Qt::yellow, 100));
    QVERIFY(scan(false));
    const auto knownInfo = value("SELECT comicInfoId FROM comic WHERE path = '/e.cbz'");
    const auto oldInfo = value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'");
    QVERIFY(rows("UPDATE comic_info SET title = 'Known title' WHERE id = " + knownInfo).isEmpty());
    QVERIFY(rows("UPDATE comic_info SET title = 'Old title' WHERE id = " + oldInfo).isEmpty());
    QVERIFY(addToListAndLabel(oldInfo));
    const auto oldEntries = entries();

    QVERIFY(writeComic("a.cbz", Qt::yellow, 100));
    QVERIFY(scan(false));

    QCOMPARE(value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'"), knownInfo);
    QCOMPARE(entries(), QStringList(oldEntries).replaceInStrings(QRegularExpression("\\|" + oldInfo + "$"), "|" + knownInfo));
    QCOMPARE(value("SELECT title FROM comic_info WHERE id = " + knownInfo), QString("Known title"));
}

void LibraryScanTest::unreadableReplacementKeepsTheComic()
{
    const auto oldComic = value("SELECT id, comicInfoId FROM comic WHERE path = '/a.cbz'");
    const auto oldInfo = value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'");
    QVERIFY(addToListAndLabel(oldInfo));
    const auto oldEntries = entries();

    QFile file(source + "/a.cbz");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(file.write(QByteArray(4096, 'x')) > 0);
    file.close();
    QVERIFY(scan(false));

    QCOMPARE(value("SELECT id, comicInfoId FROM comic WHERE path = '/a.cbz'"), oldComic);
    QCOMPARE(entries(), oldEntries);
    QCOMPARE(value("SELECT COUNT(*) FROM reading_list_entry WHERE last_known_path IS NOT NULL"), QString("0"));
}

void LibraryScanTest::replacementKeepsEntriesWhileAnotherFileHasTheOldContent()
{
    QVERIFY(writeComic("a copy.cbz", Qt::red));
    QVERIFY(scan(false));
    const auto oldInfo = value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'");
    QCOMPARE(value("SELECT comicInfoId FROM comic WHERE path = '/a copy.cbz'"), oldInfo);
    QVERIFY(addToListAndLabel(oldInfo));
    const auto oldEntries = entries();

    QVERIFY(writeComic("a.cbz", Qt::red, 4096));
    QVERIFY(scan(false));

    QVERIFY(value("SELECT comicInfoId FROM comic WHERE path = '/a.cbz'") != oldInfo);
    QCOMPARE(entries(), oldEntries);
    QCOMPARE(value("SELECT COUNT(*) FROM comic_info WHERE id = " + oldInfo), QString("1"));
}

void LibraryScanTest::updateDeletesTheMissingComicsOfRemovedFolders()
{
    QVERIFY(addMissingComics());
    const auto cover5 = LibraryPaths::placeholderCoverPathFromDataPath(LibraryPaths::libraryDataPath(source), "key5");
    const auto cover7 = LibraryPaths::placeholderCoverPathFromDataPath(LibraryPaths::libraryDataPath(source), "key7");

    QVERIFY(removePath("c"));
    QVERIFY(scan(false));

    QCOMPARE(rows("SELECT f.path, fp.placeholder_id FROM folder_placeholder fp INNER JOIN folder f ON f.id = fp.folder_id"), QStringList({ "/b|6" }));
    QCOMPARE(rows("SELECT id FROM placeholder ORDER BY id"), QStringList({ "6", "7" }));
    QVERIFY(!QFile::exists(cover5));
    QVERIFY(QFile::exists(cover7));
}

void LibraryScanTest::folderUpdateDeletesTheMissingComicsOfRemovedFolders()
{
    QVERIFY(addMissingComics());
    const auto cover5 = LibraryPaths::placeholderCoverPathFromDataPath(LibraryPaths::libraryDataPath(source), "key5");

    QVERIFY(removePath("c/d"));
    QVERIFY(scanFolder("c"));

    QCOMPARE(rows("SELECT path FROM folder WHERE path LIKE '/c%' ORDER BY path"), QStringList({ "/c" }));
    QCOMPARE(rows("SELECT f.path, fp.placeholder_id FROM folder_placeholder fp INNER JOIN folder f ON f.id = fp.folder_id"), QStringList({ "/b|6" }));
    QCOMPARE(rows("SELECT id FROM placeholder ORDER BY id"), QStringList({ "6", "7" }));
    QVERIFY(!QFile::exists(cover5));
}

void LibraryScanTest::updateGivesANewFileItsMissingComic()
{
    const auto cover = LibraryPaths::placeholderCoverPathFromDataPath(LibraryPaths::libraryDataPath(source), "key3");
    QDir().mkpath(QFileInfo(cover).absolutePath());
    QFile file(cover);
    QVERIFY(file.open(QIODevice::WriteOnly) && file.write(png(Qt::gray)) > 0);
    file.close();
    QVERIFY(withDatabase([](QSqlDatabase &db) {
        QSqlQuery query(db);
        return query.exec("INSERT INTO placeholder (id, series, number, title, coverKey) VALUES (3, 'Bee', '3', 'The third', 'key3'), (4, 'Bee', '4', NULL, NULL)") &&
                query.exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) SELECT id, 3 FROM folder WHERE path = '/b' UNION ALL SELECT id, 4 FROM folder WHERE path = '/b'") &&
                query.exec("INSERT INTO reading_list (id, name) VALUES (1, 'List')") &&
                query.exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 0, 3)");
    }));

    QVERIFY(writeComic("b/Bee 003.cbz", Qt::darkRed));
    QVERIFY(scan(false));

    const auto comicInfo = value("SELECT comicInfoId FROM comic WHERE path = '/b/Bee 003.cbz'");
    QVERIFY(!comicInfo.isEmpty());
    QCOMPARE(value("SELECT series || '|' || number || '|' || title FROM comic_info WHERE id = " + comicInfo), QString("Bee|3|The third"));
    QCOMPARE(value("SELECT comic_info_id FROM reading_list_entry"), comicInfo);
    QCOMPARE(rows("SELECT placeholder_id FROM folder_placeholder"), QStringList({ "4" }));
    QCOMPARE(rows("SELECT id FROM placeholder"), QStringList({ "4" }));
    QVERIFY(!QFile::exists(cover));
}

void LibraryScanTest::folderUpdateGivesANewFileItsMissingComic()
{
    QVERIFY(withDatabase([](QSqlDatabase &db) {
        QSqlQuery query(db);
        return query.exec("INSERT INTO placeholder (id, series, number) VALUES (2, 'Dee', '2')") &&
                query.exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) SELECT id, 2 FROM folder WHERE path = '/c/d'");
    }));

    QVERIFY(writeComic("c/d/Dee #2.cbz", Qt::darkBlue));
    QVERIFY(scanFolder("c/d"));

    QCOMPARE(value("SELECT ci.number FROM comic c INNER JOIN comic_info ci ON ci.id = c.comicInfoId WHERE c.path = '/c/d/Dee #2.cbz'"), QString("2"));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder"), QStringList({ "0" }));
}

QTEST_GUILESS_MAIN(LibraryScanTest)

#include "main.moc"
