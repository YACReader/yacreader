/**
  @file
  @author Stefan Frings
*/

#ifndef FORMCONTROLLER_H
#define FORMCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

/**
  This controller displays a HTML form and dumps the submitted input.
*/


class FormController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(FormController);
public:

    /** Constructor */
    FormController();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // FORMCONTROLLER_H
