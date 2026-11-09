#include "cover_request.h"

#include "yacreader_global.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>

std::optional<QString> coverRequestFile(const QString &libraryPath, const QString &fileName)
{
    const auto covers = QDir::cleanPath(YACReader::LibraryPaths::libraryCoversFolderPath(libraryPath));
    const auto file = QDir::cleanPath(covers + '/' + fileName);
    if (!file.startsWith(covers + '/'))
        return std::nullopt;
    return file;
}

bool serveCover(const stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response, const QString &libraryPath, const QString &fileName)
{
    const auto path = coverRequestFile(libraryPath, fileName);
    if (!path)
        return false;

    const QFileInfo info(*path);
    if (!info.isFile())
        return false;

    const auto etag = '"' + QByteArray::number(info.lastModified().toMSecsSinceEpoch(), 16) + '-' + QByteArray::number(info.size(), 16) + '"';

    if (request.getHeader("If-None-Match").contains(etag)) {
        response.setStatus(304, "Not Modified");
        response.setHeader("ETag", etag);
        response.setHeader("Cache-Control", "no-cache");
        // write(data, true) always adds Content-Length: 0, and a 304 must not say the size of the cover is 0
        response.setHeader("Connection", "close");
        response.write(QByteArray(), false);
        response.write(QByteArray(), true);
        return true;
    }

    QFile file(*path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    response.setHeader("Content-Type", "image/jpeg");
    response.setHeader("ETag", etag);
    response.setHeader("Cache-Control", "no-cache");
    response.write(file.readAll(), true);
    return true;
}
