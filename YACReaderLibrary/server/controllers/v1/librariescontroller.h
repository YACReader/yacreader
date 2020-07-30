#ifndef LIBRARIESCONTROLLER_H
#define LIBRARIESCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

/**
  This controller displays a HTML form and dumps the submitted input.
*/

class LibrariesController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(LibrariesController);

public:
    /** Constructor */
    LibrariesController();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // LIBRARIESCONTROLLER_H
