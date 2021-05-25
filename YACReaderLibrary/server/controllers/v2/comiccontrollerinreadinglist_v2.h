#ifndef COMICCONTROLLERINREADINGLISTV2_H
#define COMICCONTROLLERINREADINGLISTV2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ComicControllerInReadingListV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ComicControllerInReadingListV2)
public:
    ComicControllerInReadingListV2();

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // COMICCONTROLLERINREADINGLISTV2_H
