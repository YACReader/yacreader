#ifndef SCRAPPER_LINEEDIT_H
#define SCRAPPER_LINEEDIT_H

#include <QLineEdit>

class QLabel;

class ScraperLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    ScraperLineEdit(const QString &title, QWidget *widget = nullptr);

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    QLabel *titleLabel;
};

#endif // SCRAPPER_LINEEDIT_H
