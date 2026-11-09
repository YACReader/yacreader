#include "comic_vine_json_parser.h"
#include "selected_volume_info.h"

#include <QTest>

class ComicVineParserTest : public QObject
{
    Q_OBJECT
private slots:
    void patchHasOnlyTheReturnedFields();
    void unreadableOrEmptyResultsGiveAnEmptyPatch_data();
    void unreadableOrEmptyResultsGiveAnEmptyPatch();
    void answers();
};

void ComicVineParserTest::patchHasOnlyTheReturnedFields()
{
    using M = DescriptiveComicMetadata;
    const auto json = QStringLiteral(R"({
        "number_of_total_results": 1,
        "results": {
            "id": 99,
            "name": "An issue",
            "issue_number": "7",
            "volume": { "name": "A series" },
            "person_credits": [ { "name": "Writer", "role": "writer" }, { "name": "Artist", "role": "penciler, inker" } ],
            "cover_date": "2024-05-01",
            "description": "A synopsis",
            "location_credits": [ { "name": "A city" } ],
            "team_credits": [ { "name": "A team" } ],
            "character_credits": [ { "name": "A hero" }, { "name": "A villain" } ]
        }
    })");
    const SelectedVolumeInfo volume { "1", 12, "A publisher", "A volume" };

    const auto patch = YACReader::parseCVJSONComicInfo(json, volume);

    DescriptiveComicMetadata metadata;
    metadata.notes = QString("Kept notes");
    metadata.storyArc = QString("Kept arc");
    patch.applyTo(metadata);

    QCOMPARE(metadata.title.toString(), QString("An issue"));
    QCOMPARE(metadata.number.toString(), QString("7"));
    QCOMPARE(metadata.series.toString(), QString("A series"));
    QCOMPARE(metadata.writer.toString(), QString("Writer"));
    QCOMPARE(metadata.penciller.toString(), QString("Artist"));
    QCOMPARE(metadata.inker.toString(), QString("Artist"));
    QCOMPARE(metadata.date.toString(), QString("01/05/2024"));
    QCOMPARE(metadata.synopsis.toString(), QString("A synopsis"));
    QCOMPARE(metadata.characters.toString(), QString("A hero\nA villain"));
    QCOMPARE(metadata.locations.toString(), QString("A city"));
    QCOMPARE(metadata.teams.toString(), QString("A team"));
    QCOMPARE(metadata.count.toInt(), 12);
    QCOMPARE(metadata.publisher.toString(), QString("A publisher"));

    QCOMPARE(metadata.notes.toString(), QString("Kept notes"));
    QCOMPARE(metadata.storyArc.toString(), QString("Kept arc"));
    for (const auto field : { &M::notes, &M::review, &M::tags, &M::storyArc, &M::arcNumber, &M::arcCount, &M::genere, &M::imprint, &M::comicVineID })
        QVERIFY(!patch.contains(field));
}

void ComicVineParserTest::unreadableOrEmptyResultsGiveAnEmptyPatch_data()
{
    QTest::addColumn<QString>("json");
    QTest::newRow("not json") << "not json";
    QTest::newRow("no results") << R"({ "number_of_total_results": 0, "results": [] })";
}

void ComicVineParserTest::unreadableOrEmptyResultsGiveAnEmptyPatch()
{
    QFETCH(QString, json);
    QVERIFY(YACReader::parseCVJSONComicInfo(json, { "1", 1, "A publisher", QString() }).isEmpty());
}

void ComicVineParserTest::answers()
{
    using YACReader::ComicVineAnswer;
    const QByteArray ok = R"({ "status_code": 1, "number_of_total_results": 1, "results": { } })";
    QCOMPARE(YACReader::comicVineAnswer(false, false, ok), ComicVineAnswer::Answered);
    QCOMPARE(YACReader::comicVineAnswer(true, false, ok), ComicVineAnswer::Failed);
    QCOMPARE(YACReader::comicVineAnswer(false, true, QByteArray()), ComicVineAnswer::Failed);
    QCOMPARE(YACReader::comicVineAnswer(false, false, R"({ "status_code": 101, "error": "Object Not Found" })"), ComicVineAnswer::Failed);
    QCOMPARE(YACReader::comicVineAnswer(false, false, "not json"), ComicVineAnswer::Failed);
    // the limit answer can come with an HTTP error
    QCOMPARE(YACReader::comicVineAnswer(true, false, R"({ "status_code": 107, "error": "Rate limit exceeded" })"), ComicVineAnswer::LimitReached);
}

QTEST_GUILESS_MAIN(ComicVineParserTest)
#include "main.moc"
