
#include <QStringList>

#include "comic_item.h"

//! [0]
ComicItem::ComicItem(const QList<QVariant> &data)
	
{
	itemData = data;
}
//! [0]

//! [1]
ComicItem::~ComicItem()
{

}
//! [1]


//! [5]
int ComicItem::columnCount() const
{
	return itemData.count();
}
//! [5]

//! [6]
QVariant ComicItem::data(int column) const
{
	return itemData.value(column);
}
//! [6]

void ComicItem::setData(int column,const QVariant & value)
{
	itemData[column] = value;
}

//! [8]
int ComicItem::row() const
{

	return 0;
}
//! [8]
