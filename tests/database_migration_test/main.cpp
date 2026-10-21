#include "data_base_management.h"
#include "yacreader_global.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

using namespace YACReader;

class DatabaseMigrationTest : public QObject
{
    Q_OBJECT
private slots:
    void migratesReadingListsAndLabels();
    void failedMigrationRollsBack();
    void failedOlderUpgradeRollsBack();
    void newLibraryUsesEntryTables();
    void upgradeFrom9_17AddsTheEntrySource();
    void upgradeFrom9_18AddsSmartLists();
    void upgradeFrom9_19AddsListFolders();

private:
    static bool createOldLibrary(const QString &libraryPath, const QString &version, const QStringList &extraStatements = { });
    static QList<int> ints(const QString &libraryPath, const QString &sql);
    static QStringList strings(const QString &libraryPath, const QString &sql);
    // a new library back without list folders (and without smart lists): reading_list is rebuilt, because a column
    // with a foreign key cannot be dropped
    static bool removeListFolders(QSqlQuery &query);
};

bool DatabaseMigrationTest::removeListFolders(QSqlQuery &query)
{
    const QStringList statements = {
        "PRAGMA foreign_keys = OFF",
        "DROP TABLE smart_list",
        "DROP TABLE smart_list_folder",
        "DROP INDEX reading_list_folder_index",
        "CREATE TABLE reading_list_old (id INTEGER PRIMARY KEY, parentId INTEGER, ordering INTEGER DEFAULT 0, name TEXT NOT NULL, finished BOOLEAN DEFAULT 0, completed BOOLEAN DEFAULT 1, manga BOOLEAN DEFAULT 0, FOREIGN KEY(parentId) REFERENCES reading_list(id) ON DELETE CASCADE)",
        "INSERT INTO reading_list_old SELECT id, parentId, ordering, name, finished, completed, manga FROM reading_list",
        "DROP TABLE reading_list",
        "ALTER TABLE reading_list_old RENAME TO reading_list",
        "DROP TABLE reading_list_folder",
        "PRAGMA foreign_keys = ON",
    };
    for (const auto &statement : statements) {
        if (!query.exec(statement)) {
            qWarning() << query.lastError().text() << statement;
            return false;
        }
    }
    return true;
}

