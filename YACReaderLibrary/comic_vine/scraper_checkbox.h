#ifndef SCRAPER_CHECKBOX_H
#define SCRAPER_CHECKBOX_H

#include <QCheckBox>

#include "themable.h"

class ScraperCheckBox : public QCheckBox, protected Themable
{
public:
    ScraperCheckBox(const QString &text, QWidget *parent = nullptr);

protected:
    void applyTheme() override;
};

#endif // SCRAPER_CHECKBOX_H
