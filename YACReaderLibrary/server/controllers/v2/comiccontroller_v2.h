#ifndef COMICCONTROLLER_V2_H
#define COMICCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ComicControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ComicControllerV2)
public:
    /** Constructor */
    ComicControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // COMICCONTROLLER_H
