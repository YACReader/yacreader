#include "cbl_list_match.h"
#include "data_base_management.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

class CblListMatchTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void aComicEntryChangesOnlyItself();
    void aMissingEntryIsMatchedEverywhere();
    void entriesOfOnePlaceholderCanGetDifferentComics();
    void aDeletedComicIsMatchedEverywhere();
    void makeMissingUsesTheSource();
    void makeMissingUsesTheComicWithoutASource();
    void missingEntriesStayMissing();
    void aGoneEntryFails();
    void listEntriesComeFromTheRows();

private:
    bool exec(const QString &sql);
    QList<QVariantList> rows(const QString &sql);
    bool apply(const QList<CblListChange> &changes);

    std::unique_ptr<QTemporaryDir> root;
    QSqlDatabase db;
};

void CblListMatchTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    db = QSqlDatabase::addDatabase("QSQLITE", "cbl_list_match_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());
    QVERIFY(exec("PRAGMA foreign_keys = ON"));
    QVERIFY(DataBaseManagement::createTables(db));
    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/')"));
    QVERIFY(exec("INSERT INTO comic_info (id, hash, series, number, writer) VALUES "
                 "(10, 'h10', 'Batman', '1', 'Writer A'), (11, 'h11', 'Batman', '2', NULL), (20, 'h20', 'Batman', '3', NULL)"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES "
                 "(100, 1, 10, 'Batman 1.cbz', '/Batman 1.cbz'), (110, 1, 11, 'Batman 2.cbz', '/Batman 2.cbz')"));
    QVERIFY(exec("INSERT INTO placeholder (id, series, number) VALUES (5, 'Batman', '4')"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'Event'), (2, 'Saga')"));
    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Tag', 'red', 0)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, comic_info_id, placeholder_id, last_known_name, source, source_format) VALUES "
                 "(1, 1, 0, NULL, 5, NULL, NULL, NULL), "
                 "(2, 2, 0, NULL, 5, NULL, NULL, NULL), "
                 "(3, 1, 1, 10, NULL, NULL, '<Book Series=\"Nova\" Number=\"3\" Volume=\"2007\" Year=\"2007\" />', 'xml'), "
                 "(4, 1, 2, 11, NULL, NULL, NULL, NULL), "
                 "(5, 1, 3, 20, NULL, 'Old.cbz', NULL, NULL), "
                 "(6, 2, 1, 20, NULL, 'Old.cbz', NULL, NULL)"));
    QVERIFY(exec("INSERT INTO label_entry (id, label_id, ordering, comic_info_id) VALUES (1, 1, 0, 10)"));
}

void CblListMatchTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("cbl_list_match_test");
    root.reset();
}

bool CblListMatchTest::exec(const QString &sql)
{
    QSqlQuery query(db);
    return query.exec(sql);
}

QList<QVariantList> CblListMatchTest::rows(const QString &sql)
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

bool CblListMatchTest::apply(const QList<CblListChange> &changes)
{
    ComicMatchFiles files;
    db.transaction();
    const bool applied = applyCblListChanges(changes, root->path(), db, files) && db.commit();
    if (!applied)
        db.rollback();
    finishComicMatchFiles(root->path(), files, applied);
    return applied;
}

static ListItemId readingListEntry(qulonglong id)
{
    return { ListKind::ReadingList, id };
}

void CblListMatchTest::aComicEntryChangesOnlyItself()
{
    QVERIFY(apply({ { readingListEntry(3), ComicInfoId(11) }, { readingListEntry(4), ComicInfoId(11) } }));

    QCOMPARE(rows("SELECT id, ordering, comic_info_id, source_format FROM reading_list_entry WHERE id IN (3, 4) ORDER BY id"),
             QList<QVariantList>({ { 3, 1, 11, "xml" }, { 4, 2, 11, QVariant() } }));
    QCOMPARE(rows("SELECT comic_info_id FROM label_entry"), QList<QVariantList>({ { 10 } }));
    QCOMPARE(rows("SELECT writer FROM comic_info WHERE id = 11"), QList<QVariantList>({ { QVariant() } }));
}

