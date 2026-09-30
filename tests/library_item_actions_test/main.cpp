#include "db_helper.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

// Covers the targeted updates used by the remote browsers (/v2 item actions API).
class LibraryItemActionsTest : public QObject
{
    Q_OBJECT

private slots:
    void setComicReadOnlyChangesReadFlag();
    void setComicUnreadResetsProgressLikeTheDesktop();
    void setComicTypeOnlyChangesType();
    void setFolderFinishedAndCompletedOnlyChangeTheirFlags();
    void updateFolderTreeTypeChangesTheWholeSubtree();
};

namespace {
QSqlDatabase createDatabase(const QString &connectionName)
{
    auto db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(":memory:");
    db.open();

    QSqlQuery query(db);
    query.exec("CREATE TABLE folder (id INTEGER PRIMARY KEY, parentId INTEGER, name TEXT, path TEXT, finished BOOLEAN, completed BOOLEAN, type INTEGER, numChildren INTEGER)");
    query.exec("CREATE TABLE comic (id INTEGER PRIMARY KEY, parentId INTEGER, comicInfoId INTEGER, fileName TEXT, path TEXT)");
    query.exec("CREATE TABLE comic_info (id INTEGER PRIMARY KEY, title TEXT, read BOOLEAN, currentPage INTEGER, numPages INTEGER, hasBeenOpened BOOLEAN, lastTimeOpened INTEGER, rating INTEGER, type INTEGER)");

    query.exec("INSERT INTO folder VALUES (1, 1, 'root', '/', 0, 0, 0, 2)");
    query.exec("INSERT INTO folder VALUES (2, 1, 'Series', '/Series', 0, 1, 0, 2)");
    query.exec("INSERT INTO folder VALUES (3, 2, 'Volume 1', '/Series/Volume 1', 1, 0, 0, 1)");
    query.exec("INSERT INTO folder VALUES (4, 1, 'Other', '/Other', 0, 0, 0, 1)");

    query.exec("INSERT INTO comic VALUES (10, 2, 100, 'a.cbz', '/Series/a.cbz')");
    query.exec("INSERT INTO comic VALUES (11, 3, 101, 'b.cbz', '/Series/Volume 1/b.cbz')");
    query.exec("INSERT INTO comic VALUES (12, 4, 102, 'c.cbz', '/Other/c.cbz')");
    query.exec("INSERT INTO comic VALUES (13, 1, 103, 'd.cbz', '/d.cbz')");

    query.exec("INSERT INTO comic_info VALUES (100, 'A', 0, 7, 20, 1, 1700000000, 4, 0)");
    query.exec("INSERT INTO comic_info VALUES (101, 'B', 1, 20, 20, 1, 1700000100, 5, 0)");
    query.exec("INSERT INTO comic_info VALUES (102, 'C', 0, 3, 10, 1, 1700000200, 2, 0)");
    query.exec("INSERT INTO comic_info VALUES (103, 'D', 0, 1, 10, 0, NULL, 0, 0)");
    return db;
}

QVariant value(QSqlDatabase &db, const QString &table, int id, const QString &column)
{
    QSqlQuery query(db);
    query.prepare(QString("SELECT %1 FROM %2 WHERE id = :id").arg(column, table));
    query.bindValue(":id", id);
    query.exec();
    query.next();
    return query.value(0);
}
}

