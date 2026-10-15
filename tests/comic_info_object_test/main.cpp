#include "comic_db.h"
#include "comic_info_object.h"

#include <QBuffer>
#include <QDataStream>
#include <QMetaProperty>
#include <QSet>
#include <QSignalSpy>
#include <QTest>

#include <type_traits>

static_assert(!std::is_base_of_v<QObject, ComicInfo>);
static_assert(std::is_copy_constructible_v<ComicInfo>);
static_assert(std::is_copy_assignable_v<ComicInfo>);

class ComicInfoObjectTest : public QObject
{
    Q_OBJECT
private slots:
    void exposesTheQmlProperties();
    void readsTheComic();
    void writablePropertiesNotifyOnce_data();
    void writablePropertiesNotifyOnce();
    void streamKeepsTheExistOnDbSlot_data();
    void streamKeepsTheExistOnDbSlot();
    void streamRoundTrip();
};

static ComicDB sampleComic()
{
    ComicDB comic;
    comic.id = 11;
    comic.parentId = 3;
    comic.name = "Issue 1.cbz";
    comic.path = "/Series/Issue 1.cbz";
    comic.info.id = 42;
    comic.info.hash = "hash-42";
    comic.info.read = true;
    comic.info.rating = 4;
    comic.info.currentPage = 7;
    comic.info.title = "Title";
    comic.info.number = "1";
    comic.info.series = "Series";
    comic.info.writer = "Writer A\nWriter B";
    comic.info.storyArc = "Arc";
    comic.info.arcNumber = "2";
    comic.info.arcCount = 5;
    return comic;
}

void ComicInfoObjectTest::exposesTheQmlProperties()
{
    const QSet<QString> expected {
        "id", "read", "edited", "hash", "rating", "hasBeenOpened",
        "currentPage", "bookmark1", "bookmark2", "bookmark3", "brightness", "contrast", "gamma",
        "title", "coverPage", "numPages", "number", "isBis", "count",
        "volume", "storyArc", "arcNumber", "arcCount", "genere",
        "writer", "penciller", "inker", "colorist", "letterer", "coverArtist",
        "date", "day", "month", "year", "publisher", "format", "color", "ageRating",
        "synopsis", "characters", "notes", "comicVineID", "lastTimeOpened",
        "coverSizeRatio", "originalCoverSize", "added", "type",
        "editor", "imprint", "teams", "locations", "series", "alternateSeries", "alternateNumber", "alternateCount",
        "languageISO", "seriesGroup", "mainCharacterOrTeam", "review", "tags",
        "imageFiltersJson", "lastTimeImageFiltersSet", "lastTimeCoverSet", "usesExternalCover", "lastTimeMetadataSet",
        "isFavorite"
    };

    const auto *metaObject = &ComicInfoObject::staticMetaObject;
    QSet<QString> properties;
    for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i)
        properties.insert(QString::fromLatin1(metaObject->property(i).name()));
    QCOMPARE(properties, expected);

    for (const auto *name : { "read", "rating", "isFavorite" }) {
        const auto property = metaObject->property(metaObject->indexOfProperty(name));
        QVERIFY2(property.isWritable(), name);
        QVERIFY2(property.hasNotifySignal(), name);
    }

    for (const auto *method : { "getWriters()", "getPencillers()", "getInkers()", "getColorists()", "getLetterers()",
                                "getCoverArtists()", "getEditors()", "getImprint()", "getCharacters()", "getTeams()",
                                "getLocations()", "getTags()", "getTypeString()", "getStoryArcInfoString()",
                                "getAlternateSeriesString()", "getTitleOrFileName()", "getFileSize()",
                                "getTitleIncludingNumber()", "getInfoTitle()" })
        QVERIFY2(metaObject->indexOfMethod(method) >= 0, method);
}

