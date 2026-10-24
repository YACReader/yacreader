#include "data_base_management.h"
#include "httplistener.h"
#include "requestmapper.h"
#include "static.h"
#include "staticfilecontroller.h"
#include "yacreader_global.h"
#include "yacreader_http_session_store.h"
#include "yacreader_libraries.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

// the v2 routes are frozen: their answers must stay the ones of expected_v2_responses.json, which was recorded
// before v3 existed; set YACREADER_RECORD_V2_RESPONSES=1 to record it again
class ServerV2RoutesTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void answersDoNotChange();

private:
    bool exec(const QString &sql);
    void createLibrary();
    void saveCover(const QString &path, const QSize &size);
    QJsonObject request(const QByteArray &method, const QString &path, const QByteArray &body = { });

    std::unique_ptr<QTemporaryDir> root;
    QString libraryPath;
    int libraryId = -1;
    QString libraryUuid;
    std::unique_ptr<QSettings> listenerSettings;
    std::unique_ptr<QSettings> docrootSettings;
    std::unique_ptr<stefanfrings::HttpListener> listener;
    QNetworkAccessManager network;
};

static const QString libraryName = "Server routes";

void ServerV2RoutesTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    createLibrary();

    QVERIFY(QDir().mkpath(root->filePath("docroot")));
    docrootSettings = std::make_unique<QSettings>(root->filePath("docroot.ini"), QSettings::IniFormat);
    docrootSettings->setValue("path", root->filePath("docroot"));
    Static::staticFileController = new stefanfrings::StaticFileController(docrootSettings.get(), this);
    Static::yacreaderSessionStore = new YACReaderHttpSessionStore(this);

    listenerSettings = std::make_unique<QSettings>(root->filePath("listener.ini"), QSettings::IniFormat);
    listenerSettings->setValue("host", "127.0.0.1");
    listenerSettings->setValue("port", 0);
    listenerSettings->setValue("minThreads", 1);
    listenerSettings->setValue("maxThreads", 4);
    listener = std::make_unique<stefanfrings::HttpListener>(listenerSettings.get(), new RequestMapper(this), this);
    QVERIFY(listener->isListening());
}

void ServerV2RoutesTest::cleanupTestCase()
{
    listener.reset();
    YACReaderLibraries libraries;
    libraries.load();
    libraries.remove(libraryName);
    libraries.save();
    root.reset();
}

bool ServerV2RoutesTest::exec(const QString &sql)
{
    QString name;
    bool ok = false;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraryPath));
        name = db.connectionName();
        QSqlQuery query(db);
        ok = query.exec(sql);
        if (!ok)
            qWarning() << sql;
    }
    QSqlDatabase::removeDatabase(name);
    return ok;
}

void ServerV2RoutesTest::saveCover(const QString &path, const QSize &size)
{
    QVERIFY(QDir().mkpath(QFileInfo(path).absolutePath()));
    QImage image(size, QImage::Format_RGB32);
    image.fill(Qt::darkCyan);
    QVERIFY(image.save(path, "JPG"));
}

