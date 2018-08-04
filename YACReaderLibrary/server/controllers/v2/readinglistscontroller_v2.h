#ifndef READINGLISTSCONTROLLER_V2_H
#define READINGLISTSCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ReadingListsControllerV2 : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(ReadingListsControllerV2)
public:
    ReadingListsControllerV2();

    void service(HttpRequest& request, HttpResponse& response);

private:
    void serviceContent(const int library, HttpResponse &response);
};

#endif // READINGLISTSCONTROLLER_H
