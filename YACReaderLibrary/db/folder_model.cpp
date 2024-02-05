#include "folder_model.h"

#include "folder_item.h"
#include "data_base_management.h"
#include "folder.h"
#include "db_helper.h"
#include "qnaturalsorting.h"

#include "yacreader_global.h"
#include "yacreader_global_gui.h"

#include <QtGui>

#include <algorithm>

#ifdef Y_MAC_UI
#include <QFileIconProvider>
QIcon finishedFolderIcon;
void drawMacOSXFinishedFolderIcon()
{
    QIcon ico = QFileIconProvider().icon(QFileIconProvider::Folder);
    QPixmap pixNormalOff = ico.pixmap(16, 16, QIcon::Normal, QIcon::Off);
    QPixmap pixNormalOn = ico.pixmap(16, 16, QIcon::Normal, QIcon::On);
    QPixmap pixSelectedOff = ico.pixmap(16, 16, QIcon::Selected, QIcon::Off);
    QPixmap pixSelectedOn = ico.pixmap(16, 16, QIcon::Selected, QIcon::On);
    QPixmap tick(":/images/folder_finished_macosx.png");

    {
        QPainter p(&pixNormalOff);
        p.drawPixmap(4, 7, tick);
    }
    finishedFolderIcon.addPixmap(pixNormalOff, QIcon::Normal, QIcon::Off);

    {
        QPainter p(&pixNormalOn);
        p.drawPixmap(4, 7, tick);
    }
    finishedFolderIcon.addPixmap(pixNormalOn, QIcon::Normal, QIcon::On);

    {
        QPainter p(&pixSelectedOff);
        p.drawPixmap(4, 7, tick);
    }
    finishedFolderIcon.addPixmap(pixSelectedOff, QIcon::Selected, QIcon::Off);

    {
        QPainter p(&pixSelectedOn);
        p.drawPixmap(4, 7, tick);
    }
    finishedFolderIcon.addPixmap(pixSelectedOn, QIcon::Selected, QIcon::On);
}
#endif

#define ROOT 1

FolderItem *createRoot()
{
    QList<QVariant> rootData;
    rootData << "root";
    auto root = new FolderItem(rootData);
    root->id = ROOT;
    root->parentItem = nullptr;

    return root;
}

FolderModel::FolderModel(QObject *parent)
    : QAbstractItemModel(parent), isSubfolder(false), rootItem(nullptr), folderIcon(YACReader::noHighlightedIcon(":/images/sidebar/folder.svg")), folderFinishedIcon(YACReader::noHighlightedIcon(":/images/sidebar/folder_finished.svg")), showRecent(false), recentDays(1)
{
}

FolderModel::~FolderModel()
{
    if (rootItem != nullptr)
        delete rootItem;
}

int FolderModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<FolderItem *>(parent.internalPointer())->columnCount();
    else {
        if (rootItem == nullptr) {
            return 0;
        }
        return rootItem->columnCount();
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

    if (!isSubfolder) {
        auto newModelData = createModelData(_databasePath);

        takeUpdatedChildrenInfo(rootItem, QModelIndex(), newModelData.rootItem);

        // copy items from newModelData to this model that are not in this model
        foreach (auto key, newModelData.items.keys()) {
            if (!items.contains(key)) {
                items[key] = (newModelData.items[key]);
            }
        }

        delete newModelData.rootItem;
    } else {
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

            QSqlQuery selectQuery(db);
            selectQuery.prepare("SELECT * FROM folder WHERE parentId = :parentId and id <> 1");
            selectQuery.bindValue(":parentId", rootItem->id);
            selectQuery.exec();

            auto tempRoot = new FolderItem(rootItem->getData(), rootItem->parentItem);
            tempRoot->id = rootItem->id;
            auto newModelData = createModelData(selectQuery, tempRoot);
            takeUpdatedChildrenInfo(rootItem, QModelIndex(), newModelData.rootItem);

            items = newModelData.items;

            // copy items from newModelData to this model that are not in this model
            foreach (auto key, newModelData.items.keys()) {
                if (!items.contains(key)) {
                    items[key] = (newModelData.items[key]);
                }
            }

            delete newModelData.rootItem;

            connectionName = db.connectionName();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }
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
        auto sameFolderId = child->id == updatedChild->id; // and also same name
        if (sameFolderId) {
            // 1. check if child data needs to be udpated
            if (child->getData() != updatedChild->getData()) {
                auto modelIndexToUpdate = index(m, 0, parentModelIndex);

                child->setData(updatedChild->getData());

                emit dataChanged(modelIndexToUpdate, modelIndexToUpdate);
            }

            // 2. update children info
            takeUpdatedChildrenInfo(child, index(m, 0, parentModelIndex), updatedChild);

            m++;
            i++;
            j++;
            continue;
        }

        auto childName = child->data(Name).toString();
        auto childUpdatedName = updatedChild->data(Name).toString();

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
        QString toolTip = item->data(FolderModel::Name).toString();
        int totalNumOfChildren = item->childCount() + item->comicNames.size();
        if (totalNumOfChildren > 0) {
            toolTip = toolTip + " - " + QString::number(totalNumOfChildren);
        }

        return toolTip;
    }

    if (role == Qt::DecorationRole) {
#ifdef Y_MAC_UI
        if (item->data(FolderModel::Finished).toBool()) {
            if (finishedFolderIcon.isNull()) {
                drawMacOSXFinishedFolderIcon();
            }

            return QVariant(finishedFolderIcon);
        } else {
            return QVariant(QFileIconProvider().icon(QFileIconProvider::Folder));
        }
#else
        if (item->data(FolderModel::Finished).toBool())
            return QVariant(folderFinishedIcon);
        else
            return QVariant(folderIcon);
#endif
    }

    if (role == FolderModel::FolderNameRole) {
        return item->data(FolderModel::Name);
    }

    if (role == FolderModel::CompletedRole)
        return item->data(FolderModel::Completed);

    if (role == FolderModel::FinishedRole)
        return item->data(FolderModel::Finished);

    if (role == FolderModel::IdRole)
        return item->id;

    if (role == FolderModel::CoverPathRole)
        return getCoverUrlPathForComicHash(item->data(FirstChildHash).toString());

    if (role == FolderModel::NumChildrenRole)
        return item->data(NumChildren);

    if (role == FolderModel::TypeRole)
        return item->data(Type);

    if (role == FolderModel::AddedRole)
        return item->data(Added);

    if (role == FolderModel::UpdatedRole)
        return item->data(Updated);

    if (role == FolderModel::ShowRecentRole)
        return showRecent;

    if (role == FolderModel::RecentRangeRole)
        return recentDays * 86400;

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->data(index.column());
}

