#include "comic_files_manager.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <QsLog.h>

#include "comic.h"

ComicFilesManager::ComicFilesManager(QObject *parent)
    : QObject(parent), canceled(false)
{
}

void ComicFilesManager::copyComicsTo(const QList<QPair<QString, QString>> &sourceComics, const QString &folderDest, const QModelIndex &dest)
{
    comics = sourceComics;
    folder = folderDest;
    folderDestinationModelIndex = dest;
    move = false;
}

void ComicFilesManager::moveComicsTo(const QList<QPair<QString, QString>> &sourceComics, const QString &folderDest, const QModelIndex &dest)
{
    comics = sourceComics;
    folder = folderDest;
    folderDestinationModelIndex = dest;
    move = true;
}

QList<QPair<QString, QString>> ComicFilesManager::getDroppedFiles(const QList<QUrl> &urls)
{
    QList<QPair<QString, QString>> dropedFiles;

    QString currentPath;
    foreach (QUrl url, urls) {
        currentPath = url.toLocalFile();
        if (currentPath.endsWith('/'))
            currentPath = currentPath.remove(currentPath.length() - 1, 1); //QTBUG-35896 QUrl.toLocalFile inconsistency.
        if (Comic::fileIsComic(currentPath))
            dropedFiles << QPair<QString, QString>(currentPath, "/");
        else {
            QLOG_DEBUG() << "XXXXXXXXXXXX :" << currentPath;
            QFileInfo info(currentPath);
            if (info.isDir()) {
                QLOG_DEBUG() << "origin path prior to absoluteFilePath : " << info.absolutePath();
                foreach (QString comicPath, Comic::findValidComicFilesInFolder(info.absoluteFilePath())) {
                    QFileInfo comicInfo(comicPath);
                    QString path = comicInfo.absolutePath();
                    QLOG_DEBUG() << "comic path : " << comicPath;
                    QLOG_DEBUG() << "full comic path : " << path;
                    QLOG_DEBUG() << "origin path : " << info.absolutePath();
                    dropedFiles << QPair<QString, QString>(comicPath, path.remove(info.absolutePath()));
                }
            }
        }
    }

    return dropedFiles;
}

void ComicFilesManager::process()
{
    int i = 0;
    bool successProcesingFiles = false;
    QPair<QString, QString> source;
    foreach (source, comics) {

        if (canceled) {
            if (successProcesingFiles)
                emit success(folderDestinationModelIndex);
            emit finished();

            return; //TODO rollback?
        }

        QFileInfo info(source.first);
        QString destPath = QDir::cleanPath(folder + '/' + source.second);
        QLOG_DEBUG() << "crear : " << destPath;
        QDir().mkpath(destPath);
        if (QFile::copy(source.first, QDir::cleanPath(destPath + '/' + info.fileName()))) {
            successProcesingFiles = true;
            if (move) {
                QFile::remove(source.first); //TODO: remove the whole path....
            }
        }

        i++;
        emit progress(i);
    }

    if (successProcesingFiles)
        emit success(folderDestinationModelIndex);
    emit finished();
}

void ComicFilesManager::cancel()
{
    QLOG_DEBUG() << "Operation canceled";
    canceled = true;
}
