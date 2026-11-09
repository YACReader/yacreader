#include "comic_db.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "descriptive_comic_metadata.h"
#include "table_projection.h"
#include "yacreader_global.h"

#include <QMap>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTest>

class DescriptiveMetadataTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void columnsMatchExistingSchema();
    void descriptiveRoundTripPreservesValuesAndComicState();
    void fullComicUpdateKeepsExistingBehavior();
    void comicInfoCopyKeepsDescriptiveFields();
    void comicInfoProjectionCoversTheTable();
    void patchWritesOnlyItsFields();
    void emptyPatchWritesNothing();
    void typeAndCoverWritesKeepOtherColumns();

private:
    QSqlDatabase db;

    ComicInfo loadInfo(qulonglong id);
};

ComicInfo DescriptiveMetadataTest::loadInfo(qulonglong id)
{
    QSqlQuery q(db);
    q.prepare("SELECT " + TableProjection::selectList("ci", ComicInfo::columns()) + " FROM comic_info ci WHERE ci.id = :id");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next())
        return { };
    return ComicInfo::fromRecord(q.record(), "ci");
}

void DescriptiveMetadataTest::init()
{
    db = QSqlDatabase::addDatabase("QSQLITE", "descriptive_metadata_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());
    QVERIFY(DataBaseManagement::createComicInfoTable(db, "comic_info"));
    QSqlQuery q(db);
    QVERIFY(q.exec("INSERT INTO comic_info (id,hash,title,numPages,currentPage,read,hasBeenOpened,rating,coverPage,coverSizeRatio,originalCoverSize,type,usesExternalCover,lastTimeCoverSet,imageFiltersJson) VALUES (1,'real-stored-hash','Original',20,7,0,1,4,3,0.75,'600x800',1,1,123,'{}')"));
}

void DescriptiveMetadataTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("descriptive_metadata_test");
}

void DescriptiveMetadataTest::columnsMatchExistingSchema()
{
    QSqlQuery q(db);
    QVERIFY(q.exec("PRAGMA table_info(comic_info)"));
    QMap<QString, QString> types;
    while (q.next())
        types.insert(q.value(1).toString(), q.value(2).toString());
    QCOMPARE(types.size(), 63);
    QCOMPARE(DescriptiveComicMetadata::columns().size(), size_t(37));
    QSet<QString> names;
    for (const auto &column : DescriptiveComicMetadata::columns()) {
        const auto name = QString::fromLatin1(column.name);
        QVERIFY(!names.contains(name));
        names.insert(name);
        QCOMPARE(types.value(name), QString::fromLatin1(column.sqlType));
    }
    for (const auto &name : { "hash", "id", "numPages", "coverPage", "read", "currentPage", "rating", "type", "usesExternalCover", "lastTimeMetadataSet" })
        QVERIFY(!names.contains(QString::fromLatin1(name)));
}

void DescriptiveMetadataTest::descriptiveRoundTripPreservesValuesAndComicState()
{
    QSqlQuery q(db);
    QVERIFY(q.exec("SELECT * FROM comic_info WHERE id=1"));
    QVERIFY(q.next());
    const auto before = q.record();
    DescriptiveComicMetadata metadata;
    for (const auto &column : DescriptiveComicMetadata::columns()) {
        const auto type = QByteArray(column.sqlType);
        metadata.*column.member = type == "TEXT" ? QVariant(QString("value:%1").arg(column.name)) : QVariant(0);
    }
    metadata.title = QVariant();
    metadata.notes = QString("");
    metadata.number = QString("-1.5AU");
    metadata.arcNumber = QString("01a");
    metadata.volume = QString("2024 edition");
    metadata.date = QString("//2024");
    metadata.writer = QString("First writer\nSecond writer");
    metadata.tags = QString("First tag\nSecond tag");
    metadata.isBis = false;
    metadata.color = false;
    QVERIFY(q.prepare("UPDATE comic_info SET " + DescriptiveComicMetadata::assignments().join(",") + " WHERE id=1"));
    metadata.bindTo(q);
    QVERIFY2(q.exec(), qPrintable(q.lastError().text()));
    QVERIFY(q.exec("SELECT * FROM comic_info WHERE id=1"));
    QVERIFY(q.next());
    const auto after = q.record();
    const DescriptiveComicMetadata loaded = loadInfo(1);
    for (const auto &column : DescriptiveComicMetadata::columns()) {
        const auto expected = metadata.*column.member;
        const auto actual = loaded.*column.member;
        QCOMPARE(actual.isNull(), expected.isNull());
        if (!expected.isNull()) {
            if (QByteArray(column.sqlType) == "BOOLEAN")
                QCOMPARE(actual.toBool(), expected.toBool());
            else
                QCOMPARE(actual.toString(), expected.toString());
        }
    }
    QSet<QString> descriptive;
    for (const auto &column : DescriptiveComicMetadata::columns())
        descriptive.insert(QString::fromLatin1(column.name));
    for (int i = 0; i < before.count(); ++i) {
        if (!descriptive.contains(before.fieldName(i)))
            QCOMPARE(after.value(i), before.value(i));
    }
    QVERIFY(loaded.title.isNull());
    QVERIFY(!loaded.notes.isNull());
    QCOMPARE(loaded.notes.toString(), QString(""));
    const auto comic = loadInfo(1);
    const DescriptiveComicMetadata &comicMetadata = comic;
    for (const auto &column : DescriptiveComicMetadata::columns())
        QCOMPARE(comicMetadata.*column.member, loaded.*column.member);
    QCOMPARE(comic.number.toString(), QString("-1.5AU"));
    QCOMPARE(comic.date.toString(), QString("//2024"));
    QCOMPARE(comic.currentPage, 7);
    QCOMPARE(comic.hash, QString("real-stored-hash"));
}

