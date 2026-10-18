#include "cbl_reader.h"

#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringEncoder>
#include <QTemporaryDir>
#include <QTest>

class CblReaderTest : public QObject
{
    Q_OBJECT
private slots:
    void readsAReadingList();
    void keepsTheSourceText_data();
    void keepsTheSourceText();
    void readsComicRackFields();
    void skipsUnknownElements();
    void sourceCount_data();
    void sourceCount();
    void acceptsDeclarationsAndByteOrderMark_data();
    void acceptsDeclarationsAndByteOrderMark();
    void failsWithoutAPartialList_data();
    void failsWithoutAPartialList();
    void failsWhenTheFileCannotBeOpened();
    void readsKavitaListFields();
    void readsAJsonReadingList();
    void readsAJsonBookWithMissingValues();
    void detectsTheFormatFromTheContent_data();
    void detectsTheFormatFromTheContent();
    void failsOnBrokenJson_data();
    void failsOnBrokenJson();
    void booksKeepTheirSource();
    void readsTheDeclaredEncoding();

private:
    static CblReadResult read(const QByteArray &data);
};

CblReadResult CblReaderTest::read(const QByteArray &data)
{
    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    return readCbl(buffer);
}

void CblReaderTest::readsAReadingList()
{
    const auto result = read(R"(<?xml version="1.0" encoding="utf-8"?>
<ReadingList xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
<Name>Event &amp; Tie-ins</Name>
<NumIssues>2</NumIssues>
<Books>
<Book Series="First Series" Number="1" Volume="2008" Year="2008">
<Database Name="cv" Series="34837" Issue="228824" />
</Book>
<Book Series="Second Series" Number="10" Volume="2005" Year="2009">
<Database Name="cv" Series="18164" Issue="138934" />
</Book>
</Books>
<Matchers />
</ReadingList>
)");

    const auto list = std::get_if<CblReadingList>(&result);
    QVERIFY(list != nullptr);
    QCOMPARE(list->format, CblReadingList::Xml);
    QCOMPARE(list->name, QString("Event & Tie-ins"));
    QCOMPARE(list->sourceCount, std::optional<int>(2));
    QCOMPARE(list->books.size(), 2);

    const auto &first = list->books.at(0);
    QCOMPARE(first.series, QString("First Series"));
    QCOMPARE(first.number, QString("1"));
    QCOMPARE(first.volume, QString("2008"));
    QCOMPARE(first.year, QString("2008"));
    QCOMPARE(first.databases, QList<CblDatabaseIds>({ { "cv", "34837", "228824" } }));

    QCOMPARE(list->books.at(1).series, QString("Second Series"));
    QCOMPARE(list->books.at(1).databases.at(0).issueId, QString("138934"));
}

void CblReaderTest::keepsTheSourceText_data()
{
    QTest::addColumn<QString>("number");
    QTest::addColumn<QString>("volume");
    QTest::addColumn<QString>("year");

    QTest::newRow("negative") << "-1"
                              << "1963"
                              << "1997";
    QTest::newRow("decimal") << "23.2"
                             << "2014"
                             << "2015";
    QTest::newRow("fraction") << QString::fromUtf8("½") << "2000"
                              << "2002";
    QTest::newRow("text") << "1.MU"
                          << "2017"
                          << "2017";
    QTest::newRow("zero year") << "0.1"
                               << "1999"
                               << "0";
    QTest::newRow("empty") << ""
                           << ""
                           << "";
}

void CblReaderTest::keepsTheSourceText()
{
    QFETCH(QString, number);
    QFETCH(QString, volume);
    QFETCH(QString, year);

    const auto data = QString(R"(<ReadingList><Books><Book Series="S" Number="%1" Volume="%2" Year="%3" /></Books></ReadingList>)")
                              .arg(number, volume, year)
                              .toUtf8();
    const auto result = read(data);

    const auto list = std::get_if<CblReadingList>(&result);
    QVERIFY(list != nullptr);
    QCOMPARE(list->books.size(), 1);
    QCOMPARE(list->books.at(0).number, number);
    QCOMPARE(list->books.at(0).volume, volume);
    QCOMPARE(list->books.at(0).year, year);
}

