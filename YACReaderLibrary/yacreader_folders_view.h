#ifndef YACREADER_FOLDERS_VIEW_H
#define YACREADER_FOLDERS_VIEW_H

#include "yacreader_treeview.h"

#include <QtWidgets>

class YACReaderFoldersView : public YACReaderTreeView
{
    Q_OBJECT
public:
    explicit YACReaderFoldersView(QWidget * parent = 0);

signals:
    //Drops
    void copyComicsToFolder(QList<QPair<QString,QString> >,QModelIndex);
    void moveComicsToFolder(QList<QPair<QString,QString> >,QModelIndex);

protected:
    //Drop to import
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
};

class YACReaderFoldersViewItemDeletegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit YACReaderFoldersViewItemDeletegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


#endif // YACREADER_FOLDERS_VIEW_H
