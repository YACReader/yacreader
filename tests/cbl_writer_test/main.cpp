#include "cbl_list_match.h"
#include "cbl_reader.h"
#include "cbl_writer.h"

#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

class CblWriterTest : public QObject
{
    Q_OBJECT
private slots:
    void xmlReadsBack();
    void jsonReadsBack();
    void jsonHasTheFileDetails();
    void sourcesOfTheSameFormatAreKept();
    void sourcesAreConvertedToTheOtherFormat();
    void unknownValuesAreLeftOut();
    void textIsEscaped_data();
    void textIsEscaped();
    void metadataGivesTheCoverDate();

private:
    static CblReadingList read(const QByteArray &data);
};

CblReadingList CblWriterTest::read(const QByteArray &data)
{
    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    const auto result = readCbl(buffer);
    if (const auto *error = std::get_if<CblReadError>(&result))
        qWarning() << error->detail << error->line << error->column;
    return std::get<CblReadingList>(result);
}

CblBook book(const QString &series, const QString &number, const QString &volume, const QString &year)
{
    CblBook result;
    result.series = series;
    result.number = number;
    result.volume = volume;
    result.year = year;
    return result;
}

void CblWriterTest::xmlReadsBack()
{
    auto first = book("Batman", "1", "2016", "2016");
    first.format = "Main Series";
    first.id = "c0ffee";
    first.databases = { { "cv", "91273", "541321" }, { "gcd", "1", "2" } };
    const auto second = book("Superman", "½", "", "1987");

    const auto list = read(writeCbl("DC Rebirth", { { first, { }, { } }, { second, { }, { } } }, CblReadingList::Xml, { }));
    QCOMPARE(list.format, CblReadingList::Xml);
    QCOMPARE(list.name, QString("DC Rebirth"));
    QCOMPARE(list.sourceCount, std::optional<int>(2));
    QCOMPARE(list.books.size(), 2);
    QCOMPARE(list.books[0].series, QString("Batman"));
    QCOMPARE(list.books[0].number, QString("1"));
    QCOMPARE(list.books[0].volume, QString("2016"));
    QCOMPARE(list.books[0].year, QString("2016"));
    QCOMPARE(list.books[0].format, QString("Main Series"));
    QCOMPARE(list.books[0].id, QString("c0ffee"));
    QCOMPARE(list.books[0].databases, first.databases);
    QCOMPARE(list.books[1].number, QString("½"));
    QVERIFY(list.books[1].volume.isEmpty());
    QVERIFY(list.books[1].databases.isEmpty());
}

void CblWriterTest::jsonReadsBack()
{
    auto first = book("Batman", "1", "2016", "2016");
    first.coverDate = "2016-08-01";
    first.issueType = "Ongoing";
    // provider names of json-cbl-standard; one it has no name for is left out
    first.databases = { { "cv", "91273", "541321" }, { "gcd", "1", "2" }, { "metron", "3", "4" }, { "other", "5", "6" } };

    const auto list = read(writeCbl("DC Rebirth", { { first, { }, { } } }, CblReadingList::Json, "uuid-1"));
    QCOMPARE(list.format, CblReadingList::Json);
    QCOMPARE(list.uuid, QString("uuid-1"));
    QCOMPARE(list.name, QString("DC Rebirth"));
    QCOMPARE(list.books.size(), 1);
    const auto &written = list.books[0];
    QCOMPARE(written.series, QString("Batman"));
    QCOMPARE(written.number, QString("1"));
    QCOMPARE(written.volume, QString("2016"));
    QCOMPARE(written.coverDate, QString("2016-08-01"));
    QCOMPARE(written.year, QString("2016"));
    QCOMPARE(written.issueType, QString("Ongoing"));
    QCOMPARE(written.databases, QList<CblDatabaseIds>({ { "comicvine", "91273", "541321" }, { "grandComicsDatabase", "1", "2" }, { "metron", "3", "4" } }));
}

void CblWriterTest::jsonHasTheFileDetails()
{
    const auto root = QJsonDocument::fromJson(writeCbl("List", { { book("A", "1", "2001", "2001"), { }, { } } }, CblReadingList::Json, "uuid-2")).object();
    QCOMPARE(root.value("fileDetails").toObject().value("UUID").toString(), QString("uuid-2"));
    QCOMPARE(root.value("fileDetails").toObject().value("version").toDouble(), 1.0);
    QCOMPARE(root.value("listDetails").toObject().value("name").toString(), QString("List"));
    const auto issue = root.value("issueList").toArray().at(0).toObject();
    // seriesStartYear is a number, issueNumber a string
    QVERIFY(issue.value("seriesStartYear").isDouble());
    QVERIFY(issue.value("issueNumber").isString());
}

