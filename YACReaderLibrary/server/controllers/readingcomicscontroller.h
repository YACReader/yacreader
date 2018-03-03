#ifndef READINGCOMICSCONTROLLER_H
#define READINGCOMICSCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class ReadingComicsController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(ReadingComicsController)
public:
    ReadingComicsController();

    void service(HttpRequest& request, HttpResponse& response);

private:
    void serviceContent(const int &library, HttpResponse &response);
};

#endif // READINGCOMICSCONTROLLER_H
