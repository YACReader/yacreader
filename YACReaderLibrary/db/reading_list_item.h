#ifndef READING_LIST_ITEM_H
#define READING_LIST_ITEM_H

#include <QIcon>
#include <QVariant>

#include "yacreader_global_gui.h"
#include "reading_list_model.h"
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
    virtual ~ListItem() {}
};

//------------------------------------------------------

class SpecialListItem : public ListItem
{
public:
    SpecialListItem(const QList<QVariant> &data);
    QIcon getIcon() const;
    ReadingListModel::TypeSpecialList getType() const;

private:
    enum DataIndexes {
        Name,
        Id
    };
};

//------------------------------------------------------

class LabelItem : public ListItem
{
public:
    LabelItem(const QList<QVariant> &data);
    QIcon getIcon() const;
    YACReader::LabelColors colorid() const;
    QString name() const;
    void setName(const QString &name);
    qulonglong getId() const;

private:
    enum DataIndexes {
        Name,
        Color,
        Id,
        Ordering
    };
};

//------------------------------------------------------

class ReadingListItem : public ListItem
{
public:
    ReadingListItem(const QList<QVariant> &data, ReadingListItem *parent = nullptr);
    QIcon getIcon() const;
    ReadingListItem *parent;
    int childCount() const;
    int row() const;
    ReadingListItem *child(int row);
    void appendChild(ReadingListItem *item);
    void appendChild(ReadingListItem *item, int pos);
    void removeChild(ReadingListItem *item);
    qulonglong getId() const;
    QString name() const;
    void setName(const QString &name);
    int getOrdering() const;
    void setOrdering(const int ordering);
    QList<ReadingListItem *> children();

private:
    QList<ReadingListItem *> childItems;

    enum DataIndexes {
        Name,
        Id,
        Finished,
        Completed,
        Ordering
    };
};

//------------------------------------------------------

class ReadingListSeparatorItem : public ListItem
{
public:
    ReadingListSeparatorItem();
    QIcon getIcon() const;
};

#endif // READING_LIST_ITEM_H