void DescriptiveMetadataTest::fullComicUpdateKeepsExistingBehavior()
{
    auto info = loadInfo(1);
    QCOMPARE(info.id, ComicInfoId(1));
    info.title = QString("");
    info.synopsis = QVariant();
    info.number = QString("001a");
    info.currentPage = 20;
    info.type = QVariant::fromValue(YACReader::FileType::WebComic);
    DBHelper::update(&info, db);
    const auto loaded = loadInfo(1);
    QVERIFY(!loaded.title.isNull());
    QVERIFY(loaded.synopsis.isNull());
    QCOMPARE(loaded.number.toString(), QString("001a"));
    QVERIFY(loaded.read);
    QVERIFY(loaded.hasBeenOpened);
    QCOMPARE(loaded.currentPage, 20);
    QCOMPARE(loaded.rating, 4);
    QCOMPARE(loaded.coverPage.toInt(), 3);
    QCOMPARE(loaded.originalCoverSize.toString(), QString("600x800"));
    QCOMPARE(loaded.usesExternalCover.toBool(), true);
    QCOMPARE(loaded.lastTimeCoverSet.toInt(), 123);
    QCOMPARE(loaded.type.toInt(), static_cast<int>(YACReader::FileType::WebComic));
    QCOMPARE(loaded.hash, QString("real-stored-hash"));
}

void DescriptiveMetadataTest::comicInfoCopyKeepsDescriptiveFields()
{
    ComicInfo info;
    info.hash = "keep-hash";
    info.currentPage = 9;
    info.title = QString("Title");
    info.number = QString("1/2");
    info.synopsis = QVariant();
    info.notes = QString("");
    info.color = false;
    info.tags = QString("First tag\nSecond tag");

    ComicInfo copy(info);
    ComicInfo assigned;
    assigned = info;

    for (const ComicInfo *result : { &copy, &assigned }) {
        for (const auto &column : DescriptiveComicMetadata::columns())
            QCOMPARE(result->*column.member, info.*column.member);
        QVERIFY(result->synopsis.isNull());
        QVERIFY(!result->notes.isNull());
        QCOMPARE(result->hash, QString("keep-hash"));
        QCOMPARE(result->currentPage, 9);
    }
}

void DescriptiveMetadataTest::comicInfoProjectionCoversTheTable()
{
    QSqlQuery q(db);
    QVERIFY(q.exec("PRAGMA table_info(comic_info)"));
    QSet<QString> schema;
    while (q.next())
        schema.insert(q.value(1).toString());

    QSet<QString> projected { "id", "manga" };
    for (const auto &column : ComicInfo::columns()) {
        QVERIFY2(!projected.contains(column), qPrintable(column));
        projected.insert(column);
    }
    QCOMPARE(projected, schema);

    const auto info = loadInfo(1);
    QCOMPARE(info.id, ComicInfoId(1));
    QCOMPARE(info.hash, QString("real-stored-hash"));
    QCOMPARE(info.title.toString(), QString("Original"));
    QVERIFY(info.number.isNull());
    QVERIFY(info.comicVineID.isNull());
    QCOMPARE(info.numPages.toInt(), 20);
    QCOMPARE(info.currentPage, 7);
    QCOMPARE(info.read, false);
    QCOMPARE(info.hasBeenOpened, true);
    QCOMPARE(info.rating, 4);
    QCOMPARE(info.coverPage.toInt(), 3);
    QCOMPARE(info.originalCoverSize.toString(), QString("600x800"));
    QCOMPARE(info.type.toInt(), 1);
    QCOMPARE(info.usesExternalCover.toBool(), true);
    QCOMPARE(info.lastTimeCoverSet.toInt(), 123);
    QCOMPARE(info.imageFiltersJson.toString(), QString("{}"));
}

