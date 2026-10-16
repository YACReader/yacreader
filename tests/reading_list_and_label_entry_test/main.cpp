#include "data_base_management.h"
#include "descriptive_comic_metadata.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QTest>

#include <tuple>

class ReadingListAndLabelEntryTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void placeholderColumnsMatchSharedList();
    void acceptsValidStates_data();
    void acceptsValidStates();
    void rejectsInvalidStates_data();
    void rejectsInvalidStates();
    void rejectsInvalidUpdates_data();
    void rejectsInvalidUpdates();
    void rejectsNullParentAndOrdering_data();
    void rejectsNullParentAndOrdering();
    void sameTargetCanAppearTwice_data();
    void sameTargetCanAppearTwice();
    void foreignKeyActions_data();
    void foreignKeyActions();
    void entryIdsAreNotReused_data();
    void entryIdsAreNotReused();
    void placeholderIdsAreNotReused();

private:
    void addReadingListAndLabelRows();
    void fetchReadingListOrLabel();
    bool insertEntry(qlonglong parentId, const QVariant &comicInfoId, const QVariant &placeholderId, QString *error = nullptr);
    bool exec(const QString &sql);
    int scalar(const QString &sql);

    QSqlDatabase db;
    QString table;
    QString parentColumn;
    QString parentTable;
};

