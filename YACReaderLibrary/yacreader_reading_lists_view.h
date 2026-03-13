#ifndef YACREADER_READING_LISTS_VIEW_H
#define YACREADER_READING_LISTS_VIEW_H

#include <QPainter>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

#include "yacreader_treeview.h"

class YACReaderReadingListsView : public YACReaderTreeView
{
    Q_OBJECT
public:
    explicit YACReaderReadingListsView(QWidget *parent = nullptr);

protected:
    // Drop to import & internal Drag&Drop for resorting
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

class YACReaderReadingListsViewItemDeletegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit YACReaderReadingListsViewItemDeletegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // YACREADER_READING_LISTS_VIEW_H
