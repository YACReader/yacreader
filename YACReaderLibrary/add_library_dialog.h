#ifndef __ADD_LIBRARY_DIALOG_H
#define __ADD_LIBRARY_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QThread>

class AddLibraryDialog : public QDialog
{
    Q_OBJECT
public:
    AddLibraryDialog(QWidget *parent = 0);

private:
    QLabel *nameLabel;
    QLabel *textLabel;
    QLineEdit *path;
    QLineEdit *nameEdit;
    QPushButton *find;
    QPushButton *accept;
    QPushButton *cancel;
    void setupUI();
public slots:
    void add();
    void findPath();
    void close();
    void nameSetted(const QString &text);
    void pathSetted(const QString &text);
signals:
    void addLibrary(QString target, QString name);
};

#endif
