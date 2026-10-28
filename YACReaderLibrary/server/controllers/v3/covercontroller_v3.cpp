#include "covercontroller_v3.h"

#include "cover_request.h"
#include "db_helper.h"
#include "v3_response.h"
#include "yacreader_libraries.h"

#include <QBuffer>
#include <QImage>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

CoverControllerV3::CoverControllerV3(int libraryId, const QString &fileName)
    : libraryId(libraryId), fileName(fileName)
{
}

void CoverControllerV3::service(HttpRequest & /* request */, HttpResponse &response)
{
    QImage image;
    if (const auto file = coverRequestFile(DBHelper::getLibraries().getPath(libraryId), fileName))
        image.load(*file);
    if (image.isNull())
        return V3Response::notFound(response);

    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG");
    response.setHeader("Content-Type", "image/jpeg");
    response.write(data, true);
}
