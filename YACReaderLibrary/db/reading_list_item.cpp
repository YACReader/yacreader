#include "reading_list_item.h"
#include "qnaturalsorting.h"

#include <QFileIconProvider>

#include "QsLog.h"

ListItem::ListItem(const QList<QVariant> &data)
    : itemData(data)
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
    : ListItem(data)
{
}

QIcon SpecialListItem::getIcon() const
{
    if (itemData.count() > Id) {
        QString id = itemData.at(Id).toString();
        return YACReader::noHighlightedIcon(QString(":/images/lists/default_%1.png").arg(id));
    }

    QLOG_WARN() << "Icon for SpecialListItem not available";

    return QIcon();
}

qulonglong SpecialListItem::getId() const
{
    if (itemData.count() > Id) {
        return itemData.at(Id).toInt();
    }

    return 0;
}

ReadingListModel::TypeSpecialList SpecialListItem::getType() const
{
    if (itemData.count() > Id) {
        int id = itemData.at(Id).toInt();
        return (ReadingListModel::TypeSpecialList)id;
    }

    QLOG_WARN() << "TypeSpecialList not available";

    return (ReadingListModel::TypeSpecialList)0;
}

//------------------------------------------------------

LabelItem::LabelItem(const QList<QVariant> &data)
    : ListItem(data)
{
}

QIcon LabelItem::getIcon() const
{
    if (itemData.count() > Color) {
        QString color = itemData.at(Color).toString();
        return YACReader::noHighlightedIcon(QString(":/images/lists/label_%1.png").arg(color).toLower());
    }

    QLOG_WARN() << "Icon for label item not available";

    return QIcon();
}

YACReader::LabelColors LabelItem::colorid() const
{
    if (itemData.count() > Ordering) {
        return YACReader::LabelColors(itemData.at(Ordering).toInt());
    }

    QLOG_WARN() << "Label color for label item not available";

    return (YACReader::LabelColors)0;
}

QString LabelItem::name() const
{
    if (itemData.count() > Name) {
        return itemData.at(Name).toString();
    }

    QLOG_WARN() << "Name for label item not available";

    return "";
}

void LabelItem::setName(const QString &name)
{
    if (itemData.count() > Name) {
        itemData[Name] = name;
    }
}

qulonglong LabelItem::getId() const
{
    if (itemData.count() > Id) {
        return YACReader::LabelColors(itemData.at(Id).toULongLong());
    }

    QLOG_WARN() << "Id for Label item not available";

    return 0;
}

//------------------------------------------------------

ReadingListItem::ReadingListItem(const QList<QVariant> &data, ReadingListItem *p)
    : ListItem(data), parent(p)
{
}

QIcon ReadingListItem::getIcon() const
{
    if (parent->getId() == 0)
        return YACReader::noHighlightedIcon(":/images/lists/list.png"); //top level list
    else
#ifdef Q_OS_MAC
        return QFileIconProvider().icon(QFileIconProvider::Folder);
#else
        return YACReader::noHighlightedIcon(":/images/sidebar/folder.png"); //sublist
#endif
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
    item->parent = this;

    if (childItems.isEmpty())
        childItems.append(item);
    else {
        if (item->parent->getId() == 0) //sort by name, top level child
        {
            int i = 0;
            while (i < childItems.length() && naturalSortLessThanCI(childItems.at(i)->name(), item->name()))
                i++;
            childItems.insert(i, item);
        } else {
            int i = 0;
            while (i < childItems.length() && (childItems.at(i)->getOrdering() < item->getOrdering()))
                i++;
            childItems.insert(i, item);
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

void ReadingListItem::appendChild(ReadingListItem *item, int pos)
{
    childItems.insert(pos, item);
}

void ReadingListItem::removeChild(ReadingListItem *item)
{
    childItems.removeOne(item);
}

qulonglong ReadingListItem::getId() const
{
    if (itemData.count() > Id)
        return itemData.at(Id).toULongLong();

    QLOG_WARN() << "Name for reading list item not available";

    return 0;
}

QString ReadingListItem::name() const
{
    if (itemData.count() > Name)
        return itemData.at(Name).toString();

    QLOG_WARN() << "Name for reading list item not available";

    return "";
}

void ReadingListItem::setName(const QString &name)
{
    if (itemData.count() > Name)
        itemData[Name] = name;
}

int ReadingListItem::getOrdering() const
{
    if (itemData.count() > Ordering)
        return itemData[Ordering].toInt();

    QLOG_WARN() << "Ordering for Item not available";
    return 0;
}

void ReadingListItem::setOrdering(const int ordering)
{
    if (itemData.count() > Ordering)
        itemData[Ordering] = ordering;
}

QList<ReadingListItem *> ReadingListItem::children()
{
    return childItems;
}

int ReadingListItem::row() const
{
    if (parent)
        return parent->childItems.indexOf(const_cast<ReadingListItem *>(this));

    return 0;
}

ReadingListSeparatorItem::ReadingListSeparatorItem()
    : ListItem(QList<QVariant>())
{
}

QIcon ReadingListSeparatorItem::getIcon() const
{
    return QIcon();
}
