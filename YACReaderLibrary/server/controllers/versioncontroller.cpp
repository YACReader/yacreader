#include "versioncontroller.h"

VersionController::VersionController() { }

void VersionController::service(HttpRequest &request, HttpResponse &response)
{
    Q_UNUSED(request);

    response.write(SERVER_VERSION_NUMBER, true);
}
