#ifndef READINGLISTCONTENTCONTROLLER_V2_H
#define READINGLISTCONTENTCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ReadingListContentControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ReadingListContentControllerV2)
public:
    ReadingListContentControllerV2();

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void serviceContent(const int &library, const qulonglong &readingListId, stefanfrings::HttpResponse &response);
};

#endif // READINGLISTCONTENTCONTROLLER_H
