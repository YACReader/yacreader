#include "library_comic_opener.h"

#include "comic_db.h"

#include <QtCore>
#include <QtWidgets>

bool YACReader::openComic(const ComicDB &comic,
                          unsigned long long libraryId,
                          const QString &path,
                          OpenComicSource source)
{
    bool yacreaderFound = false;

    QString labelParam;

    if (source.source == OpenComicSource::ReadingList) {
        labelParam = QString("--readingListId=%1").arg(source.sourceId);
    }

#ifdef Q_OS_MACOS
    QStringList possiblePaths { QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../../") };
    possiblePaths += QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

    for (auto &&ypath : possiblePaths) {
        QString yacreaderPath = QDir::cleanPath(ypath + "/YACReader.app");
        if (QFileInfo(yacreaderPath).exists()) {
            yacreaderFound = true;
            QStringList parameters { "-n", yacreaderPath, "--args", path, QString("--comicId=%1").arg(comic.id), QString("--libraryId=%1").arg(libraryId), labelParam };
            QProcess::startDetached("open", parameters);
            break;
        }
    }
#endif

#ifdef Q_OS_WIN
    QStringList parameters { path, QString("--comicId=%1").arg(comic.id), QString("--libraryId=%1").arg(libraryId), labelParam };
    yacreaderFound = QProcess::startDetached(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/YACReader.exe"), parameters);
#endif

#if defined Q_OS_UNIX && !defined Q_OS_MAC
    QStringList parameters { path, QString("--comicId=%1").arg(comic.id), QString("--libraryId=%1").arg(libraryId), labelParam };
    yacreaderFound = QProcess::startDetached(QStringLiteral("YACReader"), parameters);
#endif

    return yacreaderFound;
}
