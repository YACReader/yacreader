#ifndef YACREADER_HTTP_SERVER_H
#define YACREADER_HTTP_SERVER_H

#include <QObject>
#include <QString>

namespace stefanfrings {
class HttpListener;
}

class YACReaderHttpServer : public QObject
{
    Q_OBJECT
public:
    YACReaderHttpServer();

    void start(quint16 port = 0);
    void stop();

    QString getPort();

signals:
    void clientSync();
    void comicUpdated(qulonglong libraryId, qulonglong comicId);

private:
    stefanfrings::HttpListener *listener;
};

#endif // YACREADER_HTTP_SERVER_H