Qt::ItemFlags FolderModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

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
        return rootItem->data(section);

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
        if (item->id == folderId) {
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
        QSqlQuery selectQuery("select * from folder where id <> 1 order by parentId,name", db);

        auto root = createRoot();
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
    itemsLookup.insert(parent->id, parent);

    QSqlRecord record = sqlquery.record();

    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int finished = record.indexOf("finished");
    int completed = record.indexOf("completed");
    int id = record.indexOf("id");
    int parentId = record.indexOf("parentId");
    int numChildren = record.indexOf("numChildren");
    int firstChildHash = record.indexOf("firstChildHash");
    int customImage = record.indexOf("customImage");
    int type = record.indexOf("type");
    int added = record.indexOf("added");
    int updated = record.indexOf("updated");

    while (sqlquery.next()) {
        QList<QVariant> data;

        data << sqlquery.value(name);
        data << sqlquery.value(path);
        data << sqlquery.value(finished);
        data << sqlquery.value(completed);
        data << sqlquery.value(numChildren);
        data << sqlquery.value(firstChildHash);
        data << sqlquery.value(customImage);
        data << sqlquery.value(type);
        data << sqlquery.value(added);
        data << sqlquery.value(updated);

        auto item = new FolderItem(data);

        item->id = sqlquery.value(id).toULongLong();
        // la inserci�n de hijos se hace de forma ordenada
        FolderItem *parent = itemsLookup.value(sqlquery.value(parentId).toULongLong());
        parent->appendChild(item);
        // se a�ade el item al map, de forma que se pueda encontrar como padre en siguientes iteraciones
        itemsLookup.insert(item->id, item);
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
    return static_cast<FolderItem *>(folder.internalPointer())->data(FolderModel::Path).toString();
}

void FolderModel::updateFolderCompletedStatus(const QModelIndexList &list, bool status)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        foreach (QModelIndex mi, list) {
            auto item = static_cast<FolderItem *>(mi.internalPointer());
            item->setData(FolderModel::Completed, status);

            if (!isSubfolder) {
                Folder f = DBHelper::loadFolder(item->id, db);
                f.completed = status;
                DBHelper::update(f, db);
            }
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    emit dataChanged(index(list.first().row(), FolderModel::Name), index(list.last().row(), FolderModel::Updated));
}

void FolderModel::updateFolderFinishedStatus(const QModelIndexList &list, bool status)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        foreach (QModelIndex mi, list) {
            auto item = static_cast<FolderItem *>(mi.internalPointer());
            item->setData(FolderModel::Finished, status);

            if (!isSubfolder) {
                Folder f = DBHelper::loadFolder(item->id, db);
                f.finished = status;
                DBHelper::update(f, db);
            }
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    emit dataChanged(index(list.first().row(), FolderModel::Name), index(list.last().row(), FolderModel::Updated));
}

void FolderModel::updateFolderType(const QModelIndexList &list, YACReader::FileType type)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        foreach (QModelIndex mi, list) {
            auto item = static_cast<FolderItem *>(mi.internalPointer());

            std::function<void(FolderItem *, YACReader::FileType)> setType;
            setType = [&setType](FolderItem *item, YACReader::FileType type) -> void {
                item->setData(FolderModel::Type, QVariant::fromValue(type));

                for (auto child : item->children()) {
                    setType(child, type);
                }
            };

            setType(item, type);

            if (!isSubfolder) {
                DBHelper::updateFolderTreeType(item->id, db, type);
            }
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    emit dataChanged(index(list.first().row(), FolderModel::Name), index(list.last().row(), FolderModel::Updated));
}

QStringList FolderModel::getSubfoldersNames(const QModelIndex &mi)
{
    QStringList result;
    qulonglong id = 1;
    if (mi.isValid()) {
        auto item = static_cast<FolderItem *>(mi.internalPointer());
        id = item->id;
    }
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();

        result = DBHelper::loadSubfoldersNames(id, db);

        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    std::sort(result.begin(), result.end(), naturalSortLessThanCI);
    return result;
}

FolderModel *FolderModel::getSubfoldersModel(const QModelIndex &mi)
{
    qulonglong id = 1;
    FolderItem *parent = nullptr;
    if (mi.isValid()) {
        auto item = static_cast<FolderItem *>(mi.internalPointer());
        parent = new FolderItem(item->getData(), item->parent());
        id = parent->id = item->id;
    }

    if (id == 1) {
        if (parent != nullptr) {
            delete parent;
        }
        return this;
    }

    auto model = new FolderModel();

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        QSqlQuery selectQuery(db); // TODO check
        selectQuery.prepare("SELECT * FROM folder WHERE parentId = :parentId and id <> 1");
        selectQuery.bindValue(":parentId", id);
        selectQuery.exec();

        if (parent != nullptr) {
            model->setModelData(createModelData(selectQuery, parent));
        }

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    model->_databasePath = _databasePath;

    model->isSubfolder = true;

    return model;
}

Folder FolderModel::getFolder(const QModelIndex &mi)
{
    auto folderItem = static_cast<FolderItem *>(mi.internalPointer());
    auto name = folderItem->data(FolderModel::Name).toString();
    auto parentItem = folderItem->parent();
    auto folder = Folder(folderItem->id,
                         parentItem->id,
                         name,
                         folderItem->parent()->data(Columns::Path).toString() + "/" + name,
                         folderItem->data(Columns::Completed).toBool(),
                         folderItem->data(Columns::Finished).toBool(),
                         folderItem->data(Columns::NumChildren).toInt(),
                         folderItem->data(Columns::FirstChildHash).toString(),
                         folderItem->data(Columns::CustomImage).toString(),
                         folderItem->data(Columns::Type).value<YACReader::FileType>(),
                         folderItem->data(Columns::Added).toLongLong(),
                         folderItem->data(Columns::Updated).toLongLong());

    return folder;
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

            if (folderItem->id == folderId) {
                return modelIndex;
            }

            auto childModelIndex = getIndexFromFolderId(folderId, modelIndex);

            if (childModelIndex.isValid()) {
                auto folderItem = static_cast<FolderItem *>(childModelIndex.internalPointer());

                if (folderItem->id == folderId) {
                    return childModelIndex;
                }
            }
        }
    }

    return QModelIndex();
}

