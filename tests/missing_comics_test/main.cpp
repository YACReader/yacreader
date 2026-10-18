#include "data_base_management.h"
#include "db_helper.h"
#include "missing_comic_server_helper.h"
#include "yacreader_server_data_helper.h"

#include <QImage>
#include <QSqlError>
#include <QSqlQuery>
#include <QTest>

class MissingComicsTest : public QObject
{
    Q_OBJECT
private slots:
    void placeholdersAreImportableAndDistinct();
    void collectionsPreserveRelinkAndRemoveEntries();
    void labelMigrationPreservesMembershipAndIsIdempotent();
    void missingEntryJsonHasExplicitStatusAndIdentity();
};

void MissingComicsTest::collectionsPreserveRelinkAndRemoveEntries()
{
    const auto connection = QStringLiteral("missingEntries");
    {
        auto db = QSqlDatabase::addDatabase("QSQLITE", connection);
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());
        QSqlQuery query(db);
        QVERIFY(query.exec("PRAGMA foreign_keys = ON"));
        QVERIFY(DataBaseManagement::createTables(db));
        QVERIFY(query.exec("INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/')"));
        QVERIFY(query.exec("INSERT INTO reading_list (id, name, ordering) VALUES (10, 'List', 0)"));
        // A different id catches a label foreign key accidentally referencing reading_list.
        QVERIFY(query.exec("INSERT INTO label (id, name, color, ordering) VALUES (20, 'Label', 'red', 0)"));

        ComicDB comic;
        comic.parentId = 1;
        comic.name = "issue.cbz";
        comic.path = "/issue.cbz";
        comic.info.hash = QString(40, 'a') + "100";
        comic.info.title = "Issue";
        comic.info.number = "1";
        comic.info.numPages = 25;
        comic.info.coverSizeRatio = 0.7;
        comic.info.originalCoverSize = "700x1000";
        comic.info.type = QVariant::fromValue(YACReader::FileType::Comic);
        QVERIFY(DBHelper::insert(&comic, db, false) > 0);
        DBHelper::insertComicsInReadingList({ comic }, 10, db);
        DBHelper::insertComicsInLabel({ comic }, 20, db);
        DBHelper::removeFromDB(&comic, db);

        const auto listEntries = DBHelper::getCollectionEntries(10, db);
        const auto labelEntries = DBHelper::getCollectionEntries(20, db, true);
        QCOMPARE(listEntries.size(), 1);
        QCOMPARE(labelEntries.size(), 1);
        for (const auto &entry : { listEntries[0], labelEntries[0] }) {
            QCOMPARE(entry.id, qulonglong(0));
            QCOMPARE(entry.info.id, qulonglong(0));
            QCOMPARE(entry.info.title.toString(), QStringLiteral("Issue"));
            QCOMPARE(entry.info.numPages.toInt(), 0);
        }

        const auto listPlaceholder = MissingComicServerHelper::placeholder(1, listEntries[0]);
        const auto labelPlaceholder = MissingComicServerHelper::placeholder(1, labelEntries[0], true);
        QVERIFY(listPlaceholder.id != labelPlaceholder.id);
        QCOMPARE(MissingComicServerHelper::resolve(1, listPlaceholder.id, db).info.hash, listPlaceholder.info.hash);
        QCOMPARE(MissingComicServerHelper::resolve(1, labelPlaceholder.id, db).info.hash, labelPlaceholder.info.hash);
        QVERIFY(!MissingComicServerHelper::resolve(1, MissingComicServerHelper::placeholderId(999, false), db).info.existOnDb);

        QList<ComicDB> missing;
        for (const auto &table : { QStringLiteral("reading_list_entry"), QStringLiteral("label_entry") }) {
            QVERIFY(query.exec("SELECT id, comic_id, title, number FROM " + table));
            QVERIFY(query.next());
            QVERIFY(query.value(1).isNull());
            QCOMPARE(query.value(2).toString(), QStringLiteral("Issue"));
            QCOMPARE(query.value(3).toString(), QStringLiteral("1"));
            ComicDB entry;
            entry.id = 0;
            entry.parentId = query.value(0).toULongLong();
            missing << entry;
            QVERIFY(!query.next());
        }
        DBHelper::reasignOrderToComicsInReadingList(10, QList<ComicDB> { missing[0] }, db);
        DBHelper::reasignOrderToComicsInLabel(20, QList<ComicDB> { missing[1] }, db);

        // The retained comic_info row is reused when the same hash reappears at a new path.
        comic.info.existOnDb = true;
        comic.path = "/moved/issue.cbz";
        QVERIFY(DBHelper::insert(&comic, db, false) > 0);
        QVERIFY(!MissingComicServerHelper::resolve(1, listPlaceholder.id, db).info.existOnDb);
        QVERIFY(!MissingComicServerHelper::resolve(1, labelPlaceholder.id, db).info.existOnDb);
        QCOMPARE(DBHelper::getCollectionEntries(10, db).first().path, comic.path);
        QCOMPARE(DBHelper::getCollectionEntries(20, db, true).first().id, comic.id);
        for (const auto &table : { QStringLiteral("reading_list_entry"), QStringLiteral("label_entry"), QStringLiteral("comic_reading_list"), QStringLiteral("comic_label") }) {
            QVERIFY(query.exec("SELECT comic_id, ordering FROM " + table));
            QVERIFY(query.next());
            QCOMPARE(query.value(0).toULongLong(), comic.id);
            QCOMPARE(query.value(1).toInt(), 0);
            QVERIFY(!query.next());
        }
        DBHelper::removeFromDB(&comic, db);
        DBHelper::deleteComicsFromReadingList({ missing[0] }, 10, db);
        DBHelper::deleteComicsFromLabel({ missing[1] }, 20, db);
        for (const auto &table : { QStringLiteral("reading_list_entry"), QStringLiteral("label_entry") }) {
            QVERIFY(query.exec("SELECT count(*) FROM " + table));
            QVERIFY(query.next());
            QCOMPARE(query.value(0).toInt(), 0);
        }
    }
    QSqlDatabase::removeDatabase(connection);
}

