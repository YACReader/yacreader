#include "scraper_results_paginator.h"
#include "response_parser.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

ScraperResultsPaginator::ScraperResultsPaginator(QWidget *parent)
    : QWidget(parent), customLabel("items")
{
    auto pagesButtonsLayout = new QHBoxLayout;

    nextPage = new QToolButton;
    previousPage = new QToolButton;

    connect(nextPage, &QAbstractButton::clicked, this, &ScraperResultsPaginator::loadNextPage);
    connect(previousPage, &QAbstractButton::clicked, this, &ScraperResultsPaginator::loadPreviousPage);

    numElements = new QLabel(tr("Number of volumes found : %1"));
    numPages = new QLabel(tr("page %1 of %2"));

    pagesButtonsLayout->addSpacing(15);
    pagesButtonsLayout->addWidget(numElements);
    pagesButtonsLayout->addStretch();
    pagesButtonsLayout->addWidget(numPages);
    pagesButtonsLayout->addWidget(previousPage);
    pagesButtonsLayout->addWidget(nextPage);

    setContentsMargins(0, 0, 0, 0);
    pagesButtonsLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(pagesButtonsLayout);

    initTheme(this);
}

void ScraperResultsPaginator::update(const QString &json)
{
    ResponseParser rp;
    rp.loadJSONResponse(json);

    currentPage = rp.getCurrentPage();
    numElements->setText(tr("Number of %1 found : %2").arg(customLabel).arg(rp.getNumResults()));
    numPages->setText(tr("page %1 of %2").arg(currentPage).arg(rp.getTotalPages()));

    previousPage->setDisabled(currentPage == 1);
    nextPage->setDisabled(currentPage == rp.getTotalPages());

    numPages->setHidden(rp.getTotalPages() == 1);
    previousPage->setHidden(rp.getTotalPages() == 1);
    nextPage->setHidden(rp.getTotalPages() == 1);
}

int ScraperResultsPaginator::getCurrentPage()
{
    return currentPage;
}

void ScraperResultsPaginator::setCustomLabel(const QString &label)
{
    customLabel = label;
}

void ScraperResultsPaginator::applyTheme(const Theme &theme)
{
    auto comicVineTheme = theme.comicVine;

    numElements->setStyleSheet(comicVineTheme.defaultLabelQSS);
    numPages->setStyleSheet(comicVineTheme.defaultLabelQSS);

    nextPage->setStyleSheet(comicVineTheme.noBorderToolButtonQSS);
    nextPage->setIconSize(comicVineTheme.nextPageIcon.size);
    nextPage->setIcon(comicVineTheme.nextPageIcon.icon);

    previousPage->setStyleSheet(comicVineTheme.noBorderToolButtonQSS);
    previousPage->setIconSize(comicVineTheme.previousPageIcon.size);
    previousPage->setIcon(comicVineTheme.previousPageIcon.icon);
}
