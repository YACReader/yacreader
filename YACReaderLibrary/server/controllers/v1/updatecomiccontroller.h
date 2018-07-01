#ifndef UPDATECOMICCONTROLLER_H
#define UPDATECOMICCONTROLLER_H


#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"


class UpdateComicController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdateComicController);

public:
    UpdateComicController();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // UPDATECOMICCONTROLLER_H
