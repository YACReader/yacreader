#include "reading_list_model.h"

#include "QsLog.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "list_folder.h"
#include "qnaturalsorting.h"
#include "reading_list_item.h"
#include "smart_list.h"
#include "table_projection.h"
#include "yacreader_global_gui.h"

#include <QSqlRecord>

#include <functional>

ReadingListModel::ReadingListModel(QObject *parent)
    : QAbstractItemModel(parent), rootItem(0)
{
    separator1 = new ReadingListSeparatorItem;
    separator2 = new ReadingListSeparatorItem;
    separator3 = new ReadingListSeparatorItem;
}

ReadingListModel::~ReadingListModel()
{
    cleanAll();
    delete separator1;
    delete separator2;
    delete separator3;
}

int ReadingListModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) // TOP
    {
        return readingListsRoot == nullptr ? 0 : readingListsStart() + readingListsRoot->childCount();
    } else {
        auto item = static_cast<ListItem *>(parent.internalPointer());

        if (typeid(*item) == typeid(ReadingListItem)) {
            auto item = static_cast<ReadingListItem *>(parent.internalPointer());
            return item->childCount();
        }

        if (typeid(*item) == typeid(ListFolderItem))
            return static_cast<ListFolderItem *>(item)->childCount();
    }

    return 0;
}

int ReadingListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        auto item = static_cast<ListItem *>(parent.internalPointer());
        if (typeid(*item) == typeid(ReadingListSeparatorItem))
            return 0;
    }
    return 1;
    /*if (parent.isValid())
        return static_cast<ListItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();*/
}

QVariant ReadingListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto item = static_cast<ListItem *>(index.internalPointer());

    if (role == ReadingListModel::TypeListsRole) {
        if (typeid(*item) == typeid(SpecialListItem))
            return QVariant(ReadingListModel::SpecialList);

        if (typeid(*item) == typeid(LabelItem))
            return QVariant(ReadingListModel::Label);

        if (typeid(*item) == typeid(ReadingListItem))
            return QVariant(ReadingListModel::ReadingList);

        if (typeid(*item) == typeid(SmartListItem))
            return QVariant(ReadingListModel::SmartList);

        if (typeid(*item) == typeid(ListFolderItem))
            return QVariant(static_cast<ListFolderItem *>(item)->folder.id.kind == ListFolderKind::ReadingList ? ReadingListModel::ReadingListFolder : ReadingListModel::SmartListFolder);

        if (typeid(*item) == typeid(ReadingListSeparatorItem))
            return QVariant(ReadingListModel::Separator);
    }

    if (role == ReadingListModel::LabelColorRole && typeid(*item) == typeid(LabelItem)) {
        auto labelItem = static_cast<LabelItem *>(item);
        return QVariant(labelItem->colorid());
    }

    if (role == ReadingListModel::IDRole) {
        QLOG_DEBUG() << "getting role";
        return item->getId();
    }

    if (role == ReadingListModel::SpecialListTypeRole && typeid(*item) == typeid(SpecialListItem)) {
        auto specialListItem = static_cast<SpecialListItem *>(item);
        return QVariant::fromValue(specialListItem->getType());
    }

    if (typeid(*item) == typeid(ReadingListSeparatorItem))
        return QVariant();

    if (role == Qt::DecorationRole) {
        return QVariant(item->getIcon());
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    return index.column() == 0 ? QVariant(item->name()) : QVariant();
}

Qt::ItemFlags ReadingListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return { };

    auto item = static_cast<ListItem *>(index.internalPointer());
    if (typeid(*item) == typeid(ReadingListSeparatorItem))
        return { };

    if (typeid(*item) == typeid(ReadingListItem) && static_cast<ReadingListItem *>(item)->parent->getId() != 0)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled; // only sublists are dragable

    // the comics of a smart list come from its search; a folder holds lists, not comics
    if (typeid(*item) == typeid(SmartListItem) || typeid(*item) == typeid(ListFolderItem))
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
}

QVariant ReadingListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return section == 0 ? QVariant(rootItem->name()) : QVariant();

    return QVariant();
}

