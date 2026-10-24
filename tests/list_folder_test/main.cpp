#include "data_base_management.h"
#include "db_helper.h"
#include "list_folder.h"
#include "reading_list.h"
#include "smart_list.h"

#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTest>

#include <functional>

class ListFolderTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void foldersNestInTheirOrder();
    void renameChangesTheName();
    void aFolderNeverMovesIntoItself();
    void listsMoveIntoFoldersOfTheirKind();
    void kindsCannotBeMixed();
    void removingAFolderRemovesEverythingInIt();
    void removingASmartListFolderRemovesItsSmartLists();
    void readingListCoversAreTheirFirstComic();
    void readingListCoversAreTheOnesOfTheFirstQuery();
    void helpersDoNotRunInsideATransactionOfTheCaller();

private:
    bool exec(const QString &sql);
    QList<QVariantList> rows(const QString &sql);
    qulonglong folder(ListFolderKind kind, const std::optional<qulonglong> &parentId, const QString &name);

    QSqlDatabase db;
};

void ListFolderTest::init()
{
    db = QSqlDatabase::addDatabase("QSQLITE", "list_folder_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());
    QVERIFY(exec("PRAGMA foreign_keys = ON"));
    QVERIFY(DataBaseManagement::createTables(db));
}

void ListFolderTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("list_folder_test");
}

bool ListFolderTest::exec(const QString &sql)
{
    QSqlQuery query(db);
    return query.exec(sql);
}

QList<QVariantList> ListFolderTest::rows(const QString &sql)
{
    QList<QVariantList> result;
    QSqlQuery query(db);
    if (!query.exec(sql))
        return result;
    while (query.next()) {
        QVariantList row;
        for (int i = 0; i < query.record().count(); i++)
            row.append(query.value(i).isNull() ? QVariant() : query.value(i));
        result.append(row);
    }
    return result;
}

qulonglong ListFolderTest::folder(ListFolderKind kind, const std::optional<qulonglong> &parentId, const QString &name)
{
    const auto id = DBHelper::insertListFolder(kind, parentId, name, db);
    return id ? id->id : 0;
}

void ListFolderTest::foldersNestInTheirOrder()
{
    const auto dc = folder(ListFolderKind::ReadingList, std::nullopt, "DC");
    const auto marvel = folder(ListFolderKind::ReadingList, std::nullopt, "Marvel");
    const auto crisis = folder(ListFolderKind::ReadingList, dc, "Crisis");
    const auto events = folder(ListFolderKind::ReadingList, dc, "Events");
    QVERIFY(dc && marvel && crisis && events);
    QVERIFY(!DBHelper::insertListFolder(ListFolderKind::ReadingList, 999, "Nowhere", db));

    const auto folders = DBHelper::loadListFolders(ListFolderKind::ReadingList, db);
    QVERIFY(folders);
    QCOMPARE(folders->size(), 4);
    QStringList names;
    for (const auto &item : *folders) {
        names.append(item.name);
        QCOMPARE(item.id.kind, ListFolderKind::ReadingList);
    }
    QCOMPARE(names, QStringList({ "DC", "Crisis", "Marvel", "Events" }));
    QCOMPARE(folders->at(0).parentId, std::nullopt);
    QCOMPARE(folders->at(1).parentId, std::optional<qulonglong>(dc));
    QCOMPARE(folders->at(1).ordering, 0);
    QCOMPARE(folders->at(3).ordering, 1);
    QCOMPARE(folders->at(2).ordering, 1);

    // the sections have their own folders
    QVERIFY(DBHelper::loadListFolders(ListFolderKind::SmartList, db)->isEmpty());
}

void ListFolderTest::renameChangesTheName()
{
    const auto dc = folder(ListFolderKind::ReadingList, std::nullopt, "DC");
    QVERIFY(DBHelper::renameListFolder({ ListFolderKind::ReadingList, dc }, "DC Comics", db));
    QCOMPARE(rows("SELECT name FROM reading_list_folder"), QList<QVariantList>({ { "DC Comics" } }));
    QVERIFY(!DBHelper::renameListFolder({ ListFolderKind::ReadingList, 999 }, "Gone", db));
}

