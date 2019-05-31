/**
  @file
  @author Stefan Frings
*/

#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include "httprequesthandler.h"
#include <QMutex>

class RequestMapper : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(RequestMapper)
public:
    RequestMapper(QObject *parent = nullptr);

    void service(HttpRequest &request, HttpResponse &response);
    void loadSessionV1(HttpRequest &request, HttpResponse &response);
    void loadSessionV2(HttpRequest &request, HttpResponse &response);

private:
    void serviceV1(HttpRequest &request, HttpResponse &response);
    void serviceV2(HttpRequest &request, HttpResponse &response);

    static QMutex mutex;
};

#endif // REQUESTMAPPER_H
