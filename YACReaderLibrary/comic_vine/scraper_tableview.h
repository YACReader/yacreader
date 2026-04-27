#ifndef SCRAPPER_TABLEVIEW_H
#define SCRAPPER_TABLEVIEW_H

#include "themable.h"

#include <QTableView>

class ScraperTableView : public QTableView, protected Themable
{
    Q_OBJECT
public:
    explicit ScraperTableView(QWidget *parent = nullptr);

protected:
    void applyTheme(const Theme &theme) override;
};

#endif // SCRAPPER_TABLEVIEW_H
