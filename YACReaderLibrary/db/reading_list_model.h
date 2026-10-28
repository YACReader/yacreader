#ifndef READING_LIST_MODEL_H
#define READING_LIST_MODEL_H

#include "library_ids.h"
#include "smart_list.h"
#include "yacreader_global.h"

#include <QAbstractItemModel>
#include <QHash>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include <functional>
#include <optional>

class LabelItem;
class ListFolderItem;
class ListItem;
class SmartListItem;
class SpecialListItem;
class ReadingListItem;
class ReadingListSeparatorItem;
struct ReadingList;

class ReadingListModelProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ReadingListModelProxy(QObject *parent = nullptr);
};

class ReadingListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ReadingListModel(QObject *parent = nullptr);
    ~ReadingListModel() override;

    // QAbstractItemModel methods
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QStringList mimeTypes() const override;

    // Convenience methods
    void setupReadingListsData(QString path);
    void addNewLabel(const QString &name, YACReader::LabelColors color);
    // a top level reading list, in `folder` when it is a reading list folder
    void addReadingList(const QString &name, const QModelIndex &folder = QModelIndex());
    // a top level reading list that is already in the database
    void appendReadingList(ReadingListId id, const QString &name);
    void addReadingListAt(const QString &name, const QModelIndex &mi);
    bool isEditable(const QModelIndex &mi);
    bool isReadingList(const QModelIndex &mi);
    bool isReadingSubList(const QModelIndex &mi);
    // in `folder` when it is a smart list folder, else at the top level; the index of its row, invalid when it cannot be
    // stored
    QModelIndex addSmartList(const QString &name, const QString &query, const QModelIndex &folder = QModelIndex());
    // nothing when the row is not a smart list
    std::optional<::SmartList> smartList(const QModelIndex &mi) const;
    bool updateSmartList(const QModelIndex &mi, const QString &name, const QString &query);
    // in `parentFolder` (a folder of `kind`), else at the top level of the section; the index of its row, invalid when
    // it cannot be stored
    QModelIndex addListFolder(ListFolderKind kind, const QModelIndex &parentFolder, const QString &name);
    // a folder or a list (not a child list) into `folder`, a folder of its section, or to the top level of its section
    // when `folder` is invalid; never into its current place, nor a folder into itself or a folder below it
    bool canMoveToFolder(const QModelIndex &mi, const QModelIndex &folder) const;
    bool moveToFolder(const QModelIndex &mi, const QModelIndex &folder);
    // nothing when the row is not a folder of lists
    std::optional<ListFolderId> listFolder(const QModelIndex &mi) const;
    // the section of a list or a folder of lists: reading lists or smart lists; nothing for the other rows
    std::optional<ListFolderKind> section(const QModelIndex &mi) const;
    // the row of a list or a folder of lists (`type` is a TypeList); invalid when there is none
    QModelIndex indexOf(int type, qulonglong id) const;
    QString name(const QModelIndex &mi);
    void rename(const QModelIndex &mi, const QString &name);
    void deleteItem(const QModelIndex &mi);
    const QList<LabelItem *> getLabels();

    enum Roles {
        TypeListsRole = Qt::UserRole + 1,
        IDRole,
        LabelColorRole,
        SpecialListTypeRole
    };

    enum TypeList {
        SpecialList,
        Label,
        ReadingList,
        Separator,
        SmartList,
        ReadingListFolder,
        SmartListFolder
    };

    enum class TypeSpecialList : int {
        Reading = 0,
        Favorites,
        Recent,
    };

signals:

    void addComicsToFavorites(const QList<qulonglong> &comicIds);
    void addComicsToLabel(const QList<ComicRowId> &rows, qulonglong labelId);
    void addComicsToReadingList(const QList<ComicRowId> &rows, qulonglong readingListId);

