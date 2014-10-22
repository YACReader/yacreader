#include <QStringList>

#include "treeitem.h"
#include "qnaturalsorting.h"

TreeItem::TreeItem(const QList<QVariant> &data, TreeItem *parent)
{
	parentItem = parent;
	itemData = data;
}

TreeItem::~TreeItem()
{
	qDeleteAll(childItems);
}

void TreeItem::appendChild(TreeItem *item)
{
	item->parentItem = this;

	if(childItems.isEmpty())
		childItems.append(item);
	else
	{
		TreeItem * last = childItems.back();
		QString nameLast = last->data(1).toString(); //TODO usar info name si est� disponible, sino el nombre del fichero.....
		QString nameCurrent = item->data(1).toString();
		QList<TreeItem *>::iterator i;
		i = childItems.end();
		i--;
		while (naturalSortLessThanCI(nameCurrent,nameLast) && i != childItems.begin())
		{
			i--;
			nameLast = (*i)->data(1).toString();
		}
		if(!naturalSortLessThanCI(nameCurrent,nameLast)) //si se ha encontrado un elemento menor que current, se inserta justo despu�s
			childItems.insert(++i,item);
		else
			childItems.insert(i,item);

	}

	//childItems.append(item);
}

TreeItem *TreeItem::child(int row)
{
	return childItems.value(row);
}

int TreeItem::childCount() const
{
	return childItems.count();
}

int TreeItem::columnCount() const
{
	return itemData.count();
}

QVariant TreeItem::data(int column) const
{
	return itemData.value(column);
}

void TreeItem::setData(int column, const QVariant & value)
{
    itemData[column] = value;
}

void TreeItem::removeChild(int childIndex)
{
    childItems.removeAt(childIndex);
}

void TreeItem::clearChildren()
{
    qDeleteAll(childItems);
    childItems.clear();
}

QList<TreeItem *> TreeItem::children()
{
    return childItems;
}

TreeItem *TreeItem::parent()
{
	return parentItem;
}

int TreeItem::row() const
{
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

	return 0;
}

QList<QVariant> TreeItem::getData() const
{
	return itemData;
}