bool DatabaseMigrationTest::createOldLibrary(const QString &libraryPath, const QString &version, const QStringList &extraStatements)
{
    QDir().mkpath(LibraryPaths::libraryDataPath(libraryPath));

    const QString connectionName = "old_library";
    bool success = false;
    {
        auto db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(LibraryPaths::libraryDatabasePath(libraryPath));
        success = db.open() && DataBaseManagement::createComicInfoTable(db, "comic_info");

        QStringList statements = {
            "CREATE TABLE folder (id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, name TEXT NOT NULL, path TEXT NOT NULL, "
            "FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE)",
            "CREATE TABLE comic (id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, comicInfoId INTEGER NOT NULL, fileName TEXT NOT NULL, path TEXT, "
            "FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE, FOREIGN KEY(comicInfoId) REFERENCES comic_info(id))",
            "CREATE TABLE db_info (version TEXT NOT NULL)",
            QString("INSERT INTO db_info (version) VALUES ('%1')").arg(version),
        };
        QSqlQuery query(db);
        for (const auto &statement : std::as_const(statements)) {
            success = success && query.exec(statement);
        }
        success = success && DataBaseManagement::createV8Tables(db);

        statements = {
            "INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/')",
            "INSERT INTO comic_info (id, hash) VALUES (10, 'hash10'), (11, 'hash11'), (12, 'hash12')",
            "INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES "
            "(100, 1, 10, 'a.cbz', '/a.cbz'), (101, 1, 10, 'a copy.cbz', '/a copy.cbz'), "
            "(110, 1, 11, 'b.cbz', '/b.cbz'), (120, 1, 12, 'c.cbz', '/c.cbz')",
            "INSERT INTO reading_list (id, name) VALUES (1, 'List')",
            "INSERT INTO label (id, name, color, ordering) VALUES (1, 'Tag', 'red', 0)",
            // 999 is a comic that no longer exists.
            "INSERT INTO comic_reading_list (reading_list_id, comic_id, ordering) VALUES (1, 110, 5), (1, 100, 2), (1, 101, 9), (1, 999, 1)",
            "INSERT INTO comic_label (label_id, comic_id, ordering) VALUES (1, 120, NULL), (1, 100, 0), (1, 110, NULL)",
        };
        statements += extraStatements;
        for (const auto &statement : std::as_const(statements)) {
            if (!query.exec(statement)) {
                qWarning() << query.lastError().text() << statement;
                success = false;
            }
        }
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return success;
}

QList<int> DatabaseMigrationTest::ints(const QString &libraryPath, const QString &sql)
{
    QList<int> values;
    for (const auto &value : strings(libraryPath, sql)) {
        values.append(value.toInt());
    }
    return values;
}

QStringList DatabaseMigrationTest::strings(const QString &libraryPath, const QString &sql)
{
    const QString connectionName = "migration_check";
    QStringList values;
    {
        auto db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(LibraryPaths::libraryDatabasePath(libraryPath));
        if (db.open()) {
            QSqlQuery query(db);
            query.exec(sql);
            while (query.next()) {
                values.append(query.value(0).toString());
            }
        }
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return values;
}

void DatabaseMigrationTest::migratesReadingListsAndLabels()
{
    QTemporaryDir library;
    QVERIFY(library.isValid());
    QVERIFY(createOldLibrary(library.path(), "9.16.0"));

    QVERIFY(DataBaseManagement::updateToCurrentVersion(library.path()));

    QCOMPARE(strings(library.path(), "SELECT version FROM db_info"), QStringList({ DB_VERSION }));

    QCOMPARE(ints(library.path(), "SELECT comic_info_id FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering"), QList<int>({ 10, 11, 10 }));
    QCOMPARE(ints(library.path(), "SELECT ordering FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering"), QList<int>({ 0, 1, 2 }));

    QCOMPARE(ints(library.path(), "SELECT comic_info_id FROM label_entry WHERE label_id = 1 ORDER BY ordering"), QList<int>({ 10, 12, 11 }));
    QCOMPARE(ints(library.path(), "SELECT ordering FROM label_entry WHERE label_id = 1 ORDER BY ordering"), QList<int>({ 0, 1, 2 }));

    const auto tables = strings(library.path(), "SELECT name FROM sqlite_master WHERE type = 'table'");
    QVERIFY(tables.contains("placeholder"));
    QVERIFY(!tables.contains("comic_reading_list"));
    QVERIFY(!tables.contains("comic_label"));
    QVERIFY(strings(library.path(), "SELECT name FROM sqlite_master WHERE type = 'index'").contains("comic_comic_info_id_index"));
    QCOMPARE(strings(library.path(), "SELECT name FROM sqlite_master WHERE type = 'trigger'"), QStringList({ "comic_delete_last_known_file" }));
    QVERIFY(strings(library.path(), "SELECT name FROM pragma_table_info('reading_list_entry')").contains("source"));
}

void DatabaseMigrationTest::failedMigrationRollsBack()
{
    QTemporaryDir library;
    QVERIFY(library.isValid());
    QVERIFY(createOldLibrary(library.path(), "9.16.0", { "CREATE TABLE placeholder (id INTEGER PRIMARY KEY)" }));

    QVERIFY(!DataBaseManagement::updateToCurrentVersion(library.path()));

    QCOMPARE(strings(library.path(), "SELECT version FROM db_info"), QStringList({ "9.16.0" }));
    QCOMPARE(ints(library.path(), "SELECT COUNT(*) FROM comic_reading_list"), QList<int>({ 4 }));
    QCOMPARE(ints(library.path(), "SELECT COUNT(*) FROM comic_label"), QList<int>({ 3 }));
    QVERIFY(!strings(library.path(), "SELECT name FROM sqlite_master WHERE type = 'table'").contains("reading_list_entry"));
}

// The schema is current but the version says 9.4.0: every step from 9.5 runs, and the
// upgrade fails at the end because the columns it adds already exist.
void DatabaseMigrationTest::failedOlderUpgradeRollsBack()
{
    QTemporaryDir library;
    QVERIFY(library.isValid());
    QVERIFY(createOldLibrary(library.path(), "9.4.0"));

    QVERIFY(!DataBaseManagement::updateToCurrentVersion(library.path()));

    QCOMPARE(strings(library.path(), "SELECT version FROM db_info"), QStringList({ "9.4.0" }));
    QCOMPARE(ints(library.path(), "SELECT COUNT(*) FROM comic_reading_list"), QList<int>({ 4 }));
    QCOMPARE(ints(library.path(), "SELECT COUNT(*) FROM comic_label"), QList<int>({ 3 }));
    const auto tables = strings(library.path(), "SELECT name FROM sqlite_master WHERE type = 'table'");
    QVERIFY(!tables.contains("reading_list_entry"));
    QVERIFY(!tables.contains("comic_info_migration"));
}

void DatabaseMigrationTest::newLibraryUsesEntryTables()
{
    QTemporaryDir library;
    QVERIFY(library.isValid());
    QDir().mkpath(LibraryPaths::libraryDataPath(library.path()));

    QString connectionName;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(library.path()));
        connectionName = db.connectionName();
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    QCOMPARE(strings(library.path(), "SELECT version FROM db_info"), QStringList({ DB_VERSION }));
    const auto tables = strings(library.path(), "SELECT name FROM sqlite_master WHERE type = 'table'");
    QVERIFY(tables.contains("placeholder"));
    QVERIFY(tables.contains("smart_list"));
    QVERIFY(tables.contains("reading_list_folder"));
    QVERIFY(tables.contains("smart_list_folder"));
    QVERIFY(tables.contains("reading_list_entry"));
    QVERIFY(tables.contains("label_entry"));
    QVERIFY(!tables.contains("comic_reading_list"));
    QVERIFY(!tables.contains("comic_label"));
    QCOMPARE(strings(library.path(), "SELECT name FROM sqlite_master WHERE type = 'trigger'"), QStringList({ "comic_delete_last_known_file" }));
    const auto columns = strings(library.path(), "SELECT name FROM pragma_table_info('reading_list_entry')");
    QVERIFY(columns.contains("source"));
    QVERIFY(columns.contains("source_format"));
}

// a 9.17.0 library (a development version) gets the source columns and keeps its entries
void DatabaseMigrationTest::upgradeFrom9_17AddsTheEntrySource()
{
    QTemporaryDir library;
    QVERIFY(library.isValid());
    QDir().mkpath(LibraryPaths::libraryDataPath(library.path()));

    QString connectionName;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(library.path()));
        connectionName = db.connectionName();
        QSqlQuery query(db);
        QVERIFY(query.exec("ALTER TABLE reading_list_entry DROP COLUMN source"));
        QVERIFY(query.exec("ALTER TABLE reading_list_entry DROP COLUMN source_format"));
        QVERIFY(removeListFolders(query));
        QVERIFY(query.exec("UPDATE db_info SET version = '9.17.0'"));
        QVERIFY(query.exec("INSERT INTO comic_info (id, hash) VALUES (10, 'hash10')"));
        QVERIFY(query.exec("INSERT INTO reading_list (id, name) VALUES (1, 'List')"));
        QVERIFY(query.exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES (1, 0, 10)"));
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    QVERIFY(DataBaseManagement::updateToCurrentVersion(library.path()));

    QCOMPARE(strings(library.path(), "SELECT version FROM db_info"), QStringList({ DB_VERSION }));
    QCOMPARE(ints(library.path(), "SELECT comic_info_id FROM reading_list_entry"), QList<int>({ 10 }));
    const auto columns = strings(library.path(), "SELECT name FROM pragma_table_info('reading_list_entry')");
    QVERIFY(columns.contains("source"));
    QVERIFY(columns.contains("source_format"));
    QVERIFY(strings(library.path(), "SELECT name FROM sqlite_master WHERE type = 'table'").contains("smart_list"));
}

void DatabaseMigrationTest::upgradeFrom9_18AddsSmartLists()
{
    QTemporaryDir library;
    QVERIFY(library.isValid());
    QDir().mkpath(LibraryPaths::libraryDataPath(library.path()));

    QString connectionName;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(library.path()));
        connectionName = db.connectionName();
        QSqlQuery query(db);
        QVERIFY(removeListFolders(query));
        QVERIFY(query.exec("UPDATE db_info SET version = '9.18.0'"));
        QVERIFY(query.exec("INSERT INTO reading_list (id, name) VALUES (1, 'List')"));
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    QVERIFY(DataBaseManagement::updateToCurrentVersion(library.path()));

    QCOMPARE(strings(library.path(), "SELECT version FROM db_info"), QStringList({ DB_VERSION }));
    QCOMPARE(strings(library.path(), "SELECT name FROM reading_list"), QStringList({ "List" }));
    QCOMPARE(strings(library.path(), "SELECT name FROM pragma_table_info('smart_list') ORDER BY cid"),
             QStringList({ "id", "name", "query", "ordering", "folder_id" }));
}

