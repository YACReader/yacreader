#include "test_http_server.h"

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
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSqlQuery>

TestHttpServer::TestHttpServer(const QString &rootPath)
{
    const QDir root(rootPath);
    QDir().mkpath(root.filePath("docroot"));
    docrootSettings = std::make_unique<QSettings>(root.filePath("docroot.ini"), QSettings::IniFormat);
    docrootSettings->setValue("path", root.filePath("docroot"));
    Static::staticFileController = new stefanfrings::StaticFileController(docrootSettings.get(), this);
    Static::yacreaderSessionStore = new YACReaderHttpSessionStore(this);

    listenerSettings = std::make_unique<QSettings>(root.filePath("listener.ini"), QSettings::IniFormat);
    listenerSettings->setValue("host", "127.0.0.1");
    listenerSettings->setValue("port", 0);
    listenerSettings->setValue("minThreads", 1);
    listenerSettings->setValue("maxThreads", 4);
    listener = new stefanfrings::HttpListener(listenerSettings.get(), new RequestMapper(this), this);
}

TestHttpServer::~TestHttpServer()
{
    delete listener;
    Static::staticFileController = nullptr;
    Static::yacreaderSessionStore = nullptr;
}

bool TestHttpServer::isListening() const
{
    return listener->isListening();
}

HttpAnswer TestHttpServer::request(const QByteArray &method, const QString &path, const QByteArray &body, const QList<std::pair<QByteArray, QByteArray>> &headers)
{
    QNetworkRequest networkRequest(QUrl(QString("http://127.0.0.1:%1%2").arg(listener->serverPort()).arg(path)));
    if (!body.isEmpty())
        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    for (const auto &[name, value] : headers)
        networkRequest.setRawHeader(name, value);
    auto reply = network.sendCustomRequest(networkRequest, method, body);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    HttpAnswer answer;
    answer.status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    answer.contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    answer.body = reply->readAll();
    reply->deleteLater();
    return answer;
}

int createTestLibrary(const QString &name, const QString &libraryPath)
{
    if (!QDir().mkpath(YACReader::LibraryPaths::libraryDataPath(libraryPath)))
        return -1;
    QString connectionName;
    {
        auto db = DataBaseManagement::createDatabase(YACReader::LibraryPaths::libraryDatabasePath(libraryPath));
        connectionName = db.connectionName();
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    YACReaderLibraries libraries;
    libraries.load();
    for (const auto &oldName : libraries.getNames())
        libraries.remove(oldName);
    libraries.addLibrary(name, libraryPath);
    if (!libraries.save())
        return -1;
    return libraries.getId(name);
}

bool execInLibrary(const QString &libraryPath, const QString &sql)
{
    QString connectionName;
    bool ok = false;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraryPath));
        connectionName = db.connectionName();
        QSqlQuery query(db);
        ok = query.exec(sql);
        if (!ok)
            qWarning() << sql;
    }
    QSqlDatabase::removeDatabase(connectionName);
    return ok;
}
