#include "scraper_checkbox.h"
#include "qwidget.h"

ScraperCheckBox::ScraperCheckBox(const QString &text, QWidget *parent)
    : QCheckBox(text, parent)
{
    setStyleSheet(
            "QCheckBox {"
            "    color: white;"
            "    font-size: 12px;"
            "    font-family: Arial;"
            "    spacing: 10px;"
            "}"
            "QCheckBox::indicator {"
            "    width: 13px;"
            "    height: 13px;"
            "    border: 1px solid #242424;"
            "    background: #2e2e2e;"
            "}"
            "QCheckBox::indicator:checked {"
            "    image: url(:/images/comic_vine/checkBoxTick.svg);"
            "    background: #2e2e2e;"
            "}"
            "QCheckBox::indicator:unchecked {"
            "    background: #2e2e2e;"
            "}");
}
