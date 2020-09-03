#include "versioncontroller.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

VersionController::VersionController() { }

void VersionController::service(HttpRequest &request, HttpResponse &response)
{
    Q_UNUSED(request);

    response.write(SERVER_VERSION_NUMBER, true);
}
