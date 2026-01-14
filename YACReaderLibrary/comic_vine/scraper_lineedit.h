#ifndef SCRAPPER_LINEEDIT_H
#define SCRAPPER_LINEEDIT_H

#include <QLineEdit>

#include "themable.h"

class QLabel;

class ScraperLineEdit : public QLineEdit, protected Themable
{
    Q_OBJECT
public:
    ScraperLineEdit(const QString &title, QWidget *widget = nullptr);

protected:
    void resizeEvent(QResizeEvent *) override;
    void applyTheme() override;

private:
    QLabel *titleLabel;
};

#endif // SCRAPPER_LINEEDIT_H
