#ifndef VERSIONCONTROLLER_H
#define VERSIONCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

#include <QThread>

class VersionController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(VersionController);

public:
    /** Constructor */
    VersionController();

    /** Generates the response */
    void service(HttpRequest &request, HttpResponse &response);
};

#endif // VERSIONCONTROLLER_H
