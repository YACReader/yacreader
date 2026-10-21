#ifndef NEW_LIST_FOLDER_DIALOG_H
#define NEW_LIST_FOLDER_DIALOG_H

#include "library_ids.h"

#include <QDialog>

class QLineEdit;
class QPushButton;
class QRadioButton;

// the name and the section of a new folder, when the selection does not give the section
class NewListFolderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewListFolderDialog(QWidget *parent = nullptr);

    QString name() const;
    ListFolderKind section() const;

private:
    QLineEdit *nameEdit;
    QRadioButton *readingListsButton;
    QRadioButton *smartListsButton;
    QPushButton *createButton;
};

#endif
