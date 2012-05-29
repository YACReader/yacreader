
#include <QStringList>

#include "tableitem.h"

//! [0]
TableItem::TableItem(const QList<QVariant> &data)
{
    itemData = data;
}
//! [0]

//! [1]
TableItem::~TableItem()
{

}
//! [1]


//! [5]
int TableItem::columnCount() const
{
    return itemData.count();
}
//! [5]

//! [6]
QVariant TableItem::data(int column) const
{
    return itemData.value(column);
}
//! [6]


//! [8]
int TableItem::row() const
{

    return 0;
}
//! [8]