// every kind of content the v3 work touches: comics with and without a file, placeholders in folders and lists,
// child lists, folders of lists, smart lists, labels, Favorites, and Reading
void ServerV2RoutesTest::createLibrary()
{
    libraryPath = QDir::cleanPath(root->filePath("library"));
    QVERIFY(QDir().mkpath(YACReader::LibraryPaths::libraryDataPath(libraryPath)));
    QString name;
    {
        auto db = DataBaseManagement::createDatabase(YACReader::LibraryPaths::libraryDatabasePath(libraryPath));
        name = db.connectionName();
        db.close();
    }
    QSqlDatabase::removeDatabase(name);

    YACReaderLibraries libraries;
    libraries.load();
    for (const auto &oldName : libraries.getNames())
        libraries.remove(oldName);
    libraries.addLibrary(libraryName, libraryPath);
    QVERIFY(libraries.save());
    libraryId = libraries.getId(libraryName);
    libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId).toString();

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path, numChildren, firstChildHash, added, updated) VALUES "
                 "(2, 1, 'Series', '/Series', 4, '" +
                 QString(40, '2') + "2002', 100, 200), (3, 2, 'Arc', '/Series/Arc', 1, '" + QString(40, '5') + "5005', 101, 201)"));
    QVERIFY(exec("UPDATE folder SET customImage = '/folders/3.jpg', finished = 1, type = 1 WHERE id = 3"));
    // the hash ends with the file size
    QVERIFY(exec("INSERT INTO comic_info (id, hash, number, title, series, volume, read, currentPage, hasBeenOpened, lastTimeOpened, numPages, added, coverSizeRatio, originalCoverSize, type) VALUES "
                 "(1, '" +
                 QString(40, '1') + "1001', '10', 'Ten', 'Series', 'V1', 0, 3, 1, 5000, 20, 1001, 0.75, '600x800', 0), "
                                    "(2, '" +
                 QString(40, '2') + "1002', '2', 'Two', 'Series', 'V1', 1, 20, 1, 4000, 20, 1002, 0.75, '600x800', 0), "
                                    "(3, '" +
                 QString(40, '3') + "1003', NULL, NULL, NULL, NULL, 0, 1, 0, NULL, 20, 1003, 0.75, '600x800', 0), "
                                    "(4, '" +
                 QString(40, '4') + "1004', '4', 'Four', 'Series', 'V1', 0, 7, 1, 6000, 20, 1004, 0.75, '600x800', 1), "
                                    "(5, '" +
                 QString(40, '5') + "1005', '1', 'Arc One', 'Arc', NULL, 0, 0, 0, NULL, 20, 1005, 0.75, '600x800', 0)"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES "
                 "(1, 2, 1, 'c1.cbz', '/Series/c1.cbz'), (2, 2, 2, 'c2.cbz', '/Series/c2.cbz'), (3, 2, 3, 'c3.cbz', '/Series/c3.cbz'), "
                 "(4, 2, 4, 'c4.cbz', '/Series/c4.cbz'), (5, 3, 5, 'c5.cbz', '/Series/Arc/c5.cbz')"));
    // a second file of comic_info 1, and comic_info 4 loses its file but stays in the lists
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (6, 3, 1, 'copy.cbz', '/Series/Arc/copy.cbz')"));
    QVERIFY(exec("INSERT INTO comic_default_reading_list (comic_id, default_reading_list_id, ordering) VALUES (4, 1, 0), (2, 1, 1), (1, 1, 2)"));
    QVERIFY(exec("DELETE FROM comic WHERE id = 4"));

    QVERIFY(exec("INSERT INTO placeholder (id, series, volume, number, title, coverKey) VALUES "
                 "(1, 'Series', 'V1', '3', 'Three', '00000000-0000-0000-0000-000000000001'), (2, 'Series', 'V1', '11', 'Eleven', NULL)"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 1), (3, 2)"));

    QVERIFY(exec("INSERT INTO reading_list_folder (id, parent_id, name, ordering) VALUES (1, NULL, 'Events', 0), (2, 1, 'Old events', 0)"));
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name, folder_id) VALUES "
                 "(1, NULL, 0, 'Main list', NULL), (2, 1, 0, 'Child B', NULL), (3, 1, 1, 'Child A', NULL), "
                 "(4, NULL, 0, 'An event', 1), (5, NULL, 0, 'Old event', 2), (6, NULL, 0, 'Another list', NULL)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id, placeholder_id, last_known_name, last_known_path) VALUES "
                 "(1, 0, 2, NULL, NULL, NULL), (1, 1, NULL, 1, NULL, NULL), (1, 2, 4, NULL, 'c4.cbz', '/Series/c4.cbz'), (1, 3, 1, NULL, NULL, NULL), "
                 "(1, 4, 2, NULL, NULL, NULL), (2, 0, 5, NULL, NULL, NULL), (2, 1, NULL, 2, NULL, NULL), (3, 0, 3, NULL, NULL, NULL), "
                 "(4, 0, 1, NULL, NULL, NULL), (5, 0, 4, NULL, 'c4.cbz', '/Series/c4.cbz')"));
    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Tag', 'red', 2), (2, 'Empty', 'blue', 5)"));
    QVERIFY(exec("INSERT INTO label_entry (label_id, ordering, comic_info_id, placeholder_id) VALUES (1, 0, 4, NULL), (1, 1, 3, NULL), (1, 2, NULL, 1), (1, 3, 1, NULL)"));
    QVERIFY(exec("INSERT INTO smart_list_folder (id, parent_id, name, ordering) VALUES (1, NULL, 'Saved', 0)"));
    QVERIFY(exec("INSERT INTO smart_list (id, name, query, ordering, folder_id) VALUES (1, 'Unread', 'read:false', 0, NULL), (2, 'Series', 'series:Series', 0, 1)"));

    const auto dataPath = YACReader::LibraryPaths::libraryDataPath(libraryPath);
    saveCover(YACReader::LibraryPaths::coverPathFromLibraryDataPath(dataPath, QString(40, '1') + "1001"), { 60, 80 });
    saveCover(YACReader::LibraryPaths::coverPathFromLibraryDataPath(dataPath, QString(40, '4') + "1004"), { 64, 96 });
    saveCover(YACReader::LibraryPaths::customFolderCoverPathFromDataPath(dataPath, "3"), { 50, 50 });
    saveCover(YACReader::LibraryPaths::placeholderCoverPathFromDataPath(dataPath, "00000000-0000-0000-0000-000000000001"), { 30, 40 });
}

