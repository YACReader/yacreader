#include "cbl_importer.h"
#include "data_base_management.h"
#include "db_helper.h"

#include <QBuffer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTest>

class CblImporterTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void importsEveryBookInOrder();
    void unmatchedBooksBecomePlaceholders_data();
    void unmatchedBooksBecomePlaceholders();
    void theSameIssueSharesOnePlaceholder();
    void choicesCanBeExistingPlaceholders();
    void aFailureLeavesNothing();
    void choicesMustMatchTheBooks();
    void listNameFallsBackToTheFileName();
    void readMatchAndImport();
    void placeholderEntriesKeepTheBookSource();

private:
    bool exec(const QString &sql);
    QList<QVariantList> rows(const QString &sql);
    static CblBook book(const QString &series, const QString &number, const QString &volume = { }, const QString &year = { });
    static CblLibraryComic comic(qulonglong id);

    QSqlDatabase db;
};

void CblImporterTest::init()
{
    db = QSqlDatabase::addDatabase("QSQLITE", "cbl_importer_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());
    QVERIFY(exec("PRAGMA foreign_keys = ON"));
    QVERIFY(DataBaseManagement::createTables(db));
    QVERIFY(exec("INSERT INTO comic_info (id, hash, series, number) VALUES (10, 'h10', 'Batman', '1'), (11, 'h11', 'Batman', '2')"));
    QVERIFY(exec("INSERT INTO placeholder (id, series, number) VALUES (5, 'Batman', '3')"));
}

void CblImporterTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("cbl_importer_test");
}

bool CblImporterTest::exec(const QString &sql)
{
    QSqlQuery query(db);
    return query.exec(sql);
}

QList<QVariantList> CblImporterTest::rows(const QString &sql)
{
    QList<QVariantList> result;
    QSqlQuery query(db);
    if (!query.exec(sql))
        return result;
    while (query.next()) {
        QVariantList row;
        // NULL is a null QVariant of the column type; the tests compare it with QVariant()
        for (int i = 0; i < query.record().count(); i++)
            row.append(query.value(i).isNull() ? QVariant() : query.value(i));
        result.append(row);
    }
    return result;
}

CblBook CblImporterTest::book(const QString &series, const QString &number, const QString &volume, const QString &year)
{
    CblBook result;
    result.series = series;
    result.number = number;
    result.volume = volume;
    result.year = year;
    return result;
}

static CblReadingList listOf(const QList<CblBook> &books, CblReadingList::Format format = CblReadingList::Xml)
{
    CblReadingList list;
    list.format = format;
    list.books = books;
    return list;
}

CblLibraryComic CblImporterTest::comic(qulonglong id)
{
    CblLibraryComic result;
    result.id = ComicInfoId(id);
    result.hasFile = true;
    return result;
}

void CblImporterTest::importsEveryBookInOrder()
{
    CblLibraryPlaceholder existing;
    existing.id = PlaceholderId(5);
    const auto id = importCblReadingList("Event", listOf({ book("Batman", "2"), book("Robin", "1"), book("Batman", "3"), book("Batman", "1") }),
                                         { comic(11), std::nullopt, existing, comic(10) }, db);
    QVERIFY(id);

    QCOMPARE(rows(QString("SELECT name, parentId FROM reading_list WHERE id = %1").arg(id->value())), QList<QVariantList>({ { "Event", QVariant() } }));
    const auto entries = rows(QString("SELECT ordering, comic_info_id, placeholder_id FROM reading_list_entry WHERE reading_list_id = %1 ORDER BY ordering").arg(id->value()));
    QCOMPARE(entries.size(), 4);
    QCOMPARE(entries.at(0), QVariantList({ 0, 11, QVariant() }));
    QCOMPARE(entries.at(1).at(1), QVariant());
    QVERIFY(entries.at(1).at(2).toULongLong() > 5);
    QCOMPARE(entries.at(2), QVariantList({ 2, QVariant(), 5 }));
    QCOMPARE(entries.at(3), QVariantList({ 3, 10, QVariant() }));
}

