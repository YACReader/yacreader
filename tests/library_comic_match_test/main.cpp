#include "data_base_management.h"
#include "library_comic_match.h"
#include "yacreader_global.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

using namespace YACReader;

class LibraryComicMatchTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void placeholderGivesItsComicToEveryEntry();
    void comicFillsOnlyItsEmptyFields();
    void comicWithoutACoverTakesThePlaceholderCover();
    void comicKeepsItsCover();
    void placeholderNeedsAComicWithAFile();
    void matchResolvesOnlyThePlacementsOfTheComicFolders();
    void matchDeletesThePlaceholderWhenNoPlacementIsLeft();
    void deletedComicGivesItsComicToEveryEntry();
    void deletedComicGivesReadStateOnlyWhenTheComicHasNone();
    void deletedComicMustStillBeDeleted();
    void rollbackRemovesTheWrittenCover();
    void otherListsLeaveOutTheCurrentList();
    void aLabelHoldsAComicOnce();
    void anEntryGetsAComic();

private:
    bool exec(const QString &sql);
    QList<QVariantList> rows(const QString &sql);
    QString dataPath() const;
    void writeImage(const QString &path, const QColor &color);
    // in a transaction; it commits on success and rolls back on a failure, as the callers do
    bool matchPlaceholder(qulonglong placeholderId, qulonglong comicInfoId, ComicMatchFiles &files);
    bool matchDeletedComic(qulonglong deletedId, qulonglong comicInfoId, ComicMatchFiles &files);

    std::unique_ptr<QTemporaryDir> root;
    QSqlDatabase db;
};

void LibraryComicMatchTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    db = QSqlDatabase::addDatabase("QSQLITE", "library_comic_match_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());
    QVERIFY(exec("PRAGMA foreign_keys = ON"));
    QVERIFY(DataBaseManagement::createTables(db));
    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/')"));
    QVERIFY(exec("INSERT INTO comic_info (id, hash, series, number, writer, read, hasBeenOpened, rating, currentPage, lastTimeOpened) VALUES "
                 "(10, 'h10', 'Batman', '1', 'Writer A', 0, 0, 0, 3, 100), "
                 "(20, 'h20', 'Batman', '1', NULL, 1, 1, 4, 7, 200)"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (100, 1, 10, 'Batman 1.cbz', '/Batman 1.cbz')"));
    QVERIFY(exec("INSERT INTO placeholder (id, series, number, title, writer, volume) VALUES (5, 'Batman', '1', 'Court of Owls', 'Writer B', '2011')"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'Event'), (2, 'Saga'), (3, 'Other')"));
    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Favorites tag', 'red', 0)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, placeholder_id, source, source_format) VALUES "
                 "(1, 1, 0, 5, '<Book/>', 'xml'), (2, 2, 4, 5, NULL, NULL)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, comic_info_id, last_known_name, last_known_path) VALUES "
                 "(3, 1, 1, 20, 'Old.cbz', '/Old.cbz'), (4, 3, 0, 20, 'Old.cbz', '/Old.cbz')"));
    QVERIFY(exec("INSERT INTO label_entry (id, label_id, ordering, placeholder_id) VALUES (1, 1, 0, 5)"));
    QVERIFY(exec("INSERT INTO label_entry (id, label_id, ordering, comic_info_id, last_known_name, last_known_path) VALUES (2, 1, 1, 20, 'Old.cbz', '/Old.cbz')"));
}

void LibraryComicMatchTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("library_comic_match_test");
    root.reset();
}

bool LibraryComicMatchTest::exec(const QString &sql)
{
    QSqlQuery query(db);
    return query.exec(sql);
}

QList<QVariantList> LibraryComicMatchTest::rows(const QString &sql)
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

QString LibraryComicMatchTest::dataPath() const
{
    return root->path();
}

void LibraryComicMatchTest::writeImage(const QString &path, const QColor &color)
{
    QVERIFY(QDir().mkpath(QFileInfo(path).absolutePath()));
    QImage image(20, 30, QImage::Format_RGB32);
    image.fill(color);
    QVERIFY(image.save(path, "JPG"));
}