void MissingComicsTest::labelMigrationPreservesMembershipAndIsIdempotent()
{
    const auto connection = QStringLiteral("labelMigration");
    {
        auto db = QSqlDatabase::addDatabase("QSQLITE", connection);
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());
        QSqlQuery query(db);
        QVERIFY(query.exec("PRAGMA foreign_keys = ON"));
        QVERIFY(DataBaseManagement::createTables(db));
        QVERIFY(query.exec("DROP TABLE label_entry"));
        QVERIFY(query.exec("INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/')"));
        QVERIFY(query.exec("INSERT INTO label (id, name, color, ordering) VALUES (20, 'Label', 'red', 0)"));
        QVERIFY(query.exec("INSERT INTO comic_info (id, hash, title, number) VALUES (30, 'hash', 'Issue', '2')"));
        QVERIFY(query.exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (40, 1, 30, 'issue.cbz', '/issue.cbz')"));
        QVERIFY(query.exec("INSERT INTO comic_label VALUES (40, 20, 7)"));
        QVERIFY(DataBaseManagement::createLabelEntryTable(db));
        QVERIFY(DataBaseManagement::createLabelEntryTable(db));
        QVERIFY(query.exec("SELECT label_id, comic_id, ordering, title FROM label_entry"));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 20);
        QCOMPARE(query.value(1).toInt(), 40);
        QCOMPARE(query.value(2).toInt(), 7);
        QCOMPARE(query.value(3).toString(), QStringLiteral("Issue"));
        QVERIFY(!query.next());
        // Removing the label must cascade its entries without needing a matching list id.
        QVERIFY(query.exec("DELETE FROM label WHERE id = 20"));
        QVERIFY(query.exec("SELECT count(*) FROM label_entry"));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 0);
    }
    QSqlDatabase::removeDatabase(connection);
}

void MissingComicsTest::missingEntryJsonHasExplicitStatusAndIdentity()
{
    ComicDB comic;
    comic.id = 0;
    comic.info.id = 0;
    comic.parentId = 123;
    comic.info.title = "Issue";
    comic.info.hash = QString(40, 'a') + "100";
    const auto json = YACReaderServerDataHelper::comicToJSON(1, QUuid(), comic);
    QVERIFY(json.value("missing").toBool());
    QCOMPARE(json.value("entry_id").toString(), QStringLiteral("123"));
    QVERIFY(json.value("parent_id").isNull());
    QCOMPARE(json.value("file_size").toString(), QStringLiteral("0"));
    QCOMPARE(json.value("missing_message").toString(), QStringLiteral("This comic is missing from your collection."));
    comic.id = 9;
    QVERIFY(!YACReaderServerDataHelper::comicToJSON(1, QUuid(), comic).value("missing").toBool());
}