void CblReaderTest::readsComicRackFields()
{
    const auto result = read(R"(<ReadingList>
<Books>
<Book Series=" Padded " Number=" 3 " Volume="1990" Year="1991" Format="Annual">
<Id>2f1d6c3a-3b2b-4c55-9a4c-7d6c1a2b3c4d</Id>
<FileName>Padded 003 (1991).cbz</FileName>
<Database Name="cv" Series="1" Issue="2" />
<Database Name="other" Series="a-1" Issue="b-2" />
</Book>
</Books>
</ReadingList>)");

    const auto list = std::get_if<CblReadingList>(&result);
    QVERIFY(list != nullptr);
    QVERIFY(list->name.isEmpty());
    QVERIFY(!list->sourceCount);

    const auto &book = list->books.at(0);
    QCOMPARE(book.series, QString("Padded"));
    QCOMPARE(book.number, QString("3"));
    QCOMPARE(book.format, QString("Annual"));
    QCOMPARE(book.id, QString("2f1d6c3a-3b2b-4c55-9a4c-7d6c1a2b3c4d"));
    QCOMPARE(book.fileName, QString("Padded 003 (1991).cbz"));
    QCOMPARE(book.databases, QList<CblDatabaseIds>({ { "cv", "1", "2" }, { "other", "a-1", "b-2" } }));
}

void CblReaderTest::skipsUnknownElements()
{
    const auto result = read(R"(<ReadingList>
<Name>List</Name>
<Unknown><Nested>text</Nested></Unknown>
<Books>
<Book Series="A" Number="1"><Notes><Note>skip</Note></Notes><Database Name="cv" Series="1" Issue="2" /></Book>
<NotABook Series="B" Number="2" />
<Book Series="C" Number="3" />
</Books>
<Matchers>
<ComicBookMatcher xsi:type="ComicBookSeriesMatcher" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"><MatchValue>A</MatchValue></ComicBookMatcher>
</Matchers>
</ReadingList>)");

    const auto list = std::get_if<CblReadingList>(&result);
    QVERIFY(list != nullptr);
    QCOMPARE(list->name, QString("List"));
    QCOMPARE(list->books.size(), 2);
    QCOMPARE(list->books.at(0).series, QString("A"));
    QCOMPARE(list->books.at(0).databases.size(), 1);
    QCOMPARE(list->books.at(1).series, QString("C"));
}

void CblReaderTest::sourceCount_data()
{
    QTest::addColumn<QString>("numIssues");
    QTest::addColumn<std::optional<int>>("sourceCount");

    QTest::newRow("more than the books") << "<NumIssues>5</NumIssues>" << std::optional<int>(5);
    QTest::newRow("missing") << "" << std::optional<int>();
    QTest::newRow("not a number") << "<NumIssues>many</NumIssues>" << std::optional<int>();
}

void CblReaderTest::sourceCount()
{
    QFETCH(QString, numIssues);
    QFETCH(std::optional<int>, sourceCount);

    const auto result = read(QString("<ReadingList>%1<Books><Book Series=\"A\" Number=\"1\" /><Book Series=\"A\" Number=\"2\" /></Books></ReadingList>").arg(numIssues).toUtf8());

    const auto list = std::get_if<CblReadingList>(&result);
    QVERIFY(list != nullptr);
    QCOMPARE(list->sourceCount, sourceCount);
    QCOMPARE(list->books.size(), 2);
}

void CblReaderTest::acceptsDeclarationsAndByteOrderMark_data()
{
    QTest::addColumn<QByteArray>("prefix");

    QTest::newRow("double quotes") << QByteArray(R"(<?xml version="1.0" encoding="utf-8"?>)");
    QTest::newRow("single quotes") << QByteArray("<?xml version='1.0' encoding='utf-8'?>");
    QTest::newRow("byte order mark") << QByteArray("\xEF\xBB\xBF<?xml version=\"1.0\" encoding=\"utf-8\"?>");
    QTest::newRow("no declaration") << QByteArray();
}

void CblReaderTest::acceptsDeclarationsAndByteOrderMark()
{
    QFETCH(QByteArray, prefix);

    const auto result = read(prefix + QString("<ReadingList><Name>Éclair</Name><Books><Book Series=\"A\" Number=\"½\" /></Books></ReadingList>").toUtf8());

    const auto list = std::get_if<CblReadingList>(&result);
    QVERIFY(list != nullptr);
    QCOMPARE(list->name, QString::fromUtf8("Éclair"));
    QCOMPARE(list->books.at(0).number, QString::fromUtf8("½"));
}

