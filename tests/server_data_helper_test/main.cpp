#include "yacreader_server_data_helper.h"

#include <QTest>

class ServerDataHelperTest : public QObject
{
    Q_OBJECT
private slots:
    void folderJson_data();
    void folderJson();
    void folderTextFormat_data();
    void folderTextFormat();
    void deletedComicJson();
    void placeholderJson();
    void comicItemJson();
};

static Folder sampleFolder(qint32 numChildren)
{
    Folder folder;
    folder.id = FolderId(7);
    folder.parentId = FolderId(1);
    folder.name = "Series";
    folder.path = "/Series";
    folder.numChildren = numChildren;
    folder.firstChildHash = "hash-1";
    folder.added = 10;
    folder.updated = 20;
    return folder;
}

void ServerDataHelperTest::folderJson_data()
{
    QTest::addColumn<qint32>("numChildren");
    QTest::addColumn<int>("sent");

    QTest::newRow("known") << qint32(3) << 3;
    QTest::newRow("empty") << qint32(0) << 0;
    QTest::newRow("stored unknown") << qint32(-1) << -1;
}

void ServerDataHelperTest::folderJson()
{
    QFETCH(qint32, numChildren);
    QFETCH(int, sent);

    const auto json = YACReaderServerDataHelper::folderToJSON(2, QUuid(), sampleFolder(numChildren));
    QCOMPARE(json.value("id").toString(), QString("7"));
    QCOMPARE(json.value("parent_id").toString(), QString("1"));
    QCOMPARE(json.value("folder_name").toString(), QString("Series"));
    QCOMPARE(json.value("path").toString(), QString("/Series"));
    QCOMPARE(json.value("num_children").toInt(), sent);
    QVERIFY(json.value("num_children").isDouble());
    QCOMPARE(json.value("first_comic_hash").toString(), QString("hash-1"));
    QCOMPARE(json.value("added").toInteger(), 10);
    QCOMPARE(json.value("updated").toInteger(), 20);
}

void ServerDataHelperTest::folderTextFormat_data()
{
    folderJson_data();
}

void ServerDataHelperTest::folderTextFormat()
{
    QFETCH(qint32, numChildren);
    QFETCH(int, sent);

    const auto text = YACReaderServerDataHelper::folderToYSFormat(2, sampleFolder(numChildren));
    QCOMPARE(text, QString("f\x1F\t2\x1F\t7\x1F\tSeries\x1F\t%1\x1F\thash-1\x1E\r\n").arg(sent));
}

static ComicInfo sampleInfo()
{
    ComicInfo info;
    info.id = ComicInfoId(42);
    info.hash = "hash-42";
    info.currentPage = 7;
    info.numPages = 20;
    info.read = false;
    info.hasBeenOpened = true;
    info.lastTimeOpened = 5000;
    info.type = 1;
    info.coverSizeRatio = 0.75;
    info.title = "Title";
    info.series = "Series";
    info.volume = "V1";
    info.number = "4";
    return info;
}

static Placeholder samplePlaceholder(std::optional<QString> coverKey)
{
    Placeholder placeholder;
    placeholder.id = PlaceholderId(9);
    placeholder.series = "Series";
    placeholder.number = "5";
    placeholder.coverKey = coverKey;
    return placeholder;
}

static ListEntry sampleEntry(ListKind kind)
{
    ListEntry entry;
    entry.id = { kind, 30 };
    entry.listId = 3;
    entry.lastKnownFile = LastKnownFile { "c4.cbz", "/Series/c4.cbz" };
    return entry;
}

