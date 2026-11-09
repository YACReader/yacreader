#ifndef SEARCH_FILTERS_MENU_H
#define SEARCH_FILTERS_MENU_H

#include <QString>

#include <functional>

class QMenu;
class QSettings;
class QWidget;

// the quick filters and "Search syntax…" of a search field; `apply` gets the search text of the chosen filter
QMenu *createSearchFiltersMenu(QWidget *parent, QSettings *settings, const std::function<void(const QString &)> &apply);

#endif