void CblReaderTest::failsWithoutAPartialList_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("kind");

    QTest::newRow("empty") << QByteArray() << int(CblReadError::Malformed);
    QTest::newRow("not xml") << QByteArray("Series,Number\nA,1\n") << int(CblReadError::Malformed);
    QTest::newRow("other root") << QByteArray("<ComicInfo><Series>A</Series></ComicInfo>") << int(CblReadError::NotAReadingList);
    QTest::newRow("cut off") << QByteArray("<ReadingList><Books><Book Series=\"A\" Number=\"1\" />") << int(CblReadError::Malformed);
    QTest::newRow("broken book") << QByteArray("<ReadingList><Books><Book Series=\"A\" Number=\"1\" /><Book Series=\"B><Book/></Books></ReadingList>") << int(CblReadError::Malformed);
    QTest::newRow("content after the list") << QByteArray("<ReadingList><Books /></ReadingList><ReadingList />") << int(CblReadError::Malformed);
}

void CblReaderTest::failsWithoutAPartialList()
{
    QFETCH(QByteArray, data);
    QFETCH(int, kind);

    const auto result = read(data);

    const auto error = std::get_if<CblReadError>(&result);
    QVERIFY(error != nullptr);
    QCOMPARE(int(error->kind), kind);
    QVERIFY(!error->detail.isEmpty());
    if (error->kind == CblReadError::Malformed)
        QVERIFY(error->line > 0);
}

void CblReaderTest::failsWhenTheFileCannotBeOpened()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const auto result = readCbl(directory.filePath("missing.cbl"));

    const auto error = std::get_if<CblReadError>(&result);
    QVERIFY(error != nullptr);
    QCOMPARE(error->kind, CblReadError::CannotOpen);
}

// Kavita's additions to the XML format; Book@FileType is not read
void CblReaderTest::readsKavitaListFields()
{
    const auto result = read(R"(<ReadingList>
<Name>Era</Name>
<Summary> What happens in the era. </Summary>
<StartYear>1986</StartYear>
<StartMonth>3</StartMonth>
<EndYear>1987</EndYear>
<EndMonth>not a month</EndMonth>
<Books><Book Series="A" Number="1" FileType="epub" /></Books>
</ReadingList>)");

    const auto list = std::get_if<CblReadingList>(&result);
    QVERIFY(list != nullptr);
    QCOMPARE(list->description, QString("What happens in the era."));
    QCOMPARE(list->startYear, std::optional<int>(1986));
    QCOMPARE(list->startMonth, std::optional<int>(3));
    QCOMPARE(list->endYear, std::optional<int>(1987));
    QVERIFY(!list->endMonth);
    QCOMPARE(list->books.size(), 1);
}

