#ifndef MOVE_TO_FOLDER_DIALOG_H
#define MOVE_TO_FOLDER_DIALOG_H

#include <QDialog>
#include <QPersistentModelIndex>

class QPushButton;
class QTreeWidget;
class ReadingListModel;

// the folder a folder or a list moves to: "Top level" and the folder tree of its section
class MoveToFolderDialog : public QDialog
{
    Q_OBJECT

public:
    MoveToFolderDialog(ReadingListModel *model, const QModelIndex &item, QWidget *parent = nullptr);

    // invalid for the top level
    QModelIndex folder() const;

private:
    ReadingListModel *model;
    QPersistentModelIndex item;
    QTreeWidget *tree;
    QPushButton *moveButton;
};

#endif
