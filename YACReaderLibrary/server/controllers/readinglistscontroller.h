#ifndef READINGLISTSCONTROLLER_H
#define READINGLISTSCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ReadingListsController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(ReadingListsController)
public:
    ReadingListsController();

    void service(HttpRequest& request, HttpResponse& response);

private:
    void serviceContent(const int library, HttpResponse &response);
};

#endif // READINGLISTSCONTROLLER_H
