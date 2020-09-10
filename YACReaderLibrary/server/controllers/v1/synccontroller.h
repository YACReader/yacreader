#ifndef SYNCCONTROLLER_H
#define SYNCCONTROLLER_H

#include <QObject>

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class SyncController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(SyncController)
public:
    /** Constructor */
    SyncController();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // SYNCCONTROLLER_H
