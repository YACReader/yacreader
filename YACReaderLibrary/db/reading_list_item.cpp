#include "reading_list_item.h"

ListItem::ListItem(const QList<QVariant> &data)
    :itemData(data)
{

}

int ListItem::columnCount()
{
    return itemData.count();
}

QVariant ListItem::data(int column) const
{
    return itemData.at(column);
}

qulonglong ListItem::getId() const
{
    return 0;
}

//------------------------------------------------------

SpecialListItem::SpecialListItem(const QList<QVariant> &data)
    :ListItem(data)
{

}

QIcon SpecialListItem::getIcon() const
{
    if(itemData.count()>1)
    {
        QString id = itemData.at(1).toString();
        return QIcon(QString(":/images/lists/default_%1.png").arg(id));
    }
}

//------------------------------------------------------

LabelItem::LabelItem(const QList<QVariant> &data)
    :ListItem(data)
{

}

QIcon LabelItem::getIcon() const
{
    if(itemData.count()>1)
    {
        QString color = itemData.at(1).toString();
        return QIcon(QString(":/images/lists/label_%1.png").arg(color).toLower());
    }
}

YACReader::LabelColors LabelItem::colorid() const
{
    if(itemData.count()>3)
    {
        return YACReader::LabelColors(itemData.at(3).toInt());
    }
}

QString LabelItem::name() const
{
    if(itemData.count()>0)
    {
        return itemData.at(0).toString();
    }
}

void LabelItem::setName(const QString &name)
{
    itemData[0] = name;
}

qulonglong LabelItem::getId() const
{
    if(itemData.count()>2)
    {
        return YACReader::LabelColors(itemData.at(2).toULongLong());
    }
}

//------------------------------------------------------

ReadingListItem::ReadingListItem(const QList<QVariant> &data, ReadingListItem *p)
    :ListItem(data), parent(p)
{

}

QIcon ReadingListItem::getIcon() const
{
    if(parent == 0)
        return QIcon(":/images/lists/list.png");
    else
        return QIcon(":/images/folder.png");
}

int ReadingListItem::childCount() const
{
    return childItems.count();
}

ReadingListItem *ReadingListItem::child(int row)
{
    return childItems.at(row);
}

//items are sorted by order
void ReadingListItem::appendChild(ReadingListItem *item)
{
    childItems.append(item);
    return; //TODO

    item->parent = this;

    if(childItems.isEmpty())
        childItems.append(item);
    else
    {
        if(item->parent->getId()==0) //sort by name, top level child
        {

        }
        else
        {

        }

        /*ReadingListItem * last = childItems.back();
        QString nameLast = last->data(1).toString(); //TODO usar info name si est� disponible, sino el nombre del fichero.....
        QString nameCurrent = item->data(1).toString();
        QList<FolderItem *>::iterator i;
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
            childItems.insert(i,item);*/

    }

}

qulonglong ReadingListItem::getId() const
{
    if(itemData.count()>1)
    {
        return YACReader::LabelColors(itemData.at(1).toULongLong());
    }
}

QString ReadingListItem::name() const
{
    if(itemData.count()>0)
    {
        return itemData.at(0).toString();
    }
}

void ReadingListItem::setName(const QString &name)
{
    itemData[0] = name;
}

QList<ReadingListItem *> ReadingListItem::children()
{
    return childItems;
}

int ReadingListItem::row() const
{
    if (parent)
        return parent->childItems.indexOf(const_cast<ReadingListItem*>(this));

    return 0;
}


ReadingListSeparatorItem::ReadingListSeparatorItem()
    :ListItem(QList<QVariant>())
{

}

QIcon ReadingListSeparatorItem::getIcon() const
{
    return QIcon();
}
