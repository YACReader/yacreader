#include "comic_file.h"
#include "data_base_management.h"
#include "folder.h"
#include "library_ids.h"
#include "reading_list.h"
#include "table_projection.h"
#include "yacreader_global.h"

#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTest>

#include <functional>
#include <type_traits>

static_assert(!std::is_convertible_v<qulonglong, FolderId>);
static_assert(!std::is_convertible_v<FolderId, qulonglong>);
static_assert(!std::is_convertible_v<FolderId, ComicId>);
static_assert(!std::is_constructible_v<ComicInfoId, ComicId>);
static_assert(!std::is_same_v<ReadingListId, LabelId>);

class TableProjectionTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void idsCompareAndHash();
    void columnsMatchSchema_data();
    void columnsMatchSchema();
    void comicFileRoundTrip();
    void comicFileNullPath();
    void readingListRoundTrip();
    void readingListUpdate();
    void labelRoundTrip();
    void folderReadsNullColumns();
    void folderRoundTrip();
    void folderKeepsAStoredUnknownCount();
    void aliasesKeepJoinedTypesApart();

private:
    QSqlDatabase db;

    qulonglong insertRow(const QString &table, const QStringList &columns, const std::function<void(QSqlQuery &)> &bind);
    QSqlRecord selectOne(const QString &table, const QString &alias, const QStringList &columns, qulonglong id);
};

void TableProjectionTest::init()
{
    db = QSqlDatabase::addDatabase("QSQLITE", "table_projection_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());
    QVERIFY(DataBaseManagement::createTables(db));
    QSqlQuery q(db);
    QVERIFY(q.exec("INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/')"));
    QVERIFY(q.exec("INSERT INTO folder (id, parentId, name, path) VALUES (7, 1, 'Series', '/Series')"));
    QVERIFY(q.exec("INSERT INTO comic_info (id, hash) VALUES (42, 'hash-42')"));
}

void TableProjectionTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("table_projection_test");
}

qulonglong TableProjectionTest::insertRow(const QString &table, const QStringList &columns, const std::function<void(QSqlQuery &)> &bind)
{
    QSqlQuery q(db);
    if (!q.prepare("INSERT INTO " + table + " (" + TableProjection::insertColumns(columns) + ") VALUES (" + TableProjection::insertValues(columns) + ")")) {
        qWarning() << q.lastError().text();
        return 0;
    }
    bind(q);
    if (!q.exec()) {
        qWarning() << q.lastError().text();
        return 0;
    }
    return q.lastInsertId().toULongLong();
}

QSqlRecord TableProjectionTest::selectOne(const QString &table, const QString &alias, const QStringList &columns, qulonglong id)
{
    QSqlQuery q(db);
    q.prepare("SELECT " + TableProjection::selectList(alias, columns) + " FROM " + table + " " + alias + " WHERE " + alias + ".id = :id");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) {
        qWarning() << q.lastError().text();
        return { };
    }
    return q.record();
}

void TableProjectionTest::idsCompareAndHash()
{
    QCOMPARE(FolderId().value(), 0ULL);
    QCOMPARE(FolderId(7).value(), 7ULL);
    QVERIFY(FolderId(7) == FolderId(7));
    QVERIFY(FolderId(7) != FolderId(8));
    QVERIFY(FolderId(7) < FolderId(8));

    QSet<ComicInfoId> ids { ComicInfoId(1), ComicInfoId(2), ComicInfoId(1) };
    QCOMPARE(ids.size(), 2);
    QVERIFY(ids.contains(ComicInfoId(2)));
}

void TableProjectionTest::columnsMatchSchema_data()
{
    QTest::addColumn<QString>("table");
    QTest::addColumn<QStringList>("columns");
    QTest::addColumn<QStringList>("notRead");

    QTest::newRow("comic") << "comic" << ComicFile::columns() << QStringList();
    QTest::newRow("reading_list") << "reading_list" << ReadingList::columns() << QStringList { "manga" };
    QTest::newRow("label") << "label" << Label::columns() << QStringList();
    QTest::newRow("folder") << "folder" << Folder::columns() << QStringList { "manga" };
}

void TableProjectionTest::columnsMatchSchema()
{
    QFETCH(QString, table);
    QFETCH(QStringList, columns);
    QFETCH(QStringList, notRead);

    QSqlQuery q(db);
    QVERIFY(q.exec("PRAGMA table_info(" + table + ")"));
    QSet<QString> schema;
    while (q.next())
        schema.insert(q.value(1).toString());

    QSet<QString> projected { "id" };
    for (const auto &column : columns) {
        QVERIFY2(!projected.contains(column), qPrintable(column));
        projected.insert(column);
    }
    for (const auto &column : notRead)
        projected.insert(column);

    QCOMPARE(projected, schema);
}

