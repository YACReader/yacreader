#ifndef SYNCCONTROLLER_V2_H
#define SYNCCONTROLLER_V2_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

#include <QObject>

class SyncControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(SyncControllerV2)
public:
    /** Constructor */
    SyncControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // SYNCCONTROLLER_H
