#ifndef TITLE_HEADER_H
#define TITLE_HEADER_H

#include <QWidget>

class QLabel;

class TitleHeader : public QWidget
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
};

#endif // TITLE_HEADER_H
