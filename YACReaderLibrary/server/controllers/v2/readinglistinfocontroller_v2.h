#ifndef READINGLISTINFOCONTROLLER_V2_H
#define READINGLISTINFOCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ReadingListInfoControllerV2 : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ReadingListInfoControllerV2)
public:
    ReadingListInfoControllerV2();

    void service(HttpRequest &request, HttpResponse &response);

private:
    void serviceComics(const int &library, const qulonglong &readingListId, HttpResponse &response);
};

#endif // READINGLISTINFOCONTROLLER_V2_H