void ServerDataHelperTest::deletedComicJson()
{
    const auto json = YACReaderServerDataHelper::deletedComicToJSON(2, QUuid(), sampleInfo());
    QCOMPARE(json.value("type").toString(), QString("deleted_comic"));
    QCOMPARE(json.value("comic_info_id").toString(), QString("42"));
    QCOMPARE(json.value("library_id").toString(), QString("2"));
    QCOMPARE(json.value("hash").toString(), QString("hash-42"));
    QCOMPARE(json.value("current_page").toInt(), 7);
    QCOMPARE(json.value("num_pages").toInt(), 20);
    QCOMPARE(json.value("read").toBool(), false);
    QCOMPARE(json.value("has_been_opened").toBool(), true);
    QCOMPARE(json.value("last_time_opened").toInteger(), 5000);
    QCOMPARE(json.value("file_type").toInt(), 1);
    QCOMPARE(json.value("title").toString(), QString("Title"));
    QCOMPARE(json.value("series").toString(), QString("Series"));
    QCOMPARE(json.value("volume").toString(), QString("V1"));
    QCOMPARE(json.value("universal_number").toString(), QString("4"));
    // nothing that only a file has
    for (const auto &field : { "id", "file_name", "file_size", "path", "parent_id", "library_uuid" })
        QVERIFY2(!json.contains(field), field);
}

void ServerDataHelperTest::placeholderJson()
{
    auto json = YACReaderServerDataHelper::placeholderToJSON(2, QUuid(), samplePlaceholder("00000000-0000-0000-0000-000000000001"));
    QCOMPARE(json.value("type").toString(), QString("placeholder"));
    QCOMPARE(json.value("placeholder_id").toString(), QString("9"));
    QCOMPARE(json.value("series").toString(), QString("Series"));
    QCOMPARE(json.value("universal_number").toString(), QString("5"));
    QCOMPARE(json.value("cover").toString(), QString("/placeholders/00000000-0000-0000-0000-000000000001.jpg"));
    QVERIFY(!json.contains("title"));
    QVERIFY(!json.contains("hash"));

    json = YACReaderServerDataHelper::placeholderToJSON(2, QUuid(), samplePlaceholder(std::nullopt));
    QVERIFY(!json.contains("cover"));
}

void ServerDataHelperTest::comicItemJson()
{
    ComicDB comic;
    comic.file.id = ComicId(100);
    comic.file.fileName = "c4.cbz";
    comic.info = sampleInfo();

    auto json = YACReaderServerDataHelper::comicItemToJSON(2, QUuid(), ComicItemCase::Comic { comic });
    QCOMPARE(json, YACReaderServerDataHelper::comicToJSON(2, QUuid(), comic));

    json = YACReaderServerDataHelper::comicItemToJSON(2, QUuid(), ComicItemCase::ComicInList { comic, sampleEntry(ListKind::ReadingList) });
    auto expected = YACReaderServerDataHelper::comicToJSON(2, QUuid(), comic);
    expected["entry_id"] = "30";
    expected["reading_list_id"] = "3";
    QCOMPARE(json, expected);

    // label entries have no reading list
    json = YACReaderServerDataHelper::comicItemToJSON(2, QUuid(), ComicItemCase::ComicInList { comic, sampleEntry(ListKind::Label) });
    expected.remove("reading_list_id");
    QCOMPARE(json, expected);

    json = YACReaderServerDataHelper::comicItemToJSON(2, QUuid(), ComicItemCase::DeletedComicInList { sampleInfo(), sampleEntry(ListKind::ReadingList) });
    expected = YACReaderServerDataHelper::deletedComicToJSON(2, QUuid(), sampleInfo());
    expected["entry_id"] = "30";
    expected["reading_list_id"] = "3";
    expected["last_known_name"] = "c4.cbz";
    QCOMPARE(json, expected);

    json = YACReaderServerDataHelper::comicItemToJSON(2, QUuid(), ComicItemCase::PlaceholderInList { samplePlaceholder(std::nullopt), sampleEntry(ListKind::Label) });
    expected = YACReaderServerDataHelper::placeholderToJSON(2, QUuid(), samplePlaceholder(std::nullopt));
    expected["entry_id"] = "30";
    QCOMPARE(json, expected);

    json = YACReaderServerDataHelper::comicItemToJSON(2, QUuid(), ComicItemCase::PlaceholderInFolder { samplePlaceholder(std::nullopt), FolderId(8) });
    expected = YACReaderServerDataHelper::placeholderToJSON(2, QUuid(), samplePlaceholder(std::nullopt));
    expected["parent_id"] = "8";
    QCOMPARE(json, expected);
}

QTEST_GUILESS_MAIN(ServerDataHelperTest)

#include "main.moc"
