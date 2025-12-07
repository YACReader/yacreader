#ifndef SCRAPER_CHECKBOX_H
#define SCRAPER_CHECKBOX_H

#include <QCheckBox>

class ScraperCheckBox : public QCheckBox
{
public:
    ScraperCheckBox(const QString &text, QWidget *parent = nullptr);
};

#endif // SCRAPER_CHECKBOX_H
