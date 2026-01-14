#include "scraper_scroll_label.h"

#include "theme_manager.h"

#include <QLabel>
#include <QDesktopServices>
#include <QUrl>

ScraperScrollLabel::ScraperScrollLabel(QWidget *parent)
    : QScrollArea(parent)
{
    textLabel = new QLabel(this);

    textLabel->setWordWrap(true);
    textLabel->setMinimumSize(168, 12);

    setWidget(textLabel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(textLabel, &QLabel::linkActivated, this, &ScraperScrollLabel::openLink);

    initTheme(this);
}

void ScraperScrollLabel::setAltText(const QString &text)
{
    textLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    textLabel->setText(text);
    textLabel->adjustSize();
}

void ScraperScrollLabel::setText(const QString &text)
{
    textLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    textLabel->setText(text);
    textLabel->adjustSize();
}

void ScraperScrollLabel::openLink(const QString &link)
{
    QDesktopServices::openUrl(QUrl("http://www.comicvine.com" + link));
}

void ScraperScrollLabel::applyTheme()
{
    auto comicVineTheme = ThemeManager::instance().getCurrentTheme().comicVine;

    textLabel->setStyleSheet(comicVineTheme.scraperScrollLabelTextQSS);
    setStyleSheet(comicVineTheme.scraperScrollLabelScrollAreaQSS);
}