QModelIndex ReadingListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        if (rowIsSpecialList(row, parent))
            return createIndex(row, column, specialLists.at(row));

        if (row == specialLists.count())
            return createIndex(row, column, separator1);

        if (rowIsLabel(row, parent))
            return createIndex(row, column, labels.at(row - labelsStart()));

        if (row == labelsStart() + labels.count())
            return createIndex(row, column, separator2);

        if (row >= smartListsStart() && row < smartListsStart() + smartListsRoot->childCount())
            return createIndex(row, column, smartListsRoot->child(row - smartListsStart()));

        if (smartListsRoot->childCount() > 0 && row == smartListsStart() + smartListsRoot->childCount())
            return createIndex(row, column, separator3);

        if (row >= readingListsStart())
            return createIndex(row, column, readingListsRoot->child(row - readingListsStart()));

        return QModelIndex();
    }

    auto parentItem = static_cast<ListItem *>(parent.internalPointer());
    if (typeid(*parentItem) == typeid(ListFolderItem))
        return createIndex(row, column, static_cast<ListFolderItem *>(parentItem)->child(row));

    // sublist
    return createIndex(row, column, static_cast<ReadingListItem *>(parentItem)->child(row));
}

QModelIndex ReadingListModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    auto item = static_cast<ListItem *>(index.internalPointer());

    if (typeid(*item) == typeid(ReadingListItem)) {
        auto childItem = static_cast<ReadingListItem *>(item);
        if (childItem->parent != rootItem)
            return indexOfItem(childItem->parent);
    }

    if (auto *container = containerOf(item); container != nullptr && container->container != nullptr)
        return indexOfItem(container);

    return QModelIndex();
}

bool ReadingListModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(action);
    Q_UNUSED(column);

    QLOG_DEBUG() << "trying to drop into row = " << row << "column column = " << column << "parent" << parent;

    if (row == -1)
        return false;

    const auto target = index(row, 0, parent);
    if (!target.isValid())
        return false;
    auto item = static_cast<ListItem *>(target.internalPointer());

    if (data->formats().contains(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat)) {
        // Reading and Recent are filled by the library
        if (typeid(*item) == typeid(SpecialListItem))
            return static_cast<SpecialListItem *>(item)->getType() == TypeSpecialList::Favorites;
        return typeid(*item) == typeid(LabelItem) || typeid(*item) == typeid(ReadingListItem);
    }

    // sublists are reordered inside their parent list
    if (typeid(*item) == typeid(ReadingListItem) && parent.isValid() && typeid(*static_cast<ListItem *>(parent.internalPointer())) == typeid(ReadingListItem)) {
        QList<QPair<int, int>> sublistsRows;
        QByteArray rawData = data->data(YACReader::YACReaderLibrarSubReadingListMimeDataFormat);
        QDataStream in(&rawData, QIODevice::ReadOnly);
        in >> sublistsRows; // deserialize the list of indentifiers

        if (sublistsRows.isEmpty())
            return false;

        if (parent.row() != sublistsRows.at(0).second)
            return false;

        return data->formats().contains(YACReader::YACReaderLibrarSubReadingListMimeDataFormat);
    }

    return false;
}

bool ReadingListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    QLOG_DEBUG() << "drop mimedata into row = " << row << " column = " << column << "parent" << parent;
    if (data->formats().contains(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat))
        return dropComics(data, action, row, column, parent);

    if (data->formats().contains(YACReader::YACReaderLibrarSubReadingListMimeDataFormat))
        return dropSublist(data, action, row, column, parent);

    return false;
}

