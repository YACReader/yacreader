#include "covercontroller_v2.h"

#include "db_helper.h"
#include "missing_comic_server_helper.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <QFileInfo>
#include <QImage>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

CoverControllerV2::CoverControllerV2() { }

void CoverControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "image/jpeg");

    YACReaderLibraries libraries = DBHelper::getLibraries();

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    QString libraryName = DBHelper::getLibraryName(pathElements.at(3).toInt());
    QStringList remainingPathElements = pathElements.mid(5);
    QString fileName = remainingPathElements.join('/');

    QImage img(YACReader::LibraryPaths::coverPathWithFileName(libraries.getPath(libraryName), fileName));
    if (!img.isNull()) {
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "JPG");
        response.write(ba, true);
    } else {
        const auto hash = QFileInfo(fileName).completeBaseName();
        const auto id = MissingComicServerHelper::idFromHash(hash);
        if (id != 0) {
            const auto comic = MissingComicServerHelper::resolve(pathElements.at(3).toULongLong(), id);
            if (comic.info.existOnDb && comic.info.hash == hash) {
                response.write(MissingComicServerHelper::cover(), true);
                return;
            }
        }
        response.setStatus(404, "not found");
        response.write("404 not found", true);
    }
}
