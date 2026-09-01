#include "db_helper.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

class FolderRenameTest : public QObject
{
    Q_OBJECT

private slots:
    void rewritesOnlyTheRenamedFolderTree();
    void treatsSqlWildcardCharactersLiterally();
    void missingFolderLeavesPathsUntouched();
};

namespace {
QSqlDatabase createDatabase(const QString &connectionName)
{
    auto db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(":memory:");
    db.open();

    QSqlQuery query(db);
    query.exec("CREATE TABLE folder (id INTEGER PRIMARY KEY, parentId INTEGER, name TEXT, path TEXT, finished BOOLEAN, completed BOOLEAN, added INTEGER, updated INTEGER)");
    query.exec("CREATE TABLE comic (id INTEGER PRIMARY KEY, parentId INTEGER, path TEXT)");
    query.exec("INSERT INTO folder VALUES (2, 1, 'Folder1', '/Folder1', 1, 0, 123, 456)");
    query.exec("INSERT INTO folder VALUES (3, 2, 'subfolder', '/Folder1/subfolder', 0, 1, 234, 567)");
    query.exec("INSERT INTO folder VALUES (4, 3, 'Folder1', '/Folder1/subfolder/Folder1', 0, 1, 345, 678)");
    query.exec("INSERT INTO folder VALUES (5, 1, 'Folder10', '/Folder10', 0, 1, 456, 789)");
    query.exec("INSERT INTO folder VALUES (6, 1, 'Folder1 Backup', '/Folder1 Backup', 0, 1, 567, 890)");
    query.exec("INSERT INTO folder VALUES (7, 1, 'Other', '/Other', 0, 1, 678, 901)");
    query.exec("INSERT INTO folder VALUES (8, 7, 'Folder1', '/Other/Folder1', 0, 1, 789, 012)");
    query.exec("INSERT INTO folder VALUES (9, 1, 'Folder1Backup', '/Folder1Backup', 0, 1, 890, 123)");
    query.exec("INSERT INTO folder VALUES (20, 1, 'Series_100%', '/Series_100%', 0, 1, 901, 234)");
    query.exec("INSERT INTO folder VALUES (21, 1, 'SeriesX100A', '/SeriesX100A', 0, 1, 012, 345)");
    query.exec("INSERT INTO comic VALUES (10, 4, '/Folder1/subfolder/Folder1/comic.cbz')");
    query.exec("INSERT INTO comic VALUES (11, 5, '/Folder10/comic.cbz')");
    query.exec("INSERT INTO comic VALUES (12, 6, '/Folder1 Backup/comic.cbz')");
    query.exec("INSERT INTO comic VALUES (13, 8, '/Other/Folder1/comic.cbz')");
    query.exec("INSERT INTO comic VALUES (14, 1, '/Folder1.cbz')");
    query.exec("INSERT INTO comic VALUES (15, 2, '/Folder1/direct.cbz')");
    query.exec("INSERT INTO comic VALUES (20, 20, '/Series_100%/comic.cbz')");
    query.exec("INSERT INTO comic VALUES (21, 21, '/SeriesX100A/comic.cbz')");
    return db;
}

QString value(QSqlDatabase &db, const QString &table, int id, const QString &column)
{
    QSqlQuery query(db);
    query.prepare(QString("SELECT %1 FROM %2 WHERE id = :id").arg(column, table));
    query.bindValue(":id", id);
    query.exec();
    query.next();
    return query.value(0).toString();
}
}

void FolderRenameTest::rewritesOnlyTheRenamedFolderTree()
{
    const QString connectionName = "folderRenameSuccess";
    {
        auto db = createDatabase(connectionName);
        QVERIFY(db.transaction());
        QString error;
        QVERIFY2(DBHelper::renameFolder(2, "Folder2", "/Folder1", "/Folder2", db, &error), qPrintable(error));
        QVERIFY(db.commit());

        QCOMPARE(value(db, "folder", 2, "name"), QString("Folder2"));
        QCOMPARE(value(db, "folder", 2, "path"), QString("/Folder2"));
        QCOMPARE(value(db, "folder", 2, "finished"), QString("1"));
        QCOMPARE(value(db, "folder", 2, "completed"), QString("0"));
        QCOMPARE(value(db, "folder", 2, "added"), QString("123"));
        QCOMPARE(value(db, "folder", 2, "updated"), QString("456"));
        QCOMPARE(value(db, "folder", 3, "path"), QString("/Folder2/subfolder"));
        QCOMPARE(value(db, "folder", 4, "path"), QString("/Folder2/subfolder/Folder1"));
        QCOMPARE(value(db, "comic", 10, "path"), QString("/Folder2/subfolder/Folder1/comic.cbz"));
        QCOMPARE(value(db, "comic", 15, "path"), QString("/Folder2/direct.cbz"));

        QCOMPARE(value(db, "folder", 5, "path"), QString("/Folder10"));
        QCOMPARE(value(db, "comic", 11, "path"), QString("/Folder10/comic.cbz"));
        QCOMPARE(value(db, "folder", 6, "path"), QString("/Folder1 Backup"));
        QCOMPARE(value(db, "comic", 12, "path"), QString("/Folder1 Backup/comic.cbz"));
        QCOMPARE(value(db, "folder", 8, "path"), QString("/Other/Folder1"));
        QCOMPARE(value(db, "comic", 13, "path"), QString("/Other/Folder1/comic.cbz"));
        QCOMPARE(value(db, "folder", 9, "path"), QString("/Folder1Backup"));
        QCOMPARE(value(db, "comic", 14, "path"), QString("/Folder1.cbz"));
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void FolderRenameTest::treatsSqlWildcardCharactersLiterally()
{
    const QString connectionName = "folderRenameWildcards";
    {
        auto db = createDatabase(connectionName);
        QVERIFY(db.transaction());
        QString error;
        QVERIFY2(DBHelper::renameFolder(20, "Renamed", "/Series_100%", "/Renamed", db, &error), qPrintable(error));
        QVERIFY(db.commit());

        QCOMPARE(value(db, "folder", 20, "path"), QString("/Renamed"));
        QCOMPARE(value(db, "comic", 20, "path"), QString("/Renamed/comic.cbz"));
        QCOMPARE(value(db, "folder", 21, "path"), QString("/SeriesX100A"));
        QCOMPARE(value(db, "comic", 21, "path"), QString("/SeriesX100A/comic.cbz"));
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void FolderRenameTest::missingFolderLeavesPathsUntouched()
{
    const QString connectionName = "folderRenameMissing";
    {
        auto db = createDatabase(connectionName);
        QVERIFY(db.transaction());
        QString error;
        QVERIFY(!DBHelper::renameFolder(99, "Folder2", "/Folder1", "/Folder2", db, &error));
        QVERIFY(!error.isEmpty());
        QVERIFY(db.rollback());

        QCOMPARE(value(db, "folder", 2, "path"), QString("/Folder1"));
        QCOMPARE(value(db, "folder", 3, "path"), QString("/Folder1/subfolder"));
        QCOMPARE(value(db, "comic", 10, "path"), QString("/Folder1/subfolder/Folder1/comic.cbz"));
    }
    QSqlDatabase::removeDatabase(connectionName);
}

QTEST_GUILESS_MAIN(FolderRenameTest)

#include "main.moc"
