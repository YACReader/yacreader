#ifndef READINGCOMICSCONTROLLER_V2_H
#define READINGCOMICSCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ReadingComicsControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ReadingComicsControllerV2)
public:
    ReadingComicsControllerV2();

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void serviceContent(const int &library, stefanfrings::HttpResponse &response);
};

#endif // READINGCOMICSCONTROLLER_H
