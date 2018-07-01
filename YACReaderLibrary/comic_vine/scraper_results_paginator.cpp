#include "scraper_results_paginator.h"
#include "response_parser.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QtScript>


ScraperResultsPaginator::ScraperResultsPaginator(QWidget *parent) :
	QWidget(parent),customLabel("items")
{
	QHBoxLayout * pagesButtonsLayout = new QHBoxLayout;

	QString labelStylesheet = "QLabel {color:white; font-size:12px;font-family:Arial;}";

	nextPage = new QToolButton;
	nextPage->setStyleSheet("QToolButton {border:none;}");
	QPixmap np(":/images/comic_vine/nextPage.png");
	nextPage->setIconSize(np.size());
	nextPage->setIcon(np);

	previousPage = new QToolButton;
	previousPage->setStyleSheet("QToolButton {border:none;}");
	QPixmap pp(":/images/comic_vine/previousPage.png");
	previousPage->setIconSize(pp.size());
	previousPage->setIcon(pp);

	connect(nextPage,SIGNAL(clicked()),this,SIGNAL(loadNextPage()));
	connect(previousPage,SIGNAL(clicked()),this,SIGNAL(loadPreviousPage()));

	numElements = new QLabel(tr("Number of volumes found : %1"));
	numElements->setStyleSheet(labelStylesheet);
	numPages = new QLabel(tr("page %1 of %2"));
	numPages->setStyleSheet(labelStylesheet);

	pagesButtonsLayout->addSpacing(15);
	pagesButtonsLayout->addWidget(numElements);
	pagesButtonsLayout->addStretch();
	pagesButtonsLayout->addWidget(numPages);
	pagesButtonsLayout->addWidget(previousPage);
	pagesButtonsLayout->addWidget(nextPage);

	setContentsMargins(0,0,0,0);
	pagesButtonsLayout->setContentsMargins(0,0,0,0);

	setLayout(pagesButtonsLayout);
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

	numPages->setHidden(rp.getTotalPages()==1);
	previousPage->setHidden(rp.getTotalPages()==1);
	nextPage->setHidden(rp.getTotalPages()==1);
}

int ScraperResultsPaginator::getCurrentPage()
{
	return currentPage;
}

void ScraperResultsPaginator::setCustomLabel(const QString &label)
{
	customLabel = label;
}