void CblImporterTest::unmatchedBooksBecomePlaceholders_data()
{
    QTest::addColumn<CblBook>("source");
    QTest::addColumn<QVariantList>("expected"); // series, number, volume, date, format, comicVineID

    auto full = book(" Amazing Spider-Man ", "-1", "1963", "1997");
    full.format = "Main Series";
    full.databases = { { "metron", "1", "2" }, { "cv", "2127", "38593" } };
    QTest::newRow("xml with a year") << full << QVariantList({ "Amazing Spider-Man", "-1", "1963", "//1997", "Main Series", "38593" });

    auto json = book("Heroes Reborn", "1", "2021");
    json.coverDate = "2021-07-01";
    json.year = "2021";
    json.databases = { { "comicvine", "135903", "847511" } };
    QTest::newRow("json with a cover date") << json << QVariantList({ "Heroes Reborn", "1", "2021", "01/07/2021", QVariant(), "847511" });

    auto unknownYear = book("Hulk", QString::fromUtf8("½"), { }, "0");
    unknownYear.databases = { { "cv", "1", "not a number" } };
    QTest::newRow("year zero and a bad id") << unknownYear << QVariantList({ "Hulk", QString::fromUtf8("½"), QVariant(), QVariant(), QVariant(), QVariant() });
}

void CblImporterTest::unmatchedBooksBecomePlaceholders()
{
    QFETCH(CblBook, source);
    QFETCH(QVariantList, expected);

    const auto id = importCblReadingList("List", listOf({ source }), { std::nullopt }, db);
    QVERIFY(id);

    const auto placeholders = rows(QString("SELECT p.series, p.number, p.volume, p.date, p.format, p.comicVineID, p.title, p.coverKey FROM placeholder p "
                                           "INNER JOIN reading_list_entry e ON e.placeholder_id = p.id WHERE e.reading_list_id = %1")
                                           .arg(id->value()));
    QCOMPARE(placeholders.size(), 1);
    QCOMPARE(placeholders.first().mid(0, 6), expected);
    QVERIFY(placeholders.first().at(6).isNull());
    QVERIFY(placeholders.first().at(7).isNull());
}

void CblImporterTest::theSameIssueSharesOnePlaceholder()
{
    auto byId = book("Robin", "1");
    byId.databases = { { "cv", "1", "77" } };
    auto byIdOtherName = book("Robin Annual", "1");
    byIdOtherName.databases = { { "cv", "1", "77" } };

    const auto id = importCblReadingList("List", listOf({ byId, book("Robin", "2", "1993", "1994"), byIdOtherName, book("robin", "2", "1993", "1994"), book("", ""), book("", ""), book("Robin", "2", "2010", "2010") }),
                                         QList<std::optional<CblCandidate>>(7, std::nullopt), db);
    QVERIFY(id);

    const auto placeholderIds = rows(QString("SELECT placeholder_id FROM reading_list_entry WHERE reading_list_id = %1 ORDER BY ordering").arg(id->value()));
    QCOMPARE(placeholderIds.size(), 7);
    QCOMPARE(placeholderIds.at(0), placeholderIds.at(2));
    QCOMPARE(placeholderIds.at(1), placeholderIds.at(3));
    QVERIFY(placeholderIds.at(0) != placeholderIds.at(1));
    // books without series or number, and other volumes, are other issues
    QVERIFY(placeholderIds.at(4) != placeholderIds.at(5));
    QVERIFY(placeholderIds.at(6) != placeholderIds.at(1));
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder").first().first().toInt(), 1 + 5);
}

void CblImporterTest::choicesCanBeExistingPlaceholders()
{
    CblLibraryPlaceholder existing;
    existing.id = PlaceholderId(5);
    const auto first = importCblReadingList("First", listOf({ book("Batman", "3") }), { existing }, db);
    const auto second = importCblReadingList("Second", listOf({ book("Batman", "3") }), { existing }, db);
    QVERIFY(first && second);

    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder").first().first().toInt(), 1);
    QCOMPARE(rows("SELECT placeholder_id FROM reading_list_entry ORDER BY reading_list_id"), QList<QVariantList>({ { 5 }, { 5 } }));
}

void CblImporterTest::aFailureLeavesNothing()
{
    const auto before = rows("SELECT (SELECT COUNT(*) FROM reading_list), (SELECT COUNT(*) FROM reading_list_entry), (SELECT COUNT(*) FROM placeholder)");

    // the second choice refers to a comic_info that does not exist
    const auto id = importCblReadingList("List", listOf({ book("New", "1"), book("Gone", "1") }), { std::nullopt, comic(999) }, db);

    QVERIFY(!id);
    QCOMPARE(rows("SELECT (SELECT COUNT(*) FROM reading_list), (SELECT COUNT(*) FROM reading_list_entry), (SELECT COUNT(*) FROM placeholder)"), before);
}

void CblImporterTest::choicesMustMatchTheBooks()
{
    QVERIFY(!importCblReadingList("List", listOf({ book("A", "1"), book("B", "1") }), { std::nullopt }, db));
    QCOMPARE(rows("SELECT COUNT(*) FROM reading_list").first().first().toInt(), 0);
}