void TableProjectionTest::comicFileRoundTrip()
{
    ComicFile file;
    file.folderId = FolderId(7);
    file.comicInfoId = ComicInfoId(42);
    file.fileName = "Issue 1.cbz";
    file.path = "/Series/Issue 1.cbz";

    const auto id = insertRow("comic", ComicFile::columns(), [&](QSqlQuery &q) { file.bindTo(q); });
    QVERIFY(id != 0);

    const auto read = ComicFile::fromRecord(selectOne("comic", "c", ComicFile::columns(), id), "c");
    QCOMPARE(read.id, ComicId(id));
    QCOMPARE(read.folderId, FolderId(7));
    QCOMPARE(read.comicInfoId, ComicInfoId(42));
    QCOMPARE(read.fileName, QString("Issue 1.cbz"));
    QCOMPARE(read.path, QString("/Series/Issue 1.cbz"));
}

void TableProjectionTest::comicFileNullPath()
{
    ComicFile file;
    file.folderId = FolderId(7);
    file.comicInfoId = ComicInfoId(42);
    file.fileName = "Issue 2.cbz";

    const auto id = insertRow("comic", ComicFile::columns(), [&](QSqlQuery &q) { file.bindTo(q); });
    QVERIFY(id != 0);

    QSqlQuery q(db);
    QVERIFY(q.exec(QString("SELECT path IS NULL FROM comic WHERE id = %1").arg(id)));
    QVERIFY(q.next());
    QVERIFY(q.value(0).toBool());

    const auto read = ComicFile::fromRecord(selectOne("comic", "c", ComicFile::columns(), id), "c");
    QVERIFY(read.path.isNull());
}

void TableProjectionTest::readingListRoundTrip()
{
    ReadingList parent;
    parent.name = "Events";
    parent.ordering = 3;
    parent.finished = true;
    parent.completed = false;

    const auto parentId = insertRow("reading_list", ReadingList::columns(), [&](QSqlQuery &q) { parent.bindTo(q); });
    QVERIFY(parentId != 0);

    QSqlQuery q(db);
    QVERIFY(q.exec(QString("SELECT parentId IS NULL FROM reading_list WHERE id = %1").arg(parentId)));
    QVERIFY(q.next());
    QVERIFY(q.value(0).toBool());

    const auto readParent = ReadingList::fromRecord(selectOne("reading_list", "rl", ReadingList::columns(), parentId), "rl");
    QCOMPARE(readParent.id, ReadingListId(parentId));
    QVERIFY(!readParent.parentId.has_value());
    QCOMPARE(readParent.ordering, 3);
    QCOMPARE(readParent.name, QString("Events"));
    QCOMPARE(readParent.finished, true);
    QCOMPARE(readParent.completed, false);

    ReadingList child;
    child.parentId = ReadingListId(parentId);
    child.name = "Crossover";
    child.ordering = 1;

    const auto childId = insertRow("reading_list", ReadingList::columns(), [&](QSqlQuery &q) { child.bindTo(q); });
    QVERIFY(childId != 0);

    const auto readChild = ReadingList::fromRecord(selectOne("reading_list", "rl", ReadingList::columns(), childId), "rl");
    QCOMPARE(readChild.parentId, std::optional<ReadingListId>(ReadingListId(parentId)));
    QCOMPARE(readChild.name, QString("Crossover"));
    QCOMPARE(readChild.ordering, 1);
    QCOMPARE(readChild.finished, false);
    QCOMPARE(readChild.completed, true);
}

void TableProjectionTest::readingListUpdate()
{
    QSqlQuery q(db);
    QVERIFY(q.exec("INSERT INTO reading_list (id, name) VALUES (5, 'Top')"));
    QVERIFY(q.exec("INSERT INTO reading_list (id, parentId, name) VALUES (6, 5, 'Child')"));

    auto list = ReadingList::fromRecord(selectOne("reading_list", "rl", ReadingList::columns(), 6), "rl");
    QCOMPARE(list.parentId, std::optional<ReadingListId>(ReadingListId(5)));
    list.parentId.reset();
    list.name = "Moved";

    QSqlQuery update(db);
    QVERIFY(update.prepare("UPDATE reading_list SET " + TableProjection::assignments(ReadingList::columns()) + " WHERE id = :id"));
    list.bindTo(update);
    update.bindValue(":id", list.id.value());
    QVERIFY2(update.exec(), qPrintable(update.lastError().text()));

    const auto read = ReadingList::fromRecord(selectOne("reading_list", "rl", ReadingList::columns(), 6), "rl");
    QVERIFY(!read.parentId.has_value());
    QCOMPARE(read.name, QString("Moved"));
}

