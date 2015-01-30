#include "comics_view.h"
#include "comic.h"
#include "comic_files_manager.h"

#include "QsLog.h"

ComicsView::ComicsView(QWidget *parent) :
    QWidget(parent),model(NULL)
{
    setAcceptDrops(true);
}

void ComicsView::setModel(ComicModel *m)
{
    model = m;
}

void ComicsView::dragEnterEvent(QDragEnterEvent *event)
{
    if(model->canDropMimeData(event->mimeData(),event->proposedAction(),0,0,QModelIndex()))
        event->acceptProposedAction();
    else
    {
    QLOG_INFO() << "dragEnterEvent";
    QList<QUrl> urlList;

    if (event->mimeData()->hasUrls() && event->dropAction() == Qt::CopyAction)
    {
        urlList = event->mimeData()->urls();
        QString currentPath;
        foreach (QUrl url, urlList)
        {
            //comics or folders are accepted, folders' content is validate in dropEvent (avoid any lag before droping)
            currentPath = url.toLocalFile();
            if(Comic::fileIsComic(currentPath) || QFileInfo(currentPath).isDir())
            {
                event->acceptProposedAction();
                return;
            }
        }
    }
    }
}

void ComicsView::dropEvent(QDropEvent *event)
{
    QLOG_DEBUG() << "drop" << event->dropAction();

    bool validAction = event->dropAction() == Qt::CopyAction;// || event->dropAction() & Qt::MoveAction;  TODO move

    if(event->mimeData()->hasUrls() && validAction)
    {

        QList<QPair<QString, QString> > droppedFiles = ComicFilesManager::getDroppedFiles(event->mimeData()->urls());

        if(event->dropAction() == Qt::CopyAction)
        {
            QLOG_DEBUG() << "copy :" << droppedFiles;
            emit copyComicsToCurrentFolder(droppedFiles);
        }
        else if(event->dropAction() & Qt::MoveAction)
        {
            QLOG_DEBUG() << "move :" << droppedFiles;
            emit moveComicsToCurrentFolder(droppedFiles);
        }

        event->acceptProposedAction();
    }
}