void CblWriterTest::sourcesOfTheSameFormatAreKept()
{
    const QString xmlSource = "<!-- part 1 -->\n<Book Series=\"Batman\" Number=\"1\" Volume=\"2016\" Year=\"2016\" Custom=\"kept\">\n<Database Name=\"cv\" Series=\"1\" Issue=\"2\" />\n<Unknown>kept</Unknown>\n</Book>";
    const auto xml = QString::fromUtf8(writeCbl("List", { { book("Other", "9", "", ""), xmlSource, "xml" } }, CblReadingList::Xml, { }));
    QVERIFY(xml.contains(xmlSource));
    QCOMPARE(read(xml.toUtf8()).books[0].series, QString("Batman"));

    const QString jsonSource = R"({"custom":"kept","issueNumber":"1","seriesName":"Batman","seriesStartYear":2016})";
    const auto root = QJsonDocument::fromJson(writeCbl("List", { { book("Other", "9", "", ""), jsonSource, "json" } }, CblReadingList::Json, "uuid")).object();
    QCOMPARE(root.value("issueList").toArray().at(0).toObject(), QJsonDocument::fromJson(jsonSource.toUtf8()).object());
}

void CblWriterTest::sourcesAreConvertedToTheOtherFormat()
{
    const QString xmlSource = "<Book Series=\"Batman\" Number=\"1\" Volume=\"2016\" Year=\"2016\">\n<Database Name=\"cv\" Series=\"1\" Issue=\"2\" />\n<Database Name=\"gcd\" Series=\"3\" Issue=\"4\" />\n</Book>";
    const auto fromXml = readCblBook(xmlSource, "xml");
    QVERIFY(fromXml);
    const auto json = read(writeCbl("List", { { *fromXml, xmlSource, "xml" } }, CblReadingList::Json, "uuid"));
    QCOMPARE(json.books[0].series, QString("Batman"));
    QCOMPARE(json.books[0].volume, QString("2016"));
    QCOMPARE(json.books[0].databases, QList<CblDatabaseIds>({ { "comicvine", "1", "2" }, { "grandComicsDatabase", "3", "4" } }));

    const QString jsonSource = R"({"id":[{"issue":"2","name":"comicvine","series":"1"}],"issueCoverDate":"2016-08-01","issueNumber":"1","seriesName":"Batman","seriesStartYear":2016})";
    const auto fromJson = readCblBook(jsonSource, "json");
    QVERIFY(fromJson);
    const auto xml = read(writeCbl("List", { { *fromJson, jsonSource, "json" } }, CblReadingList::Xml, { }));
    QCOMPARE(xml.books[0].series, QString("Batman"));
    QCOMPARE(xml.books[0].volume, QString("2016"));
    // the year of the cover date, and ComicRack's name for Comic Vine
    QCOMPARE(xml.books[0].year, QString("2016"));
    QCOMPARE(xml.books[0].databases, QList<CblDatabaseIds>({ { "cv", "1", "2" } }));
}

void CblWriterTest::unknownValuesAreLeftOut()
{
    // a volume that is not a year, and a year with no full date
    const auto root = QJsonDocument::fromJson(writeCbl("List", { { book("Batman", "1", "Vol. 2", "2016"), { }, { } } }, CblReadingList::Json, "uuid")).object();
    const auto issue = root.value("issueList").toArray().at(0).toObject();
    QVERIFY(!issue.contains("seriesStartYear"));
    QVERIFY(!issue.contains("issueCoverDate"));
    QVERIFY(!issue.contains("id"));
    QVERIFY(!issue.contains("issueType"));

    const auto xml = QString::fromUtf8(writeCbl("List", { { book("Batman", "1", "", ""), { }, { } } }, CblReadingList::Xml, { }));
    QVERIFY(xml.contains("<Book Series=\"Batman\" Number=\"1\" />"));
}

void CblWriterTest::textIsEscaped_data()
{
    QTest::addColumn<int>("format");
    QTest::newRow("xml") << int(CblReadingList::Xml);
    QTest::newRow("json") << int(CblReadingList::Json);
}

void CblWriterTest::textIsEscaped()
{
    QFETCH(int, format);
    const auto list = read(writeCbl("Tom & Jerry <\"best\">", { { book("Tom & \"Jerry\" <1>", "1", "", ""), { }, { } } }, CblReadingList::Format(format), "uuid"));
    QCOMPARE(list.name, QString("Tom & Jerry <\"best\">"));
    QCOMPARE(list.books[0].series, QString("Tom & \"Jerry\" <1>"));
}

void CblWriterTest::metadataGivesTheCoverDate()
{
    DescriptiveComicMetadata metadata;
    metadata.series = "Batman";
    metadata.date = "5/8/2016";
    QCOMPARE(cblBookOf(metadata).coverDate, QString("2016-08-05"));
    QCOMPARE(cblBookOf(metadata).year, QString("2016"));

    // a year alone has no cover date
    metadata.date = "//2016";
    QVERIFY(cblBookOf(metadata).coverDate.isEmpty());
    QCOMPARE(cblBookOf(metadata).year, QString("2016"));
}

QTEST_MAIN(CblWriterTest)

#include "main.moc"