bool ReadingListModel::dropComics(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action);

    QList<qulonglong> comicIds = YACReader::mimeDataToComicsIds(data);

    QLOG_DEBUG() << "dropped : " << comicIds;

    const auto dest = row == -1 ? parent : index(row, column, parent);
    if (!dest.isValid())
        return false;
    auto item = static_cast<ListItem *>(dest.internalPointer());

    if (typeid(*item) == typeid(SpecialListItem) && static_cast<SpecialListItem *>(item)->getType() == TypeSpecialList::Favorites) {
        QLOG_DEBUG() << "-------addComicsToFavorites : " << comicIds << " to " << dest.data(IDRole).toULongLong();
        emit addComicsToFavorites(comicIds);
        return true;
    }

    if (typeid(*item) == typeid(LabelItem)) {
        emit addComicsToLabel(YACReader::mimeDataToRows(data), dest.data(IDRole).toULongLong());
        return true;
    }

    if (typeid(*item) == typeid(ReadingListItem)) {
        emit addComicsToReadingList(YACReader::mimeDataToRows(data), dest.data(IDRole).toULongLong());
        return true;
    }

    return false;
}

bool ReadingListModel::dropSublist(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action);
    Q_UNUSED(column);

    QList<QPair<int, int>> sublistsRows;
    QByteArray rawData = data->data(YACReader::YACReaderLibrarSubReadingListMimeDataFormat);
    QDataStream in(&rawData, QIODevice::ReadOnly);
    in >> sublistsRows; // deserialize the list of indentifiers

    QLOG_DEBUG() << "dropped : " << sublistsRows;

    int sourceRow = sublistsRows.at(0).first;
    int destRow = row;
    QModelIndex destParent = parent;
    if (row == -1) {
        QLOG_DEBUG() << "droping inside parent";
        destRow = parent.row();
        destParent = parent.parent();
    }
    QLOG_DEBUG() << "move " << sourceRow << "-" << destRow;

    if (sourceRow == destRow)
        return false;

    // beginMoveRows(destParent,sourceRow,sourceRow,destParent,destRow);

    auto parentItem = static_cast<ReadingListItem *>(destParent.internalPointer());
    ReadingListItem *child = parentItem->child(sourceRow);
    parentItem->removeChild(child);
    parentItem->appendChild(child, destRow);

    reorderingChildren(parentItem->children());
    // endMoveRows();

    return true;
}

QMimeData *ReadingListModel::mimeData(const QModelIndexList &indexes) const
{
    QLOG_DEBUG() << "mimeData requested" << indexes;

    if (indexes.length() == 0) {
        QLOG_ERROR() << "mimeData requested: indexes is empty";
        return new QMimeData(); // TODO what happens if 0 is returned?
    }

    if (indexes.length() > 1) {
        QLOG_DEBUG() << "mimeData requested for more than one index, this shouldn't be possible";
    }

    QModelIndex modelIndex = indexes.at(0);

    QList<QPair<int, int>> rows;
    rows << QPair<int, int>(modelIndex.row(), modelIndex.parent().row());
    QLOG_DEBUG() << "mimeData requested for row : " << modelIndex.row();

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << rows; // serialize the list of identifiers

    auto mimeData = new QMimeData();
    mimeData->setData(YACReader::YACReaderLibrarSubReadingListMimeDataFormat, data);

    return mimeData;
}

void ReadingListModel::setupReadingListsData(QString path)
{
    beginResetModel();

    cleanAll();

    _databasePath = path;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(path);

        // setup special lists
        specialLists = setupSpecialLists(db);

        // separator--------------------------------------------

        // setup labels
        setupLabels(db);

        // separator--------------------------------------------

        setupSmartLists(db);

        // separator, when there are smart lists----------------

        // setup reading list
        setupReadingLists(db);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    endResetModel();
}

void ReadingListModel::addNewLabel(const QString &name, YACReader::LabelColors color)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        qulonglong id = DBHelper::insertLabel(name, color, db);

        int newPos = addLabelIntoList(new LabelItem(::Label { LabelId(id), name, YACReader::colorToName(color), int(color) }));
        beginInsertRows(QModelIndex(), specialLists.count() + 1 + newPos + 1, specialLists.count() + 1 + newPos + 1);

        endInsertRows();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void ReadingListModel::addReadingList(const QString &name, const QModelIndex &folder)
{
    auto *container = containerFor(ListFolderKind::ReadingList, folder);
    std::optional<ReadingListId> id;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        id = ReadingListId(DBHelper::insertReadingList(name, db));
        if (container != readingListsRoot && !DBHelper::moveReadingListToFolder(*id, container->getId(), db))
            container = readingListsRoot;
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    ::ReadingList list;
    list.id = *id;
    list.name = name;
    if (container != readingListsRoot)
        list.folderId = container->getId();
    auto newItem = new ReadingListItem(list, rootItem);
    newItem->container = container;
    insertChild(container, container->rowForListName(name), [container, newItem] { container->insertListByName(newItem); });
    items.insert(id->value(), newItem);
}

