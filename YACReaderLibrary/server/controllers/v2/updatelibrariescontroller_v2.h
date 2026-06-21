#ifndef UPDATELIBRARIESCONTROLLER_V2_H
#define UPDATELIBRARIESCONTROLLER_V2_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

/**
  Triggers and reports the status of library updates.

  Updates can take a long time, so the trigger endpoints return immediately
  (202 Accepted) and the actual work runs in the background through the
  application's LibrariesUpdateCoordinator. Clients poll the status endpoint to
  know when the update finished.

  Routes (dispatched by RequestMapper):
    - POST /v2/libraries/update          -> update all libraries
    - POST /v2/library/<id>/update       -> update a single library
    - GET  /v2/libraries/update/status   -> { "running": bool }
    - POST /v2/libraries/update/cancel   -> cancel a running update
*/

class UpdateLibrariesControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdateLibrariesControllerV2)
public:
    /** Constructor */
    UpdateLibrariesControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // UPDATELIBRARIESCONTROLLER_V2_H
