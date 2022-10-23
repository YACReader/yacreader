#ifndef STATUSPAGE_CONTROLLER
#define STATUSPAGE_CONTROLLER

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class StatusPageController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(StatusPageController);

public:
    StatusPageController();

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // STATUSPAGE_CONTROLLER
