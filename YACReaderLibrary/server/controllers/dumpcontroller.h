/**
  @file
  @author Stefan Frings
*/

#ifndef DUMPCONTROLLER_H
#define DUMPCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

/**
  This controller dumps the received HTTP request in the response.
*/

class DumpController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(DumpController);
public:

    /** Constructor */
    DumpController();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // DUMPCONTROLLER_H
