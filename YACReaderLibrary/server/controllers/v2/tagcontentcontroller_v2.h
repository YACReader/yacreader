#ifndef TAGCONTENTCONTROLLER_V2_H
#define TAGCONTENTCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class TagContentControllerV2 : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(TagContentControllerV2)
public:
    /** Constructor */
    TagContentControllerV2();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);

private:
    void serviceContent(const int &library, const qulonglong &tagId, HttpResponse &response);
};

#endif // TAGCONTENTCONTROLLER_H