void ListFolderTest::aFolderNeverMovesIntoItself()
{
    const auto a = folder(ListFolderKind::ReadingList, std::nullopt, "A");
    const auto b = folder(ListFolderKind::ReadingList, a, "B");
    const auto c = folder(ListFolderKind::ReadingList, b, "C");
    const auto other = folder(ListFolderKind::ReadingList, std::nullopt, "Other");

    QVERIFY(!DBHelper::moveListFolder({ ListFolderKind::ReadingList, a }, a, db));
    QVERIFY(!DBHelper::moveListFolder({ ListFolderKind::ReadingList, a }, c, db));
    QVERIFY(!DBHelper::moveListFolder({ ListFolderKind::ReadingList, a }, 999, db));
    QCOMPARE(rows(QString("SELECT parent_id FROM reading_list_folder WHERE id = %1").arg(a)), QList<QVariantList>({ { QVariant() } }));

    QVERIFY(DBHelper::moveListFolder({ ListFolderKind::ReadingList, c }, std::nullopt, db));
    QVERIFY(DBHelper::moveListFolder({ ListFolderKind::ReadingList, b }, other, db));
    QCOMPARE(rows(QString("SELECT id, parent_id, ordering FROM reading_list_folder WHERE id IN (%1, %2) ORDER BY id").arg(b).arg(c)),
             QList<QVariantList>({ { b, other, 0 }, { c, QVariant(), 2 } }));
}

void ListFolderTest::listsMoveIntoFoldersOfTheirKind()
{
    const auto readingFolder = folder(ListFolderKind::ReadingList, std::nullopt, "Crisis");
    const auto smartFolder = folder(ListFolderKind::SmartList, std::nullopt, "Unread");
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'Infinite Crisis'), (2, 'Final Crisis'), (3, 'Tie-ins')"));
    QVERIFY(exec("UPDATE reading_list SET parentId = 1 WHERE id = 3"));
    const auto smart = DBHelper::insertSmartList("Unread Batman", "series:batman read:false", db);
    QVERIFY(smart);

    QVERIFY(DBHelper::moveReadingListToFolder(ReadingListId(1), readingFolder, db));
    QVERIFY(DBHelper::moveReadingListToFolder(ReadingListId(2), readingFolder, db));
    QCOMPARE(rows("SELECT id, folder_id, ordering FROM reading_list WHERE id IN (1, 2) ORDER BY id"),
             QList<QVariantList>({ { 1, readingFolder, 0 }, { 2, readingFolder, 1 } }));

    // a child list stays under its parent list
    QVERIFY(!DBHelper::moveReadingListToFolder(ReadingListId(3), readingFolder, db));
    QVERIFY(!DBHelper::moveReadingListToFolder(ReadingListId(2), 999, db));
    QVERIFY(!DBHelper::moveReadingListToFolder(ReadingListId(99), readingFolder, db));
    QCOMPARE(rows("SELECT folder_id FROM reading_list WHERE id = 3"), QList<QVariantList>({ { QVariant() } }));

    QVERIFY(DBHelper::moveReadingListToFolder(ReadingListId(1), std::nullopt, db));
    QCOMPARE(rows("SELECT folder_id FROM reading_list WHERE id = 1"), QList<QVariantList>({ { QVariant() } }));

    QVERIFY(DBHelper::moveSmartListToFolder(*smart, smartFolder, db));
    QCOMPARE(DBHelper::loadSmartList(*smart, db)->folderId, std::optional<qulonglong>(smartFolder));
    QVERIFY(DBHelper::moveSmartListToFolder(*smart, std::nullopt, db));
    QCOMPARE(DBHelper::loadSmartList(*smart, db)->folderId, std::nullopt);
}

void ListFolderTest::kindsCannotBeMixed()
{
    const auto smartFolder = folder(ListFolderKind::SmartList, std::nullopt, "Unread");
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'Crisis')"));

    // the foreign keys name the folder table of each kind
    QVERIFY(!exec(QString("UPDATE reading_list SET folder_id = %1 WHERE id = 1").arg(smartFolder)));
    QVERIFY(!exec(QString("INSERT INTO reading_list_folder (id, parent_id, name, ordering) VALUES (50, %1, 'Mixed', 0)").arg(smartFolder)));
    QVERIFY(!DBHelper::moveReadingListToFolder(ReadingListId(1), smartFolder, db));
}