void ComicInfoObjectTest::readsTheComic()
{
    const auto comic = sampleComic();
    ComicInfoObject object(comic, true);

    QCOMPARE(object.property("id").toULongLong(), 42ULL);
    QCOMPARE(object.property("hash").toString(), QString("hash-42"));
    QCOMPARE(object.property("read").toBool(), true);
    QCOMPARE(object.property("rating").toInt(), 4);
    QCOMPARE(object.property("currentPage").toInt(), 7);
    QCOMPARE(object.property("title").toString(), QString("Title"));
    QCOMPARE(object.property("isFavorite").toBool(), true);
    QCOMPARE(object.getWriters(), QStringList({ "Writer A", "Writer B" }));
    QCOMPARE(object.getStoryArcInfoString(), comic.info.getStoryArcInfoString());
    QCOMPARE(object.getInfoTitle(), comic.getInfoTitle());
    QCOMPARE(object.getTitleIncludingNumber(), comic.getTitleIncludingNumber());

    ComicInfoObject empty;
    QCOMPARE(empty.property("isFavorite").toBool(), false);
    QCOMPARE(empty.property("id").toULongLong(), 0ULL);
}

void ComicInfoObjectTest::writablePropertiesNotifyOnce_data()
{
    QTest::addColumn<QByteArray>("property");
    QTest::addColumn<QVariant>("value");

    QTest::newRow("read") << QByteArray("read") << QVariant(false);
    QTest::newRow("rating") << QByteArray("rating") << QVariant(2);
    QTest::newRow("isFavorite") << QByteArray("isFavorite") << QVariant(false);
}

void ComicInfoObjectTest::writablePropertiesNotifyOnce()
{
    QFETCH(QByteArray, property);
    QFETCH(QVariant, value);

    ComicInfoObject object(sampleComic(), true);
    const auto metaProperty = object.metaObject()->property(object.metaObject()->indexOfProperty(property));
    QSignalSpy spy(&object, metaProperty.notifySignal());

    QVERIFY(object.setProperty(property, value));
    QCOMPARE(object.property(property), value);
    QCOMPARE(spy.count(), 1);

    QVERIFY(object.setProperty(property, value));
    QCOMPARE(spy.count(), 1);
}

void ComicInfoObjectTest::streamKeepsTheExistOnDbSlot_data()
{
    QTest::addColumn<bool>("loaded");

    QTest::newRow("loaded from the database") << true;
    QTest::newRow("not in the database") << false;
}

void ComicInfoObjectTest::streamKeepsTheExistOnDbSlot()
{
    QFETCH(bool, loaded);

    auto comic = sampleComic();
    if (!loaded) {
        comic.info = ComicInfo();
        comic.info.hash = "hash-42";
        comic.info.rating = 4;
        comic.info.currentPage = 7;
    }

    QByteArray data;
    {
        QDataStream out(&data, QIODevice::WriteOnly);
        out << comic.info;
    }

    QDataStream in(data);
    qulonglong id;
    bool read;
    bool edited;
    QString hash;
    bool existOnDb = false;
    bool hasBeenOpened;
    int rating;
    int currentPage;
    in >> id >> read >> edited >> hash >> existOnDb >> hasBeenOpened >> rating >> currentPage;
    QCOMPARE(in.status(), QDataStream::Ok);
    QCOMPARE(id, loaded ? 42ULL : 0ULL);
    QCOMPARE(hash, QString("hash-42"));
    QCOMPARE(existOnDb, loaded);
    QCOMPARE(rating, 4);
    QCOMPARE(currentPage, 7);
}

void ComicInfoObjectTest::streamRoundTrip()
{
    const auto comic = sampleComic();

    QByteArray data;
    {
        QDataStream out(&data, QIODevice::WriteOnly);
        out << comic;
    }

    ComicDB read;
    QDataStream in(data);
    in >> read;
    QCOMPARE(in.status(), QDataStream::Ok);
    QVERIFY(in.atEnd());
    QCOMPARE(read.id, comic.id);
    QCOMPARE(read.path, comic.path);
    QCOMPARE(read.info.id, comic.info.id);
    QCOMPARE(read.info.hash, comic.info.hash);
    QCOMPARE(read.info.rating, comic.info.rating);
    QCOMPARE(read.info.title, comic.info.title);
    QCOMPARE(read.info.writer, comic.info.writer);
    QCOMPARE(read.info.arcCount, comic.info.arcCount);
}

QTEST_GUILESS_MAIN(ComicInfoObjectTest)

#include "main.moc"