void DatabaseMigrationTest::upgradeFrom9_19AddsListFolders()
{
    QTemporaryDir library;
    QVERIFY(library.isValid());
    QDir().mkpath(LibraryPaths::libraryDataPath(library.path()));

    QString connectionName;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(library.path()));
        connectionName = db.connectionName();
        QSqlQuery query(db);
        QVERIFY(removeListFolders(query));
        QVERIFY(DataBaseManagement::createSmartListTable(db));
        QVERIFY(query.exec("UPDATE db_info SET version = '9.19.0'"));
        QVERIFY(query.exec("INSERT INTO reading_list (id, name) VALUES (1, 'List'), (2, 'Child')"));
        QVERIFY(query.exec("UPDATE reading_list SET parentId = 1 WHERE id = 2"));
        QVERIFY(query.exec("INSERT INTO smart_list (id, name, query, ordering) VALUES (1, 'Unread', 'read:false', 0)"));
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    QVERIFY(DataBaseManagement::updateToCurrentVersion(library.path()));

    QCOMPARE(strings(library.path(), "SELECT version FROM db_info"), QStringList({ DB_VERSION }));
    QCOMPARE(strings(library.path(), "SELECT name FROM reading_list ORDER BY id"), QStringList({ "List", "Child" }));
    QCOMPARE(strings(library.path(), "SELECT name FROM smart_list"), QStringList({ "Unread" }));
    QCOMPARE(ints(library.path(), "SELECT COUNT(*) FROM reading_list WHERE folder_id IS NULL"), QList<int>({ 2 }));
    const auto tables = strings(library.path(), "SELECT name FROM sqlite_master WHERE type = 'table'");
    QVERIFY(tables.contains("reading_list_folder"));
    QVERIFY(tables.contains("smart_list_folder"));
    QVERIFY(strings(library.path(), "SELECT name FROM pragma_table_info('reading_list')").contains("folder_id"));
    QCOMPARE(strings(library.path(), "SELECT \"table\" FROM pragma_foreign_key_list('smart_list')"), QStringList({ "smart_list_folder" }));
}

QTEST_GUILESS_MAIN(DatabaseMigrationTest)
#include "main.moc"
