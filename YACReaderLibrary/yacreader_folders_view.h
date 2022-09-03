#ifndef YACREADER_FOLDERS_VIEW_H
#define YACREADER_FOLDERS_VIEW_H

#include "yacreader_treeview.h"

#include <QtWidgets>

class YACReaderFoldersView : public YACReaderTreeView
{
    Q_OBJECT
public:
    explicit YACReaderFoldersView(QWidget *parent = nullptr);

signals:
    // Drops
    void copyComicsToFolder(QList<QPair<QString, QString>>, QModelIndex);
    void moveComicsToFolder(QList<QPair<QString, QString>>, QModelIndex);

protected:
    // Drop to import
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

class YACReaderFoldersViewItemDeletegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit YACReaderFoldersViewItemDeletegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // YACREADER_FOLDERS_VIEW_H
