/**
  @file
  @author Stefan Frings
*/

#ifndef SESSIONCONTROLLER_H
#define SESSIONCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

/**
  This controller demonstrates how to use sessions.
*/

class SessionController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(SessionController);
public:

    /** Constructor */
    SessionController();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // SESSIONCONTROLLER_H