void CblReaderTest::readsAJsonReadingList()
{
    const auto result = read(R"({
  "fileDetails": { "UUID": "a59e4ad5-0d51-4afe-b0f5-6d3e01eb7cc7", "version": 1.0 },
  "listDetails": {
    "name": "Part 1",
    "description": "The first part.",
    "startYear": 2021,
    "endYear": 2022,
    "publisher": "Publisher",
    "imprint": "Imprint",
    "type": "universal",
    "tags": ["team", "era"],
    "coverImageURLs": ["https://example.com/cover.jpg"],
    "relationships": [
      { "name": "Part 0", "UUID": "e1eecedf-df97-4ab4-a476-ee85204e3b78", "relationship": "previous" },
      { "name": "Part 2", "UUID": "507e7444-8f5f-46fc-a2cc-17f08c000982", "relationship": "following" }
    ],
    "source": [ { "name": "A Guide", "url": "https://example.com/guide", "id": "g-1" } ]
  },
  "notes": "Read in order.",
  "issueList": [
    {
      "seriesName": "First Series",
      "seriesStartYear": 2021,
      "issueNumber": "1",
      "issueCoverDate": "2021-07-01",
      "issueType": "Event Core",
      "id": [
        { "name": "comicvine", "series": "135903", "issue": "847511" },
        { "name": "metron", "series": "2139", "issue": "29926" }
      ]
    },
    {
      "seriesName": "Second Series",
      "seriesStartYear": 1963,
      "issueNumber": "-1",
      "issueCoverDate": "1997-07-01",
      "id": [ { "name": "grandComicsDatabase", "series": "77", "issue": "88" } ]
    }
  ]
})");

    const auto list = std::get_if<CblReadingList>(&result);
    QVERIFY(list != nullptr);
    QCOMPARE(list->format, CblReadingList::Json);
    QCOMPARE(list->uuid, QString("a59e4ad5-0d51-4afe-b0f5-6d3e01eb7cc7"));
    QCOMPARE(list->name, QString("Part 1"));
    QCOMPARE(list->description, QString("The first part."));
    QCOMPARE(list->startYear, std::optional<int>(2021));
    QCOMPARE(list->endYear, std::optional<int>(2022));
    QVERIFY(!list->startMonth);
    QVERIFY(!list->sourceCount);
    QCOMPARE(list->publisher, QString("Publisher"));
    QCOMPARE(list->imprint, QString("Imprint"));
    QCOMPARE(list->type, QString("universal"));
    QCOMPARE(list->tags, QStringList({ "team", "era" }));
    QCOMPARE(list->coverImageUrls, QStringList({ "https://example.com/cover.jpg" }));
    QCOMPARE(list->relationships, QList<CblRelationship>({ { "Part 0", "e1eecedf-df97-4ab4-a476-ee85204e3b78", "previous" }, { "Part 2", "507e7444-8f5f-46fc-a2cc-17f08c000982", "following" } }));
    QCOMPARE(list->sources, QList<CblSource>({ { "A Guide", "https://example.com/guide", "g-1" } }));
    QCOMPARE(list->notes, QString("Read in order."));

    QCOMPARE(list->books.size(), 2);
    const auto &first = list->books.at(0);
    QCOMPARE(first.series, QString("First Series"));
    QCOMPARE(first.volume, QString("2021"));
    QCOMPARE(first.number, QString("1"));
    QCOMPARE(first.coverDate, QString("2021-07-01"));
    QCOMPARE(first.year, QString("2021"));
    QCOMPARE(first.issueType, QString("Event Core"));
    QCOMPARE(first.databases, QList<CblDatabaseIds>({ { "comicvine", "135903", "847511" }, { "metron", "2139", "29926" } }));

    const auto &second = list->books.at(1);
    QCOMPARE(second.number, QString("-1"));
    QCOMPARE(second.volume, QString("1963"));
    QCOMPARE(second.year, QString("1997"));
    QCOMPARE(second.databases, QList<CblDatabaseIds>({ { "grandComicsDatabase", "77", "88" } }));
}

// the schema requires these values, but real files leave some out
void CblReaderTest::readsAJsonBookWithMissingValues()
{
    const auto result = read(R"({ "listDetails": { "name": "L" }, "issueList": [ { "seriesName": "A", "issueNumber": 5, "seriesStartYear": "1990" } ] })");

    const auto list = std::get_if<CblReadingList>(&result);
    QVERIFY(list != nullptr);
    QVERIFY(list->uuid.isEmpty());
    QCOMPARE(list->books.size(), 1);
    QCOMPARE(list->books.at(0).number, QString("5"));
    QCOMPARE(list->books.at(0).volume, QString("1990"));
    QVERIFY(list->books.at(0).coverDate.isEmpty());
    QVERIFY(list->books.at(0).year.isEmpty());
    QVERIFY(list->books.at(0).databases.isEmpty());
}

void CblReaderTest::detectsTheFormatFromTheContent_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("format");

    QTest::newRow("json") << QByteArray(R"({"issueList":[]})") << int(CblReadingList::Json);
    QTest::newRow("json after spaces and a byte order mark") << QByteArray("\xEF\xBB\xBF \r\n\t{\"issueList\":[]}") << int(CblReadingList::Json);
    QTest::newRow("xml") << QByteArray("<ReadingList><Books /></ReadingList>") << int(CblReadingList::Xml);
    QTest::newRow("xml after spaces") << QByteArray("\n  <ReadingList><Books /></ReadingList>") << int(CblReadingList::Xml);
}

