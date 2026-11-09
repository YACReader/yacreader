#include "data_base_management.h"
#include "libraries_update_coordinator.h"
#include "static.h"
#include "test_http_server.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

class LibraryMaintenanceHttpTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void staleLockNeedsItsCurrentToken();
    void recoveryReleasesStaleLock();
    void changedOwnerIsPreserved();
    void liveOwnerCannotBeRecovered();
    void xmlRescanProblemKeepsItsOperation();

private:
    void writeLock(const QByteArray &contents);
    void startUpdate();
    QJsonArray problems();
    int recover(const QString &token);
    QString endpoint;
    QString recoveryEndpoint;
    QString lockPath;
    QString registryPath;
    QString oldName;
    QString oldOrganization;
    bool oldTestMode;
    std::unique_ptr<QTemporaryDir> root;
    std::unique_ptr<YACReaderLibraries> libraries;
    std::unique_ptr<QSettings> settings;
    std::unique_ptr<LibrariesUpdateCoordinator> coordinator;
    std::unique_ptr<TestHttpServer> server;
};

void LibraryMaintenanceHttpTest::init()
{
    oldName = QCoreApplication::applicationName();
    oldOrganization = QCoreApplication::organizationName();
    oldTestMode = QStandardPaths::isTestModeEnabled();
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("YACReaderTests");
    QCoreApplication::setApplicationName(QUuid::createUuid().toString(QUuid::WithoutBraces));
    registryPath = YACReader::getSettingsPath() + "/" + QCoreApplication::applicationName() + ".ini";
    QVERIFY(QDir().mkpath(YACReader::getSettingsPath()));
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    const auto libraryPath = root->filePath("library");
    const auto id = createTestLibrary("Library", libraryPath);
    QVERIFY(id >= 0);
    endpoint = QString("/v2/library/%1/").arg(id);
    recoveryEndpoint = QString("/v3/library/%1/maintenance/recover").arg(id);
    lockPath = QDir(YACReader::LibraryPaths::libraryDataPath(libraryPath)).filePath("maintenance.lock");
    libraries = std::make_unique<YACReaderLibraries>();
    settings = std::make_unique<QSettings>(root->filePath("settings.ini"), QSettings::IniFormat);
    coordinator = std::make_unique<LibrariesUpdateCoordinator>(settings.get(), *libraries, [] { return true; });
    Static::librariesUpdateCoordinator = coordinator.get();
    server = std::make_unique<TestHttpServer>(root->filePath("http"));
    QVERIFY(server->isListening());
}

void LibraryMaintenanceHttpTest::cleanup()
{
    server.reset();
    Static::librariesUpdateCoordinator = nullptr;
    coordinator.reset();
    settings.reset();
    libraries.reset();
    root.reset();
    QFile::remove(registryPath);
    QCoreApplication::setApplicationName(oldName);
    QCoreApplication::setOrganizationName(oldOrganization);
    QStandardPaths::setTestModeEnabled(oldTestMode);
}

