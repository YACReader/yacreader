#include "scraper_checkbox.h"

ScraperCheckBox::ScraperCheckBox(const QString &text, QWidget *parent)
    : QCheckBox(text, parent)
{
    initTheme(this);
}

void ScraperCheckBox::applyTheme(const Theme &theme)
{
    auto metadataScraperDialogTheme = theme.metadataScraperDialog;

    setStyleSheet(metadataScraperDialogTheme.checkBoxQSS);
}
