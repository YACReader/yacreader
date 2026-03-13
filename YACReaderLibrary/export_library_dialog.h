#ifndef EXPORT_LIBRARY_DIALOG_H
#define EXPORT_LIBRARY_DIALOG_H

#include "themable.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QString>
#include <QThread>
#include <QTimer>

class ExportLibraryDialog : public QDialog, protected Themable
{
    Q_OBJECT
public:
    ExportLibraryDialog(QWidget *parent = nullptr);

protected:
    void applyTheme(const Theme &theme) override;

public slots:
    void exportLibrary();
    void findPath();
    void close();

private:
    QLabel *imgLabel;
    int progressCount;
    QProgressBar *progressBar;
    QLabel *textLabel;
    QLineEdit *path;
    QPushButton *find;
    QPushButton *accept;
    QPushButton *cancel;
    void run();
signals:
    void exportPath(QString);
};

#endif
