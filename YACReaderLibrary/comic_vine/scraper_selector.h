#ifndef SCRAPER_SELECTOR_H
#define SCRAPER_SELECTOR_H

#include <QWidget>

#include "scraper_results_paginator.h"

class ScraperSelector : public QWidget
{
    Q_OBJECT
public:
    explicit ScraperSelector(QWidget *parent = nullptr);
    virtual void load(const QString &json, const QString &searchString);
public slots:

signals:
    void loadPage(QString, int);

private slots:
    void loadNextPage();
    void loadPreviousPage();

protected:
    QString currentSearchString;
    ScraperResultsPaginator *paginator;
};

#endif // SCRAPER_SELECTOR_H
