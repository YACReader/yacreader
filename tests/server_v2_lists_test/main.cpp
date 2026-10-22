#include "data_base_management.h"
#include "db_helper.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <QDir>
#include <QJsonArray>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include <algorithm>
#include <memory>

// the v2 list routes read DBHelper::getReadingListFullContent and DBHelper::getLabelComics; the folder routes read
// DBHelper::getFolderComicsFromLibrary
class ServerV2ListsTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void readingListLeavesOutDeletedComics();
    void labelLeavesOutDeletedComics();
    void comicInfoWithSeveralFilesGivesItsFirstFile();
    void readerSiblingsLeaveOutDeletedComics();
    void outputIsTheSameWithoutTheDeletedEntries();
    void readingListGivesTheEntryOfEachComic();
    void repeatedComicsAreNavigatedAtTheirFirstOccurrence();
    void folderLeavesOutMissingComics();

private:
    bool exec(const QString &sql);
    QList<qulonglong> entryIdsOf(const QString &sql);
    QJsonArray toJson(const QList<ComicDB> &comics) const;
    static QList<qulonglong> ids(const QList<ComicDB> &comics);

    std::unique_ptr<QTemporaryDir> root;
    QString libraryPath;
    int libraryId = -1;
};

static const QString libraryName = "server_v2_lists_test";

void ServerV2ListsTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void ServerV2ListsTest::cleanupTestCase()
{
    YACReaderLibraries libraries;
    libraries.load();
    if (libraries.contains(libraryName)) {
        libraries.remove(libraryName);
        libraries.save();
    }
}

void ServerV2ListsTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    libraryPath = QDir::cleanPath(root->filePath("library"));
    QVERIFY(QDir().mkpath(YACReader::LibraryPaths::libraryDataPath(libraryPath)));

    QString name;
    {
        auto db = DataBaseManagement::createDatabase(YACReader::LibraryPaths::libraryDatabasePath(libraryPath));
        name = db.connectionName();
        db.close();
    }
    QSqlDatabase::removeDatabase(name);

    YACReaderLibraries libraries;
    libraries.load();
    if (libraries.contains(libraryName))
        libraries.remove(libraryName);
    libraries.addLibrary(libraryName, libraryPath);
    QVERIFY(libraries.save());
    libraryId = libraries.getId(libraryName);

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (2, 1, 'Series', '/Series')"));
    for (int i = 1; i <= 5; i++) {
        QVERIFY(exec(QString("INSERT INTO comic_info (id, hash, title, number, numPages, currentPage, read, hasBeenOpened, added) "
                             "VALUES (%1, 'hash-%1', 'Title %1', '%1', 20, %1, %2, 1, %3)")
                             .arg(i)
                             .arg(i % 2)
                             .arg(1000 + i)));
        QVERIFY(exec(QString("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (%1, 2, %1, 'c%1.cbz', '/Series/c%1.cbz')").arg(i)));
    }
    // a second file with the same content as comic_info 1
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (6, 2, 1, 'c6.cbz', '/Series/c6.cbz')"));

    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'List')"));
    QVERIFY(exec("INSERT INTO reading_list (id, name, parentId, ordering) VALUES (2, 'Child', 1, 0)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES "
                 "(1, 0, 2), (1, 1, 4), (1, 2, 1), (1, 3, 4), (1, 4, 3), (2, 0, 4), (2, 1, 5)"));
    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Tag', 'red', 0)"));
    QVERIFY(exec("INSERT INTO label_entry (label_id, ordering, comic_info_id) VALUES (1, 0, 4), (1, 1, 3), (1, 2, 1)"));

    // comic_info 4 loses its file but stays, because the lists refer to it
    QVERIFY(exec("DELETE FROM comic WHERE id = 4"));
}

void ServerV2ListsTest::cleanup()
{
    root.reset();
}

bool ServerV2ListsTest::exec(const QString &sql)
{
    QString name;
    bool ok = false;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraryPath));
        name = db.connectionName();
        QSqlQuery query(db);
        ok = query.exec(sql);
        if (!ok)
            qWarning() << sql;
    }
    QSqlDatabase::removeDatabase(name);
    return ok;
}

QList<qulonglong> ServerV2ListsTest::entryIdsOf(const QString &sql)
{
    QList<qulonglong> ids;
    QString name;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraryPath));
        name = db.connectionName();
        QSqlQuery query(db);
        if (query.exec(sql)) {
            while (query.next())
                ids.append(query.value(0).toULongLong());
        }
    }
    QSqlDatabase::removeDatabase(name);
    return ids;
}

QJsonArray ServerV2ListsTest::toJson(const QList<ComicDB> &comics) const
{
    const auto libraryUuid = DBHelper::getLibraries().getLibraryIdFromLegacyId(libraryId);
    QJsonArray items;
    for (const auto &comic : comics)
        items.append(YACReaderServerDataHelper::comicToJSON(libraryId, libraryUuid, comic));
    return items;
}

QList<qulonglong> ServerV2ListsTest::ids(const QList<ComicDB> &comics)
{
    QList<qulonglong> ids;
    for (const auto &comic : comics)
        ids.append(comic.file.id.value());
    return ids;
}

