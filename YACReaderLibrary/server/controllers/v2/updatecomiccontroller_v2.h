#ifndef UPDATECOMICCONTROLLER_V2_H
#define UPDATECOMICCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class UpdateComicControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdateComicControllerV2)

public:
    UpdateComicControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // UPDATECOMICCONTROLLER_H
