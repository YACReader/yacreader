#ifndef EXPORT_COMICS_INFO_DIALOG_H
#define EXPORT_COMICS_INFO_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class ExportComicsInfoDialog : public QDialog
{
    Q_OBJECT

public:
    ExportComicsInfoDialog(QWidget *parent = 0);
    ~ExportComicsInfoDialog();
    QString source;

public slots:
    void findPath();
    void exportComicsInfo();
    void close();

private:
    QLabel *progress;
    QLabel *textLabel;
    QLineEdit *path;
    QPushButton *find;
    QPushButton *accept;
    QPushButton *cancel;
};

#endif // EXPORT_COMICS_INFO_DIALOG_H
