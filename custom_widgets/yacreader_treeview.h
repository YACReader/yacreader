#ifndef YACREADER_TREEVIEW_H
#define YACREADER_TREEVIEW_H

#include <QtWidgets>

class YACReaderTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit YACReaderTreeView(QWidget *parent = 0);
    void mousePressEvent(QMouseEvent *event) override;
protected slots:
    // fix for drop auto expand
    void expandCurrent();

protected:
    // Drop to import
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    // fix for drop auto expand
    QTimer expandTimer;
    QTimer t;
    QPoint expandPos;
};

#endif // YACREADER_TREEVIEW_H