void ReadingListAndLabelEntryTest::init()
{
    db = QSqlDatabase::addDatabase("QSQLITE", "reading_list_and_label_entry_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());

    QVERIFY(exec("PRAGMA foreign_keys = ON"));
    QVERIFY(DataBaseManagement::createTables(db));
    QVERIFY(DataBaseManagement::createPlaceholderTable(db));
    QVERIFY(DataBaseManagement::createReadingListEntryTable(db));
    QVERIFY(DataBaseManagement::createLabelEntryTable(db));

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/')"));
    QVERIFY(exec("INSERT INTO comic_info (id, hash) VALUES (10, 'hash10'), (11, 'hash11')"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES "
                 "(100, 1, 10, 'a.cbz', '/a.cbz'), (110, 1, 11, 'b.cbz', '/b.cbz')"));
    QVERIFY(exec("INSERT INTO placeholder (id, title) VALUES (1, 'Expected issue')"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'List A'), (2, 'List B')"));
    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Tag A', 'red', 0), (2, 'Tag B', 'blue', 1)"));
}

void ReadingListAndLabelEntryTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("reading_list_and_label_entry_test");
}

void ReadingListAndLabelEntryTest::addReadingListAndLabelRows()
{
    QTest::addColumn<QString>("table");
    QTest::addColumn<QString>("parentColumn");
    QTest::addColumn<QString>("parentTable");
    QTest::newRow("reading list") << "reading_list_entry"
                                  << "reading_list_id"
                                  << "reading_list";
    QTest::newRow("label") << "label_entry"
                           << "label_id"
                           << "label";
}

void ReadingListAndLabelEntryTest::fetchReadingListOrLabel()
{
    QFETCH(QString, table);
    QFETCH(QString, parentColumn);
    QFETCH(QString, parentTable);
    this->table = table;
    this->parentColumn = parentColumn;
    this->parentTable = parentTable;
}

bool ReadingListAndLabelEntryTest::insertEntry(qlonglong parentId, const QVariant &comicInfoId, const QVariant &placeholderId, QString *error)
{
    QSqlQuery q(db);
    q.prepare(QString("INSERT INTO %1 (%2, ordering, comic_info_id, placeholder_id) "
                      "VALUES (:parent, (SELECT COALESCE(MAX(ordering), -1) + 1 FROM %1 WHERE %2 = :parent2), :comicInfo, :placeholder)")
                      .arg(table, parentColumn));
    q.bindValue(":parent", parentId);
    q.bindValue(":parent2", parentId);
    q.bindValue(":comicInfo", comicInfoId);
    q.bindValue(":placeholder", placeholderId);
    const bool success = q.exec();
    if (error) {
        *error = q.lastError().text();
    }
    return success;
}

bool ReadingListAndLabelEntryTest::exec(const QString &sql)
{
    QSqlQuery q(db);
    return q.exec(sql);
}

int ReadingListAndLabelEntryTest::scalar(const QString &sql)
{
    QSqlQuery q(db);
    if (!q.exec(sql) || !q.next()) {
        return -1;
    }
    return q.value(0).toInt();
}

void ReadingListAndLabelEntryTest::placeholderColumnsMatchSharedList()
{
    auto tableColumns = [this](const QString &tableName) {
        QMap<QString, QString> types;
        QSqlQuery q(db);
        q.exec("PRAGMA table_info(" + tableName + ")");
        while (q.next()) {
            types.insert(q.value(1).toString(), q.value(2).toString());
        }
        return types;
    };

    const auto placeholder = tableColumns("placeholder");
    const auto comicInfo = tableColumns("comic_info");

    QCOMPARE(int(placeholder.size()), int(DescriptiveComicMetadata::columns().size()) + 2);
    QCOMPARE(placeholder.value("id"), QString("INTEGER"));
    QCOMPARE(placeholder.value("coverKey"), QString("TEXT"));
    for (const auto &column : DescriptiveComicMetadata::columns()) {
        const auto name = QString::fromLatin1(column.name);
        QCOMPARE(placeholder.value(name), QString::fromLatin1(column.sqlType));
        QCOMPARE(placeholder.value(name), comicInfo.value(name));
    }
}

void ReadingListAndLabelEntryTest::acceptsValidStates_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::acceptsValidStates()
{
    fetchReadingListOrLabel();

    QString error;
    QVERIFY2(insertEntry(1, 10, QVariant(), &error), qPrintable(error));
    QVERIFY2(insertEntry(1, QVariant(), 1, &error), qPrintable(error));
    QCOMPARE(scalar("SELECT COUNT(*) FROM " + table), 2);
}

void ReadingListAndLabelEntryTest::rejectsInvalidStates_data()
{
    QTest::addColumn<QString>("table");
    QTest::addColumn<QString>("parentColumn");
    QTest::addColumn<QString>("parentTable");
    QTest::addColumn<QVariant>("comicInfoId");
    QTest::addColumn<QVariant>("placeholderId");

    const QList<std::tuple<QString, QString, QString>> entryTables = {
        { "reading_list_entry", "reading_list_id", "reading_list" },
        { "label_entry", "label_id", "label" },
    };
    for (const auto &[table, parentColumn, parentTable] : entryTables) {
        auto row = [&](const char *name) -> QTestData & {
            return QTest::newRow(qPrintable(table + ": " + name)) << table << parentColumn << parentTable;
        };
        row("no target") << QVariant() << QVariant();
        row("metadata and placeholder") << QVariant(10) << QVariant(1);
        row("unknown metadata") << QVariant(999) << QVariant();
        row("unknown placeholder") << QVariant() << QVariant(999);
    }
}

void ReadingListAndLabelEntryTest::rejectsInvalidStates()
{
    fetchReadingListOrLabel();
    QFETCH(QVariant, comicInfoId);
    QFETCH(QVariant, placeholderId);

    QVERIFY(!insertEntry(1, comicInfoId, placeholderId));
    QVERIFY(!insertEntry(999, 10, QVariant()));
    QCOMPARE(scalar("SELECT COUNT(*) FROM " + table), 0);
}

void ReadingListAndLabelEntryTest::rejectsInvalidUpdates_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::rejectsInvalidUpdates()
{
    fetchReadingListOrLabel();

    QVERIFY(insertEntry(1, 10, QVariant()));
    const auto id = scalar("SELECT id FROM " + table);

    QVERIFY(!exec(QString("UPDATE %1 SET placeholder_id = 1 WHERE id = %2").arg(table).arg(id)));
    QVERIFY(!exec(QString("UPDATE %1 SET comic_info_id = NULL WHERE id = %2").arg(table).arg(id)));
    QVERIFY(!exec(QString("UPDATE %1 SET comic_info_id = 999 WHERE id = %2").arg(table).arg(id)));
    QCOMPARE(scalar("SELECT comic_info_id FROM " + table), 10);

    QVERIFY(exec(QString("UPDATE %1 SET comic_info_id = 11 WHERE id = %2").arg(table).arg(id)));
    QVERIFY(exec(QString("UPDATE %1 SET comic_info_id = NULL, placeholder_id = 1 WHERE id = %2").arg(table).arg(id)));
    QCOMPARE(scalar("SELECT placeholder_id FROM " + table), 1);
}

void ReadingListAndLabelEntryTest::rejectsNullParentAndOrdering_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::rejectsNullParentAndOrdering()
{
    fetchReadingListOrLabel();

    QVERIFY(!exec(QString("INSERT INTO %1 (%2, ordering, comic_info_id) VALUES (NULL, 0, 10)").arg(table, parentColumn)));
    QVERIFY(!exec(QString("INSERT INTO %1 (%2, ordering, comic_info_id) VALUES (1, NULL, 10)").arg(table, parentColumn)));
    QVERIFY(!exec(QString("INSERT INTO %1 (%2, comic_info_id) VALUES (1, 10)").arg(table, parentColumn)));
    QCOMPARE(scalar("SELECT COUNT(*) FROM " + table), 0);

    QVERIFY(insertEntry(1, 10, QVariant()));
    const auto id = scalar("SELECT id FROM " + table);
    QVERIFY(!exec(QString("UPDATE %1 SET %2 = NULL WHERE id = %3").arg(table, parentColumn).arg(id)));
    QVERIFY(!exec(QString("UPDATE %1 SET ordering = NULL WHERE id = %2").arg(table).arg(id)));
    QCOMPARE(scalar(QString("SELECT %1 FROM %2").arg(parentColumn, table)), 1);
    QCOMPARE(scalar("SELECT ordering FROM " + table), 0);
}

void ReadingListAndLabelEntryTest::sameTargetCanAppearTwice_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::sameTargetCanAppearTwice()
{
    fetchReadingListOrLabel();

    QVERIFY(insertEntry(1, 10, QVariant()));
    QVERIFY(insertEntry(1, 10, QVariant()));
    QVERIFY(insertEntry(1, QVariant(), 1));
    QVERIFY(insertEntry(1, QVariant(), 1));
    QVERIFY(insertEntry(2, 10, QVariant()));
    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE %2 = 1").arg(table, parentColumn)), 4);
    QCOMPARE(scalar(QString("SELECT COUNT(DISTINCT ordering) FROM %1 WHERE %2 = 1").arg(table, parentColumn)), 4);
}

void ReadingListAndLabelEntryTest::foreignKeyActions_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::foreignKeyActions()
{
    fetchReadingListOrLabel();

    QVERIFY(insertEntry(1, 11, QVariant()));
    QVERIFY(insertEntry(1, QVariant(), 1));
    QVERIFY(insertEntry(2, 10, QVariant()));

    QVERIFY(exec("DELETE FROM comic WHERE id = 110"));
    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE comic_info_id = 11").arg(table)), 1);

    QVERIFY(!exec("DELETE FROM comic_info WHERE id = 11"));
    QVERIFY(!exec("DELETE FROM placeholder WHERE id = 1"));

    QVERIFY(exec(QString("DELETE FROM %1 WHERE id = 1").arg(parentTable)));
    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE %2 = 1").arg(table, parentColumn)), 0);
    QCOMPARE(scalar("SELECT COUNT(*) FROM " + table), 1);
    QCOMPARE(scalar("SELECT COUNT(*) FROM comic_info WHERE id = 11"), 1);
    QCOMPARE(scalar("SELECT COUNT(*) FROM placeholder WHERE id = 1"), 1);

    QVERIFY(exec("DELETE FROM comic_info WHERE id = 11"));
    QVERIFY(exec("DELETE FROM placeholder WHERE id = 1"));
}

void ReadingListAndLabelEntryTest::entryIdsAreNotReused_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::entryIdsAreNotReused()
{
    fetchReadingListOrLabel();

    QVERIFY(insertEntry(1, 10, QVariant()));
    QVERIFY(insertEntry(1, 11, QVariant()));
    const auto lastId = scalar("SELECT MAX(id) FROM " + table);

    QVERIFY(exec(QString("DELETE FROM %1 WHERE id = %2").arg(table).arg(lastId)));
    QVERIFY(insertEntry(1, 11, QVariant()));
    QVERIFY(scalar("SELECT MAX(id) FROM " + table) > lastId);
}

void ReadingListAndLabelEntryTest::placeholderIdsAreNotReused()
{
    QVERIFY(exec("INSERT INTO placeholder (title) VALUES ('Second')"));
    const auto lastId = scalar("SELECT MAX(id) FROM placeholder");

    QVERIFY(exec(QString("DELETE FROM placeholder WHERE id = %1").arg(lastId)));
    QVERIFY(exec("INSERT INTO placeholder (title) VALUES ('Third')"));
    QVERIFY(scalar("SELECT MAX(id) FROM placeholder") > lastId);
}

QTEST_GUILESS_MAIN(ReadingListAndLabelEntryTest)
#include "main.moc"
