#include "scraper_lineedit.h"

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

void ScraperLineEdit::applyTheme(const Theme &theme)
{
    auto metadataScraperDialogTheme = theme.metadataScraperDialog;

    titleLabel->setStyleSheet(metadataScraperDialogTheme.scraperLineEditTitleLabelQSS);
    setStyleSheet(metadataScraperDialogTheme.scraperLineEditQSS.arg(titleLabel->sizeHint().width() + 6));
}
