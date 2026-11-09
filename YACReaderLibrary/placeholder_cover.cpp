#include "placeholder_cover.h"

#include "cover_utils.h"
#include "yacreader_global.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QUuid>

std::optional<QString> savePlaceholderCover(const QString &libraryDataPath, const QImage &image)
{
    const auto key = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const auto path = YACReader::LibraryPaths::placeholderCoverPathFromDataPath(libraryDataPath, key);
    if (image.isNull() || !QDir().mkpath(QFileInfo(path).absolutePath()) || !YACReader::saveCover(path, image)) {
        QFile::remove(path);
        return std::nullopt;
    }
    return key;
}
