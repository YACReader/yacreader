#ifndef EXPORT_COMICS_INFO_DIALOG_H
#define EXPORT_COMICS_INFO_DIALOG_H

#include "themable.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class ExportComicsInfoDialog : public QDialog, protected Themable
{
    Q_OBJECT

public:
    ExportComicsInfoDialog(QWidget *parent = nullptr);
    ~ExportComicsInfoDialog() override;
    QString source;

protected:
    void applyTheme(const Theme &theme) override;

public slots:
    void findPath();
    void exportComicsInfo();
    void close();

private:
    QLabel *imgLabel;
    QLabel *progress;
    QLabel *textLabel;
    QLineEdit *path;
    QPushButton *find;
    QPushButton *accept;
    QPushButton *cancel;
};

#endif // EXPORT_COMICS_INFO_DIALOG_H
