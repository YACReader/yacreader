#ifndef READINGLISTCONTENTCONTROLLER_H
#define READINGLISTCONTENTCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ReadingListContentController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(ReadingListContentController)
public:
    ReadingListContentController();

    void service(HttpRequest& request, HttpResponse& response);

private:
    void serviceContent(const int &library, const qulonglong &readingListId, HttpResponse &response);
};

#endif // READINGLISTCONTENTCONTROLLER_H
