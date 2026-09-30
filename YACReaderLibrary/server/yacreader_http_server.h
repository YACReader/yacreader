#ifndef YACREADER_HTTP_SERVER_H
#define YACREADER_HTTP_SERVER_H

#include <QObject>
#include <QString>
#include <QUuid>

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

    bool isRunning();

    QString getPort();
    QString errorString() const;

signals:
    void comicUpdated(qulonglong libraryId, qulonglong comicId);
    void libraryContentChanged(const QUuid &libraryId);

private:
    stefanfrings::HttpListener *listener;
};

#endif // YACREADER_HTTP_SERVER_H
