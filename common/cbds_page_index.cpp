#include "cbds_page_index.h"

#include "qnaturalsorting.h"

#include <QFileInfo>

#include <algorithm>

namespace {

const QString bookIniName = QStringLiteral("ComicBookDS_book.ini");
const QString pagesFolder = QStringLiteral("IMAGE");

// Leading separators are ignored, the names are relative to the archive root
// anyway. Directory entries (trailing separator) have no parts so they never match.
QStringList archivePathParts(QString name)
{
    name.replace('\\', '/');
    if (name.endsWith('/')) {
        return { };
    }
    return name.split('/', Qt::SkipEmptyParts);
}

bool isBookIni(const QString &fileName)
{
    const QStringList parts = archivePathParts(fileName);
    return parts.size() == 1 && parts.first().compare(bookIniName, Qt::CaseInsensitive) == 0;
}

// Only files directly inside the top level IMAGE folder are pages, nested folders
// and entries trying to leave the archive root never match.
bool isPageEntry(const QString &fileName)
{
    const QStringList parts = archivePathParts(fileName);
    return parts.size() == 2 && parts.first().compare(pagesFolder, Qt::CaseInsensitive) == 0 && parts.last() != QStringLiteral("..") && parts.last() != QStringLiteral(".");
}

}

bool YACReaderCbds::isCbds(const QString &path, const QStringList &fileNames)
{
    if (QFileInfo(path).suffix().compare(QStringLiteral("cbds"), Qt::CaseInsensitive) == 0) {
        return true;
    }

    return std::any_of(fileNames.cbegin(), fileNames.cend(), isBookIni);
}

QStringList YACReaderCbds::pageCandidates(const QString &path, const QStringList &fileNames)
{
    if (!isCbds(path, fileNames)) {
        return fileNames;
    }

    QStringList pages;
    for (const QString &fileName : fileNames) {
        if (isPageEntry(fileName)) {
            pages.append(fileName);
        }
    }

    // A book without IMAGE/ entries does not follow the known layout, falling back
    // to every image in the archive is better than refusing to open it.
    return pages.isEmpty() ? fileNames : pages;
}

void YACReaderCbds::sortPages(QStringList &pages)
{
    std::stable_sort(pages.begin(), pages.end(), [](const QString &left, const QString &right) {
        return naturalSortLessThanCI(archivePathParts(left).join('/'), archivePathParts(right).join('/'));
    });
}
