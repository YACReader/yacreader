#include "comic_files_manager.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <QsLog.h>

ComicFilesManager::ComicFilesManager(QObject *parent) :
    QObject(parent), canceled(false)
{
}

void ComicFilesManager::copyComicsTo(const QList<QString> &sourceComics, const QString &folderDest)
{
    comics = sourceComics;
    folder = folderDest;
    move = false;
}

void ComicFilesManager::moveComicsTo(const QList<QString> &sourceComics, const QString &folderDest)
{
    comics = sourceComics;
    folder = folderDest;
    move = true;
}

void ComicFilesManager::process()
{
    int i=0;
    bool successProcesingFiles = false;
    foreach (QString source, comics) {

        if(canceled)
        {
            if(successProcesingFiles)
                emit success();
            emit finished();

            return; //TODO rollback?
        }

        QFileInfo info(source);
        if(QFile::copy(source, QDir::cleanPath(folder+'/'+info.fileName())))
        {
            successProcesingFiles = true;
            if(move)
                QFile::remove(source);
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