void CblReaderTest::detectsTheFormatFromTheContent()
{
    QFETCH(QByteArray, data);
    QFETCH(int, format);

    const auto result = read(data);

    const auto list = std::get_if<CblReadingList>(&result);
    QVERIFY(list != nullptr);
    QCOMPARE(int(list->format), format);
    QVERIFY(list->books.isEmpty());
}

void CblReaderTest::failsOnBrokenJson_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("kind");
    QTest::addColumn<qint64>("line");

    QTest::newRow("cut off") << QByteArray("{\n  \"issueList\": [\n    { \"seriesName\": \"A\"") << int(CblReadError::Malformed) << qint64(3);
    QTest::newRow("no issue list") << QByteArray(R"({ "listDetails": { "name": "L" } })") << int(CblReadError::NotAReadingList) << qint64(0);
    QTest::newRow("issue list is not a list") << QByteArray(R"({ "issueList": { "seriesName": "A" } })") << int(CblReadError::NotAReadingList) << qint64(0);
    QTest::newRow("an issue is not an object") << QByteArray(R"({ "issueList": [ { "seriesName": "A" }, "B 2" ] })") << int(CblReadError::Malformed) << qint64(0);
}

void CblReaderTest::failsOnBrokenJson()
{
    QFETCH(QByteArray, data);
    QFETCH(int, kind);
    QFETCH(qint64, line);

    const auto result = read(data);

    const auto error = std::get_if<CblReadError>(&result);
    QVERIFY(error != nullptr);
    QCOMPARE(int(error->kind), kind);
    QVERIFY(!error->detail.isEmpty());
    QCOMPARE(error->line, line);
}

void CblReaderTest::booksKeepTheirSource()
{
    const auto xml = read(R"(<ReadingList><Books>
    <Book Series="A" Number="1" />
    <Book Series="B &amp; C" Number="2">
      <Database Name="cv" Series="1" Issue="2" />
      <Unknown>kept</Unknown>
    </Book>
  </Books></ReadingList>)");
    const auto xmlList = std::get_if<CblReadingList>(&xml);
    QVERIFY(xmlList != nullptr);
    QCOMPARE(xmlList->books.at(0).source, QString(R"(<Book Series="A" Number="1" />)"));
    QCOMPARE(xmlList->books.at(1).source, QString(R"(<Book Series="B &amp; C" Number="2">
      <Database Name="cv" Series="1" Issue="2" />
      <Unknown>kept</Unknown>
    </Book>)"));

    const auto json = read(R"({ "issueList": [ { "seriesName": "A", "issueNumber": "1", "extra": { "kept": true }, "id": [ { "name": "metron", "series": "1", "issue": "2" } ] } ] })");
    const auto jsonList = std::get_if<CblReadingList>(&json);
    QVERIFY(jsonList != nullptr);
    const auto source = QJsonDocument::fromJson(jsonList->books.at(0).source.toUtf8()).object();
    QCOMPARE(source.value("extra").toObject().value("kept").toBool(), true);
    QCOMPARE(source.value("id").toArray().at(0).toObject().value("name").toString(), QString("metron"));
}

void CblReaderTest::readsTheDeclaredEncoding()
{
    QByteArray latin1 = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?><ReadingList><Name>";
    latin1 += char(0xC9); // É
    latin1 += "clair</Name><Books><Book Series=\"A\" Number=\"1\" /></Books></ReadingList>";
    const auto result = read(latin1);
    const auto list = std::get_if<CblReadingList>(&result);
    QVERIFY(list != nullptr);
    QCOMPARE(list->name, QString::fromUtf8("Éclair"));

    QByteArray utf16;
    {
        QStringEncoder encoder(QStringConverter::Utf16LE, QStringConverter::Flag::WriteBom);
        utf16 = encoder(QString::fromUtf8("<ReadingList><Name>Ñame</Name><Books /></ReadingList>"));
    }
    const auto fromUtf16 = read(utf16);
    QVERIFY(std::get_if<CblReadingList>(&fromUtf16) != nullptr);
    QCOMPARE(std::get<CblReadingList>(fromUtf16).name, QString::fromUtf8("Ñame"));
}

QTEST_GUILESS_MAIN(CblReaderTest)
#include "main.moc"
