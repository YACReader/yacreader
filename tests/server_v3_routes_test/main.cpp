#include "static.h"
#include "test_http_server.h"
#include "yacreader_http_session_store.h"
#include "yacreader_libraries.h"

#include <QDir>
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

private:
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

QTEST_GUILESS_MAIN(ServerV3RoutesTest)

#include "main.moc"
