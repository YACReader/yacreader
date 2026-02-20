#include "scraper_tableview.h"

#include <QHeaderView>

ScraperTableView::ScraperTableView(QWidget *parent)
    : QTableView(parent)
{
    setShowGrid(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionsClickable(false);
    // comicView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->setDefaultSectionSize(24);
    verticalHeader()->setSectionsClickable(false); // TODO comportamiento anómalo

    setCornerButtonEnabled(false);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setAlternatingRowColors(true);

    horizontalHeader()->setMinimumSectionSize(85);

    verticalHeader()->hide();

    setSelectionMode(QAbstractItemView::SingleSelection);

    initTheme(this);
}

void ScraperTableView::applyTheme(const Theme &theme)
{
    auto comicVineTheme = theme.comicVine;

    setStyleSheet(comicVineTheme.scraperTableViewQSS);
}
