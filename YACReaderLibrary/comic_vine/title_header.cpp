#include "title_header.h"

#include "theme_manager.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

TitleHeader::TitleHeader(QWidget *parent)
    : QWidget(parent)
{
    mainTitleLabel = new QLabel();
    subTitleLabel = new QLabel();

    auto titleLayout = new QHBoxLayout;
    auto titleLabelsLayout = new QVBoxLayout;

    titleLabelsLayout->addWidget(mainTitleLabel);
    titleLabelsLayout->addWidget(subTitleLabel);
    titleLabelsLayout->setSpacing(0);

    titleLayout->addLayout(titleLabelsLayout);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(titleLayout);

    setContentsMargins(0, 0, 0, 0);

    setTitle(tr("SEARCH"));

    initTheme(this);
}

void TitleHeader::setTitle(const QString &title)
{
    mainTitleLabel->setText(title);
}

void TitleHeader::setSubTitle(const QString &title)
{
    subTitleLabel->setText(title);
}

void TitleHeader::showButtons(bool show)
{
    if (show) {

    } else {
    }
}

void TitleHeader::applyTheme()
{
    auto comicVineTheme = ThemeManager::instance().getCurrentTheme().comicVine;

    mainTitleLabel->setStyleSheet(comicVineTheme.titleLabelQSS);
    subTitleLabel->setStyleSheet(comicVineTheme.defaultLabelQSS);
}
