#ifndef IMPORT_COMICS_INFO_DIALOG_H
#define IMPORT_COMICS_INFO_DIALOG_H

#include "themable.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>

class Importer : public QThread
{
public:
    QString source;
    QString dest;

private:
    void run() override;
};

class ImportComicsInfoDialog : public QDialog, protected Themable
{
    Q_OBJECT

public:
    ImportComicsInfoDialog(QWidget *parent = nullptr);
    ~ImportComicsInfoDialog();
    QString dest;

protected:
    void applyTheme(const Theme &theme) override;

private:
    QLabel *imgLabel;
    QLabel *nameLabel;
    QLabel *textLabel;
    QLabel *destLabel;
    QLineEdit *path;
    QLineEdit *destPath;
    QLineEdit *nameEdit;
    QPushButton *find;
    QPushButton *findDest;
    QPushButton *accept;
    QPushButton *cancel;
    QLabel *progress;
    void setupUI();
    int progressCount;
    QProgressBar *progressBar;

public slots:
    void findPath();
    void import();
    void close();
};

#endif // IMPORT_COMICS_INFO_DIALOG_H
