#ifndef READING_LIST_ITEM_H
#define READING_LIST_ITEM_H

#include "reading_list.h"
#include "reading_list_model.h"

#include <QIcon>
#include <QString>

// Favorites (default_reading_list), Reading, and Recent: not rows of a table type
struct SpecialList {
    qulonglong id;
    QString name;
};

class ListItem
{
public:
    virtual QIcon getIcon() const = 0;
    virtual QString name() const = 0;
    virtual qulonglong getId() const;
    virtual ~ListItem() { }
};

class SpecialListItem : public ListItem
{
public:
    explicit SpecialListItem(const SpecialList &list);
    QIcon getIcon() const override;
    QString name() const override;
    qulonglong getId() const override;
    ReadingListModel::TypeSpecialList getType() const;

    SpecialList list;
};

class LabelItem : public ListItem
{
public:
    explicit LabelItem(const Label &label);
    QIcon getIcon() const override;
    YACReader::LabelColors colorid() const;
    QString name() const override;
    void setName(const QString &name);
    qulonglong getId() const override;

    Label label;
};

class ReadingListItem : public ListItem
{
public:
    explicit ReadingListItem(const ReadingList &list, ReadingListItem *parent = nullptr);
    QIcon getIcon() const override;
    ReadingListItem *parent;
    int childCount() const;
    int row() const;
    ReadingListItem *child(int row);
    void appendChild(ReadingListItem *item);
    void appendChild(ReadingListItem *item, int pos);
    void removeChild(ReadingListItem *item);
    qulonglong getId() const override;
    QString name() const override;
    void setName(const QString &name);
    int getOrdering() const;
    void setOrdering(const int ordering);
    QList<ReadingListItem *> children();

    ReadingList list;

private:
    QList<ReadingListItem *> childItems;
};

class ReadingListSeparatorItem : public ListItem
{
public:
    QIcon getIcon() const override;
    QString name() const override;
};

#endif // READING_LIST_ITEM_H
