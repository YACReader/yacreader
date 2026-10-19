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

QTEST_GUILESS_MAIN(ServerDataHelperTest)

#include "main.moc"