void LibraryItemActionsTest::setComicReadOnlyChangesReadFlag()
{
    const QString connectionName = "setComicRead";
    {
        auto db = createDatabase(connectionName);
        QVERIFY(DBHelper::setComicRead(10, true, db));

        QCOMPARE(value(db, "comic_info", 100, "read").toInt(), 1);
        QCOMPARE(value(db, "comic_info", 100, "currentPage").toInt(), 7);
        QCOMPARE(value(db, "comic_info", 100, "hasBeenOpened").toInt(), 1);
        QCOMPARE(value(db, "comic_info", 100, "lastTimeOpened").toLongLong(), 1700000000LL);
        QCOMPARE(value(db, "comic_info", 100, "rating").toInt(), 4);
        QCOMPARE(value(db, "comic_info", 102, "read").toInt(), 0);
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void LibraryItemActionsTest::setComicUnreadResetsProgressLikeTheDesktop()
{
    const QString connectionName = "setComicUnread";
    {
        auto db = createDatabase(connectionName);
        QVERIFY(DBHelper::setComicRead(11, false, db));

        QCOMPARE(value(db, "comic_info", 101, "read").toInt(), 0);
        QCOMPARE(value(db, "comic_info", 101, "currentPage").toInt(), 1);
        QCOMPARE(value(db, "comic_info", 101, "hasBeenOpened").toInt(), 0);
        QVERIFY(value(db, "comic_info", 101, "lastTimeOpened").isNull());
        QCOMPARE(value(db, "comic_info", 101, "title").toString(), QString("B"));
        QCOMPARE(value(db, "comic_info", 101, "rating").toInt(), 5);

        // other comics keep their progress
        QCOMPARE(value(db, "comic_info", 100, "currentPage").toInt(), 7);
        QCOMPARE(value(db, "comic_info", 100, "hasBeenOpened").toInt(), 1);
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void LibraryItemActionsTest::setComicTypeOnlyChangesType()
{
    const QString connectionName = "setComicType";
    {
        auto db = createDatabase(connectionName);
        QVERIFY(DBHelper::setComicType(12, YACReader::FileType::WebComic, db));

        QCOMPARE(value(db, "comic_info", 102, "type").toInt(), static_cast<int>(YACReader::FileType::WebComic));
        QCOMPARE(value(db, "comic_info", 102, "currentPage").toInt(), 3);
        QCOMPARE(value(db, "comic_info", 102, "read").toInt(), 0);
        QCOMPARE(value(db, "comic_info", 100, "type").toInt(), 0);
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void LibraryItemActionsTest::setFolderFinishedAndCompletedOnlyChangeTheirFlags()
{
    const QString connectionName = "setFolderStatus";
    {
        auto db = createDatabase(connectionName);
        QVERIFY(DBHelper::setFolderFinished(2, true, db));
        QVERIFY(DBHelper::setFolderCompleted(2, false, db));

        QCOMPARE(value(db, "folder", 2, "finished").toInt(), 1);
        QCOMPARE(value(db, "folder", 2, "completed").toInt(), 0);
        QCOMPARE(value(db, "folder", 2, "name").toString(), QString("Series"));
        QCOMPARE(value(db, "folder", 2, "numChildren").toInt(), 2);

        // same as YACReaderLibrary: the comics in the folder do not change
        QCOMPARE(value(db, "comic_info", 100, "read").toInt(), 0);
        QCOMPARE(value(db, "folder", 3, "finished").toInt(), 1);
        QCOMPARE(value(db, "folder", 4, "finished").toInt(), 0);
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void LibraryItemActionsTest::updateFolderTreeTypeChangesTheWholeSubtree()
{
    const QString connectionName = "updateFolderTreeType";
    {
        auto db = createDatabase(connectionName);
        QVERIFY(DBHelper::updateFolderTreeType(2, db, YACReader::FileType::Manga));

        const auto manga = static_cast<int>(YACReader::FileType::Manga);
        QCOMPARE(value(db, "folder", 2, "type").toInt(), manga);
        QCOMPARE(value(db, "folder", 3, "type").toInt(), manga);
        QCOMPARE(value(db, "comic_info", 100, "type").toInt(), manga);
        QCOMPARE(value(db, "comic_info", 101, "type").toInt(), manga);

        QCOMPARE(value(db, "folder", 1, "type").toInt(), 0);
        QCOMPARE(value(db, "folder", 4, "type").toInt(), 0);
        QCOMPARE(value(db, "comic_info", 102, "type").toInt(), 0);
        QCOMPARE(value(db, "comic_info", 103, "type").toInt(), 0);
    }
    QSqlDatabase::removeDatabase(connectionName);
}

QTEST_GUILESS_MAIN(LibraryItemActionsTest)

#include "main.moc"
