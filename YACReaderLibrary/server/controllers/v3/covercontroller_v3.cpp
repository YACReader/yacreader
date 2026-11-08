#include "covercontroller_v3.h"

#include "cover_request.h"
#include "db_helper.h"
#include "v3_response.h"
#include "yacreader_libraries.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

CoverControllerV3::CoverControllerV3(int libraryId, const QString &fileName)
    : libraryId(libraryId), fileName(fileName)
{
}

void CoverControllerV3::service(HttpRequest &request, HttpResponse &response)
{
    if (!serveCover(request, response, DBHelper::getLibraries().getPath(libraryId), fileName))
        V3Response::notFound(response);
}
