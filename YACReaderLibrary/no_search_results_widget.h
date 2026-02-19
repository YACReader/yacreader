#ifndef NO_SEARCH_RESULTS_WIDGET_H
#define NO_SEARCH_RESULTS_WIDGET_H

#include "empty_container_info.h"

class NoSearchResultsWidget : public EmptyContainerInfo
{
    Q_OBJECT
public:
    explicit NoSearchResultsWidget(QWidget *parent = nullptr);

protected:
    void applyTheme(const Theme &theme) override;
};

#endif // NO_SEARCH_RESULTS_WIDGET_H
