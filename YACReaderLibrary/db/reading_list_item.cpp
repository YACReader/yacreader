#include "reading_list_item.h"

#include "QsLog.h"
#include "qnaturalsorting.h"
#include "theme_manager.h"

qulonglong ListItem::getId() const
{
    return 0;
}

//------------------------------------------------------

SpecialListItem::SpecialListItem(const SpecialList &list)
    : list(list)
{
}

QIcon SpecialListItem::getIcon() const
{
    const auto &icons = ThemeManager::instance().getCurrentTheme().readingListIcons;
    switch (list.id) {
    case 0:
        return icons.readingListIcon;
    case 1:
        return icons.favoritesIcon;
    case 2:
        return icons.currentlyReadingIcon;
    default:
        break;
    }

    QLOG_WARN() << "Icon for SpecialListItem not available";

    return QIcon();
}

QString SpecialListItem::name() const
{
    return list.name;
}

qulonglong SpecialListItem::getId() const
{
    return list.id;
}

ReadingListModel::TypeSpecialList SpecialListItem::getType() const
{
    return (ReadingListModel::TypeSpecialList)list.id;
}

//------------------------------------------------------

LabelItem::LabelItem(const Label &label)
    : label(label)
{
}

QIcon LabelItem::getIcon() const
{
    const QString color = label.color.toLower();
    const auto &icons = ThemeManager::instance().getCurrentTheme().readingListIcons;
    if (icons.labelIcons.contains(color)) {
        return icons.labelIcons[color];
    }

    QLOG_WARN() << "Icon for label item not available";

    return QIcon();
}

YACReader::LabelColors LabelItem::colorid() const
{
    return YACReader::LabelColors(label.ordering);
}

QString LabelItem::name() const
{
    return label.name;
}

void LabelItem::setName(const QString &name)
{
    label.name = name;
}

qulonglong LabelItem::getId() const
{
    return label.id.value();
}

//------------------------------------------------------

ReadingListItem::ReadingListItem(const ReadingList &list, ReadingListItem *p)
    : parent(p), list(list)
{
}

// a child list is a list too; folders of lists have their own icon
QIcon ReadingListItem::getIcon() const
{
    return ThemeManager::instance().getCurrentTheme().readingListIcons.listIcon;
}

int ReadingListItem::childCount() const
{
    return childItems.count();
}

ReadingListItem *ReadingListItem::child(int row)
{
    return childItems.at(row);
}

// items are sorted by order
void ReadingListItem::appendChild(ReadingListItem *item)
{
    item->parent = this;

    if (childItems.isEmpty())
        childItems.append(item);
    else {
        if (item->parent->getId() == 0) // sort by name, top level child
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
    return list.id.value();
}

QString ReadingListItem::name() const
{
    return list.name;
}

void ReadingListItem::setName(const QString &name)
{
    list.name = name;
}

int ReadingListItem::getOrdering() const
{
    return list.ordering;
}

void ReadingListItem::setOrdering(const int ordering)
{
    list.ordering = ordering;
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

SmartListItem::SmartListItem(const ::SmartList &list)
    : list(list)
{
}

QIcon SmartListItem::getIcon() const
{
    return ThemeManager::instance().getCurrentTheme().readingListIcons.smartListIcon;
}

QString SmartListItem::name() const
{
    return list.name;
}

qulonglong SmartListItem::getId() const
{
    return list.id.value();
}

//------------------------------------------------------

QIcon ReadingListSeparatorItem::getIcon() const
{
    return QIcon();
}

QString ReadingListSeparatorItem::name() const
{
    return QString();
}
