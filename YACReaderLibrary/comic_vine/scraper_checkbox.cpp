#include "scraper_checkbox.h"

#include "theme_manager.h"

ScraperCheckBox::ScraperCheckBox(const QString &text, QWidget *parent)
    : QCheckBox(text, parent)
{
    initTheme(this);
}

void ScraperCheckBox::applyTheme()
{
    auto comicVineTheme = ThemeManager::instance().getCurrentTheme().comicVine;

    setStyleSheet(comicVineTheme.checkBoxQSS);
}
