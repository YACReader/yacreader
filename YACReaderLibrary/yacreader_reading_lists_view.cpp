#include "yacreader_reading_lists_view.h"

#include "reading_list_item.h"
#include "reading_list_model.h"

YACReaderReadingListsView::YACReaderReadingListsView(QWidget *parent)
    :YACReaderTreeView(parent)
{
    setItemDelegate(new YACReaderReadingListsViewItemDeletegate(this));
    setUniformRowHeights(false);

    //enabling internal drag&drop
    setDragDropMode(QAbstractItemView::DragDrop);
}

void YACReaderReadingListsView::dragEnterEvent(QDragEnterEvent *event)
{
    YACReaderTreeView::dragEnterEvent(event);

    /*QModelIndex destinationIndex = indexAt(event->pos());
    if(model()->canDropMimeData(event->mimeData(), event->proposedAction(), destinationIndex.row(), destinationIndex.column(), destinationIndex.parent()))*/
        event->acceptProposedAction();
}

void YACReaderReadingListsView::dragMoveEvent(QDragMoveEvent *event)
{
    YACReaderTreeView::dragMoveEvent(event);
    QModelIndex destinationIndex = indexAt(event->pos());
    if(model()->canDropMimeData(event->mimeData(), event->proposedAction(), destinationIndex.row(), destinationIndex.column(), destinationIndex.parent()))
        event->acceptProposedAction();
}

void YACReaderReadingListsView::dropEvent(QDropEvent *event)
{
    YACReaderTreeView::dropEvent(event);


}

//----------------------------------------------------------------------

YACReaderReadingListsViewItemDeletegate::YACReaderReadingListsViewItemDeletegate(QObject *parent)
    :QStyledItemDelegate(parent)
{

}

void YACReaderReadingListsViewItemDeletegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ReadingListModel::TypeList typeList = (ReadingListModel::TypeList)index.data(ReadingListModel::TypeListsRole).toInt();

    if(typeList == ReadingListModel::Separator)
    {
        return;
    }

    QStyledItemDelegate::paint(painter, option, index);
}

QSize YACReaderReadingListsViewItemDeletegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ReadingListModel::TypeList typeList = (ReadingListModel::TypeList)index.data(ReadingListModel::TypeListsRole).toInt();

    if(typeList == ReadingListModel::Separator)
    {
        QSize newSize = QStyledItemDelegate::sizeHint(option, index);
        newSize.setHeight(7);
        return newSize;
    }

    return QStyledItemDelegate::sizeHint(option, index);
}
