#ifndef SCRAPPER_LINEEDIT_H
#define SCRAPPER_LINEEDIT_H

#include "themable.h"

#include <QLineEdit>

class QLabel;

class ScraperLineEdit : public QLineEdit, protected Themable
{
    Q_OBJECT
public:
    ScraperLineEdit(const QString &title, QWidget *widget = nullptr);

protected:
    void resizeEvent(QResizeEvent *) override;
    void applyTheme(const Theme &theme) override;

private:
    QLabel *titleLabel;
};

#endif // SCRAPPER_LINEEDIT_H
