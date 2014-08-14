#include "comics_view.h"

ComicsView::ComicsView(QWidget *parent) :
    QWidget(parent),model(NULL)
{
}

void ComicsView::setModel(TableModel *m)
{
   model = m;
}
