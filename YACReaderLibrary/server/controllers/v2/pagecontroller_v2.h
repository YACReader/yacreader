#ifndef PAGECONTROLLER_V2_H
#define PAGECONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class PageControllerV2 : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(PageControllerV2)
public:
    /** Constructor */
    PageControllerV2();

    /** Generates the response */
    void service(HttpRequest &request, HttpResponse &response) override;
};

#endif // PAGECONTROLLER_H
