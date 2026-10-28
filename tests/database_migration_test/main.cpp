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
    void upgradesHistoricalSchemas_data();
    void upgradesHistoricalSchemas();
    void failedFinalSchemaStepRollsBack();

private:
    static bool createOldLibrary(const QString &libraryPath, const QString &version, const QStringList &extraStatements = { });
    static bool createHistoricalLibrary(const QString &libraryPath, const QString &version);
    static QList<int> ints(const QString &libraryPath, const QString &sql);
    static QStringList strings(const QString &libraryPath, const QString &sql);
};

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

// Reconstruct the columns present at each historical migration boundary. Existing list data is
// meaningful from v8 onward; older libraries did not have lists or tags yet.
bool DatabaseMigrationTest::createHistoricalLibrary(const QString &libraryPath, const QString &version)
{
    if (!createOldLibrary(libraryPath, version))
        return false;

    QString connectionName;
    bool success = true;
    {
        auto db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        connectionName = db.connectionName();
        QSqlQuery query(db);
        const auto exec = [&](const QString &sql) {
            if (!query.exec(sql)) {
                qWarning() << query.lastError().text() << sql;
                success = false;
            }
        };
        const QList<QPair<QString, QStringList>> comicColumns = {
            { "7.0.0", { "hasBeenOpened", "rating", "currentPage", "bookmark1", "bookmark2", "bookmark3", "brightness", "contrast", "gamma" } },
            { "7.0.3", { "comicVineID" } },
            { "9.5.0", { "lastTimeOpened", "coverSizeRatio", "originalCoverSize" } },
            { "9.8.0", { "manga" } },
            { "9.13.0", { "added", "type", "editor", "imprint", "teams", "locations", "series", "alternateSeries", "alternateNumber", "alternateCount", "languageISO", "seriesGroup", "mainCharacterOrTeam", "review", "tags" } },
            { "9.16.0", { "imageFiltersJson", "lastTimeImageFiltersSet", "lastTimeCoverSet", "usesExternalCover", "lastTimeMetadataSet" } },
        };
        for (const auto &[introduced, columns] : comicColumns) {
            if (DataBaseManagement::compareVersions(version, introduced) < 0) {
                for (const auto &column : columns)
                    exec("ALTER TABLE comic_info DROP COLUMN " + column);
            }
        }
        const QList<QPair<QString, QStringList>> folderColumns = {
            { "7.0.3", { "finished BOOLEAN DEFAULT 0", "completed BOOLEAN DEFAULT 1" } },
            { "9.5.0", { "numChildren INTEGER", "firstChildHash TEXT", "customImage TEXT" } },
            { "9.8.0", { "manga BOOLEAN DEFAULT 0" } },
            { "9.13.0", { "added INTEGER", "updated INTEGER", "type INTEGER DEFAULT 0" } },
        };
        for (const auto &[introduced, columns] : folderColumns) {
            if (DataBaseManagement::compareVersions(version, introduced) >= 0) {
                for (const auto &column : columns)
                    exec("ALTER TABLE folder ADD COLUMN " + column);
            }
        }
        if (DataBaseManagement::compareVersions(version, "9.5.0") >= 0)
            exec("CREATE INDEX last_time_opened_index ON comic_info (lastTimeOpened)");
        if (DataBaseManagement::compareVersions(version, "8.0.0") < 0) {
            for (const auto &table : { "comic_reading_list", "comic_label", "comic_default_reading_list", "reading_list", "label", "default_reading_list" })
                exec(QString("DROP TABLE %1").arg(table));
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

void DatabaseMigrationTest::upgradesHistoricalSchemas_data()
{
    QTest::addColumn<QString>("version");
    for (const auto &version : { "6.0.0", "7.0.0", "7.0.3", "8.0.0", "9.4.0", "9.5.0", "9.8.0", "9.13.0", "9.14.0", "9.16.0" })
        QTest::newRow(version) << QString(version);
}

void DatabaseMigrationTest::upgradesHistoricalSchemas()
{
    QFETCH(QString, version);
    QTemporaryDir library;
    QTemporaryDir freshLibrary;
    QVERIFY(library.isValid());
    QVERIFY(freshLibrary.isValid());
    QVERIFY(createHistoricalLibrary(library.path(), version));
    QVERIFY(DataBaseManagement::updateToCurrentVersion(library.path()));
    QCOMPARE(strings(library.path(), "SELECT version FROM db_info"), QStringList({ "10.5.0" }));
    QCOMPARE(ints(library.path(), "SELECT id FROM comic ORDER BY id"), QList<int>({ 100, 101, 110, 120 }));
    if (DataBaseManagement::compareVersions(version, "8.0.0") >= 0) {
        QCOMPARE(ints(library.path(), "SELECT comic_info_id FROM reading_list_entry ORDER BY ordering"), QList<int>({ 10, 11, 10 }));
        QCOMPARE(ints(library.path(), "SELECT comic_info_id FROM label_entry ORDER BY ordering"), QList<int>({ 10, 12, 11 }));
    } else {
        QCOMPARE(ints(library.path(), "SELECT COUNT(*) FROM reading_list_entry"), QList<int>({ 0 }));
        QCOMPARE(ints(library.path(), "SELECT COUNT(*) FROM label_entry"), QList<int>({ 0 }));
    }

    QDir().mkpath(LibraryPaths::libraryDataPath(freshLibrary.path()));
    QString connectionName;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(freshLibrary.path()));
        connectionName = db.connectionName();
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    // The release tables, indexes, trigger, and foreign keys match those of a newly created library.
    const QString schema = "SELECT type || ':' || name || ':' || sql FROM sqlite_master WHERE "
                           "tbl_name IN ('placeholder', 'reading_list_entry', 'label_entry', 'smart_list', "
                           "'reading_list_folder', 'smart_list_folder', 'folder_placeholder') "
                           "OR name = 'comic_delete_last_known_file' OR name = 'comic_comic_info_id_index' ORDER BY type, name";
    QCOMPARE(strings(library.path(), schema), strings(freshLibrary.path(), schema));
    QCOMPARE(strings(library.path(), "SELECT name FROM pragma_table_info('comic_info') ORDER BY name"),
             strings(freshLibrary.path(), "SELECT name FROM pragma_table_info('comic_info') ORDER BY name"));
    QVERIFY(strings(library.path(), "PRAGMA foreign_key_check").isEmpty());
    const auto backups = DataBaseManagement::libraryBackups(library.path());
    QCOMPARE(backups.size(), 1);
}

void DatabaseMigrationTest::failedFinalSchemaStepRollsBack()
{
    QTemporaryDir library;
    QVERIFY(library.isValid());
    QVERIFY(createOldLibrary(library.path(), "9.16.0", { "CREATE TABLE folder_placeholder (sentinel INTEGER)" }));
    QVERIFY(!DataBaseManagement::updateToCurrentVersion(library.path()));
    QCOMPARE(strings(library.path(), "SELECT version FROM db_info"), QStringList({ "9.16.0" }));
    QCOMPARE(ints(library.path(), "SELECT COUNT(*) FROM comic_reading_list"), QList<int>({ 4 }));
    QCOMPARE(ints(library.path(), "SELECT COUNT(*) FROM comic_label"), QList<int>({ 3 }));
    const auto tables = strings(library.path(), "SELECT name FROM sqlite_master WHERE type = 'table'");
    QVERIFY(!tables.contains("reading_list_entry"));
    QVERIFY(!tables.contains("smart_list"));
    QVERIFY(!tables.contains("reading_list_folder"));
    QCOMPARE(strings(library.path(), "SELECT name FROM pragma_table_info('folder_placeholder')"), QStringList({ "sentinel" }));
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
    QVERIFY(tables.contains("folder_placeholder"));
    QVERIFY(tables.contains("reading_list_entry"));
    QVERIFY(tables.contains("label_entry"));
    QVERIFY(!tables.contains("comic_reading_list"));
    QVERIFY(!tables.contains("comic_label"));
    QCOMPARE(strings(library.path(), "SELECT name FROM sqlite_master WHERE type = 'trigger'"), QStringList({ "comic_delete_last_known_file" }));
    const auto columns = strings(library.path(), "SELECT name FROM pragma_table_info('reading_list_entry')");
    QVERIFY(columns.contains("source"));
    QVERIFY(columns.contains("source_format"));
}

QTEST_GUILESS_MAIN(DatabaseMigrationTest)
#include "main.moc"
