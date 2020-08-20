#ifndef READINGLISTINFOCONTROLLER_V2_H
#define READINGLISTINFOCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ReadingListInfoControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ReadingListInfoControllerV2)
public:
    ReadingListInfoControllerV2();

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void serviceComics(const int &library, const qulonglong &readingListId, stefanfrings::HttpResponse &response);
};

#endif // READINGLISTINFOCONTROLLER_V2_H
