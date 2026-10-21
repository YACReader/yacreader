#ifndef READING_LIST_ITEM_H
#define READING_LIST_ITEM_H

#include "list_folder.h"
#include "reading_list.h"
#include "reading_list_model.h"
#include "smart_list.h"

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

class ListFolderItem;

class ReadingListItem : public ListItem
{
public:
    explicit ReadingListItem(const ReadingList &list, ReadingListItem *parent = nullptr);
    QIcon getIcon() const override;
    ReadingListItem *parent;
    // the folder (or the root of the reading lists section) that holds a top level list; nothing for a child list
    ListFolderItem *container = nullptr;
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

class SmartListItem : public ListItem
{
public:
    explicit SmartListItem(const ::SmartList &list);
    QIcon getIcon() const override;
    QString name() const override;
    qulonglong getId() const override;

    ::SmartList list;
    // the folder, or the root of the smart lists section
    ListFolderItem *container = nullptr;
};

// a folder of lists, or the root of a section (no id, not a row); its children are its folders, then its lists. It
// deletes what it holds.
class ListFolderItem : public ListItem
{
public:
    explicit ListFolderItem(const ListFolder &folder);
    ~ListFolderItem() override;
    QIcon getIcon() const override;
    QString name() const override;
    qulonglong getId() const override;

    int childCount() const;
    ListItem *child(int row) const;
    // -1 when it is not a child
    int rowOf(const ListItem *child) const;
    // among its folders, or among its lists, by name; gives its row
    int insertFolderByName(ListFolderItem *item);
    int insertListByName(ListItem *item);
    // the row a folder or a list with that name gets, with `ignored` left out
    int rowForFolderName(const QString &name, const ListItem *ignored = nullptr) const;
    int rowForListName(const QString &name, const ListItem *ignored = nullptr) const;
    void removeChild(ListItem *item);

    ListFolder folder;
    // the folder that holds it; nothing for the root of a section
    ListFolderItem *container = nullptr;
    QList<ListFolderItem *> folders;
    // ReadingListItem or SmartListItem, of the kind of the folder
    QList<ListItem *> lists;
};

class ReadingListSeparatorItem : public ListItem
{
public:
    QIcon getIcon() const override;
    QString name() const override;
};

#endif // READING_LIST_ITEM_H