bool LibraryComicMatchTest::matchPlaceholder(qulonglong placeholderId, qulonglong comicInfoId, ComicMatchFiles &files)
{
    db.transaction();
    const bool matched = matchPlaceholderToComic(PlaceholderId(placeholderId), ComicInfoId(comicInfoId), dataPath(), db, files) && db.commit();
    if (!matched)
        db.rollback();
    finishComicMatchFiles(dataPath(), files, matched);
    return matched;
}

bool LibraryComicMatchTest::matchDeletedComic(qulonglong deletedId, qulonglong comicInfoId, ComicMatchFiles &files)
{
    db.transaction();
    const bool matched = matchDeletedComicToComic(ComicInfoId(deletedId), ComicInfoId(comicInfoId), db, files) && db.commit();
    if (!matched)
        db.rollback();
    finishComicMatchFiles(dataPath(), files, matched);
    return matched;
}

void LibraryComicMatchTest::placeholderGivesItsComicToEveryEntry()
{
    ComicMatchFiles files;
    QVERIFY(matchPlaceholder(5, 10, files));

    QCOMPARE(rows("SELECT id, reading_list_id, ordering, comic_info_id, placeholder_id, source FROM reading_list_entry WHERE id IN (1, 2) ORDER BY id"),
             QList<QVariantList>({ { 1, 1, 0, 10, QVariant(), "<Book/>" }, { 2, 2, 4, 10, QVariant(), QVariant() } }));
    QCOMPARE(rows("SELECT id, label_id, ordering, comic_info_id, placeholder_id FROM label_entry WHERE id = 1"), QList<QVariantList>({ { 1, 1, 0, 10, QVariant() } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder"), QList<QVariantList>({ { 0 } }));
}

void LibraryComicMatchTest::comicFillsOnlyItsEmptyFields()
{
    QVERIFY(exec("UPDATE comic_info SET title = '' WHERE id = 10"));
    ComicMatchFiles files;
    QVERIFY(matchPlaceholder(5, 10, files));

    QCOMPARE(rows("SELECT series, number, title, writer, volume, edited FROM comic_info WHERE id = 10"),
             QList<QVariantList>({ { "Batman", "1", "Court of Owls", "Writer A", "2011", 1 } }));
}

void LibraryComicMatchTest::comicWithoutACoverTakesThePlaceholderCover()
{
    const auto placeholderCover = LibraryPaths::placeholderCoverPathFromDataPath(dataPath(), "key");
    const auto comicCover = LibraryPaths::coverPathFromLibraryDataPath(dataPath(), "h10");
    writeImage(placeholderCover, Qt::red);
    QVERIFY(exec("UPDATE placeholder SET coverKey = 'key' WHERE id = 5"));

    ComicMatchFiles files;
    QVERIFY(matchPlaceholder(5, 10, files));
    QVERIFY(QFile::exists(comicCover));
    QVERIFY(!QFile::exists(placeholderCover));
    QCOMPARE(rows("SELECT usesExternalCover, originalCoverSize FROM comic_info WHERE id = 10"), QList<QVariantList>({ { 1, "20x30" } }));
}

void LibraryComicMatchTest::comicKeepsItsCover()
{
    const auto placeholderCover = LibraryPaths::placeholderCoverPathFromDataPath(dataPath(), "key");
    const auto comicCover = LibraryPaths::coverPathFromLibraryDataPath(dataPath(), "h10");
    writeImage(placeholderCover, Qt::red);
    writeImage(comicCover, Qt::blue);
    QVERIFY(exec("UPDATE placeholder SET coverKey = 'key' WHERE id = 5"));

    ComicMatchFiles files;
    QVERIFY(matchPlaceholder(5, 10, files));
    QVERIFY(QImage(comicCover).pixelColor(10, 10).blue() > 200);
    QVERIFY(!QFile::exists(placeholderCover));
    QCOMPARE(rows("SELECT usesExternalCover FROM comic_info WHERE id = 10"), QList<QVariantList>({ { 0 } }));
}

void LibraryComicMatchTest::placeholderNeedsAComicWithAFile()
{
    ComicMatchFiles files;
    QVERIFY(!matchPlaceholder(5, 20, files));
    QVERIFY(!matchPlaceholder(6, 10, files));

    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder"), QList<QVariantList>({ { 1 } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM reading_list_entry WHERE placeholder_id = 5"), QList<QVariantList>({ { 2 } }));
    QCOMPARE(rows("SELECT title FROM comic_info WHERE id = 10"), QList<QVariantList>({ { QVariant() } }));
}

void LibraryComicMatchTest::matchResolvesOnlyThePlacementsOfTheComicFolders()
{
    const auto placeholderCover = LibraryPaths::placeholderCoverPathFromDataPath(dataPath(), "key");
    writeImage(placeholderCover, Qt::red);
    QVERIFY(exec("UPDATE placeholder SET coverKey = 'key' WHERE id = 5"));
    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (2, 1, 'Other', '/Other')"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (1, 5), (2, 5)"));

    ComicMatchFiles files;
    QVERIFY(matchPlaceholder(5, 10, files));

    QCOMPARE(rows("SELECT COUNT(*) FROM reading_list_entry WHERE placeholder_id = 5"), QList<QVariantList>({ { 0 } }));
    QCOMPARE(rows("SELECT folder_id FROM folder_placeholder WHERE placeholder_id = 5"), QList<QVariantList>({ { 2 } }));
    QCOMPARE(rows("SELECT coverKey FROM placeholder WHERE id = 5"), QList<QVariantList>({ { "key" } }));
    QVERIFY(QFile::exists(placeholderCover));
}