void LibraryMaintenanceHttpTest::writeLock(const QByteArray &contents)
{
    QFile file(lockPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(file.write(contents), contents.size());
}

void LibraryMaintenanceHttpTest::startUpdate()
{
    QCOMPARE(server->request("POST", endpoint + "update").status, 202);
    QTRY_VERIFY_WITH_TIMEOUT(!coordinator->isRunning(), 30000);
}

QJsonArray LibraryMaintenanceHttpTest::problems()
{
    const auto response = server->request("GET", "/v3/libraries/maintenance");
    return QJsonDocument::fromJson(response.body).object().value("problems").toArray();
}

int LibraryMaintenanceHttpTest::recover(const QString &token)
{
    return server->request("POST", recoveryEndpoint, QJsonDocument(QJsonObject { { "recovery_token", token } }).toJson()).status;
}

void LibraryMaintenanceHttpTest::staleLockNeedsItsCurrentToken()
{
    writeLock("123\nold-app\nold-host\nold-machine\n");
    startUpdate();
    QCOMPARE(problems().size(), 1);
    const auto problem = problems().first().toObject();
    QCOMPARE(problem.value("state").toString(), QString("stale"));
    QCOMPARE(problem.value("operation").toString(), QString("update"));
    QVERIFY(problem.value("owner").toString().contains("old-host"));
    QVERIFY(!problem.value("recovery_token").toString().isEmpty());
    QCOMPARE(server->request("GET", recoveryEndpoint).status, 405);
    QCOMPARE(server->request("POST", recoveryEndpoint).status, 400);
    QCOMPARE(recover("invalid-token"), 409);
    QVERIFY(QFile::exists(lockPath));
    QCOMPARE(problems().size(), 1);
}

void LibraryMaintenanceHttpTest::recoveryReleasesStaleLock()
{
    writeLock("123\nold-app\nold-host\nold-machine\n");
    startUpdate();
    const auto token = problems().first().toObject().value("recovery_token").toString();
    QCOMPARE(recover(token), 200);
    QVERIFY(!QFile::exists(lockPath));
    QVERIFY(problems().isEmpty());
    startUpdate();
    QVERIFY(problems().isEmpty());
}

void LibraryMaintenanceHttpTest::changedOwnerIsPreserved()
{
    writeLock("123\nold-app\nold-host\nold-machine\n");
    startUpdate();
    const auto token = problems().first().toObject().value("recovery_token").toString();
    const QByteArray successor("456\nnew-app\nnew-host\nnew-machine\n");
    writeLock(successor);
    QCOMPARE(recover(token), 409);
    QFile file(lockPath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), successor);
    file.close();

    // the rejection shows the new owner, so it can be reviewed and recovered
    const auto problem = problems().first().toObject();
    QVERIFY(problem.value("owner").toString().contains("new-host"));
    const auto successorToken = problem.value("recovery_token").toString();
    QVERIFY(!successorToken.isEmpty());
    QVERIFY(successorToken != token);
    QCOMPARE(recover(successorToken), 200);
    QVERIFY(!QFile::exists(lockPath));
}

void LibraryMaintenanceHttpTest::liveOwnerCannotBeRecovered()
{
    LibraryMaintenanceLock owner(root->filePath("library"));
    QVERIFY(owner.tryLock());
    startUpdate();
    const auto problem = problems().first().toObject();
    QCOMPARE(problem.value("state").toString(), QString("busy"));
    QVERIFY(!problem.contains("recovery_token"));
    LibraryMaintenanceLock inspector(root->filePath("library"));
    QVERIFY(!inspector.tryLock());
    QCOMPARE(recover(inspector.recoveryToken()), 409);
    QVERIFY(QFile::exists(lockPath));
}

void LibraryMaintenanceHttpTest::xmlRescanProblemKeepsItsOperation()
{
    const auto path = root->filePath("library");
    QVERIFY(execInLibrary(path, "INSERT INTO comic_info (id, hash, title) VALUES (1, 'missing-hash', 'Missing')"));
    QVERIFY(execInLibrary(path, "INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (1, 1, 1, 'missing.cbz', '/missing.cbz')"));
    writeLock("123\nold-app\nold-host\nold-machine\n");
    QCOMPARE(server->request("POST", endpoint + "rescan-xml").status, 202);
    QTRY_VERIFY_WITH_TIMEOUT(!coordinator->isRunning(), 30000);
    const auto problem = problems().first().toObject();
    QCOMPARE(problem.value("operation").toString(), QString("rescan-xml"));
    QCOMPARE(recover(problem.value("recovery_token").toString()), 200);
    QCOMPARE(server->request("POST", endpoint + "rescan-xml").status, 202);
    QTRY_VERIFY_WITH_TIMEOUT(!coordinator->isRunning(), 30000);
    QVERIFY(problems().isEmpty());
    QString connection;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(path));
        connection = db.connectionName();
        QSqlQuery query(db);
        QVERIFY(query.exec("SELECT COUNT(*) FROM comic WHERE fileName = 'missing.cbz'"));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);
    }
    QSqlDatabase::removeDatabase(connection);
}

QTEST_GUILESS_MAIN(LibraryMaintenanceHttpTest)
#include "http_test.moc"