void ListFolderTest::removingAFolderRemovesEverythingInIt()
{
    const auto dc = folder(ListFolderKind::ReadingList, std::nullopt, "DC");
    const auto crisis = folder(ListFolderKind::ReadingList, dc, "Crisis");
    QVERIFY(exec("INSERT INTO comic_info (id, hash) VALUES (10, 'h10')"));
    QVERIFY(exec("INSERT INTO placeholder (id, series, coverKey) VALUES (5, 'Only here', 'cover-5'), (6, 'Also outside', NULL), (7, 'In a subfolder', 'cover-7')"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'Infinite Crisis'), (2, 'Tie-ins'), (3, 'Final Crisis'), (4, 'Outside')"));
    QVERIFY(exec("UPDATE reading_list SET parentId = 1 WHERE id = 2"));
    QVERIFY(DBHelper::moveReadingListToFolder(ReadingListId(1), dc, db));
    QVERIFY(DBHelper::moveReadingListToFolder(ReadingListId(3), crisis, db));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id, placeholder_id) VALUES "
                 "(1, 0, 10, NULL), (2, 0, NULL, 5), (3, 0, NULL, 6), (3, 1, NULL, 7), (4, 0, NULL, 6), (4, 1, 10, NULL)"));
    const auto other = folder(ListFolderKind::ReadingList, std::nullopt, "Marvel");

    const auto covers = DBHelper::removeListFolder({ ListFolderKind::ReadingList, dc }, db);
    QVERIFY(covers);
    auto sortedCovers = *covers;
    sortedCovers.sort();
    QCOMPARE(sortedCovers, QStringList({ "cover-5", "cover-7" }));

    QCOMPARE(rows("SELECT id FROM reading_list_folder"), QList<QVariantList>({ { other } }));
    QCOMPARE(rows("SELECT id FROM reading_list"), QList<QVariantList>({ { 4 } }));
    QCOMPARE(rows("SELECT reading_list_id FROM reading_list_entry ORDER BY id"), QList<QVariantList>({ { 4 }, { 4 } }));
    QCOMPARE(rows("SELECT id FROM placeholder"), QList<QVariantList>({ { 6 } }));
    // the comics stay
    QCOMPARE(rows("SELECT id FROM comic_info"), QList<QVariantList>({ { 10 } }));

    QVERIFY(!DBHelper::removeListFolder({ ListFolderKind::ReadingList, dc }, db));
}

void ListFolderTest::removingASmartListFolderRemovesItsSmartLists()
{
    const auto unread = folder(ListFolderKind::SmartList, std::nullopt, "Unread");
    const auto inner = folder(ListFolderKind::SmartList, unread, "DC");
    const auto first = DBHelper::insertSmartList("Unread Batman", "series:batman read:false", db);
    const auto second = DBHelper::insertSmartList("Unread Flash", "series:flash read:false", db);
    const auto kept = DBHelper::insertSmartList("Rated", "rating>=4", db);
    QVERIFY(first && second && kept);
    QVERIFY(DBHelper::moveSmartListToFolder(*first, unread, db));
    QVERIFY(DBHelper::moveSmartListToFolder(*second, inner, db));

    QCOMPARE(DBHelper::removeListFolder({ ListFolderKind::SmartList, unread }, db), std::optional<QStringList>(QStringList()));
    QVERIFY(DBHelper::loadListFolders(ListFolderKind::SmartList, db)->isEmpty());
    QCOMPARE(rows("SELECT name FROM smart_list"), QList<QVariantList>({ { "Rated" } }));
}

void ListFolderTest::readingListCoversAreTheirFirstComic()
{
    QVERIFY(exec("INSERT INTO comic_info (id, hash) VALUES (10, 'h10'), (11, 'h11'), (12, 'h12')"));
    QVERIFY(exec("INSERT INTO placeholder (id, series) VALUES (5, 'Missing')"));
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name) VALUES "
                 "(1, NULL, 0, 'Own entries'), (2, NULL, 0, 'Only child lists'), (3, 2, 1, 'Second child'), (4, 2, 0, 'First child'), "
                 "(5, NULL, 0, 'Only missing'), (6, NULL, 0, 'Empty')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id, placeholder_id) VALUES "
                 "(1, 1, 11, NULL), (1, 0, NULL, 5), (1, 2, 10, NULL), "
                 "(3, 0, 12, NULL), (4, 0, 10, NULL), "
                 "(5, 0, NULL, 5)"));

    const auto hashes = DBHelper::readingListCoverHashes({ ReadingListId(1), ReadingListId(2), ReadingListId(5), ReadingListId(6) }, db);
    QVERIFY(hashes);
    QCOMPARE(hashes->size(), 2);
    // the first comic of its entries; a missing entry has no cover
    QCOMPARE(hashes->value(1), QString("h11"));
    // the first comic of its child lists, in their order
    QCOMPARE(hashes->value(2), QString("h10"));
    QVERIFY(DBHelper::readingListCoverHashes({ }, db)->isEmpty());
}

