#ifndef ERRORCONTROLLER_H
#define ERRORCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ErrorController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ErrorController);

public:
    /** Constructor */
    ErrorController(int errorCode);

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int error;
};

#endif // ERRORCONTROLLER_H
