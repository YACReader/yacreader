#include "data_base_management.h"
#include "db_helper.h"
#include "folder.h"
#include "placeholder.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTest>

class FolderPlaceholderTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void aPlaceholderIsInAFolderOnce();
    void aFolderLoadsOnlyItsPlaceholders();
    void removingThePlacementDeletesAnUnusedPlaceholder();
    void removingThePlacementKeepsAUsedPlaceholder();
    void removingTheLastEntryKeepsAPlacedPlaceholder();
    void aPlacedPlaceholderCannotBeDeleted();
    void deletingAFolderRemovesThePlacementsBelowIt();
    void anIssueGetsANewPlaceholderInTheFolder();
    void anIssueReusesItsPlaceholder();
    void anIssueWithAComicInTheFolderIsNotPlaced();

private:
    bool exec(const QString &sql);
    QList<QVariantList> rows(const QString &sql);

    QSqlDatabase db;
};

void FolderPlaceholderTest::init()
{
    db = QSqlDatabase::addDatabase("QSQLITE", "folder_placeholder_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());
    QVERIFY(exec("PRAGMA foreign_keys = ON"));
    QVERIFY(DataBaseManagement::createTables(db));
    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/'), (2, 1, 'Batman', '/Batman'), "
                 "(3, 2, 'Annuals', '/Batman/Annuals'), (4, 1, 'Robin', '/Robin')"));
    QVERIFY(exec("INSERT INTO placeholder (id, series, number, coverKey) VALUES (5, 'Batman', '1', 'key5'), (6, 'Batman', '2', NULL), "
                 "(7, 'Batman Annual', '1', 'key7')"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'Event')"));
}

void FolderPlaceholderTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("folder_placeholder_test");
}

bool FolderPlaceholderTest::exec(const QString &sql)
{
    QSqlQuery query(db);
    return query.exec(sql);
}

QList<QVariantList> FolderPlaceholderTest::rows(const QString &sql)
{
    QList<QVariantList> result;
    QSqlQuery query(db);
    if (!query.exec(sql))
        return result;
    while (query.next()) {
        QVariantList row;
        for (int i = 0; i < query.record().count(); i++)
            row.append(query.value(i));
        result.append(row);
    }
    return result;
}

void FolderPlaceholderTest::aPlaceholderIsInAFolderOnce()
{
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(5), FolderId(2), db));
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(5), FolderId(2), db));
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(5), FolderId(4), db));
    QVERIFY(!DBHelper::placePlaceholderInFolder(PlaceholderId(99), FolderId(2), db));
    QVERIFY(!DBHelper::placePlaceholderInFolder(PlaceholderId(5), FolderId(99), db));

    QCOMPARE(rows("SELECT folder_id, placeholder_id FROM folder_placeholder ORDER BY folder_id"), QList<QVariantList>({ { 2, 5 }, { 4, 5 } }));
}

void FolderPlaceholderTest::aFolderLoadsOnlyItsPlaceholders()
{
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(6), FolderId(2), db));
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(5), FolderId(2), db));
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(7), FolderId(3), db));

    const auto placeholders = DBHelper::loadFolderPlaceholders(FolderId(2), db);
    QVERIFY(placeholders);
    QCOMPARE(placeholders->size(), 2);
    QCOMPARE(placeholders->at(0).id, PlaceholderId(5));
    QCOMPARE(placeholders->at(0).number.toString(), QString("1"));
    QCOMPARE(placeholders->at(0).coverKey, std::optional<QString>("key5"));
    QCOMPARE(placeholders->at(1).id, PlaceholderId(6));
    QCOMPARE(DBHelper::loadFolderPlaceholders(FolderId(4), db)->size(), 0);
}

void FolderPlaceholderTest::removingThePlacementDeletesAnUnusedPlaceholder()
{
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(5), FolderId(2), db));

    QCOMPARE(DBHelper::removePlaceholdersFromFolder({ PlaceholderId(5) }, FolderId(2), db), std::optional<QStringList>({ "key5" }));
    QCOMPARE(rows("SELECT COUNT(*) FROM folder_placeholder"), QList<QVariantList>({ { 0 } }));
    QCOMPARE(rows("SELECT id FROM placeholder ORDER BY id"), QList<QVariantList>({ { 6 }, { 7 } }));
}

void FolderPlaceholderTest::removingThePlacementKeepsAUsedPlaceholder()
{
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(5), FolderId(2), db));
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(5), FolderId(4), db));
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(6), FolderId(2), db));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 0, 6)"));

    QCOMPARE(DBHelper::removePlaceholdersFromFolder({ PlaceholderId(5), PlaceholderId(6) }, FolderId(2), db), std::optional<QStringList>(QStringList()));

    QCOMPARE(rows("SELECT folder_id, placeholder_id FROM folder_placeholder"), QList<QVariantList>({ { 4, 5 } }));
    QCOMPARE(rows("SELECT id FROM placeholder ORDER BY id"), QList<QVariantList>({ { 5 }, { 6 }, { 7 } }));
}

void FolderPlaceholderTest::removingTheLastEntryKeepsAPlacedPlaceholder()
{
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(5), FolderId(2), db));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, placeholder_id) VALUES (1, 1, 0, 5), (2, 1, 1, 6)"));

    const auto coverKeys = DBHelper::removeReadingListEntries(ReadingListId(1), { ListItemId { ListKind::ReadingList, 1 }, ListItemId { ListKind::ReadingList, 2 } }, db);

    QCOMPARE(coverKeys, std::optional<QStringList>(QStringList()));
    QCOMPARE(rows("SELECT id FROM placeholder ORDER BY id"), QList<QVariantList>({ { 5 }, { 7 } }));
}

