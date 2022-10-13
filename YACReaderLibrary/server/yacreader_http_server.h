#ifndef YACREADER_HTTP_SERVER_H
#define YACREADER_HTTP_SERVER_H

#include <QString>

namespace stefanfrings {
class HttpListener;
}

class YACReaderHttpServer
{
private:
    stefanfrings::HttpListener *listener;

public:
    YACReaderHttpServer();

    void start(quint16 port = 0);
    void stop();

    QString getPort();

protected:
};

#endif // YACREADER_HTTP_SERVER_H