void CblListMatchTest::aMissingEntryIsMatchedEverywhere()
{
    QVERIFY(apply({ { readingListEntry(1), ComicInfoId(11) } }));

    QCOMPARE(rows("SELECT id, comic_info_id, placeholder_id FROM reading_list_entry WHERE id IN (1, 2) ORDER BY id"),
             QList<QVariantList>({ { 1, 11, QVariant() }, { 2, 11, QVariant() } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder"), QList<QVariantList>({ { 0 } }));
}

void CblListMatchTest::entriesOfOnePlaceholderCanGetDifferentComics()
{
    QVERIFY(apply({ { readingListEntry(1), ComicInfoId(10) }, { readingListEntry(2), ComicInfoId(11) } }));

    QCOMPARE(rows("SELECT id, comic_info_id FROM reading_list_entry WHERE id IN (1, 2) ORDER BY id"), QList<QVariantList>({ { 1, 10 }, { 2, 11 } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder"), QList<QVariantList>({ { 0 } }));
}

void CblListMatchTest::aDeletedComicIsMatchedEverywhere()
{
    QVERIFY(apply({ { readingListEntry(5), ComicInfoId(10) } }));

    QCOMPARE(rows("SELECT id, comic_info_id, last_known_name FROM reading_list_entry WHERE id IN (5, 6) ORDER BY id"),
             QList<QVariantList>({ { 5, 10, QVariant() }, { 6, 10, QVariant() } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM comic_info WHERE id = 20"), QList<QVariantList>({ { 0 } }));
}

void CblListMatchTest::makeMissingUsesTheSource()
{
    QVERIFY(apply({ { readingListEntry(3), std::nullopt } }));

    const auto entry = rows("SELECT comic_info_id, placeholder_id, source_format FROM reading_list_entry WHERE id = 3");
    QCOMPARE(entry.size(), 1);
    QCOMPARE(entry.at(0).at(0), QVariant());
    QCOMPARE(entry.at(0).at(2), QVariant("xml"));
    QCOMPARE(rows(QString("SELECT series, number, volume, date, writer FROM placeholder WHERE id = %1").arg(entry.at(0).at(1).toULongLong())),
             QList<QVariantList>({ { "Nova", "3", "2007", "//2007", QVariant() } }));
    // the comic stays for its other entries
    QCOMPARE(rows("SELECT comic_info_id FROM label_entry"), QList<QVariantList>({ { 10 } }));
}

void CblListMatchTest::makeMissingUsesTheComicWithoutASource()
{
    QVERIFY(apply({ { { ListKind::Label, 1 }, std::nullopt } }));

    const auto entry = rows("SELECT comic_info_id, placeholder_id FROM label_entry WHERE id = 1");
    QCOMPARE(entry.size(), 1);
    QCOMPARE(entry.at(0).at(0), QVariant());
    QCOMPARE(rows(QString("SELECT series, number, writer FROM placeholder WHERE id = %1").arg(entry.at(0).at(1).toULongLong())),
             QList<QVariantList>({ { "Batman", "1", "Writer A" } }));
}

void CblListMatchTest::missingEntriesStayMissing()
{
    QVERIFY(apply({ { readingListEntry(1), std::nullopt }, { readingListEntry(5), std::nullopt }, { readingListEntry(4), ComicInfoId(11) } }));

    QCOMPARE(rows("SELECT id, comic_info_id, placeholder_id FROM reading_list_entry WHERE id IN (1, 4, 5) ORDER BY id"),
             QList<QVariantList>({ { 1, QVariant(), 5 }, { 4, 11, QVariant() }, { 5, 20, QVariant() } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder"), QList<QVariantList>({ { 1 } }));
}

void CblListMatchTest::aGoneEntryFails()
{
    QVERIFY(!apply({ { readingListEntry(3), ComicInfoId(11) }, { readingListEntry(99), ComicInfoId(11) } }));
    QCOMPARE(rows("SELECT comic_info_id FROM reading_list_entry WHERE id = 3"), QList<QVariantList>({ { 10 } }));

    // a missing entry needs a comic with a file
    QVERIFY(!apply({ { readingListEntry(1), ComicInfoId(20) } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder"), QList<QVariantList>({ { 1 } }));
}

void CblListMatchTest::listEntriesComeFromTheRows()
{
    ListEntry entry;
    entry.id = readingListEntry(3);
    entry.listId = 1;
    entry.source = EntrySource { R"(<Book Series="Nova" Number="3" />)", "xml" };

    ComicDB comic;
    comic.info.id = ComicInfoId(10);
    comic.info.hash = "h10";
    comic.info.series = "Batman";
    comic.info.number = "1";
    comic.file.path = "/Batman/Batman 1.cbz";

    const auto withSource = cblListEntry(ComicRecordCase::Comic { comic }, entry);
    QCOMPARE(withSource.id, entry.id);
    QCOMPARE(withSource.listId, qulonglong(1));
    QCOMPARE(withSource.book.series, QString("Nova"));
    const auto *current = std::get_if<CblLibraryComic>(&withSource.current);
    QVERIFY(current != nullptr);
    QVERIFY(current->hasFile);
    QCOMPARE(current->path, QString("/Batman/Batman 1.cbz"));
    QCOMPARE(current->series, QString("Batman"));

    entry.source.reset();
    Placeholder placeholder;
    placeholder.id = PlaceholderId(5);
    placeholder.series = "Batman";
    placeholder.number = "4";
    placeholder.date = "01/02/2011";
    placeholder.comicVineID = "77";
    const auto withoutSource = cblListEntry(ComicRecordCase::Placeholder { placeholder }, entry);
    QCOMPARE(withoutSource.book.series, QString("Batman"));
    QCOMPARE(withoutSource.book.number, QString("4"));
    QCOMPARE(withoutSource.book.year, QString("2011"));
    QCOMPARE(cblComicVineIssueId(withoutSource.book), QString("77"));
    QVERIFY(std::holds_alternative<CblLibraryPlaceholder>(withoutSource.current));

    const auto deleted = cblListEntry(ComicRecordCase::DeletedComic { comic.info, std::nullopt }, entry);
    QVERIFY(!std::get<CblLibraryComic>(deleted.current).hasFile);
}

QTEST_GUILESS_MAIN(CblListMatchTest)

#include "main.moc"
