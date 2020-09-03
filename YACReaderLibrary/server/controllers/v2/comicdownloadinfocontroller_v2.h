#ifndef COMICDOWNLOADINFOCONTROLLER_V2_H
#define COMICDOWNLOADINFOCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ComicDownloadInfoControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ComicDownloadInfoControllerV2)
public:
    /** Constructor **/
    ComicDownloadInfoControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // COMICDOWNLOADINFOCONTROLLER_H