void DescriptiveMetadataTest::patchWritesOnlyItsFields()
{
    QSqlQuery q(db);
    QVERIFY(q.exec("UPDATE comic_info SET synopsis = 'Old synopsis', writer = 'Old writer', edited = 0 WHERE id = 1"));
    QVERIFY(q.exec("SELECT * FROM comic_info WHERE id = 1"));
    QVERIFY(q.next());
    const auto before = q.record();

    DescriptiveMetadataPatch patch;
    patch.set(&DescriptiveComicMetadata::title, QString("New title"));
    patch.set(&DescriptiveComicMetadata::synopsis, QVariant());
    QVERIFY(patch.contains(&DescriptiveComicMetadata::title));
    QVERIFY(patch.contains(&DescriptiveComicMetadata::synopsis));
    QVERIFY(!patch.contains(&DescriptiveComicMetadata::writer));
    QCOMPARE(patch.assignments(), QStringList({ "title = :title", "synopsis = :synopsis" }));

    QVERIFY(DBHelper::updateComicInfoMetadata(ComicInfoId(1), patch, db));

    QVERIFY(q.exec("SELECT * FROM comic_info WHERE id = 1"));
    QVERIFY(q.next());
    const auto after = q.record();
    for (int i = 0; i < before.count(); ++i) {
        const auto name = before.fieldName(i);
        if (name == "title") {
            QCOMPARE(after.value(i).toString(), QString("New title"));
        } else if (name == "synopsis") {
            QVERIFY(after.value(i).isNull());
        } else if (name == "edited") {
            QCOMPARE(after.value(i).toInt(), 1);
        } else {
            QVERIFY2(after.value(i) == before.value(i), qPrintable(name));
        }
    }

    DescriptiveComicMetadata metadata;
    metadata.writer = QString("Kept writer");
    metadata.synopsis = QString("Replaced");
    patch.applyTo(metadata);
    QCOMPARE(metadata.title.toString(), QString("New title"));
    QVERIFY(metadata.synopsis.isNull());
    QCOMPARE(metadata.writer.toString(), QString("Kept writer"));
}

void DescriptiveMetadataTest::emptyPatchWritesNothing()
{
    QSqlQuery q(db);
    QVERIFY(q.exec("UPDATE comic_info SET edited = 0 WHERE id = 1"));

    const DescriptiveMetadataPatch patch;
    QVERIFY(patch.isEmpty());
    QVERIFY(DBHelper::updateComicInfoMetadata(ComicInfoId(1), patch, db));

    QVERIFY(q.exec("SELECT edited FROM comic_info WHERE id = 1"));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 0);
}

void DescriptiveMetadataTest::typeAndCoverWritesKeepOtherColumns()
{
    QSqlQuery q(db);
    QVERIFY(q.exec("UPDATE comic_info SET edited = 0 WHERE id = 1"));

    QVERIFY(DBHelper::updateComicInfoType(ComicInfoId(1), YACReader::FileType::Manga, db));
    auto loaded = loadInfo(1);
    QCOMPARE(loaded.type.toInt(), static_cast<int>(YACReader::FileType::Manga));
    QVERIFY(loaded.edited);
    QCOMPARE(loaded.title.toString(), QString("Original"));
    QCOMPARE(loaded.currentPage, 7);

    ComicInfo cover = loaded;
    cover.coverPage = 5;
    cover.originalCoverSize = QString("100x200");
    cover.coverSizeRatio = 0.5;
    cover.lastTimeCoverSet = 456;
    cover.usesExternalCover = false;
    cover.title = QString("Not written");
    cover.currentPage = 1;
    QVERIFY(q.exec("UPDATE comic_info SET edited = 0 WHERE id = 1"));
    QVERIFY(DBHelper::updateComicInfoCover(cover, db));

    loaded = loadInfo(1);
    QVERIFY(loaded.edited);
    QCOMPARE(loaded.coverPage.toInt(), 5);
    QCOMPARE(loaded.originalCoverSize.toString(), QString("100x200"));
    QCOMPARE(loaded.coverSizeRatio.toDouble(), 0.5);
    QCOMPARE(loaded.lastTimeCoverSet.toInt(), 456);
    QCOMPARE(loaded.usesExternalCover.toBool(), false);
    QCOMPARE(loaded.title.toString(), QString("Original"));
    QCOMPARE(loaded.currentPage, 7);
    QCOMPARE(loaded.numPages.toInt(), 20);
    QCOMPARE(loaded.hash, QString("real-stored-hash"));
}

QTEST_GUILESS_MAIN(DescriptiveMetadataTest)
#include "main.moc"
