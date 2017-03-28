#ifndef YACREADER_TREEVIEW_H
#define YACREADER_TREEVIEW_H

#include <QtWidgets>

class YACReaderTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit YACReaderTreeView(QWidget *parent = 0);
    void mousePressEvent(QMouseEvent *event);
protected slots:
    //fix for drop auto expand
    void expandCurrent();

protected:
    //Drop to import
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    //fix for drop auto expand
    QTimer expandTimer;
    QTimer t;
    QPoint expandPos;
};

#endif // YACREADER_TREEVIEW_H
