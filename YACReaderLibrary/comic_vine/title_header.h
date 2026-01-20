#ifndef TITLE_HEADER_H
#define TITLE_HEADER_H

#include <QWidget>

#include "themable.h"

class QLabel;

class TitleHeader : public QWidget, protected Themable
{
    Q_OBJECT
public:
    TitleHeader(QWidget *parent = nullptr);
public slots:
    void setTitle(const QString &title);
    void setSubTitle(const QString &title);
    void showButtons(bool show);

private:
    QLabel *mainTitleLabel;
    QLabel *subTitleLabel;

protected:
    void applyTheme(const Theme &theme) override;
};

#endif // TITLE_HEADER_H
