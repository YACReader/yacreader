#ifndef COMICFULLINFOCONTROLLER_V2_H
#define COMICFULLINFOCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ComicFullinfoController_v2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ComicFullinfoController_v2)
public:
    ComicFullinfoController_v2();

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void serviceContent(const int &library, const qulonglong &comicId, stefanfrings::HttpResponse &response);
};

#endif // COMICFULLINFOCONTROLLER_V2_H
