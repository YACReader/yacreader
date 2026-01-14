#ifndef SCRAPPER_TABLEVIEW_H
#define SCRAPPER_TABLEVIEW_H

#include <QTableView>

#include "themable.h"

class ScraperTableView : public QTableView, protected Themable
{
    Q_OBJECT
public:
    explicit ScraperTableView(QWidget *parent = nullptr);

protected:
    void applyTheme() override;
};

#endif // SCRAPPER_TABLEVIEW_H
