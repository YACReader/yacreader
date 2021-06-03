#include "yacreader_treeview.h"

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

#ifdef Q_OS_MAC

    setStyleSheet("QTreeView {background-color:transparent; border: none;}"
                  "QTreeView::item:selected {background-color:#91c4f4; border-top: 1px solid #91c4f4; border-left:none;border-right:none;border-bottom:1px solid #91c4f4;}"
                  "QTreeView::branch:selected {background-color:#91c4f4; border-top: 1px solid #91c4f4; border-left:none;border-right:none;border-bottom:1px solid #91c4f4;}"
                  "QTreeView::branch:open:selected:has-children {image: url(':/images/sidebar/expanded_branch_osx.png');}"
                  "QTreeView::branch:closed:selected:has-children {image: url(':/images/sidebar/collapsed_branch_osx.png');}"

    );

#else
    setStyleSheet("QTreeView {background-color:transparent; border: none; color:#DDDFDF; outline:0; show-decoration-selected: 0;}"
                  "QTreeView::item:selected {background-color: #2E2E2E; color:white; font:bold;}"
                  "QTreeView::item:hover {background-color:#2E2E2E; color:white; font:bold;}"
                  "QTreeView::branch:selected {background-color:#2E2E2E;}"

                  "QScrollBar:vertical { border: none; background: #404040; width: 7px; margin: 0 3px 0 0; }"
                  "QScrollBar::handle:vertical { background: #DDDDDD; width: 7px; min-height: 20px; }"
                  "QScrollBar::add-line:vertical { border: none; background: #404040; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"

                  "QScrollBar::sub-line:vertical {  border: none; background: #404040; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                  "QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
                  "QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"

                  "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none; }"

                  "QTreeView::branch:has-children:!has-siblings:closed,QTreeView::branch:closed:has-children:has-siblings {border-image: none;image: url(':/images/sidebar/branch-closed.png');}"
                  "QTreeView::branch:has-children:selected:!has-siblings:closed,QTreeView::branch:closed:selected:has-children:has-siblings {border-image: none;image: url(':/images/sidebar/collapsed_branch_selected.png');}"

                  "QTreeView::branch:open:has-children:!has-siblings,QTreeView::branch:open:has-children:has-siblings  {border-image: none;image: url(':/images/sidebar/branch-open.png');}"
                  "QTreeView::branch:open:has-children:selected:!has-siblings,QTreeView::branch:open:has-children:selected:has-siblings {border-image: none;image: url(':/images/sidebar/expanded_branch_selected.png');}");
#endif
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
        connect(&expandTimer, &QTimer::timeout, this, &YACReaderTreeView::expandCurrent);
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
