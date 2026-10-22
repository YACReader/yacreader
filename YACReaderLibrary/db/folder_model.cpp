#include "folder_model.h"

#include "data_base_management.h"
#include "db_helper.h"
#include "folder.h"
#include "folder_item.h"
#include "qnaturalsorting.h"
#include "table_projection.h"
#include "yacreader_global.h"

#include <QDir>
#include <QFileIconProvider>
#include <QPainter>
#include <QSqlError>
#include <QSqlRecord>

#include <algorithm>

using namespace YACReader;

QIcon drawFinishedFolderIcon(const QPixmap &overlay)
{
    QIcon finishedIcon;
    QIcon ico = QFileIconProvider().icon(QFileIconProvider::Folder);
    QPixmap pixNormalOff = ico.pixmap(16, 16, QIcon::Normal, QIcon::Off);
    QPixmap pixNormalOn = ico.pixmap(16, 16, QIcon::Normal, QIcon::On);
    QPixmap pixSelectedOff = ico.pixmap(16, 16, QIcon::Selected, QIcon::Off);
    QPixmap pixSelectedOn = ico.pixmap(16, 16, QIcon::Selected, QIcon::On);

    {
        QPainter p(&pixNormalOff);
        p.drawPixmap(4, 7, overlay);
    }
    finishedIcon.addPixmap(pixNormalOff, QIcon::Normal, QIcon::Off);

    {
        QPainter p(&pixNormalOn);
        p.drawPixmap(4, 7, overlay);
    }
    finishedIcon.addPixmap(pixNormalOn, QIcon::Normal, QIcon::On);

    {
        QPainter p(&pixSelectedOff);
        p.drawPixmap(4, 7, overlay);
    }
    finishedIcon.addPixmap(pixSelectedOff, QIcon::Selected, QIcon::Off);

    {
        QPainter p(&pixSelectedOn);
        p.drawPixmap(4, 7, overlay);
    }
    finishedIcon.addPixmap(pixSelectedOn, QIcon::Selected, QIcon::On);

    return finishedIcon;
}

// The values have the types the views got from the SQL query.
QVariant columnValue(const Folder &folder, int column)
{
    // SQLite gives a NULL as a null QString whatever the column type
    const QVariant sqlNull(QMetaType::fromType<QString>());
    switch (column) {
    case FolderModel::Name:
        return folder.name;
    case FolderModel::Path:
        return folder.path;
    case FolderModel::Finished:
        return qlonglong(folder.finished ? 1 : 0);
    case FolderModel::Completed:
        return qlonglong(folder.completed ? 1 : 0);
    case FolderModel::NumChildren:
        return qlonglong(folder.numChildren);
    case FolderModel::FirstChildHash:
        return folder.firstChildHash.isNull() ? sqlNull : QVariant(folder.firstChildHash);
    case FolderModel::CustomImage:
        return folder.customImage.isNull() ? sqlNull : QVariant(folder.customImage);
    case FolderModel::Type:
        return qlonglong(static_cast<int>(folder.type));
    case FolderModel::Added:
        return folder.added != 0 ? QVariant(qlonglong(folder.added)) : sqlNull;
    case FolderModel::Updated:
        return folder.updated != 0 ? QVariant(qlonglong(folder.updated)) : sqlNull;
    }
    return QVariant();
}

// TODO: load from DB, type is pretty much needed
FolderItem *createRoot(QSqlDatabase &db)
{
    auto rootFolder = DBHelper::loadFolder(FolderModel::RootFolderId, db);
    if (!rootFolder) {
        return nullptr;
    }

    rootFolder->name = "root";

    auto root = new FolderItem(*rootFolder);
    root->parentItem = nullptr;

    return root;
}

FolderModel::FolderModel(QObject *parent)
    : QAbstractItemModel(parent), rootItem(nullptr), showRecent(false), recentDays(1)
{
    initTheme(this);
}

void FolderModel::applyTheme(const Theme &theme)
{
    const auto &sidebarIcons = theme.sidebarIcons;

    if (sidebarIcons.useSystemFolderIcons) {
        folderIcon = QFileIconProvider().icon(QFileIconProvider::Folder);
        folderFinishedIcon = drawFinishedFolderIcon(sidebarIcons.folderReadOverlay);
    } else {
        folderIcon = theme.navigationTree.folderIcon;
        folderFinishedIcon = theme.navigationTree.folderFinishedIcon;
    }
}