private:
    void cleanAll();
    void setupReadingListsData(const QList<::ReadingList> &lists, ReadingListItem *parent, const QHash<qulonglong, ListFolderItem *> &folders);
    QList<SpecialListItem *> setupSpecialLists(QSqlDatabase &db);
    void setupLabels(QSqlDatabase &db);
    void setupSmartLists(QSqlDatabase &db);
    void setupReadingLists(QSqlDatabase &db);
    // the root of the section with its folders; the folders by id
    QHash<qulonglong, ListFolderItem *> setupListFolders(ListFolderKind kind, ListFolderItem *root, QSqlDatabase &db);
    // after a load, each folder of a section once
    static void sortByName(ListFolderItem *root, const QHash<qulonglong, ListFolderItem *> &folders);
    int addLabelIntoList(LabelItem *item);
    void reorderingChildren(QList<ReadingListItem *> children);

    // top level rows: special lists, a separator, labels, a separator, the smart lists section and a separator when it
    // has rows, the reading lists section
    int labelsStart() const;
    int smartListsStart() const;
    int readingListsStart() const;
    bool rowIsSpecialList(int row, const QModelIndex &parent = QModelIndex()) const;
    bool rowIsLabel(int row, const QModelIndex &parent = QModelIndex()) const;

    // where an item is: its folder (or the root of its section) for folders, smart lists, and top level reading lists
    ListFolderItem *containerOf(ListItem *item) const;
    QModelIndex indexOfItem(ListItem *item) const;
    // the parent index of the rows of a container, and the row of its first child
    QModelIndex containerIndex(ListFolderItem *container) const;
    int firstRow(ListFolderItem *container) const;
    // the container of `folder` when it is a folder of `kind`, else the root of that section
    ListFolderItem *containerFor(ListFolderKind kind, const QModelIndex &folder) const;
    // `insert` puts the item at `childRow` of the container; the rows are announced around it
    void insertChild(ListFolderItem *container, int childRow, const std::function<void()> &insert);
    void removeChild(ListItem *item);
    // the row of `item` goes to its place by name in `to`, a container of its section (its own container after a rename)
    void moveChild(ListItem *item, ListFolderItem *to);
    bool canMove(ListItem *item, ListFolderItem *to) const;
    // in the database, then in the rows
    bool moveItem(ListItem *item, ListFolderItem *to);

    bool isChildList(const ListItem *item) const;
    // the row a drag of rows carries; nullptr when it is gone
    ListItem *draggedItem(const QMimeData *data) const;
    // `row` -1 drops onto `parent` (empty space when it is invalid), else between the rows of `parent`
    QModelIndexList dropTargets(int row, const QModelIndex &parent) const;
    // where a dropped folder or list goes: onto a folder, inside it; onto a list or between rows, next to that row; on
    // empty space, the top level of its section; nullptr when it cannot go there
    ListFolderItem *dropContainer(ListItem *item, int row, const QModelIndex &parent) const;
    // another child list of the same parent, whose row a dropped child list takes; nullptr when there is none
    ReadingListItem *childListDropTarget(ReadingListItem *item, int row, const QModelIndex &parent) const;
    bool dropComics(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    bool dropSublist(ReadingListItem *item, ReadingListItem *destination);
    // Special lists
    QList<SpecialListItem *> specialLists;

    // Label
    QList<LabelItem *> labels;

    // the roots of the two sections of folders and lists
    ListFolderItem *smartListsRoot = nullptr;
    ListFolderItem *readingListsRoot = nullptr;

    // the parent of every top level reading list (child lists have their parent list)
    ReadingListItem *rootItem; //
    QMap<unsigned long long int, ReadingListItem *> items; // lists relationship

    // separators
    ReadingListSeparatorItem *separator1;
    ReadingListSeparatorItem *separator2;
    ReadingListSeparatorItem *separator3;

    QString _databasePath;
};

Q_DECLARE_METATYPE(ReadingListModel::TypeSpecialList)

#endif // READING_LIST_MODEL_H