void ReadingListModel::appendReadingList(ReadingListId id, const QString &name)
{
    ::ReadingList list;
    list.id = id;
    list.name = name;
    auto newItem = new ReadingListItem(list, rootItem);
    newItem->container = readingListsRoot;
    insertChild(readingListsRoot, readingListsRoot->rowForListName(name), [this, newItem] { readingListsRoot->insertListByName(newItem); });
    items.insert(id.value(), newItem);
}

void ReadingListModel::addReadingListAt(const QString &name, const QModelIndex &mi)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        beginInsertRows(mi, 0, 0); // TODO calculate the right coordinates before inserting

        auto readingListParent = static_cast<ReadingListItem *>(mi.internalPointer());
        qulonglong id = DBHelper::insertReadingSubList(name, mi.data(IDRole).toULongLong(), readingListParent->childCount(), db);
        ReadingListItem *newItem;

        ::ReadingList list;
        list.id = ReadingListId(id);
        list.parentId = ReadingListId(readingListParent->getId());
        list.name = name;
        list.ordering = readingListParent->childCount();
        readingListParent->appendChild(newItem = new ReadingListItem(list));

        items.insert(id, newItem);
        endInsertRows();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

bool ReadingListModel::isEditable(const QModelIndex &mi)
{
    if (!mi.isValid())
        return false;
    auto item = static_cast<ListItem *>(mi.internalPointer());
    return typeid(*item) == typeid(LabelItem) || typeid(*item) == typeid(ReadingListItem) || typeid(*item) == typeid(SmartListItem) ||
            typeid(*item) == typeid(ListFolderItem);
}

QModelIndex ReadingListModel::addSmartList(const QString &name, const QString &query, const QModelIndex &folder)
{
    auto *container = containerFor(ListFolderKind::SmartList, folder);
    std::optional<SmartListId> id;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        id = DBHelper::insertSmartList(name, query, db);
        if (id && container != smartListsRoot && !DBHelper::moveSmartListToFolder(*id, container->getId(), db))
            container = smartListsRoot;
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    if (!id)
        return { };

    ::SmartList list;
    list.id = *id;
    list.name = name;
    list.query = query;
    if (container != smartListsRoot)
        list.folderId = container->getId();
    auto newItem = new SmartListItem(list);
    newItem->container = container;
    insertChild(container, container->childCount(), [container, newItem] { container->appendList(newItem); });
    return indexOfItem(newItem);
}