FolderModel::~FolderModel()
{
    if (rootItem != nullptr)
        delete rootItem;
}

int FolderModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return FolderModel::Updated + 1;
    else {
        if (rootItem == nullptr) {
            return 0;
        }
        return 1;
    }
}

QHash<int, QByteArray> FolderModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[FinishedRole] = "is_finished";
    roles[CompletedRole] = "is_completed";
    roles[IdRole] = "id";
    roles[CoverPathRole] = "cover_path";
    roles[FolderNameRole] = "name";
    roles[NumChildrenRole] = "num_children";
    roles[TypeRole] = "type";
    roles[AddedRole] = "added";
    roles[UpdatedRole] = "updated";
    roles[ShowRecentRole] = "show_recent";
    roles[RecentRangeRole] = "recent_range";

    return roles;
}

void FolderModel::reload()
{
    if (rootItem == nullptr)
        return;

    auto newModelData = createModelData(_databasePath);

    takeUpdatedChildrenInfo(rootItem, QModelIndex(), newModelData.rootItem);

    // copy items from newModelData to this model that are not in this model
    const auto newItemKeys = newModelData.items.keys();
    for (const auto key : newItemKeys) {
        if (!items.contains(key)) {
            items[key] = (newModelData.items[key]);
        }
    }

    delete newModelData.rootItem;
}

void FolderModel::takeUpdatedChildrenInfo(FolderItem *parent, const QModelIndex &parentModelIndex, FolderItem *updated)
{
    auto currentChildren = parent->children();
    auto updatedChildren = updated->children();

    int lenght = currentChildren.size();
    int lenghtUpdated = updatedChildren.size();

    int m; // index that reflects modifications on the actual model for this parent
    int i; // index of the original children before update
    int j; // index of the updated children
    for (m = 0, i = 0, j = 0; (i < lenght) && (j < lenghtUpdated);) {
        auto child = currentChildren[i];
        auto updatedChild = updatedChildren[j];

        // same folder
        auto sameFolderId = child->folder.id.value() == updatedChild->folder.id.value(); // and also same name
        if (sameFolderId) {
            // 1. check if child data needs to be udpated
            if (child->folder != updatedChild->folder) {
                auto modelIndexToUpdate = index(m, 0, parentModelIndex);

                child->folder = updatedChild->folder;

                emit dataChanged(modelIndexToUpdate, modelIndexToUpdate);
            }

            // 2. update children info
            takeUpdatedChildrenInfo(child, index(m, 0, parentModelIndex), updatedChild);

            m++;
            i++;
            j++;
            continue;
        }

        auto childName = child->folder.name;
        auto childUpdatedName = updatedChild->folder.name;

        // folder added
        if (!naturalSortLessThanCI(childName, childUpdatedName)) {
            beginInsertRows(parentModelIndex, m, m);

            parent->addChild(updatedChild, m);
            updated->removeChild(updatedChild);

            endInsertRows();

            m++;
            j++;
            continue;
        }

        // folder removed
        if (naturalSortLessThanCI(childName, childUpdatedName)) {
            beginRemoveRows(parentModelIndex, m, m);

            delete parent->child(m);
            parent->removeChild(m);

            endRemoveRows();

            i++;
            continue;
        }
    }

    // add remaining children
    for (; j < lenghtUpdated; j++) {
        auto updatedChild = updatedChildren[j];

        beginInsertRows(parentModelIndex, m, m);

        parent->addChild(updatedChild, m);
        updated->removeChild(updatedChild);

        endInsertRows();

        m++;
    }

    // remove remaining children
    for (; i < lenght; i++) {
        beginRemoveRows(parentModelIndex, m, m);

        delete parent->child(m);
        parent->removeChild(m);

        endRemoveRows();
    }
}

Folder FolderModel::folderFromItem(FolderItem *folderItem) const
{
    return folderItem->folder;
}

