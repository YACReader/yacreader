#ifndef COVERCONTROLLER_H
#define COVERCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class CoverController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(CoverController);

public:
    /** Constructor */
    CoverController();

    /** Generates the response */
    void service(HttpRequest &request, HttpResponse &response);
};

#endif // COVERCONTROLLER_H