void TableProjectionTest::labelRoundTrip()
{
    Label label;
    label.name = "To read";
    label.color = YACReader::colorToName(YACReader::YGreen);
    label.ordering = YACReader::YGreen;

    const auto id = insertRow("label", Label::columns(), [&](QSqlQuery &q) { label.bindTo(q); });
    QVERIFY(id != 0);

    const auto read = Label::fromRecord(selectOne("label", "l", Label::columns(), id), "l");
    QCOMPARE(read.id, LabelId(id));
    QCOMPARE(read.name, QString("To read"));
    QCOMPARE(read.color, QString("green"));
    QCOMPARE(read.ordering, int(YACReader::YGreen));
}

void TableProjectionTest::folderReadsNullColumns()
{
    const auto folder = Folder::fromRecord(selectOne("folder", "f", Folder::columns(), 7), "f");
    QCOMPARE(folder.id, FolderId(7));
    QCOMPARE(folder.parentId, FolderId(1));
    QCOMPARE(folder.name, QString("Series"));
    QCOMPARE(folder.path, QString("/Series"));
    QCOMPARE(folder.finished, false);
    QCOMPARE(folder.completed, true);
    QVERIFY(!folder.numChildren.has_value());
    QVERIFY(folder.firstChildHash.isNull());
    QVERIFY(folder.customImage.isNull());
    QCOMPARE(folder.type, YACReader::FileType::Comic);
    QCOMPARE(folder.added, 0);
    QCOMPARE(folder.updated, 0);
}

void TableProjectionTest::folderRoundTrip()
{
    auto folder = Folder::fromRecord(selectOne("folder", "f", Folder::columns(), 7), "f");
    folder.name = "Renamed";
    folder.path = "/Renamed";
    folder.finished = true;
    folder.completed = false;
    folder.numChildren = 3;
    folder.firstChildHash = "hash-3";
    folder.customImage = "cover.jpg";
    folder.type = YACReader::FileType::Manga;
    folder.added = 10;
    folder.updated = 20;

    QSqlQuery update(db);
    QVERIFY(update.prepare("UPDATE folder SET " + TableProjection::assignments(Folder::columns()) + " WHERE id = :id"));
    folder.bindTo(update);
    update.bindValue(":id", folder.id.value());
    QVERIFY2(update.exec(), qPrintable(update.lastError().text()));

    const auto read = Folder::fromRecord(selectOne("folder", "f", Folder::columns(), 7), "f");
    QCOMPARE(read.name, QString("Renamed"));
    QCOMPARE(read.path, QString("/Renamed"));
    QCOMPARE(read.finished, true);
    QCOMPARE(read.completed, false);
    QCOMPARE(read.numChildren, std::optional<qint32>(3));
    QCOMPARE(read.firstChildHash, QString("hash-3"));
    QCOMPARE(read.customImage, QString("cover.jpg"));
    QCOMPARE(read.type, YACReader::FileType::Manga);
    QCOMPARE(read.added, 10);
    QCOMPARE(read.updated, 20);

    folder.numChildren.reset();
    folder.bindTo(update);
    update.bindValue(":id", folder.id.value());
    QVERIFY2(update.exec(), qPrintable(update.lastError().text()));

    QSqlQuery q(db);
    QVERIFY(q.exec("SELECT numChildren IS NULL FROM folder WHERE id = 7"));
    QVERIFY(q.next());
    QVERIFY(q.value(0).toBool());
}

void TableProjectionTest::folderKeepsAStoredUnknownCount()
{
    QSqlQuery q(db);
    QVERIFY(q.exec("UPDATE folder SET numChildren = -1 WHERE id = 7"));

    const auto folder = Folder::fromRecord(selectOne("folder", "f", Folder::columns(), 7), "f");
    QCOMPARE(folder.numChildren, std::optional<qint32>(-1));
}

void TableProjectionTest::aliasesKeepJoinedTypesApart()
{
    QSqlQuery q(db);
    QVERIFY(q.exec("INSERT INTO reading_list (id, name, ordering) VALUES (10, 'Parent', 4)"));
    QVERIFY(q.exec("INSERT INTO reading_list (id, parentId, name, ordering) VALUES (11, 10, 'Child', 2)"));

    QVERIFY(q.exec("SELECT " + TableProjection::selectList("c", ReadingList::columns()) + ", " +
                   TableProjection::selectList("p", ReadingList::columns()) +
                   " FROM reading_list c JOIN reading_list p ON c.parentId = p.id"));
    QVERIFY(q.next());

    const auto child = ReadingList::fromRecord(q.record(), "c");
    const auto parent = ReadingList::fromRecord(q.record(), "p");
    QCOMPARE(child.id, ReadingListId(11));
    QCOMPARE(child.name, QString("Child"));
    QCOMPARE(child.ordering, 2);
    QCOMPARE(parent.id, ReadingListId(10));
    QCOMPARE(parent.name, QString("Parent"));
    QCOMPARE(parent.ordering, 4);
    QVERIFY(!parent.parentId.has_value());
}

QTEST_GUILESS_MAIN(TableProjectionTest)

#include "main.moc"
