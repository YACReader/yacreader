#ifndef READING_LIST_ITEM_H
#define READING_LIST_ITEM_H

#include <QIcon>
#include <QVariant>

class ListItem
{
public:
    ListItem(const QList<QVariant> &data);
    int columnCount();
    virtual QIcon getIcon() const = 0;
    QVariant data(int column) const;
protected:
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
};

//------------------------------------------------------

class ReadingListItem : public ListItem
{
public:
    ReadingListItem(const QList<QVariant> &data, ReadingListItem * parent = 0);
    QIcon getIcon() const;
    int childCount() const;
    ReadingListItem * child(int row);
    void appendChild(ReadingListItem *item);

private:
    QList<ReadingListItem*> childItems;
    ReadingListItem * parent;
};

#endif // READING_LIST_ITEM_H