void FolderModel::reload(const QModelIndex &index)
{
    // TODO: reload just the content under index for better efficiency
    reload();
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto item = static_cast<FolderItem *>(index.internalPointer());

    if (role == Qt::ToolTipRole) {
        QString toolTip = item->folder.name;
        int totalNumOfChildren = item->childCount();
        if (totalNumOfChildren > 0) {
            toolTip = toolTip + " - " + QString::number(totalNumOfChildren);
        }

        return toolTip;
    }

    if (role == Qt::DecorationRole) {
        if (item->folder.finished)
            return QVariant(folderFinishedIcon);
        else
            return QVariant(folderIcon);
    }

    if (role == FolderModel::FolderNameRole) {
        return columnValue(item->folder, FolderModel::Name);
    }

    if (role == FolderModel::CompletedRole)
        return columnValue(item->folder, FolderModel::Completed);

    if (role == FolderModel::FinishedRole)
        return columnValue(item->folder, FolderModel::Finished);

    if (role == FolderModel::IdRole)
        return item->folder.id.value();

    if (role == FolderModel::CoverPathRole) {
        if (item->folder.customImage.isEmpty())
            return getCoverUrlPathForComicHash(item->folder.firstChildHash);
        else
            return getCoverUrlPathForFolderId(item->folder.id.value());
    }

    if (role == FolderModel::NumChildrenRole)
        return columnValue(item->folder, NumChildren);

    if (role == FolderModel::TypeRole)
        return columnValue(item->folder, Type);

    if (role == FolderModel::AddedRole)
        return columnValue(item->folder, Added);

    if (role == FolderModel::UpdatedRole)
        return columnValue(item->folder, Updated);

    if (role == FolderModel::ShowRecentRole)
        return showRecent;

    if (role == FolderModel::RecentRangeRole)
        return recentDays * 86400;

    if (role != Qt::DisplayRole)
        return QVariant();

    return columnValue(item->folder, index.column());
}

Qt::ItemFlags FolderModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return { };

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
}

QVariant FolderModel::headerData(int section,
                                 Qt::Orientation orientation,
                                 int role) const
{
    if (rootItem == nullptr) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return columnValue(rootItem->folder, section);

    return QVariant();
}

QModelIndex FolderModel::index(int row, int column, const QModelIndex &parent)
        const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    FolderItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FolderItem *>(parent.internalPointer());

    FolderItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex FolderModel::index(qulonglong folderId) const
{
    QModelIndex index;
    YACReader::iterate(QModelIndex(), this, [&](const QModelIndex &idx) {
        if (index.isValid()) {
            return false;
        }

        auto item = static_cast<FolderItem *>(idx.internalPointer());
        if (item->folder.id.value() == folderId) {
            index = idx;
            return false;
        }

        return true;
    });

    return index;
}

QModelIndex FolderModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    auto childItem = static_cast<FolderItem *>(index.internalPointer());
    FolderItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int FolderModel::rowCount(const QModelIndex &parent) const
{
    FolderItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid()) {
        if (rootItem == nullptr) {
            return 0;
        }
        parentItem = rootItem;
    } else {
        parentItem = static_cast<FolderItem *>(parent.internalPointer());
    }

    return parentItem->childCount();
}

void FolderModel::setupModelData(QString path)
{
    beginResetModel();

    if (rootItem != nullptr)
        delete rootItem; // TODO comprobar que se libera bien la memoria

    rootItem = nullptr;
    _databasePath = path;

    setModelData(createModelData(path));

    endResetModel();
}

void FolderModel::setModelData(const ModelData &modelData)
{
    rootItem = modelData.rootItem;
    items = modelData.items;
}

FolderModel::ModelData FolderModel::createModelData(const QString &path) const
{
    ModelData modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(path);
        QSqlQuery selectQuery("SELECT " + TableProjection::selectList("f", Folder::columns()) + " FROM folder f WHERE f.id <> 1 ORDER BY f.parentId, f.name", db);

        auto root = createRoot(db);
        modelData = createModelData(selectQuery, root);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return modelData;
}