QJsonObject ServerV2RoutesTest::request(const QByteArray &method, const QString &path, const QByteArray &body)
{
    QNetworkRequest networkRequest(QUrl(QString("http://127.0.0.1:%1%2").arg(listener->serverPort()).arg(path)));
    if (!body.isEmpty())
        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto reply = network.sendCustomRequest(networkRequest, method, body);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QJsonObject answer;
    answer["status"] = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const auto contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    answer["content_type"] = contentType;
    const auto data = reply->readAll();
    // the encoder can change between Qt versions, the image cannot
    if (contentType == "image/jpeg" && answer["status"].toInt() == 200) {
        const auto image = QImage::fromData(data, "JPG");
        answer["body"] = QString("image %1x%2").arg(image.width()).arg(image.height());
    } else {
        answer["body"] = QString::fromUtf8(data).replace(libraryUuid, "{library_uuid}");
    }
    reply->deleteLater();
    return answer;
}

void ServerV2RoutesTest::answersDoNotChange()
{
    const auto library = QString("/v2/library/%1").arg(libraryId);
    const QList<std::pair<QByteArray, QString>> gets = {
        { "GET", "/v2/version" },
        { "GET", "/v2/libraries" },
        { "GET", library + "/folder/1/content" },
        { "GET", library + "/folder/2/content" },
        { "GET", library + "/folder/3/content" },
        { "GET", library + "/folder/1/info" },
        { "GET", library + "/folder/2/metadata" },
        { "GET", library + "/folder/99/metadata" },
        { "GET", library + "/comic/1/info" },
        { "GET", library + "/comic/1/fullinfo" },
        { "GET", library + "/comic/4/fullinfo" },
        { "GET", library + "/tags" },
        { "GET", library + "/tag/1/content" },
        { "GET", library + "/tag/1/info" },
        { "GET", library + "/tag/2/content" },
        { "GET", library + "/favs" },
        { "GET", library + "/reading" },
        { "GET", library + "/reading_lists" },
        { "GET", library + "/reading_list/1/content" },
        { "GET", library + "/reading_list/2/content" },
        { "GET", library + "/reading_list/4/content" },
        { "GET", library + "/reading_list/5/content" },
        { "GET", library + "/reading_list/1/info" },
        { "GET", library + "/cover/" + QString(40, '1') + "1001.jpg" },
        { "GET", library + "/cover/" + QString(40, '4') + "1004.jpg" },
        { "GET", library + "/cover/" + QString(40, '2') + "1002.jpg" },
        { "GET", library + "/cover/folders/3.jpg" },
        { "GET", library + "/cover/placeholders/00000000-0000-0000-0000-000000000001.jpg" },
        { "GET", library + "/unknown" },
        { "GET", "/v2/library/77/folder/1/content" },
    };
    const QList<QByteArray> searches = { R"({"query":"Series"})", R"({"query":"read:false"})", R"({"query":"Arc"})" };

    QJsonObject answers;
    for (const auto &[method, path] : gets)
        answers[method + " " + QString(path).replace(library, "/v2/library/{library}")] = request(method, path);
    for (const auto &search : searches)
        answers["POST /v2/library/{library}/search " + QString::fromUtf8(search)] = request("POST", library + "/search", search);

    QFile expectedFile(EXPECTED_RESPONSES_PATH);
    if (qEnvironmentVariableIsSet("YACREADER_RECORD_V2_RESPONSES")) {
        QVERIFY(expectedFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
        expectedFile.write(QJsonDocument(answers).toJson(QJsonDocument::Indented));
        QSKIP("v2 responses recorded");
    }
    QVERIFY(expectedFile.open(QIODevice::ReadOnly));
    const auto expected = QJsonDocument::fromJson(expectedFile.readAll()).object();
    QCOMPARE(answers.keys(), expected.keys());
    for (const auto &key : answers.keys()) {
        if (answers.value(key) != expected.value(key))
            qWarning().noquote() << key << "\nexpected:" << QJsonDocument(expected.value(key).toObject()).toJson() << "\nactual:" << QJsonDocument(answers.value(key).toObject()).toJson();
        QCOMPARE(answers.value(key), expected.value(key));
    }
}

QTEST_GUILESS_MAIN(ServerV2RoutesTest)

#include "main.moc"
