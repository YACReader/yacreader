#include "cover_request.h"

#include "yacreader_global.h"

#include <QDir>

std::optional<QString> coverRequestFile(const QString &libraryPath, const QString &fileName)
{
    const auto covers = QDir::cleanPath(YACReader::LibraryPaths::libraryCoversFolderPath(libraryPath));
    const auto file = QDir::cleanPath(covers + '/' + fileName);
    if (!file.startsWith(covers + '/'))
        return std::nullopt;
    return file;
}