void ServerV2ListsTest::readingListLeavesOutDeletedComics()
{
    const auto comics = DBHelper::getReadingListFullContent(libraryId, 1);
    QCOMPARE(ids(comics), QList<qulonglong>({ 2, 1, 3, 5 }));

    const auto items = toJson(comics);
    for (int i = 0; i < comics.size(); i++) {
        const auto comic = DBHelper::getComicInfo(libraryId, comics.at(i).file.id.value());
        QVERIFY(comic.has_value());
        QCOMPARE(items.at(i).toObject(), toJson({ *comic }).at(0).toObject());
    }
}

void ServerV2ListsTest::labelLeavesOutDeletedComics()
{
    const auto comics = DBHelper::getLabelComics(libraryId, 1);
    QCOMPARE(ids(comics), QList<qulonglong>({ 3, 1 }));

    const auto items = toJson(comics);
    for (int i = 0; i < comics.size(); i++) {
        const auto comic = DBHelper::getComicInfo(libraryId, comics.at(i).file.id.value());
        QVERIFY(comic.has_value());
        QCOMPARE(items.at(i).toObject(), toJson({ *comic }).at(0).toObject());
    }
}

void ServerV2ListsTest::comicInfoWithSeveralFilesGivesItsFirstFile()
{
    for (const auto &comics : { DBHelper::getReadingListFullContent(libraryId, 1), DBHelper::getLabelComics(libraryId, 1) }) {
        QVERIFY(!ids(comics).contains(6));
        const auto first = std::find_if(comics.cbegin(), comics.cend(), [](const ComicDB &comic) { return comic.info.id == ComicInfoId(1); });
        QVERIFY(first != comics.cend());
        QCOMPARE(first->file.id, ComicId(1));
        QCOMPARE(first->file.fileName, QString("c1.cbz"));
        QCOMPARE(first->file.path, QString("/Series/c1.cbz"));
    }
}

// ComicControllerInReadingListV2 gives the comics before and after the open comic in this list
void ServerV2ListsTest::readerSiblingsLeaveOutDeletedComics()
{
    const auto siblings = ids(DBHelper::getReadingListFullContent(libraryId, 1));
    const auto i = siblings.indexOf(1);
    QVERIFY(i > 0 && i < siblings.size() - 1);
    QCOMPARE(siblings.at(i - 1), qulonglong(2));
    QCOMPARE(siblings.at(i + 1), qulonglong(3));
}

void ServerV2ListsTest::outputIsTheSameWithoutTheDeletedEntries()
{
    const auto readingList = toJson(DBHelper::getReadingListFullContent(libraryId, 1));
    const auto label = toJson(DBHelper::getLabelComics(libraryId, 1));

    QVERIFY(exec("DELETE FROM reading_list_entry WHERE comic_info_id = 4"));
    QVERIFY(exec("DELETE FROM label_entry WHERE comic_info_id = 4"));

    QCOMPARE(toJson(DBHelper::getReadingListFullContent(libraryId, 1)), readingList);
    QCOMPARE(toJson(DBHelper::getLabelComics(libraryId, 1)), label);
}

// the desktop reader finds the opened occurrence by its entry
void ServerV2ListsTest::readingListGivesTheEntryOfEachComic()
{
    QList<ListItemId> entryIds;
    const auto comics = DBHelper::getReadingListFullContent(libraryId, 1, &entryIds);
    QCOMPARE(entryIds.size(), comics.size());

    const auto expected = entryIdsOf("SELECT e.id FROM reading_list_entry e INNER JOIN comic c ON c.comicInfoId = e.comic_info_id "
                                     "WHERE c.id = (SELECT MIN(id) FROM comic WHERE comicInfoId = e.comic_info_id) "
                                     "ORDER BY e.reading_list_id, e.ordering, e.id");
    QCOMPARE(entryIds.size(), expected.size());
    for (int i = 0; i < entryIds.size(); i++) {
        QCOMPARE(entryIds.at(i).listKind, ListKind::ReadingList);
        QCOMPARE(entryIds.at(i).id, expected.at(i));
        const auto comicInfoId = entryIdsOf(QString("SELECT comic_info_id FROM reading_list_entry WHERE id = %1").arg(expected.at(i)));
        QCOMPARE(comics.at(i).info.id.value(), comicInfoId.value(0));
    }
}

// FolderContentControllerV2 and FolderInfoControllerV2 read the comics of a folder; the reader reads its siblings
void ServerV2ListsTest::folderLeavesOutMissingComics()
{
    const auto folder = toJson(DBHelper::getFolderComicsFromLibrary(libraryId, 2));
    const auto reading = toJson(DBHelper::getFolderComicsFromLibraryForReading(libraryId, 2));

    QVERIFY(exec("INSERT INTO placeholder (id, series, number, title) VALUES (1, 'Series', '0', 'Missing')"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 1)"));

    QCOMPARE(toJson(DBHelper::getFolderComicsFromLibrary(libraryId, 2)), folder);
    QCOMPARE(toJson(DBHelper::getFolderComicsFromLibraryForReading(libraryId, 2)), reading);
}

void ServerV2ListsTest::repeatedComicsAreNavigatedAtTheirFirstOccurrence()
{
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES (1, 5, 2)"));

    const auto comics = DBHelper::getReadingListFullContent(libraryId, 1);
    QCOMPARE(ids(comics), QList<qulonglong>({ 2, 1, 3, 2, 5 }));
    QCOMPARE(ids(YACReaderServerDataHelper::firstOccurrences(comics)), QList<qulonglong>({ 2, 1, 3, 5 }));
}

QTEST_GUILESS_MAIN(ServerV2ListsTest)

#include "main.moc"
