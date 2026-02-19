#ifndef __RENAME_LIBRARY_DIALOG_H
#define __RENAME_LIBRARY_DIALOG_H

#include "themable.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class RenameLibraryDialog : public QDialog, protected Themable
{
    Q_OBJECT
public:
    RenameLibraryDialog(QWidget *parent = nullptr);

protected:
    void applyTheme(const Theme &theme) override;

private:
    QLabel *imgLabel;
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
