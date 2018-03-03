#ifndef TAGSCONTROLLER_H
#define TAGSCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"



class TagsController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(TagsController)
public:

    /** Constructor */
    TagsController();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // TAGSCONTROLLER_H
