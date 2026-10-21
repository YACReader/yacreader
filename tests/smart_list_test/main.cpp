#include "data_base_management.h"
#include "db_helper.h"
#include "smart_list.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

class SmartListTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void insertedListsGoToTheEnd();
    void updateChangesTheNameAndTheQuery();
    void removeDeletesTheList();

private:
    bool exec(const QString &sql);

    QSqlDatabase db;
};

void SmartListTest::init()
{
    db = QSqlDatabase::addDatabase("QSQLITE", "smart_list_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());
    QVERIFY(DataBaseManagement::createTables(db));
}

void SmartListTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("smart_list_test");
}

bool SmartListTest::exec(const QString &sql)
{
    QSqlQuery query(db);
    return query.exec(sql);
}

void SmartListTest::insertedListsGoToTheEnd()
{
    const auto unread = DBHelper::insertSmartList("Unread", "read:false", db);
    const auto rated = DBHelper::insertSmartList("Highly rated", "rating>=4", db);
    QVERIFY(unread);
    QVERIFY(rated);

    const auto lists = DBHelper::loadSmartLists(db);
    QVERIFY(lists);
    QCOMPARE(lists->size(), 2);
    QCOMPARE(lists->at(0).id, *unread);
    QCOMPARE(lists->at(0).name, QString("Unread"));
    QCOMPARE(lists->at(0).query, QString("read:false"));
    QCOMPARE(lists->at(0).ordering, 0);
    QCOMPARE(lists->at(1).id, *rated);
    QCOMPARE(lists->at(1).ordering, 1);

    // the order is the ordering column, not the id
    QVERIFY(exec(QString("UPDATE smart_list SET ordering = 5 WHERE id = %1").arg(unread->value())));
    QCOMPARE(DBHelper::loadSmartLists(db)->constFirst().id, *rated);
}

void SmartListTest::updateChangesTheNameAndTheQuery()
{
    const auto id = DBHelper::insertSmartList("Unread", "read:false", db);
    QVERIFY(id);
    QVERIFY(DBHelper::updateSmartList(*id, "Unread Batman", "series:batman read:false", db));
    const auto list = DBHelper::loadSmartList(*id, db);
    QVERIFY(list);
    QCOMPARE(list->name, QString("Unread Batman"));
    QCOMPARE(list->query, QString("series:batman read:false"));
    QCOMPARE(list->ordering, 0);

    QVERIFY(!DBHelper::updateSmartList(SmartListId(99), "Gone", "read:true", db));
    QVERIFY(!DBHelper::loadSmartList(SmartListId(99), db));
}

void SmartListTest::removeDeletesTheList()
{
    const auto id = DBHelper::insertSmartList("Unread", "read:false", db);
    QVERIFY(id);
    QVERIFY(DBHelper::removeSmartList(*id, db));
    QVERIFY(DBHelper::loadSmartLists(db)->isEmpty());
    QVERIFY(!DBHelper::removeSmartList(*id, db));
}

QTEST_GUILESS_MAIN(SmartListTest)

#include "main.moc"
