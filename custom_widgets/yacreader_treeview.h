#ifndef YACREADER_TREEVIEW_H
#define YACREADER_TREEVIEW_H

#include <QtWidgets>

class YACReaderTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit YACReaderTreeView(QWidget *parent = 0);
    
signals:
    //Drops
    void copyComicsToFolder(QList<QPair<QString,QString> >,QModelIndex);
    void moveComicsToFolder(QList<QPair<QString,QString> >,QModelIndex);

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

class YACReaderTreeViewItemDeletegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit YACReaderTreeViewItemDeletegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // YACREADER_TREEVIEW_H
