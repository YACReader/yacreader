#ifndef UPDATECOMICCONTROLLER_H
#define UPDATECOMICCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class UpdateComicController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdateComicController);

public:
    UpdateComicController();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // UPDATECOMICCONTROLLER_H
