#include <QStringList>

#include "folder_item.h"
#include "qnaturalsorting.h"

FolderItem::FolderItem(const QList<QVariant> &data, FolderItem *parent)
{
    parentItem = parent;
    itemData = data;
}

FolderItem::~FolderItem()
{
    qDeleteAll(childItems);
}

void FolderItem::appendChild(FolderItem *item)
{
    item->parentItem = this;

    if (childItems.isEmpty())
        childItems.append(item);
    else {
        FolderItem *last = childItems.back();
        QString nameLast = last->data(0).toString(); // TODO usar info name si est� disponible, sino el nombre del fichero.....
        QString nameCurrent = item->data(0).toString();
        QList<FolderItem *>::iterator i;
        i = childItems.end();
        i--;
        while (naturalSortLessThanCI(nameCurrent, nameLast) && i != childItems.begin()) {
            i--;
            nameLast = (*i)->data(0).toString();
        }
        if (!naturalSortLessThanCI(nameCurrent, nameLast)) // si se ha encontrado un elemento menor que current, se inserta justo despu�s
            childItems.insert(++i, item);
        else
            childItems.insert(i, item);
    }

    // childItems.append(item);
}

FolderItem *FolderItem::child(int row)
{
    return childItems.value(row);
}

int FolderItem::childCount() const
{
    return childItems.count();
}

int FolderItem::columnCount() const
{
    return itemData.count();
}

QVariant FolderItem::data(int column) const
{
    return itemData.value(column);
}

void FolderItem::setData(int column, const QVariant &value)
{
    itemData[column] = value;
}

void FolderItem::addChild(FolderItem *child, int childIndex)
{
    child->parentItem = this;
    childItems.insert(childIndex, child);
}

void FolderItem::removeChild(FolderItem *child)
{
    childItems.removeOne(child);
}

void FolderItem::removeChild(int childIndex)
{
    childItems.removeAt(childIndex);
}

void FolderItem::clearChildren()
{
    qDeleteAll(childItems);
    childItems.clear();
}

QList<FolderItem *> FolderItem::children()
{
    return childItems;
}

FolderItem *FolderItem::parent()
{
    return parentItem;
}

void FolderItem::setData(const QList<QVariant> &data)
{
    itemData = data;
}

int FolderItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<FolderItem *>(this));

    return 0;
}

QList<QVariant> FolderItem::getData() const
{
    return itemData;
}
