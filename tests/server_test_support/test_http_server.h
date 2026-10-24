#ifndef TEST_HTTP_SERVER_H
#define TEST_HTTP_SERVER_H

#include <QByteArray>
#include <QList>
#include <QNetworkAccessManager>
#include <QObject>
#include <QSettings>
#include <QString>

#include <memory>
#include <utility>

namespace stefanfrings {
class HttpListener;
}

struct HttpAnswer {
    int status = 0;
    QString contentType;
    QByteArray body;
};

// the request mapper of the server on a local port, with its static files in rootPath/docroot
class TestHttpServer : public QObject
{
    Q_OBJECT
public:
    explicit TestHttpServer(const QString &rootPath);
    ~TestHttpServer() override;

    bool isListening() const;
    HttpAnswer request(const QByteArray &method, const QString &path, const QByteArray &body = { }, const QList<std::pair<QByteArray, QByteArray>> &headers = { });

private:
    std::unique_ptr<QSettings> docrootSettings;
    std::unique_ptr<QSettings> listenerSettings;
    stefanfrings::HttpListener *listener = nullptr;
    QNetworkAccessManager network;
};

// a new library database at libraryPath, the only library in the test settings; returns its id
int createTestLibrary(const QString &name, const QString &libraryPath);
bool execInLibrary(const QString &libraryPath, const QString &sql);

#endif // TEST_HTTP_SERVER_H