FolderModel::ModelData FolderModel::createModelData(QSqlQuery &sqlquery, FolderItem *parent) const
{
    QMap<unsigned long long int, FolderItem *> itemsLookup;

    // add parent to the lookup
    itemsLookup.insert(parent->folder.id.value(), parent);

    const auto indexes = Folder::columnIndexes(sqlquery.record(), "f");
    while (sqlquery.next()) {
        auto item = new FolderItem(Folder::fromQuery(sqlquery, indexes));

        // la inserci�n de hijos se hace de forma ordenada
        FolderItem *parent = itemsLookup.value(item->folder.parentId.value());
        parent->appendChild(item);
        // se a�ade el item al map, de forma que se pueda encontrar como padre en siguientes iteraciones
        itemsLookup.insert(item->folder.id.value(), item);
    }

    return FolderModel::ModelData { parent, itemsLookup };
}

QString FolderModel::getDatabase()
{
    return _databasePath;
}

QString FolderModel::getFolderPath(const QModelIndex &folder)
{
    if (!folder.isValid()) // root folder
        return "/";
    return static_cast<FolderItem *>(folder.internalPointer())->folder.path;
}

void FolderModel::updateFolderCompletedStatus(const QModelIndexList &list, bool status)
{
    if (list.isEmpty())
        return;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        for (const auto &mi : list) {
            auto item = static_cast<FolderItem *>(mi.internalPointer());
            item->folder.completed = status;

            if (auto f = DBHelper::loadFolder(item->folder.id.value(), db)) {
                f->completed = status;
                DBHelper::update(*f, db);
            }
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    const auto parent = list.first().parent();
    emit dataChanged(index(list.first().row(), FolderModel::Name, parent), index(list.last().row(), FolderModel::Updated, parent));
}

void FolderModel::updateFolderFinishedStatus(const QModelIndexList &list, bool status)
{
    if (list.isEmpty())
        return;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        for (const auto &mi : list) {
            auto item = static_cast<FolderItem *>(mi.internalPointer());
            item->folder.finished = status;

            if (auto f = DBHelper::loadFolder(item->folder.id.value(), db)) {
                f->finished = status;
                DBHelper::update(*f, db);
            }
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    const auto parent = list.first().parent();
    emit dataChanged(index(list.first().row(), FolderModel::Name, parent), index(list.last().row(), FolderModel::Updated, parent));
}

void FolderModel::updateFolderType(const QModelIndexList &list, YACReader::FileType type)
{
    if (list.isEmpty())
        return;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        for (const auto &mi : list) {
            auto item = static_cast<FolderItem *>(mi.internalPointer());

            std::function<void(FolderItem *, YACReader::FileType)> setType;
            setType = [&setType](FolderItem *item, YACReader::FileType type) -> void {
                item->folder.type = type;

                const auto children = item->children();
                for (auto child : children) {
                    setType(child, type);
                }
            };

            setType(item, type);

            DBHelper::updateFolderTreeType(item->folder.id.value(), db, type);
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    const auto parent = list.first().parent();
    emit dataChanged(index(list.first().row(), FolderModel::Name, parent), index(list.last().row(), FolderModel::Updated, parent));
}

bool FolderModel::renameFolder(const QModelIndex &folder, const QString &name, QString *error)
{
    if (!folder.isValid())
        return false;

    auto item = static_cast<FolderItem *>(folder.internalPointer());
    const auto oldPath = item->folder.path;
    const auto parentPath = item->parent()->folder.path;
    const auto newPath = QDir::cleanPath(parentPath + "/" + name);

    QString connectionName;
    bool success = false;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        connectionName = db.connectionName();

        if (!db.isValid() || !db.isOpen()) {
            if (error != nullptr)
                *error = db.lastError().text();
        } else if (!db.transaction()) {
            if (error != nullptr)
                *error = db.lastError().text();
        } else if (!DBHelper::renameFolder(item->folder.id.value(), name, oldPath, newPath, db, error)) {
            db.rollback();
        } else if (!db.commit()) {
            if (error != nullptr)
                *error = db.lastError().text();
            db.rollback();
        } else {
            success = true;
        }
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!success)
        return false;

    item->folder.name = name;

    const auto updatePath = [&oldPath, &newPath](auto &&self, FolderItem *folderItem) -> void {
        const auto path = folderItem->folder.path;
        folderItem->folder.path = newPath + path.mid(oldPath.size());
        const auto children = folderItem->children();
        for (auto child : children)
            self(self, child);
    };
    updatePath(updatePath, item);

    auto parentItem = item->parent();
    const auto oldRow = item->row();
    auto newRow = 0;
    const auto siblings = parentItem->children();
    for (auto sibling : siblings) {
        if (sibling != item && !naturalSortLessThanCI(name, sibling->folder.name))
            ++newRow;
    }

    if (newRow != oldRow) {
        const auto destination = newRow > oldRow ? newRow + 1 : newRow;
        beginMoveRows(folder.parent(), oldRow, oldRow, folder.parent(), destination);
        parentItem->removeChild(item);
        parentItem->appendChild(item);
        endMoveRows();
    }

    const auto renamedIndex = index(item->row(), FolderModel::Name, folder.parent());
    emit dataChanged(renamedIndex, index(item->row(), FolderModel::Path, folder.parent()));
    return true;
}

void FolderModel::updateTreeType(YACReader::FileType type)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();

        auto item = rootItem;

        std::function<void(FolderItem *, YACReader::FileType)> setType;
        setType = [&setType](FolderItem *item, YACReader::FileType type) -> void {
            item->folder.type = type;

            const auto children = item->children();
            for (auto child : children) {
                setType(child, type);
            }
        };

        setType(item, type);

        DBHelper::updateDBType(db, type);
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void FolderModel::setCustomFolderCover(const QModelIndex &index, const QString &path)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();

        auto item = static_cast<FolderItem *>(index.internalPointer());
        item->folder.customImage = path;

        if (auto f = DBHelper::loadFolder(item->folder.id.value(), db)) {
            f->customImage = path;
            DBHelper::update(*f, db);
        }

        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    ++coverRevisions[index.data(FolderModel::IdRole).toULongLong()];
    emit dataChanged(index, index, { CoverPathRole });
}

void FolderModel::resetFolderCover(const QModelIndex &index)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();

        auto item = static_cast<FolderItem *>(index.internalPointer());
        item->folder.customImage = "";

        if (auto f = DBHelper::loadFolder(item->folder.id.value(), db)) {
            f->customImage = "";
            DBHelper::update(*f, db);
        }

        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    emit dataChanged(index, index);
}