void LibraryComicMatchTest::matchDeletesThePlaceholderWhenNoPlacementIsLeft()
{
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (1, 5)"));

    ComicMatchFiles files;
    QVERIFY(matchPlaceholder(5, 10, files));

    QCOMPARE(rows("SELECT COUNT(*) FROM folder_placeholder"), QList<QVariantList>({ { 0 } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder"), QList<QVariantList>({ { 0 } }));
}

void LibraryComicMatchTest::deletedComicGivesItsComicToEveryEntry()
{
    const auto deletedCover = LibraryPaths::coverPathFromLibraryDataPath(dataPath(), "h20");
    writeImage(deletedCover, Qt::green);

    ComicMatchFiles files;
    QVERIFY(matchDeletedComic(20, 10, files));

    QCOMPARE(rows("SELECT id, reading_list_id, ordering, comic_info_id, last_known_name, last_known_path FROM reading_list_entry WHERE id IN (3, 4) ORDER BY id"),
             QList<QVariantList>({ { 3, 1, 1, 10, QVariant(), QVariant() }, { 4, 3, 0, 10, QVariant(), QVariant() } }));
    QCOMPARE(rows("SELECT id, comic_info_id, last_known_name FROM label_entry WHERE id = 2"), QList<QVariantList>({ { 2, 10, QVariant() } }));
    QCOMPARE(rows("SELECT COUNT(*) FROM comic_info WHERE id = 20"), QList<QVariantList>({ { 0 } }));
    QVERIFY(!QFile::exists(deletedCover));
    QCOMPARE(rows("SELECT read, hasBeenOpened, rating, currentPage, lastTimeOpened FROM comic_info WHERE id = 10"),
             QList<QVariantList>({ { 1, 1, 4, 3, 200 } }));
}

void LibraryComicMatchTest::deletedComicGivesReadStateOnlyWhenTheComicHasNone()
{
    QVERIFY(exec("UPDATE comic_info SET hasBeenOpened = 1, rating = 2, lastTimeOpened = 300 WHERE id = 10"));
    QVERIFY(exec("UPDATE comic_info SET title = 'Old title' WHERE id = 20"));

    ComicMatchFiles files;
    QVERIFY(matchDeletedComic(20, 10, files));
    QCOMPARE(rows("SELECT read, hasBeenOpened, rating, currentPage, lastTimeOpened, title FROM comic_info WHERE id = 10"),
             QList<QVariantList>({ { 0, 1, 2, 3, 300, "Old title" } }));
}

void LibraryComicMatchTest::deletedComicMustStillBeDeleted()
{
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (200, 1, 20, 'Back.cbz', '/Back.cbz')"));
    ComicMatchFiles files;
    QVERIFY(!matchDeletedComic(20, 10, files));
    QVERIFY(!matchDeletedComic(10, 10, files));
    QCOMPARE(rows("SELECT COUNT(*) FROM reading_list_entry WHERE comic_info_id = 20"), QList<QVariantList>({ { 2 } }));
}

void LibraryComicMatchTest::rollbackRemovesTheWrittenCover()
{
    const auto placeholderCover = LibraryPaths::placeholderCoverPathFromDataPath(dataPath(), "key");
    const auto comicCover = LibraryPaths::coverPathFromLibraryDataPath(dataPath(), "h10");
    writeImage(placeholderCover, Qt::red);
    QVERIFY(exec("UPDATE placeholder SET coverKey = 'key' WHERE id = 5"));

    ComicMatchFiles files;
    db.transaction();
    QVERIFY(matchPlaceholderToComic(PlaceholderId(5), ComicInfoId(10), dataPath(), db, files));
    QVERIFY(QFile::exists(comicCover));
    QVERIFY(db.rollback());
    finishComicMatchFiles(dataPath(), files, false);

    QVERIFY(!QFile::exists(comicCover));
    QVERIFY(QFile::exists(placeholderCover));
    QCOMPARE(rows("SELECT coverKey FROM placeholder WHERE id = 5"), QList<QVariantList>({ { "key" } }));
}

void LibraryComicMatchTest::otherListsLeaveOutTheCurrentList()
{
    QCOMPARE(otherListsWithEntriesOf({ PlaceholderId(5) }, ListKind::ReadingList, { 1 }, db), QStringList({ "Favorites tag", "Saga" }));
    QCOMPARE(otherListsWithEntriesOf({ PlaceholderId(5) }, ListKind::Label, { 1 }, db), QStringList({ "Event", "Saga" }));
    QCOMPARE(otherListsWithEntriesOf({ ComicInfoId(20) }, ListKind::ReadingList, { 3 }, db), QStringList({ "Event", "Favorites tag" }));
    QCOMPARE(otherListsWithEntriesOf({ ComicInfoId(10) }, ListKind::ReadingList, { 1 }, db), QStringList());
    QCOMPARE(otherListsWithEntriesOf({ PlaceholderId(5), ComicInfoId(20) }, ListKind::ReadingList, { 1, 2 }, db), QStringList({ "Favorites tag", "Other" }));
    QCOMPARE(otherListsWithEntriesOf({ }, ListKind::ReadingList, { 1 }, db), QStringList());
}

void LibraryComicMatchTest::aLabelHoldsAComicOnce()
{
    QVERIFY(exec("INSERT INTO label_entry (id, label_id, ordering, comic_info_id) VALUES (3, 1, 2, 10)"));

    ComicMatchFiles files;
    QVERIFY(matchPlaceholder(5, 10, files));
    QVERIFY(matchDeletedComic(20, 10, files));
    QCOMPARE(rows("SELECT id, comic_info_id FROM label_entry ORDER BY id"), QList<QVariantList>({ { 3, 10 } }));
    // reading lists keep every entry
    QCOMPARE(rows("SELECT COUNT(*) FROM reading_list_entry WHERE comic_info_id = 10"), QList<QVariantList>({ { 4 } }));
}

void LibraryComicMatchTest::anEntryGetsAComic()
{
    QVERIFY(exec("INSERT INTO comic_info (id, hash) VALUES (30, 'h30')"));
    QVERIFY(exec("INSERT INTO label_entry (id, label_id, ordering, comic_info_id) VALUES (3, 1, 2, 30)"));

    QVERIFY(giveComicToEntry({ ListKind::ReadingList, 3 }, ComicInfoId(30), db));
    QCOMPARE(rows("SELECT comic_info_id, last_known_name, ordering FROM reading_list_entry WHERE id = 3"), QList<QVariantList>({ { 30, QVariant(), 1 } }));
    QCOMPARE(rows("SELECT comic_info_id FROM reading_list_entry WHERE id = 4"), QList<QVariantList>({ { 20 } }));

    // the label already has comic 30, so its entry of the deleted comic goes
    QVERIFY(giveComicToEntry({ ListKind::Label, 2 }, ComicInfoId(30), db));
    QCOMPARE(rows("SELECT id, comic_info_id FROM label_entry WHERE id IN (2, 3) ORDER BY id"), QList<QVariantList>({ { 3, 30 } }));
    // it is not removed for itself
    QVERIFY(giveComicToEntry({ ListKind::Label, 3 }, ComicInfoId(30), db));
    QCOMPARE(rows("SELECT COUNT(*) FROM label_entry WHERE id = 3"), QList<QVariantList>({ { 1 } }));

    QVERIFY(!giveComicToEntry({ ListKind::ReadingList, 99 }, ComicInfoId(30), db));
}

QTEST_GUILESS_MAIN(LibraryComicMatchTest)

#include "main.moc"
