#ifndef LIBRARIESCONTROLLER_V2_H
#define LIBRARIESCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

/**
  This controller displays a HTML form and dumps the submitted input.
*/

class LibrariesControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(LibrariesControllerV2)
public:
    /** Constructor */
    LibrariesControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // LIBRARIESCONTROLLER_H
