#include "versioncontroller.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

VersionController::VersionController(const QByteArray &version)
    : version(version) { }

void VersionController::service(HttpRequest &request, HttpResponse &response)
{
    Q_UNUSED(request);

    response.write(version, true);
}
