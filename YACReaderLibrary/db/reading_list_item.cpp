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
    if(itemData.count()>Id)
    {
        QString id = itemData.at(Id).toString();
        return YACReader::noHighlightedIcon(QString(":/images/lists/default_%1.png").arg(id));
    }
}

ReadingListModel::TypeSpecialList SpecialListItem::getType() const
{
    if(itemData.count()>Id)
    {
        int id = itemData.at(Id).toInt();
        return (ReadingListModel::TypeSpecialList)id;
    }
}

//------------------------------------------------------

LabelItem::LabelItem(const QList<QVariant> &data)
    :ListItem(data)
{

}

QIcon LabelItem::getIcon() const
{
    if(itemData.count()>Color)
    {
        QString color = itemData.at(Color).toString();
        return YACReader::noHighlightedIcon(QString(":/images/lists/label_%1.png").arg(color).toLower());
    }
}

YACReader::LabelColors LabelItem::colorid() const
{
    if(itemData.count()>Ordering)
    {
        return YACReader::LabelColors(itemData.at(Ordering).toInt());
    }
}

QString LabelItem::name() const
{
    if(itemData.count()>Name)
    {
        return itemData.at(Name).toString();
    }
}

void LabelItem::setName(const QString &name)
{
    if(itemData.count()>Name)
    {
        itemData[Name] = name;
    }
}

qulonglong LabelItem::getId() const
{
    if(itemData.count()>Id)
    {
        return YACReader::LabelColors(itemData.at(Id).toULongLong());
    }
}

//------------------------------------------------------

ReadingListItem::ReadingListItem(const QList<QVariant> &data, ReadingListItem *p)
    :ListItem(data), parent(p)
{

}

QIcon ReadingListItem::getIcon() const
{
    if(parent->getId() == 0)
        return YACReader::noHighlightedIcon(":/images/lists/list.png"); //top level list
    else
        return YACReader::noHighlightedIcon(":/images/folder.png"); //sublist
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
    item->parent = this;
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

void ReadingListItem::removeChild(ReadingListItem *item)
{
    childItems.removeOne(item);
}

qulonglong ReadingListItem::getId() const
{
    if(itemData.count()>Id)
        return itemData.at(Id).toULongLong();
}

QString ReadingListItem::name() const
{
    if(itemData.count()>Name)
        return itemData.at(Name).toString();
}

void ReadingListItem::setName(const QString &name)
{
    if(itemData.count()>Name)
        itemData[Name] = name;
}

int ReadingListItem::getOrdering() const
{
    if(itemData.count()>Ordering)
        return itemData[Ordering].toInt();
}

void ReadingListItem::setOrdering(const int ordering)
{
    if(itemData.count()>Ordering)
        itemData[Ordering] = ordering;
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
