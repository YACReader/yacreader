#ifndef YACREADER_TREEVIEW_H
#define YACREADER_TREEVIEW_H

#include <QTreeView>
#include <QStyledItemDelegate>

class YACReaderTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit YACReaderTreeView(QWidget *parent = 0);
    
signals:
    
public slots:
    
};

class YACReaderTreeViewItemDeletegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit YACReaderTreeViewItemDeletegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // YACREADER_TREEVIEW_H
