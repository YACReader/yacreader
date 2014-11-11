#ifndef READING_LIST_ITEM_H
#define READING_LIST_ITEM_H

#include <QIcon>
#include <QVariant>

#include "yacreader_global.h"
//TODO add propper constructors, using QList<QVariant> is not safe

class ListItem
{
public:
    ListItem(const QList<QVariant> &data);
    int columnCount();
    virtual QIcon getIcon() const = 0;
    QVariant data(int column) const;
    virtual qulonglong getId() const;
    QList<QVariant> itemData;
};

//------------------------------------------------------

class SpecialListItem : public ListItem
{
public:
    SpecialListItem(const QList<QVariant> &data);
    QIcon getIcon() const;
};

//------------------------------------------------------

class LabelItem : public ListItem
{
public:
    LabelItem(const QList<QVariant> &data);
    QIcon getIcon() const;
    YACReader::LabelColors colorid() const;
    QString name() const;
    void setName(const QString & name);
    qulonglong getId() const;

};

//------------------------------------------------------

class ReadingListItem : public ListItem
{
public:
    ReadingListItem(const QList<QVariant> &data, ReadingListItem * parent = 0);
    QIcon getIcon() const;
    ReadingListItem * parent;
    int childCount() const;
    int row() const;
    ReadingListItem * child(int row);
    void appendChild(ReadingListItem *item);
    qulonglong getId() const;
    QString name() const;
    void setName(const QString & name);

private:
    QList<ReadingListItem*> childItems;

};

//------------------------------------------------------

class ReadingListSeparatorItem : public ListItem
{
public:
    ReadingListSeparatorItem();
    QIcon getIcon() const;
};

#endif // READING_LIST_ITEM_H
