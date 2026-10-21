#include "data_base_management.h"
#include "db_helper.h"
#include "list_folder.h"
#include "reading_list.h"
#include "smart_list.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTest>

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

QTEST_GUILESS_MAIN(ListFolderTest)

#include "main.moc"
