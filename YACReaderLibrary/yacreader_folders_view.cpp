#include "yacreader_folders_view.h"

#include "folder_item.h"
#include "folder_model.h"

#include "comic.h"
#include "comic_files_manager.h"

#include "QsLog.h"

YACReaderFoldersView::YACReaderFoldersView(QWidget *parent)
    : YACReaderTreeView(parent)
{
    setItemDelegate(new YACReaderFoldersViewItemDeletegate(this));
}

void YACReaderFoldersView::dragEnterEvent(QDragEnterEvent *event)
{
    YACReaderTreeView::dragEnterEvent(event);

    QList<QUrl> urlList;

    if (event->mimeData()->hasUrls() && event->dropAction() == Qt::CopyAction) {
        urlList = event->mimeData()->urls();
        QString currentPath;
        foreach (QUrl url, urlList) {
            // comics or folders are accepted, folders' content is validate in dropEvent (avoid any lag before droping)
            currentPath = url.toLocalFile();
            if (Comic::fileIsComic(currentPath) || QFileInfo(currentPath).isDir()) {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void YACReaderFoldersView::dragLeaveEvent(QDragLeaveEvent *event)
{
    YACReaderTreeView::dragLeaveEvent(event);
}

void YACReaderFoldersView::dragMoveEvent(QDragMoveEvent *event)
{
    YACReaderTreeView::dragMoveEvent(event);
    event->acceptProposedAction();
}

void YACReaderFoldersView::dropEvent(QDropEvent *event)
{
    YACReaderTreeView::dropEvent(event);

    QLOG_DEBUG() << "drop on tree" << event->dropAction();

    bool validAction = event->dropAction() == Qt::CopyAction; // || event->dropAction() & Qt::MoveAction; TODO move

    if (validAction) {
        QList<QPair<QString, QString>> droppedFiles = ComicFilesManager::getDroppedFiles(event->mimeData()->urls());
        QModelIndex destinationIndex = indexAt(event->pos());

        if (event->dropAction() == Qt::CopyAction) {
            QLOG_DEBUG() << "copy - tree :" << droppedFiles;
            emit copyComicsToFolder(droppedFiles, destinationIndex);
        } else if (event->dropAction() & Qt::MoveAction) {
            QLOG_DEBUG() << "move - tree :" << droppedFiles;
            emit moveComicsToFolder(droppedFiles, destinationIndex);
        }

        event->acceptProposedAction();
    }
}

//----------------------------------------------------------

YACReaderFoldersViewItemDeletegate::YACReaderFoldersViewItemDeletegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void YACReaderFoldersViewItemDeletegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.data(FolderModel::CompletedRole).toBool()) {
        painter->save();
#ifdef Q_OS_MAC
        painter->setBrush(QBrush(QColor(85, 95, 127)));
#else
        painter->setBrush(QBrush(QColor(237, 197, 24)));
#endif
        painter->setPen(QPen(QBrush(), 0));
        painter->drawRect(0, option.rect.y(), 2, option.rect.height());
        painter->restore();
    }

    QStyledItemDelegate::paint(painter, option, index);
}
