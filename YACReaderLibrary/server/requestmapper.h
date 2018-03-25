/**
  @file
  @author Stefan Frings
*/

#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include "httprequesthandler.h"



class RequestMapper : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(RequestMapper)
public:
    RequestMapper(QObject* parent=0);

    void service(HttpRequest& request, HttpResponse& response);
    void loadSession(HttpRequest & request, HttpResponse& response);

private:
    void serviceV1(HttpRequest& request, HttpResponse& response);
    void serviceV2(HttpRequest& request, HttpResponse& response);
};

#endif // REQUESTMAPPER_H