Folder FolderModel::getRootFolder() const
{
    if (this->rootItem == nullptr) {
        return Folder();
    }

    return folderFromItem(this->rootItem);
}

Folder FolderModel::getFolder(const QModelIndex &mi) const
{
    if (!mi.isValid()) {
        return Folder();
    }

    auto folderItem = static_cast<FolderItem *>(mi.internalPointer());
    return folderFromItem(folderItem);
}

QModelIndex FolderModel::getIndexFromFolderId(qulonglong folderId, const QModelIndex &parent)
{
    if (rootItem == nullptr) {
        return QModelIndex();
    }

    auto numRows = rowCount(parent);
    for (auto i = 0; i < numRows; i++) {
        auto modelIndex = index(i, 0, parent);

        if (modelIndex.isValid()) {
            auto folderItem = static_cast<FolderItem *>(modelIndex.internalPointer());

            if (folderItem->folder.id.value() == folderId) {
                return modelIndex;
            }

            auto childModelIndex = getIndexFromFolderId(folderId, modelIndex);

            if (childModelIndex.isValid()) {
                auto folderItem = static_cast<FolderItem *>(childModelIndex.internalPointer());

                if (folderItem->folder.id.value() == folderId) {
                    return childModelIndex;
                }
            }
        }
    }

    return QModelIndex();
}

QModelIndex FolderModel::getIndexFromFolder(const Folder &folder, const QModelIndex &parent)
{
    return getIndexFromFolderId(folder.id.value(), parent);
}

