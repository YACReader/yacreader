#ifndef TAGCONTENTCONTROLLER_H
#define TAGCONTENTCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class TagContentController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(TagContentController);
public:
    /** Constructor */
    TagContentController();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);

private:
    void serviceContent(const int &library, const qulonglong &tagId, HttpResponse &response);
};

#endif // TAGCONTENTCONTROLLER_H
