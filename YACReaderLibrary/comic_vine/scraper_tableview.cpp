#include "scraper_tableview.h"

#include "theme_manager.h"

#include <QHeaderView>

ScraperTableView::ScraperTableView(QWidget *parent)
    : QTableView(parent)
{
    setShowGrid(false);
#if QT_VERSION >= 0x050000
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#else
    verticalHeader()->setResizeMode(QHeaderView::Fixed);
#endif

    horizontalHeader()->setStretchLastSection(true);
#if QT_VERSION >= 0x050000
    horizontalHeader()->setSectionsClickable(false);
#else
    horizontalHeader()->setClickable(false);
#endif
    // comicView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->setDefaultSectionSize(24);
#if QT_VERSION >= 0x050000
    verticalHeader()->setSectionsClickable(false); // TODO comportamiento anómalo
#else
    verticalHeader()->setClickable(false); // TODO comportamiento anómalo
#endif

    setCornerButtonEnabled(false);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setAlternatingRowColors(true);

    horizontalHeader()->setMinimumSectionSize(85);

    verticalHeader()->hide();

    setSelectionMode(QAbstractItemView::SingleSelection);

    initTheme(this);
}

void ScraperTableView::applyTheme()
{
    auto comicVineTheme = ThemeManager::instance().getCurrentTheme().comicVine;

    setStyleSheet(comicVineTheme.scraperTableViewQSS);
}
