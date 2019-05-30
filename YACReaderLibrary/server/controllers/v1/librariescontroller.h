#ifndef LIBRARIESCONTROLLER_H
#define LIBRARIESCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

/**
  This controller displays a HTML form and dumps the submitted input.
*/

class LibrariesController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(LibrariesController);

public:
    /** Constructor */
    LibrariesController();

    /** Generates the response */
    void service(HttpRequest &request, HttpResponse &response);
};

#endif // LIBRARIESCONTROLLER_H
