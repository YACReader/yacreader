#include "no_search_results_widget.h"

NoSearchResultsWidget::NoSearchResultsWidget(QWidget *parent)
    : EmptyContainerInfo(parent)
{
    setUpDefaultLayout(true);

    setPixmap(theme.emptyContainer.noSearchResultsIcon);
    setText(tr("No results"));

    setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy ::Expanding, QSizePolicy ::Expanding);
}

void NoSearchResultsWidget::applyTheme(const Theme &theme)
{
    EmptyContainerInfo::applyTheme(theme);
    setPixmap(theme.emptyContainer.noSearchResultsIcon);
}