void ListFolderTest::readingListCoversAreTheOnesOfTheFirstQuery()
{
    QVERIFY(exec("INSERT INTO placeholder (id, series) VALUES (1, 'Missing')"));
    QSqlQuery insert(db);
    for (int i = 1; i <= 50; i++)
        QVERIFY(insert.exec(QString("INSERT INTO comic_info (id, hash) VALUES (%1, 'h%1')").arg(i)));

    // top lists with child lists and lists inside child lists, equal orderings, missing entries, and empty lists
    QRandomGenerator random(20261023);
    QList<int> topLists;
    QList<ReadingListId> allLists;
    int listId = 0;
    int entryOrdering = 0;
    const std::function<void(QVariant, int)> addList = [&](QVariant parent, int depth) {
        const auto id = ++listId;
        allLists.append(ReadingListId(id));
        if (parent.isNull())
            topLists.append(id);
        QVERIFY(insert.exec(QString("INSERT INTO reading_list (id, parentId, ordering, name) VALUES (%1, %2, %3, 'List')")
                                    .arg(id)
                                    .arg(parent.isNull() ? QString("NULL") : parent.toString())
                                    .arg(random.bounded(3))));
        const auto entries = random.bounded(4);
        for (int e = 0; e < entries; e++) {
            const auto missing = random.bounded(3) == 0;
            QVERIFY(insert.exec(QString("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id, placeholder_id) VALUES (%1, %2, %3, %4)")
                                        .arg(id)
                                        .arg(random.bounded(2) == 0 ? entryOrdering++ : 0)
                                        .arg(missing ? QString("NULL") : QString::number(random.bounded(50) + 1))
                                        .arg(missing ? QString("1") : QString("NULL"))));
        }
        if (depth < 2) {
            const auto children = random.bounded(3);
            for (int c = 0; c < children; c++)
                addList(id, depth + 1);
        }
    };
    for (int l = 0; l < 60; l++)
        addList(QVariant(), 0);

    QStringList idTexts;
    for (const auto &id : allLists)
        idTexts.append(QString::number(id.value()));
    QSqlQuery reference(db);
    QVERIFY(reference.exec(QString("SELECT rl.id, (SELECT ci.hash FROM reading_list_entry e "
                                   "INNER JOIN comic_info ci ON (ci.id = e.comic_info_id) "
                                   "INNER JOIN reading_list l ON (l.id = e.reading_list_id) "
                                   "WHERE l.id = rl.id OR l.parentId = rl.id "
                                   "ORDER BY l.parentId IS NOT NULL, l.ordering, e.ordering, e.id LIMIT 1) "
                                   "FROM reading_list rl WHERE rl.id IN (%1)")
                                   .arg(idTexts.join(','))));
    QHash<qulonglong, QString> expected;
    while (reference.next()) {
        if (!reference.value(1).isNull())
            expected.insert(reference.value(0).toULongLong(), reference.value(1).toString());
    }
    QVERIFY(expected.size() > 20);
    QVERIFY(expected.size() < allLists.size());

    const auto hashes = DBHelper::readingListCoverHashes(allLists, db);
    QVERIFY(hashes);
    QCOMPARE(*hashes, expected);
}

void ListFolderTest::helpersDoNotRunInsideATransactionOfTheCaller()
{
    QVERIFY(exec("INSERT INTO reading_list_folder (id, parent_id, name, ordering) VALUES (1, NULL, 'Folder', 0)"));
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name) VALUES (1, NULL, 0, 'One'), (2, NULL, 1, 'Two')"));
    QVERIFY(exec("INSERT INTO placeholder (id, series) VALUES (5, 'Missing')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, placeholder_id) VALUES (1, 1, 0, 5)"));

    QVERIFY(db.transaction());
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name) VALUES (9, NULL, 2, 'Caller')"));

    QVERIFY(!DBHelper::moveReadingListToFolder(ReadingListId(1), 1, db));
    QVERIFY(!DBHelper::appendRowsToReadingList(ReadingListId(1), { ComicRowId(PlaceholderId(5)) }, db));
    QVERIFY(!DBHelper::reorderReadingListEntries(ReadingListId(1), { ListItemId { ListKind::ReadingList, 1 } }, db));
    QVERIFY(!DBHelper::removeListFromDB(2, db));

    // the work of the caller was not committed by them
    QVERIFY(db.rollback());
    QCOMPARE(rows("SELECT id, folder_id FROM reading_list ORDER BY id"), QList<QVariantList>({ { 1, QVariant() }, { 2, QVariant() } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM reading_list_entry"), QList<QVariantList>({ { 1 } }));
}

QTEST_GUILESS_MAIN(ListFolderTest)

#include "main.moc"
