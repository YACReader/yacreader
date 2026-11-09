#include "folder_item.h"

#include "qnaturalsorting.h"

#include <QStringList>

FolderItem::FolderItem(const Folder &folder, FolderItem *parent)
    : parentItem(parent), folder(folder)
{
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
        QString nameLast = last->folder.name;
        QString nameCurrent = item->folder.name;
        QList<FolderItem *>::iterator i;
        i = childItems.end();
        i--;
        while (naturalSortLessThanCI(nameCurrent, nameLast) && i != childItems.begin()) {
            i--;
            nameLast = (*i)->folder.name;
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

QList<FolderItem *> FolderItem::children()
{
    return childItems;
}

FolderItem *FolderItem::parent()
{
    return parentItem;
}

int FolderItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<FolderItem *>(this));

    return 0;
}
