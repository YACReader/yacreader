#include "static.h"
#include "test_http_server.h"
#include "yacreader_http_session_store.h"
#include "yacreader_libraries.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

class ServerV3RoutesTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void versionIsTheVersionOfV3();
    void versionAcceptsOnlyGet();
    void unknownRoutesAreNotFound();
    void v3UsesTheSessionOfV2();
    void folderContentHasTheOrderOfTheDesktop();
    void folderContentOfAMissingFolderIsNotFound();

private:
    bool exec(const QString &sql);
    QJsonArray items(const QString &path);
    std::unique_ptr<QTemporaryDir> root;
    QString libraryPath;
    int libraryId = -1;
    std::unique_ptr<TestHttpServer> server;
};

static const QString libraryName = "Server v3 routes";

void ServerV3RoutesTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    libraryPath = QDir::cleanPath(root->filePath("library"));
    libraryId = createTestLibrary(libraryName, libraryPath);
    QVERIFY(libraryId >= 0);

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (2, 1, 'Series', '/Series'), "
                 "(3, 2, 'A sub', '/Series/A sub'), (4, 2, 'b sub', '/Series/b sub'), (5, 2, 'a sub', '/Series/a sub')"));
    QVERIFY(exec("INSERT INTO comic_info (id, hash, number, title, numPages) VALUES "
                 "(1, 'hash-1', '10', 'Ten', 20), (2, 'hash-2', '2', 'Two', 20), (3, 'hash-3', NULL, NULL, 20), (4, 'hash-4', '1', 'Deleted', 20)"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES "
                 "(1, 2, 1, 'c1.cbz', '/Series/c1.cbz'), (2, 2, 2, 'c2.cbz', '/Series/c2.cbz'), (3, 2, 3, 'c3.cbz', '/Series/c3.cbz')"));
    QVERIFY(exec("INSERT INTO placeholder (id, series, number, coverKey) VALUES (1, 'Series', '3', 'key-1'), (2, 'Series', NULL, NULL)"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 1), (2, 2)"));

    server = std::make_unique<TestHttpServer>(root->path());
    QVERIFY(server->isListening());
}

void ServerV3RoutesTest::cleanupTestCase()
{
    server.reset();
    YACReaderLibraries libraries;
    libraries.load();
    libraries.remove(libraryName);
    libraries.save();
    root.reset();
}

void ServerV3RoutesTest::versionIsTheVersionOfV3()
{
    for (const auto &path : { QString("/v3/version"), QString("/v3/version/") }) {
        const auto answer = server->request("GET", path);
        QCOMPARE(answer.status, 200);
        QCOMPARE(answer.body, QByteArray("3.0"));
    }
    QCOMPARE(server->request("GET", "/v2/version").body, QByteArray("2.3"));
}

void ServerV3RoutesTest::versionAcceptsOnlyGet()
{
    for (const auto &method : { QByteArray("POST"), QByteArray("PATCH"), QByteArray("PUT"), QByteArray("DELETE") })
        QCOMPARE(server->request(method, "/v3/version").status, 405);
}

void ServerV3RoutesTest::unknownRoutesAreNotFound()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    // the routes that stay in v2 are not in v3
    for (const auto &path : { QString("/v3"), QString("/v3/"), QString("/v3/versions"), QString("/v3/libraries"), library + "/favs", library + "/reading",
                              library + "/comic/1/fullinfo", QString("/v3/library/77/folder/1/content") }) {
        const auto answer = server->request("GET", path);
        QCOMPARE(answer.status, 404);
        QCOMPARE(answer.body, QByteArray("404 not found"));
    }
}

void ServerV3RoutesTest::v3UsesTheSessionOfV2()
{
    const QByteArray token = "v3-session-token";
    QVERIFY(Static::yacreaderSessionStore->getYACReaderSessionHttpSession(token) == nullptr);
    QCOMPARE(server->request("GET", "/v3/version", { }, { { "x-request-id", token } }).status, 200);
    const auto session = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(token);
    QVERIFY(session != nullptr);

    QCOMPARE(server->request("GET", "/v2/version", { }, { { "x-request-id", token } }).status, 200);
    QCOMPARE(Static::yacreaderSessionStore->getYACReaderSessionHttpSession(token), session);
}

bool ServerV3RoutesTest::exec(const QString &sql)
{
    return execInLibrary(libraryPath, sql);
}

QJsonArray ServerV3RoutesTest::items(const QString &path)
{
    const auto answer = server->request("GET", path);
    if (answer.status != 200 || answer.contentType != "application/json")
        qWarning() << path << answer.status << answer.contentType;
    return QJsonDocument::fromJson(answer.body).array();
}

static QStringList itemKeys(const QJsonArray &items)
{
    QStringList keys;
    for (const auto &item : items) {
        const auto object = item.toObject();
        const auto type = object.value("type").toString();
        keys.append(type + " " + object.value(type == "placeholder" ? "placeholder_id" : "id").toString());
    }
    return keys;
}

// subfolders by name, as the folders tree; then comics and placeholders by number, numbered first, then file name
void ServerV3RoutesTest::folderContentHasTheOrderOfTheDesktop()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    const auto content = items(library + "/folder/2/content");
    QCOMPARE(itemKeys(content), QStringList({ "folder 3", "folder 5", "folder 4", "comic 2", "placeholder 1", "comic 1", "placeholder 2", "comic 3" }));

    const auto placeholder = content.at(4).toObject();
    QCOMPARE(placeholder.value("parent_id").toString(), QString("2"));
    QCOMPARE(placeholder.value("cover").toString(), QString("/placeholders/key-1.jpg"));
    QCOMPARE(placeholder.value("library_id").toString(), QString::number(libraryId));
    QVERIFY(!content.at(6).toObject().contains("cover"));
    QCOMPARE(content.at(3).toObject().value("file_name").toString(), QString("c2.cbz"));

    QCOMPARE(itemKeys(items(library + "/folder/1/content")), QStringList({ "folder 2" }));
    QCOMPARE(itemKeys(items(library + "/folder/4/content")), QStringList());
    QCOMPARE(server->request("POST", library + "/folder/2/content").status, 405);
}

void ServerV3RoutesTest::folderContentOfAMissingFolderIsNotFound()
{
    QCOMPARE(server->request("GET", QString("/v3/library/%1/folder/99/content").arg(libraryId)).status, 404);
    QCOMPARE(server->request("GET", "/v3/library/77/folder/2/content").status, 404);
}

QTEST_GUILESS_MAIN(ServerV3RoutesTest)

#include "main.moc"
