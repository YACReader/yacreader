#ifndef __GOTODIALOG_H
#define __GOTODIALOG_H

#include "themable.h"

#include <QDialog>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class GoToDialog : public QDialog, protected Themable
{
    Q_OBJECT
public:
    GoToDialog(QWidget *parent = nullptr);

protected:
    void applyTheme(const Theme &theme) override;

private:
    QLabel *numPagesLabel;
    QLabel *textLabel;
    QLabel *imgLabel;
    QLineEdit *pageNumber;
    QIntValidator *v;
    QPushButton *accept;
    QPushButton *cancel;
    void setupUI();
public slots:
    void goTo();
    void setNumPages(unsigned int numPages);
    void open() override;
signals:
    void goToPage(unsigned int page);
};

#endif
