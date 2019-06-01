#include "yacreader_treeview.h"

#include "theme.h"

YACReaderTreeView::YACReaderTreeView(QWidget *parent)
    : QTreeView(parent)
{
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DropOnly);
    setItemsExpandable(true);

    //setDragEnabled(true);
    /*viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);*/

    setContextMenuPolicy(Qt::CustomContextMenu);

    header()->hide();
    setUniformRowHeights(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    setStyleSheet(Theme::currentTheme().treeViewStyle);
}

void YACReaderTreeView::mousePressEvent(QMouseEvent *event)
{
    QTreeView::mousePressEvent(event);

    QModelIndex destinationIndex = indexAt(event->pos());

    if (!destinationIndex.isValid() && event->button() == Qt::LeftButton) {
        clearSelection();
    }
}

void YACReaderTreeView::expandCurrent()
{
    QModelIndex index = indexAt(expandPos);
    if (index.isValid())
        expand(index);
}

void YACReaderTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeView::dragEnterEvent(event);
}

void YACReaderTreeView::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
}

void YACReaderTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeView::dragMoveEvent(event);

    //fix for drop auto expand
    QModelIndex underMouse = indexAt(event->pos());
    if (underMouse.isValid()) {
        expandPos = event->pos();
        connect(&expandTimer, SIGNAL(timeout()), this, SLOT(expandCurrent()));
        expandTimer.setSingleShot(true);
        expandTimer.start(500);
    }
    //force mouse hover decoration, TODO why the event loop is not working here?
    if (!t.isActive()) {
        t.setSingleShot(true);
        t.setInterval(50);
        t.start();
        repaint();
    }
}

void YACReaderTreeView::dropEvent(QDropEvent *event)
{
    t.stop();

    QTreeView::dropEvent(event);
}
