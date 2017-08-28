#include "reading_list.h"

ReadingList::ReadingList(const QString &name, qulonglong id, int ordering)
    :name(name), id(id), ordering(ordering)
{

}

qulonglong ReadingList::getId() const
{
    return id;
}

QString ReadingList::getName() const
{
    return name;
}

int ReadingList::getOrdering() const
{
    return ordering;
}


Label::Label(const QString &name, qulonglong id, YACReader::LabelColors colorid)
    :name(name), id(id), colorid(colorid)
{

}

YACReader::LabelColors Label::getColorID() const
{
    return colorid;
}

QString Label::getName() const
{
    return name;
}

qulonglong Label::getId() const
{
    return id;
}