void FolderPlaceholderTest::aPlacedPlaceholderCannotBeDeleted()
{
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(5), FolderId(2), db));

    QVERIFY(!exec("DELETE FROM placeholder WHERE id = 5"));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder WHERE id = 5"), QList<QVariantList>({ { 1 } }));
}

void FolderPlaceholderTest::deletingAFolderRemovesThePlacementsBelowIt()
{
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(5), FolderId(2), db));
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(6), FolderId(2), db));
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(6), FolderId(4), db));
    QVERIFY(DBHelper::placePlaceholderInFolder(PlaceholderId(7), FolderId(3), db));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 0, 5)"));

    Folder folder;
    folder.id = FolderId(2);
    DBHelper::removeFromDB(&folder, db);
    QCOMPARE(rows("SELECT folder_id, placeholder_id FROM folder_placeholder"), QList<QVariantList>({ { 4, 6 } }));

    QCOMPARE(DBHelper::removeUnusedPlaceholders(db), std::optional<QStringList>({ "key7" }));
    QCOMPARE(rows("SELECT id FROM placeholder ORDER BY id"), QList<QVariantList>({ { 5 }, { 6 } }));
}

void FolderPlaceholderTest::anIssueGetsANewPlaceholderInTheFolder()
{
    DescriptiveMetadataPatch patch;
    patch.set(&DescriptiveComicMetadata::series, "Batman");
    patch.set(&DescriptiveComicMetadata::number, "12");
    patch.set(&DescriptiveComicMetadata::comicVineID, "4012");

    bool coverUsed = false;
    QVERIFY(DBHelper::placeIssueInFolder("4012", patch, QString("new"), FolderId(2), db, coverUsed));
    QVERIFY(coverUsed);
    QCOMPARE(rows("SELECT p.series, p.number, p.comicVineID, p.coverKey, fp.folder_id FROM placeholder p INNER JOIN folder_placeholder fp ON fp.placeholder_id = p.id"),
             QList<QVariantList>({ { "Batman", "12", "4012", "new", 2 } }));

    // the same issue in the same folder again changes nothing but the metadata
    QVERIFY(DBHelper::placeIssueInFolder("4012", patch, QString("other"), FolderId(2), db, coverUsed));
    QVERIFY(!coverUsed);
    QCOMPARE(rows("SELECT COUNT(*) FROM folder_placeholder"), QList<QVariantList>({ { 1 } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder WHERE comicVineID = '4012'"), QList<QVariantList>({ { 1 } }));
}

void FolderPlaceholderTest::anIssueReusesItsPlaceholder()
{
    QVERIFY(exec("UPDATE placeholder SET comicVineID = '4005' WHERE id = 5"));
    QVERIFY(exec("UPDATE placeholder SET comicVineID = '4006' WHERE id = 6"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, placeholder_id) VALUES (1, 0, 5)"));
    DescriptiveMetadataPatch patch;
    patch.set(&DescriptiveComicMetadata::title, "Tagged");

    bool coverUsed = false;
    QVERIFY(DBHelper::placeIssueInFolder("4005", patch, QString("new"), FolderId(4), db, coverUsed));
    QVERIFY(!coverUsed);
    QVERIFY(DBHelper::placeIssueInFolder("4006", patch, QString("new6"), FolderId(4), db, coverUsed));
    QVERIFY(coverUsed);

    QCOMPARE(rows("SELECT placeholder_id FROM folder_placeholder WHERE folder_id = 4 ORDER BY placeholder_id"), QList<QVariantList>({ { 5 }, { 6 } }));
    QCOMPARE(rows("SELECT id, title, coverKey FROM placeholder WHERE id IN (5, 6) ORDER BY id"), QList<QVariantList>({ { 5, "Tagged", "key5" }, { 6, "Tagged", "new6" } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder"), QList<QVariantList>({ { 3 } }));
}

void FolderPlaceholderTest::anIssueWithAComicInTheFolderIsNotPlaced()
{
    QVERIFY(exec("INSERT INTO comic_info (id, hash, comicVineID) VALUES (1, 'h1', '4001')"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (1, 2, 1, 'a.cbz', '/Batman/a.cbz')"));
    DescriptiveMetadataPatch patch;
    patch.set(&DescriptiveComicMetadata::comicVineID, "4001");

    bool coverUsed = true;
    QVERIFY(DBHelper::placeIssueInFolder("4001", patch, QString("new"), FolderId(2), db, coverUsed));
    QVERIFY(!coverUsed);
    QCOMPARE(rows("SELECT COUNT(*) FROM folder_placeholder"), QList<QVariantList>({ { 0 } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder"), QList<QVariantList>({ { 3 } }));

    // another folder does not have it
    QVERIFY(DBHelper::placeIssueInFolder("4001", patch, std::nullopt, FolderId(4), db, coverUsed));
    QCOMPARE(rows("SELECT folder_id FROM folder_placeholder"), QList<QVariantList>({ { 4 } }));
}

QTEST_GUILESS_MAIN(FolderPlaceholderTest)
#include "main.moc"
