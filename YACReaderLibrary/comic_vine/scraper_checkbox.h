#ifndef SCRAPER_CHECKBOX_H
#define SCRAPER_CHECKBOX_H

#include "themable.h"

#include <QCheckBox>

class ScraperCheckBox : public QCheckBox, protected Themable
{
public:
    ScraperCheckBox(const QString &text, QWidget *parent = nullptr);

protected:
    void applyTheme(const Theme &theme) override;
};

#endif // SCRAPER_CHECKBOX_H
