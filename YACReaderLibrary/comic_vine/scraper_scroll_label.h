#ifndef SCRAPER_SCROLL_LABEL_H
#define SCRAPER_SCROLL_LABEL_H

#include "themable.h"

#include <QScrollArea>

class QLabel;

class ScraperScrollLabel : public QScrollArea, protected Themable
{
    Q_OBJECT
public:
    explicit ScraperScrollLabel(QWidget *parent = nullptr);

public slots:
    void setText(const QString &text);
    void setAltText(const QString &text);

    void openLink(const QString &link);

private:
    QLabel *textLabel;

protected:
    void applyTheme(const Theme &theme) override;
};

#endif // SCRAPER_SCROLL_LABEL_H
