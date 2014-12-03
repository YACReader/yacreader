#ifndef YACREADER_READING_LISTS_VIEW_H
#define YACREADER_READING_LISTS_VIEW_H

#include "yacreader_treeview.h"

#include <QtWidgets>

class YACReaderReadingListsView : public YACReaderTreeView
{
    Q_OBJECT
public:
    explicit YACReaderReadingListsView(QWidget * parent = 0);

protected:
    //Drop to import
    void dragEnterEvent(QDragEnterEvent *event);
};

class YACReaderReadingListsViewItemDeletegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit YACReaderReadingListsViewItemDeletegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


#endif // YACREADER_READING_LISTS_VIEW_H
