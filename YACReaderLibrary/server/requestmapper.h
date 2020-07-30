/**
  @file
  @author Stefan Frings
*/

#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include "httprequesthandler.h"
#include <QMutex>

class RequestMapper : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(RequestMapper)
public:
    RequestMapper(QObject *parent = nullptr);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
    void loadSessionV1(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response);
    void loadSessionV2(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response);

private:
    void serviceV1(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response);
    void serviceV2(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response);

    static QMutex mutex;
};

#endif // REQUESTMAPPER_H
