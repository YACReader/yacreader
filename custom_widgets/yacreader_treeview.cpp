#include "yacreader_treeview.h"

#include "yacreader_global.h"

YACReaderTreeView::YACReaderTreeView(QWidget *parent)
    : QTreeView(parent), clicking(false)
{
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DropOnly);
    setItemsExpandable(true);

    setContextMenuPolicy(Qt::CustomContextMenu);

    header()->hide();
    setUniformRowHeights(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    initTheme(this);
}

void YACReaderTreeView::applyTheme(const Theme &theme)
{
    setStyleSheet(theme.treeView.treeViewQSS);
}

void YACReaderTreeView::mousePressEvent(QMouseEvent *event)
{
    clicking = true;

    QTreeView::mousePressEvent(event);

    QModelIndex destinationIndex = indexAt(event->pos());

    if (!destinationIndex.isValid() && event->button() == Qt::LeftButton) {
        clearSelection();
    }
}

void YACReaderTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    QTreeView::mouseReleaseEvent(event);

    clicking = false;
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

    // fix for drop auto expand
    QModelIndex underMouse = indexAt(event->pos());
    if (underMouse.isValid()) {
        expandPos = event->pos();
        connect(&expandTimer, &QTimer::timeout, this, &YACReaderTreeView::expandCurrent);
        expandTimer.setSingleShot(true);
        expandTimer.start(500);
    }
    // force mouse hover decoration, TODO why the event loop is not working here?
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

void YACReaderTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);

    // This is a custom signal emitted to ensure that we know when an item in the tree view is selected (e.g. when keyboard navigation is used)
    // By default Qt calls currentChanged while the left mouse button is pressed down an you move the mouse around,
    // this causes troubles when the tree view is showing a filtered model and the model changes under the mouse cursor (e.g. after clicking on an item when the view is showing search results)
    // so this view filters `currentIndexChanged` calls when the mouse is being pressed down.
    if (!clicking)
        emit currentIndexChanged(current);
}
