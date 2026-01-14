#ifndef SCRAPER_RESULTS_PAGINATOR_H
#define SCRAPER_RESULTS_PAGINATOR_H

#include "themable.h"

#include <QWidget>

class QToolButton;
class QLabel;

class ScraperResultsPaginator : public QWidget, protected Themable
{
    Q_OBJECT
public:
    explicit ScraperResultsPaginator(QWidget *parent = nullptr);
    void update(const QString &json);
    int getCurrentPage();
    void setCustomLabel(const QString &label);
signals:
    void loadNextPage();
    void loadPreviousPage();

private:
    QToolButton *nextPage;
    QToolButton *previousPage;
    QLabel *numElements;
    QLabel *numPages;

    int currentPage;

    QString customLabel;

protected:
    void applyTheme() override;
};

#endif // SCRAPER_RESULTS_PAGINATOR_H