QModelIndex FolderModel::getIndexFromFolder(const Folder &folder, const QModelIndex &parent)
{
    return getIndexFromFolderId(folder.id, parent);
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
    newFolder.parentId = parentItem->id;
    newFolder.path = parentItem->data(Columns::Path).toString() + "/" + folderName;
    newFolder.type = parentItem->data(Columns::Type).value<YACReader::FileType>();

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        newFolder.id = DBHelper::insert(&newFolder, db);
        DBHelper::updateChildrenInfo(parentItem->id, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    int destRow = 0;

    QList<QVariant> data;
    data << newFolder.name;
    data << newFolder.path;
    data << false; // finished
    data << true; // completed
    data << 0; // numChildren
    data << QVariant(); // first child hash, new folder is empty
    data << QVariant(); // custom cover
    data << QVariant::fromValue(newFolder.type);
    data << newFolder.added;
    data << newFolder.updated;

    auto item = new FolderItem(data);
    item->id = newFolder.id;

    beginInsertRows(parent, 0, 0); // TODO calculate the destRow before inserting the new child

    parentItem->appendChild(item);
    destRow = parentItem->children().indexOf(item); // TODO optimize this, appendChild should return the index of the new item
    items.insert(item->id, item);

    endInsertRows();

    return index(destRow, 0, parent);
}

QUrl FolderModel::getCoverUrlPathForComicHash(const QString &hash) const
{
    return QUrl::fromLocalFile(_databasePath + "/covers/" + hash + ".jpg");
}

void FolderModel::setShowRecent(bool showRecent)
{
    if (this->showRecent == showRecent)
        return;

    this->showRecent = showRecent;

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), { FolderModel::ShowRecentRole });
}

void FolderModel::setRecentRange(int days)
{
    if (this->recentDays == days)
        return;

    this->recentDays = days;

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), { FolderModel::RecentRangeRole });
}

void FolderModel::deleteFolder(const QModelIndex &mi)
{
    beginRemoveRows(mi.parent(), mi.row(), mi.row());

    auto item = static_cast<FolderItem *>(mi.internalPointer());

    FolderItem *parent = item->parent();
    parent->removeChild(mi.row());

    Folder f;
    f.setId(item->id);

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        DBHelper::removeFromDB(&f, db);
        auto folder = DBHelper::updateChildrenInfo(item->parent()->id, db);
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

    return filteredItems.contains(item->id);
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