void CblImporterTest::listNameFallsBackToTheFileName()
{
    CblReadingList list;
    list.name = "  Named  ";
    QCOMPARE(cblListName(list, "/lists/file name.cbl"), QString("Named"));
    list.name = " ";
    QCOMPARE(cblListName(list, "/lists/[Marvel] Event (WEB).cbl"), QString("[Marvel] Event (WEB)"));
}

// a second import of the same file reuses the placeholders of the first
void CblImporterTest::readMatchAndImport()
{
    QVERIFY(exec("UPDATE comic_info SET volume = '2016', date = '01/06/2016' WHERE id IN (10, 11)"));
    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/')"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (100, 1, 10, 'b1.cbz', '/b1.cbz')"));

    QBuffer file;
    file.setData(R"(<ReadingList><Name>Event</Name><Books>
<Book Series="Batman" Number="001" Volume="2016" Year="2016" />
<Book Series="Robin" Number="1" Volume="2016" Year="2016"><Database Name="cv" Series="1" Issue="4242" /></Book>
<Book Series="The Batman" Number="2" Volume="2016" Year="2016" />
</Books></ReadingList>)");
    file.open(QIODevice::ReadOnly);
    const auto read = readCbl(file);
    const auto list = std::get<CblReadingList>(read);

    const auto import = [&] {
        const CblMatcher matcher(*CblMatcher::loadLibraryComics(db), *CblMatcher::loadLibraryPlaceholders(db));
        QList<std::optional<CblCandidate>> choices;
        for (const auto &book : list.books)
            choices.append(matcher.match(book).accepted);
        return importCblReadingList(cblListName(list, "event.cbl"), list, choices, db);
    };

    const auto first = import();
    QVERIFY(first);
    QCOMPARE(rows(QString("SELECT COALESCE(comic_info_id, 0), placeholder_id IS NOT NULL FROM reading_list_entry WHERE reading_list_id = %1 ORDER BY ordering").arg(first->value())),
             QList<QVariantList>({ { 10, 0 }, { 0, 1 }, { 11, 0 } }));

    const auto second = import();
    QVERIFY(second);
    QCOMPARE(rows(QString("SELECT placeholder_id FROM reading_list_entry WHERE reading_list_id IN (%1, %2) AND placeholder_id IS NOT NULL").arg(first->value()).arg(second->value())).size(), 2);
    QCOMPARE(rows("SELECT COUNT(*) FROM placeholder WHERE comicVineID = '4242'").first().first().toInt(), 1);
}

// only entries of a placeholder keep the book as in the file
void CblImporterTest::placeholderEntriesKeepTheBookSource()
{
    QBuffer file;
    file.setData(R"(<?xml version="1.0" encoding="utf-8"?>
<ReadingList><Name>Sources</Name><Books>
  <Book Series="Batman" Number="1" />
  <!-- not in the library -->
  <Book Series="Robin" Number="7" Volume="1993" Year="1994"><Database Name="metron" Series="9" Issue="99" /><Extra>kept</Extra></Book>
</Books></ReadingList>)");
    file.open(QIODevice::ReadOnly);
    const auto list = std::get<CblReadingList>(readCbl(file));

    const auto id = importCblReadingList("Sources", list, { comic(10), std::nullopt }, db);
    QVERIFY(id);
    const auto entries = rows(QString("SELECT source, source_format FROM reading_list_entry WHERE reading_list_id = %1 ORDER BY ordering").arg(id->value()));
    QCOMPARE(entries.at(0), QVariantList({ QVariant(), QVariant() }));
    QCOMPARE(entries.at(1), QVariantList({ R"(<!-- not in the library -->
  <Book Series="Robin" Number="7" Volume="1993" Year="1994"><Database Name="metron" Series="9" Issue="99" /><Extra>kept</Extra></Book>)",
                                           "xml" }));

    auto json = book("Heroes Reborn", "2");
    json.source = R"({"id":[{"issue":"1","name":"metron","series":"2"}],"issueNumber":"2","seriesName":"Heroes Reborn"})";
    const auto fromJson = importCblReadingList("Json", listOf({ json }, CblReadingList::Json), { std::nullopt }, db);
    QVERIFY(fromJson);
    QCOMPARE(rows(QString("SELECT source, source_format FROM reading_list_entry WHERE reading_list_id = %1").arg(fromJson->value())),
             QList<QVariantList>({ { json.source, "json" } }));
}

QTEST_GUILESS_MAIN(CblImporterTest)
#include "main.moc"
