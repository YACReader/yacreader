#ifndef COVERCONTROLLER_V2_H
#define COVERCONTROLLER_V2_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

class CoverControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(CoverControllerV2)
public:
    /** Constructor */
    CoverControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // COVERCONTROLLER_H