void MissingComicsTest::placeholdersAreImportableAndDistinct()
{
    ComicDB entry;
    entry.id = 0;
    entry.parentId = 123;
    entry.info.title = "Issue";
    entry.info.hash = QString(40, 'a') + "100";
    auto comic = MissingComicServerHelper::placeholder(1, entry);
    QVERIFY(MissingComicServerHelper::isPlaceholderId(comic.id));
    QVERIFY(comic.id < (qulonglong(1) << 53));
    QCOMPARE(comic.info.id, comic.id);
    QVERIFY(comic.info.existOnDb);
    QCOMPARE(comic.info.numPages.toInt(), 1);
    QCOMPARE(comic.info.coverPage.toInt(), 0);
    QCOMPARE(comic.info.synopsis.toString(), MissingComicServerHelper::description());
    QCOMPARE(comic.getFileName(), QStringLiteral("[MISSING] Issue.cbz"));
    entry.info.series = "G.I. Joe - Duke";
    entry.info.number = "1";
    QCOMPARE(MissingComicServerHelper::placeholder(1, entry).getFileName(), QStringLiteral("[MISSING] G.I. Joe - Duke #1.cbz"));
    QCOMPARE(MissingComicServerHelper::placeholder(1, entry).info.title.toString(), QStringLiteral("[MISSING] G.I. Joe - Duke"));
    const auto namedPlaceholder = MissingComicServerHelper::placeholder(1, entry);
    const auto navigationJson = YACReaderServerDataHelper::comicToJSON(1, QUuid(), namedPlaceholder);
    QCOMPARE(navigationJson.value("number").toInt(), 1);
    QCOMPARE(navigationJson.value("universal_number").toString(), QStringLiteral("1"));
    QVERIFY(!navigationJson.contains("title"));
    QCOMPARE(navigationJson.value("file_name").toString(), QStringLiteral("[MISSING] G.I. Joe - Duke #1.cbz"));
    const auto importJson = YACReaderServerDataHelper::fullComicToJSON(1, QUuid(), namedPlaceholder);
    QCOMPARE(importJson.value("universal_number").toString(), QStringLiteral("1"));
    QCOMPARE(importJson.value("title").toString(), QStringLiteral("[MISSING] G.I. Joe - Duke"));
    entry.info.number = "#1.A";
    QCOMPARE(MissingComicServerHelper::placeholder(1, entry).getFileName(), QStringLiteral("[MISSING] G.I. Joe - Duke #1.A.cbz"));
    entry.info.series = "Series: Part/Two";
    QCOMPARE(MissingComicServerHelper::placeholder(1, entry).getFileName(), QStringLiteral("[MISSING] Series- Part-Two #1.A.cbz"));
    entry.info.series = QVariant();
    entry.info.title = QVariant();
    entry.info.number = QVariant();
    QCOMPARE(MissingComicServerHelper::placeholder(1, entry).getFileName(), QStringLiteral("[MISSING].cbz"));
    const auto bytes = MissingComicServerHelper::cover();
    QVERIFY(!QImage::fromData(bytes, "JPG").isNull());
    QCOMPARE(comic.getFileSize(), qulonglong(bytes.size()));
    QVERIFY(comic.info.hash != entry.info.hash);
    QCOMPARE(MissingComicServerHelper::idFromHash(comic.info.hash), comic.id);
    // An ordinary hash whose numeric prefix happens to fall in the reserved id
    // range must still participate in normal progress sync.
    QCOMPARE(MissingComicServerHelper::idFromHash(QStringLiteral("10000000000002") + QString(26, 'a') + "100"), qulonglong(0));
    QCOMPARE(MissingComicServerHelper::placeholder(1, entry).info.hash, comic.info.hash);
    QVERIFY(MissingComicServerHelper::placeholder(2, entry).info.hash != comic.info.hash);
    QVERIFY(MissingComicServerHelper::placeholder(1, entry, true).id != comic.id);
    entry.parentId++;
    QVERIFY(MissingComicServerHelper::placeholder(1, entry).id != comic.id);
    QVERIFY(comic.toTXT().contains("numpages:1\r\n"));
    QVERIFY(comic.toTXT().contains("synopsis:" + MissingComicServerHelper::description()));
    const auto json = YACReaderServerDataHelper::fullComicToJSON(1, QUuid(), comic);
    QVERIFY(json.value("missing").toBool());
    QCOMPARE(json.value("num_pages").toInt(), 1);
    QCOMPARE(json.value("id").toString(), QString::number(comic.id));
    QCOMPARE(json.value("file_size").toString().toULongLong(), qulonglong(bytes.size()));
    QCOMPARE(json.value("synopsis").toString(), MissingComicServerHelper::description());
    QVERIFY(json.value("parent_id").isNull());
}

QTEST_GUILESS_MAIN(MissingComicsTest)
#include "main.moc"
