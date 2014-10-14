#include "comic_files_manager.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <QsLog.h>

#include "comic.h"

ComicFilesManager::ComicFilesManager(QObject *parent) :
    QObject(parent), canceled(false)
{
}

void ComicFilesManager::copyComicsTo(const QList<QPair<QString,QString> > &sourceComics, const QString &folderDest)
{
    comics = sourceComics;
    folder = folderDest;
    move = false;
}

void ComicFilesManager::moveComicsTo(const QList<QPair<QString, QString> > &sourceComics, const QString &folderDest)
{
    comics = sourceComics;
    folder = folderDest;
    move = true;
}

QList<QPair<QString, QString> > ComicFilesManager::getDroppedFiles(const QList<QUrl> &urls)
{
    QList<QPair<QString,QString> > dropedFiles;

    QString currentPath;
    foreach(QUrl url, urls)
    {
        currentPath = url.toLocalFile();
        if(Comic::fileIsComic(currentPath))
            dropedFiles << QPair<QString, QString>(currentPath,"/");
        else
        {
            QFileInfo info(currentPath);
            if(info.isDir())
            {
                foreach(QString comicPath, Comic::findValidComicFilesInFolder(info.absoluteFilePath()))
                {
                    QFileInfo comicInfo(comicPath);
                    QString path = comicInfo.absolutePath();
                    dropedFiles << QPair<QString, QString>(comicPath, path.remove(info.absolutePath()));
                }
            }
        }
    }

    return dropedFiles;
}

void ComicFilesManager::process()
{
    int i=0;
    bool successProcesingFiles = false;
    QPair<QString, QString> source;
    foreach (source, comics) {

        if(canceled)
        {
            if(successProcesingFiles)
                emit success();
            emit finished();

            return; //TODO rollback?
        }

        QFileInfo info(source.first);
        QString destPath = QDir::cleanPath(folder+'/'+source.second);
        QLOG_DEBUG() << "crear : " << destPath;
        QDir().mkpath(destPath);
        if(QFile::copy(source.first, QDir::cleanPath(destPath+'/'+info.fileName())))
        {
            successProcesingFiles = true;
            if(move)
            {
                QFile::remove(source.first); //TODO: remove the whole path....
            }
        }

        i++;
        emit progress(i);
    }

    if(successProcesingFiles)
        emit success();
    emit finished();
}

void ComicFilesManager::cancel()
{
    QLOG_DEBUG() << "Operation canceled";
    canceled = true;
}
