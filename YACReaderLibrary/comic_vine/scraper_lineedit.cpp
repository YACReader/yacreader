#include "scraper_lineedit.h"

#include "theme_manager.h"

#include <QLabel>

ScraperLineEdit::ScraperLineEdit(const QString &title, QWidget *widget)
    : QLineEdit(widget)
{
    titleLabel = new QLabel(title, this);

    setFixedHeight(22);

    initTheme(this);
}

void ScraperLineEdit::resizeEvent(QResizeEvent *)
{
    QSize szl = titleLabel->sizeHint();
    titleLabel->move(6, (rect().bottom() + 1 - szl.height()) / 2);
}

void ScraperLineEdit::applyTheme()
{
    auto comicVineTheme = ThemeManager::instance().getCurrentTheme().comicVine;

    titleLabel->setStyleSheet(comicVineTheme.scraperLineEditTitleLabelQSS);
    setStyleSheet(comicVineTheme.scraperLineEditQSS.arg(titleLabel->sizeHint().width() + 6));
}