QModelIndex FolderModel::addFolderAtParent(const QString &folderName, const QModelIndex &parent)
{
    FolderItem *parentItem;

    if (parent.isValid())
        parentItem = static_cast<FolderItem *>(parent.internalPointer());
    else
        parentItem = rootItem;

    Folder newFolder;
    newFolder.name = folderName;
    newFolder.parentId = FolderId(parentItem->folder.id.value());
    newFolder.path = parentItem->folder.path + "/" + folderName;
    newFolder.type = parentItem->folder.type;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        newFolder.id = FolderId(DBHelper::insert(&newFolder, db));
        DBHelper::updateChildrenInfo(parentItem->folder.id.value(), db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    int destRow = 0;

    auto item = new FolderItem(newFolder);

    beginInsertRows(parent, 0, 0); // TODO calculate the destRow before inserting the new child

    parentItem->appendChild(item);
    destRow = parentItem->children().indexOf(item); // TODO optimize this, appendChild should return the index of the new item
    items.insert(item->folder.id.value(), item);

    endInsertRows();

    return index(destRow, 0, parent);
}

QUrl FolderModel::getCoverUrlPathForComicHash(const QString &hash) const
{
    auto coverPath = LibraryPaths::coverPathFromLibraryDataPath(_databasePath, hash);
    return QUrl::fromLocalFile(coverPath);
}

QUrl FolderModel::getCoverUrlPathForFolderId(qulonglong folderId) const
{
    auto coverPath = LibraryPaths::customFolderCoverPathFromDataPath(_databasePath, QString::number(folderId));
    auto coverUrl = QUrl::fromLocalFile(coverPath);
    const auto revision = coverRevisions.value(folderId);
    if (revision > 0)
        coverUrl.setQuery(QStringLiteral("revision=%1").arg(revision));
    return coverUrl;
}

void FolderModel::setShowRecent(bool showRecent)
{
    if (this->showRecent == showRecent)
        return;

    this->showRecent = showRecent;

    emitDataChangedRecursively({ }, FolderModel::ShowRecentRole);
}

void FolderModel::setRecentRange(int days)
{
    if (this->recentDays == days)
        return;

    this->recentDays = days;

    emitDataChangedRecursively({ }, FolderModel::RecentRangeRole);
}

void FolderModel::emitDataChangedRecursively(const QModelIndex &parent, int role)
{
    const auto rows = rowCount(parent);
    if (rows == 0)
        return;

    emit dataChanged(index(0, 0, parent), index(rows - 1, 0, parent), { role });
    for (int row = 0; row < rows; ++row)
        emitDataChangedRecursively(index(row, 0, parent), role);
}

void FolderModel::deleteFolder(const QModelIndex &mi)
{
    beginRemoveRows(mi.parent(), mi.row(), mi.row());

    auto item = static_cast<FolderItem *>(mi.internalPointer());

    FolderItem *parent = item->parent();
    parent->removeChild(mi.row());

    Folder f;
    f.id = item->folder.id;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        DBHelper::removeFromDB(&f, db);
        if (const auto coverKeys = DBHelper::removeUnusedPlaceholders(db))
            DBHelper::removePlaceholderCovers(_databasePath, *coverKeys);
        auto folder = DBHelper::updateChildrenInfo(item->parent()->folder.id.value(), db);
        DBHelper::propagateFolderUpdatesToParent(folder, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    endRemoveRows();
}

void FolderModel::updateFolderChildrenInfo(qulonglong folderId)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        auto folder = DBHelper::updateChildrenInfo(folderId, db);
        DBHelper::propagateFolderUpdatesToParent(folder, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

// PROXY

FolderModelProxy::FolderModelProxy(QObject *parent)
    : QSortFilterProxyModel(parent), rootItem(0), filterEnabled(false)
{
}

FolderModelProxy::~FolderModelProxy()
{
}

bool FolderModelProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!filterEnabled)
        return true;

    auto parent = static_cast<FolderItem *>(source_parent.internalPointer());

    if (parent == 0)
        parent = static_cast<FolderModel *>(sourceModel())->rootItem;

    FolderItem *item = parent->children().at(source_row);

    return filteredItems.contains(item->folder.id.value());
}

void FolderModelProxy::setFilterData(QMap<unsigned long long, FolderItem *> *filteredItems, FolderItem *root)
{
    clear();
    filterEnabled = true;

    beginResetModel();

    if (rootItem != 0)
        delete rootItem; // TODO comprobar que se libera bien la memoria

    rootItem = root;

    this->filteredItems.insert(*filteredItems);

    endResetModel();

    delete filteredItems;
}

void FolderModelProxy::clear()
{
    filterEnabled = false;

    filteredItems.clear();

    QSortFilterProxyModel::invalidate();
}