QModelIndex ReadingListModel::addListFolder(ListFolderKind kind, const QModelIndex &parentFolder, const QString &name)
{
    auto *container = containerFor(kind, parentFolder);
    const auto root = kind == ListFolderKind::ReadingList ? readingListsRoot : smartListsRoot;
    std::optional<ListFolderId> id;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        id = DBHelper::insertListFolder(kind, container == root ? std::nullopt : std::optional(container->getId()), name, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    if (!id)
        return { };

    ListFolder folder;
    folder.id = *id;
    if (container != root)
        folder.parentId = container->getId();
    folder.name = name;
    auto newItem = new ListFolderItem(folder);
    insertChild(container, container->folders.count(), [container, newItem] { container->appendFolder(newItem); });
    return indexOfItem(newItem);
}

std::optional<ListFolderId> ReadingListModel::listFolder(const QModelIndex &mi) const
{
    if (!mi.isValid())
        return std::nullopt;
    auto item = static_cast<ListItem *>(mi.internalPointer());
    if (typeid(*item) != typeid(ListFolderItem))
        return std::nullopt;
    return static_cast<ListFolderItem *>(item)->folder.id;
}

std::optional<ListFolderKind> ReadingListModel::section(const QModelIndex &mi) const
{
    if (!mi.isValid())
        return std::nullopt;
    auto item = static_cast<ListItem *>(mi.internalPointer());
    if (typeid(*item) == typeid(ReadingListItem))
        return ListFolderKind::ReadingList;
    if (typeid(*item) == typeid(SmartListItem))
        return ListFolderKind::SmartList;
    if (typeid(*item) == typeid(ListFolderItem))
        return static_cast<ListFolderItem *>(item)->folder.id.kind;
    return std::nullopt;
}

std::optional<::SmartList> ReadingListModel::smartList(const QModelIndex &mi) const
{
    if (!mi.isValid())
        return std::nullopt;
    auto item = static_cast<ListItem *>(mi.internalPointer());
    if (typeid(*item) != typeid(SmartListItem))
        return std::nullopt;
    return static_cast<SmartListItem *>(item)->list;
}

bool ReadingListModel::updateSmartList(const QModelIndex &mi, const QString &name, const QString &query)
{
    if (!smartList(mi))
        return false;

    auto item = static_cast<SmartListItem *>(mi.internalPointer());
    bool updated = false;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        updated = DBHelper::updateSmartList(item->list.id, name, query, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    if (!updated)
        return false;

    item->list.name = name;
    item->list.query = query;
    emit dataChanged(mi, mi);
    return true;
}

bool ReadingListModel::isReadingList(const QModelIndex &mi)
{
    if (!mi.isValid())
        return false;
    auto item = static_cast<ListItem *>(mi.internalPointer());
    return typeid(*item) == typeid(ReadingListItem);
}

bool ReadingListModel::isReadingSubList(const QModelIndex &mi)
{
    if (!mi.isValid())
        return false;
    auto item = static_cast<ListItem *>(mi.internalPointer());
    if (typeid(*item) == typeid(ReadingListItem)) {
        auto readingListItem = static_cast<ReadingListItem *>(item);
        if (readingListItem->parent == rootItem)
            return false;
        else
            return true;
    } else
        return false;
}

QString ReadingListModel::name(const QModelIndex &mi)
{
    return data(mi, Qt::DisplayRole).toString();
}

void ReadingListModel::rename(const QModelIndex &mi, const QString &name)
{
    if (!isEditable(mi))
        return;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        auto item = static_cast<ListItem *>(mi.internalPointer());

        if (typeid(*item) == typeid(ReadingListItem)) {
            auto rli = static_cast<ReadingListItem *>(item);
            rli->setName(name);
            DBHelper::renameList(item->getId(), name, db);

            if (rli->parent->getId() != 0) {
                // TODO
                // move row depending on the name
            } else
                emit dataChanged(mi, mi);
        } else if (typeid(*item) == typeid(LabelItem)) {
            auto li = static_cast<LabelItem *>(item);
            li->setName(name);
            DBHelper::renameLabel(item->getId(), name, db);
            emit dataChanged(index(mi.row(), 0), index(mi.row(), 0));
        } else if (typeid(*item) == typeid(ListFolderItem)) {
            auto folder = static_cast<ListFolderItem *>(item);
            if (DBHelper::renameListFolder(folder->folder.id, name, db)) {
                folder->folder.name = name;
                emit dataChanged(mi, mi);
            }
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void ReadingListModel::deleteItem(const QModelIndex &mi)
{
    if (!isEditable(mi))
        return;

    auto item = static_cast<ListItem *>(mi.internalPointer());
    std::optional<QStringList> placeholderCovers;
    bool removed = false;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        if (typeid(*item) == typeid(SmartListItem)) {
            removed = DBHelper::removeSmartList(static_cast<SmartListItem *>(item)->list.id, db);
        } else if (typeid(*item) == typeid(ListFolderItem)) {
            placeholderCovers = DBHelper::removeListFolder(static_cast<ListFolderItem *>(item)->folder.id, db);
            removed = placeholderCovers.has_value();
        } else if (typeid(*item) == typeid(ReadingListItem)) {
            placeholderCovers = DBHelper::removeListFromDB(item->getId(), db);
            removed = placeholderCovers.has_value();
        } else if (typeid(*item) == typeid(LabelItem)) {
            placeholderCovers = DBHelper::removeLabelFromDB(item->getId(), db);
            removed = placeholderCovers.has_value();
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    if (placeholderCovers)
        DBHelper::removePlaceholderCovers(_databasePath, *placeholderCovers);
    if (!removed)
        return;

    if (typeid(*item) == typeid(LabelItem)) {
        beginRemoveRows(mi.parent(), mi.row(), mi.row());
        labels.removeOne(static_cast<LabelItem *>(item));
        endRemoveRows();
        delete item;
        return;
    }

    if (typeid(*item) == typeid(ReadingListItem)) {
        auto rli = static_cast<ReadingListItem *>(item);
        if (rli->parent != rootItem) {
            beginRemoveRows(mi.parent(), mi.row(), mi.row());
            rli->parent->removeChild(rli);
            reorderingChildren(rli->parent->children());
            endRemoveRows();
            return;
        }
    }

    removeChild(item);
}

const QList<LabelItem *> ReadingListModel::getLabels()
{
    return labels;
}

void ReadingListModel::cleanAll()
{
    if (rootItem != 0) {
        delete rootItem;
        delete smartListsRoot;
        delete readingListsRoot;

        qDeleteAll(specialLists);
        qDeleteAll(labels);

        specialLists.clear();
        labels.clear();

        items.clear();
    }

    rootItem = 0;
    smartListsRoot = nullptr;
    readingListsRoot = nullptr;
}

void ReadingListModel::setupReadingListsData(QSqlQuery &sqlquery, ReadingListItem *parent, const QHash<qulonglong, ListFolderItem *> &folders)
{
    items.insert(parent->getId(), parent);

    const auto indexes = ::ReadingList::columnIndexes(sqlquery.record(), "rl");
    while (sqlquery.next()) {
        ReadingListItem *rli = new ReadingListItem(::ReadingList::fromQuery(sqlquery, indexes));

        if (!rli->list.parentId) {
            // a top level list is in its folder, or at the top level of the section
            rli->parent = rootItem;
            rli->container = rli->list.folderId ? folders.value(*rli->list.folderId, readingListsRoot) : readingListsRoot;
            rli->container->insertListByName(rli);
        } else {
            items.value(rli->list.parentId->value())->appendChild(rli);
        }

        items.insert(rli->getId(), rli);
    }
}

QList<SpecialListItem *> ReadingListModel::setupSpecialLists(QSqlDatabase &db)
{
    QList<SpecialListItem *> list;

    QSqlQuery selectQuery("SELECT * FROM default_reading_list ORDER BY id,name", db);

    QSqlRecord record = selectQuery.record();

    int name = record.indexOf("name");
    int id = record.indexOf("id");

    while (selectQuery.next()) {
        list << new SpecialListItem(::SpecialList { selectQuery.value(id).toULongLong(), selectQuery.value(name).toString() });
    }

    list.insert(1, new SpecialListItem(::SpecialList { 0, "Reading" }));
    list.insert(2, new SpecialListItem(::SpecialList { 2, "Recent" }));

    return list;
}

void ReadingListModel::setupLabels(QSqlDatabase &db)
{
    QSqlQuery selectQuery("SELECT " + TableProjection::selectList("l", ::Label::columns()) + " FROM label l ORDER BY l.ordering, l.name", db);

    const auto indexes = ::Label::columnIndexes(selectQuery.record(), "l");
    while (selectQuery.next()) {
        addLabelIntoList(new LabelItem(::Label::fromQuery(selectQuery, indexes)));
    }

    // TEST

    //    INSERT INTO label (name, color, ordering) VALUES ("Oh Oh", "red", 1);
    //    INSERT INTO label (name, color, ordering) VALUES ("lalala", "orange", 2);
    //    INSERT INTO label (name, color, ordering) VALUES ("we are not sorry", "yellow", 3);
    //    INSERT INTO label (name, color, ordering) VALUES ("there we go", "green", 4);
    //    INSERT INTO label (name, color, ordering) VALUES ("oklabunga", "cyan", 5);
    //    INSERT INTO label (name, color, ordering) VALUES ("hailer mailer", "blue", 6);
    //    INSERT INTO label (name, color, ordering) VALUES ("lol", "violet", 7);
    //    INSERT INTO label (name, color, ordering) VALUES ("problems", "purple", 8);
    //    INSERT INTO label (name, color, ordering) VALUES ("me gussssta", "pink", 9);
    //    INSERT INTO label (name, color, ordering) VALUES (":D", "white", 10);
    //    INSERT INTO label (name, color, ordering) VALUES ("ainsss", "light", 11);
    //    INSERT INTO label (name, color, ordering) VALUES ("put a smile on my face", "dark", 12);
}

void ReadingListModel::setupSmartLists(QSqlDatabase &db)
{
    smartListsRoot = new ListFolderItem(ListFolder { { ListFolderKind::SmartList, 0 }, std::nullopt, QString(), 0 });
    const auto folders = setupListFolders(ListFolderKind::SmartList, smartListsRoot, db);

    if (const auto lists = DBHelper::loadSmartLists(db)) {
        for (const auto &list : *lists) {
            auto item = new SmartListItem(list);
            item->container = list.folderId ? folders.value(*list.folderId, smartListsRoot) : smartListsRoot;
            item->container->appendList(item);
        }
    }
}

QHash<qulonglong, ListFolderItem *> ReadingListModel::setupListFolders(ListFolderKind kind, ListFolderItem *root, QSqlDatabase &db)
{
    QHash<qulonglong, ListFolderItem *> folders;
    const auto loaded = DBHelper::loadListFolders(kind, db);
    if (!loaded)
        return folders;

    // in their order, so every folder holds its folders in that order
    for (const auto &folder : *loaded)
        folders.insert(folder.id.id, new ListFolderItem(folder));
    for (const auto &folder : *loaded) {
        auto *container = folder.parentId ? folders.value(*folder.parentId, root) : root;
        container->appendFolder(folders.value(folder.id.id));
    }
    return folders;
}

void ReadingListModel::setupReadingLists(QSqlDatabase &db)
{
    // setup root item
    ::ReadingList root;
    root.name = "ROOT";
    root.finished = true;
    root.completed = false;
    rootItem = new ReadingListItem(root);
    readingListsRoot = new ListFolderItem(ListFolder { { ListFolderKind::ReadingList, 0 }, std::nullopt, QString(), 0 });
    const auto folders = setupListFolders(ListFolderKind::ReadingList, readingListsRoot, db);

    QSqlQuery selectQuery("SELECT " + TableProjection::selectList("rl", ::ReadingList::columns()) + " FROM reading_list rl ORDER BY rl.parentId IS NULL DESC", db);

    // setup reading lists
    setupReadingListsData(selectQuery, rootItem, folders);

    // TEST
    //     ReadingListItem * node1;
    //     rootItem->appendChild(node1 = new ReadingListItem(QList<QVariant>() /*<< 0*/ << "My reading list" << "atr"));
    //     rootItem->appendChild(new ReadingListItem(QList<QVariant>() /*<< 0*/ << "X timeline" << "atr"));

    //    node1->appendChild(new ReadingListItem(QList<QVariant>() /*<< 0*/ << "sublist" << "atr",node1));
}

int ReadingListModel::addLabelIntoList(LabelItem *item)
{
    if (labels.isEmpty())
        labels << item;
    else {
        int i = 0;

        while (i < labels.count() && (labels.at(i)->colorid() < item->colorid()))
            i++;

        if (i < labels.count()) {
            if (labels.at(i)->colorid() == item->colorid()) // sort by name
            {
                while (i < labels.count() && labels.at(i)->colorid() == item->colorid() && naturalSortLessThanCI(labels.at(i)->name(), item->name()))
                    i++;
            }
        }

        if (i >= labels.count()) {
            QLOG_DEBUG() << "insertando label al final " << item->name();
            labels << item;
        } else {
            QLOG_DEBUG() << "insertando label en  " << i << "-" << item->name();
            labels.insert(i, item);
        }

        return i;
    }

    return 0;
}

void ReadingListModel::reorderingChildren(QList<ReadingListItem *> children)
{
    QList<qulonglong> childrenIds;
    int i = 0;
    for (auto *item : children) {
        item->setOrdering(i++);
        childrenIds << item->getId();
    }
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        DBHelper::reasignOrderToSublists(childrenIds, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

int ReadingListModel::labelsStart() const
{
    return specialLists.count() + 1;
}

int ReadingListModel::smartListsStart() const
{
    return labelsStart() + labels.count() + 1;
}

int ReadingListModel::readingListsStart() const
{
    const auto smartRows = smartListsRoot->childCount();
    return smartListsStart() + smartRows + (smartRows > 0 ? 1 : 0);
}

bool ReadingListModel::rowIsSpecialList(int row, const QModelIndex &parent) const
{
    if (parent.isValid())
        return false; // by now no sublists in special list

    if (row >= 0 && row < specialLists.count())
        return true;

    return false;
}

bool ReadingListModel::rowIsLabel(int row, const QModelIndex &parent) const
{
    if (parent.isValid())
        return false; // by now no sublists in labels

    return row >= labelsStart() && row < labelsStart() + labels.count();
}

ListFolderItem *ReadingListModel::containerOf(ListItem *item) const
{
    if (typeid(*item) == typeid(ListFolderItem))
        return static_cast<ListFolderItem *>(item)->container;
    if (typeid(*item) == typeid(SmartListItem))
        return static_cast<SmartListItem *>(item)->container;
    if (typeid(*item) == typeid(ReadingListItem))
        return static_cast<ReadingListItem *>(item)->container;
    return nullptr;
}

QModelIndex ReadingListModel::indexOfItem(ListItem *item) const
{
    if (typeid(*item) == typeid(ReadingListItem)) {
        auto rli = static_cast<ReadingListItem *>(item);
        if (rli->parent != rootItem)
            return createIndex(rli->parent->children().indexOf(rli), 0, rli);
    }

    auto *container = containerOf(item);
    return createIndex(firstRow(container) + container->rowOf(item), 0, item);
}

QModelIndex ReadingListModel::containerIndex(ListFolderItem *container) const
{
    return container->container == nullptr ? QModelIndex() : indexOfItem(container);
}

int ReadingListModel::firstRow(ListFolderItem *container) const
{
    if (container == smartListsRoot)
        return smartListsStart();
    if (container == readingListsRoot)
        return readingListsStart();
    return 0;
}

ListFolderItem *ReadingListModel::containerFor(ListFolderKind kind, const QModelIndex &folder) const
{
    const auto id = listFolder(folder);
    if (id && id->kind == kind)
        return static_cast<ListFolderItem *>(folder.internalPointer());
    return kind == ListFolderKind::ReadingList ? readingListsRoot : smartListsRoot;
}

// the first row of the smart lists section also brings the separator before the reading lists
void ReadingListModel::insertChild(ListFolderItem *container, int childRow, const std::function<void()> &insert)
{
    const auto row = firstRow(container) + childRow;
    const bool withSeparator = container == smartListsRoot && container->childCount() == 0;
    beginInsertRows(containerIndex(container), row, withSeparator ? row + 1 : row);
    insert();
    endInsertRows();
}

void ReadingListModel::removeChild(ListItem *item)
{
    auto *container = containerOf(item);
    const auto parent = containerIndex(container);
    const auto row = firstRow(container) + container->rowOf(item);
    const bool withSeparator = container == smartListsRoot && container->childCount() == 1;
    beginRemoveRows(parent, row, withSeparator ? row + 1 : row);
    container->removeChild(item);
    endRemoveRows();
    delete item;
}

ReadingListModelProxy::ReadingListModelProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}
