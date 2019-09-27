#ifndef __RENAME_LIBRARY_DIALOG_H
#define __RENAME_LIBRARY_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class RenameLibraryDialog : public QDialog
{
    Q_OBJECT
public:
    RenameLibraryDialog(QWidget *parent = nullptr);

private:
    QLabel *newNameLabel;
    QLineEdit *newNameEdit;
    QPushButton *accept;
    QPushButton *cancel;
    void setupUI();
public slots:
    void rename();
    void close();
    void nameSetted(const QString &name);
signals:
    void renameLibrary(QString newName);
};

#endif
