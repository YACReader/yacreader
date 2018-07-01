#include "scraper_selector.h"

ScraperSelector::ScraperSelector(QWidget *parent) :
	QWidget(parent)
{
	paginator = new ScraperResultsPaginator;
	connect(paginator,SIGNAL(loadNextPage()),this,SLOT(loadNextPage()));
	connect(paginator,SIGNAL(loadPreviousPage()),this,SLOT(loadPreviousPage()));
}

void ScraperSelector::load(const QString &json, const QString &searchString)
{
	currentSearchString = searchString;
	paginator->update(json);
}

void ScraperSelector::loadNextPage()
{
	emit loadPage(currentSearchString,paginator->getCurrentPage()+1);
}

void ScraperSelector::loadPreviousPage()
{
	emit loadPage(currentSearchString,paginator->getCurrentPage()-1);
}
