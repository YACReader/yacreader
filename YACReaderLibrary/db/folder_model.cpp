#include "folder_model.h"

#include "folder_item.h"
#include "data_base_management.h"
#include "folder.h"
#include "db_helper.h"
#include "qnaturalsorting.h"
#include "yacreader_global_gui.h"
#include "QsLog.h"
#include "query_parser.h"

#include <QtGui>

#include <algorithm>

#ifdef Q_OS_MAC
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

FolderModel::FolderModel(QObject *parent)
    : QAbstractItemModel(parent), rootItem(0)
{
}

FolderModel::FolderModel(QSqlQuery &sqlquery, QObject *parent)
    : QAbstractItemModel(parent), rootItem(0)
{
    //lo m�s probable es que el nodo ra�z no necesite tener informaci�n
    QList<QVariant> rootData;
    rootData << "root"; //id 0, padre 0, title "root" (el id, y el id del padre van a ir en la clase TreeItem)
    rootItem = new FolderItem(rootData);
    rootItem->id = ROOT;
    rootItem->parentItem = 0;
    setupModelData(sqlquery, rootItem);
    //sqlquery.finish();
}

FolderModel::~FolderModel()
{
    if (rootItem != 0)
        delete rootItem;
}

int FolderModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<FolderItem *>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
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
#ifdef Q_OS_MAC
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
            return QVariant(YACReader::noHighlightedIcon(":/images/sidebar/folder_finished.png"));
        else
            return QVariant(YACReader::noHighlightedIcon(":/images/sidebar/folder.png"));
#endif
    }

    if (role == FolderModel::CompletedRole)
        return item->data(FolderModel::Completed);

    if (role == FolderModel::FinishedRole)
        return item->data(FolderModel::Finished);

    if (role == FolderModel::MangaRole)
        return item->data(FolderModel::Manga);

    if (role == FolderModel::IdRole)
        return item->id;

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

QVariant FolderModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
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

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<FolderItem *>(parent.internalPointer());

    return parentItem->childCount();
}

void FolderModel::setupModelData(QString path)
{
    beginResetModel();
    if (rootItem != 0)
        delete rootItem; //TODO comprobar que se libera bien la memoria

    rootItem = 0;

    //inicializar el nodo ra�z
    QList<QVariant> rootData;
    rootData << "root"; //id 0, padre 0, title "root" (el id, y el id del padre van a ir en la clase TreeItem)
    rootItem = new FolderItem(rootData);
    rootItem->id = ROOT;
    rootItem->parentItem = 0;

    //cargar la base de datos
    _databasePath = path;
    //crear la consulta
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(path);
        QSqlQuery selectQuery("select * from folder where id <> 1 order by parentId,name", db);

        setupModelData(selectQuery, rootItem);
        connectionName = db.connectionName();
    }
    //selectQuery.finish();
    QSqlDatabase::removeDatabase(connectionName);
    endResetModel();
}

void FolderModel::setupModelData(QSqlQuery &sqlquery, FolderItem *parent)
{
    //64 bits para la primary key, es decir la misma precisi�n que soporta sqlit 2^64
    //el diccionario permitir� encontrar cualquier nodo del �rbol r�pidamente, de forma que a�adir un hijo a un padre sea O(1)
    items.clear();
    //se a�ade el nodo 0
    items.insert(parent->id, parent);

    QSqlRecord record = sqlquery.record();

    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int finished = record.indexOf("finished");
    int completed = record.indexOf("completed");
    int manga = record.indexOf("manga");
    int id = record.indexOf("id");
    int parentId = record.indexOf("parentId");

    while (sqlquery.next()) {
        QList<QVariant> data;

        data << sqlquery.value(name).toString();
        data << sqlquery.value(path).toString();
        data << sqlquery.value(finished).toBool();
        data << sqlquery.value(completed).toBool();
        data << sqlquery.value(manga).toBool();
        auto item = new FolderItem(data);

        item->id = sqlquery.value(id).toULongLong();
        //la inserci�n de hijos se hace de forma ordenada
        FolderItem *parent = items.value(sqlquery.value(parentId).toULongLong());
        //if(parent !=0) //TODO if parent==0 the parent of item was removed from the DB and delete on cascade didn't work, ERROR.
        parent->appendChild(item);
        //se a�ade el item al map, de forma que se pueda encontrar como padre en siguientes iteraciones
        items.insert(item->id, item);
    }
}

void FolderModel::updateFolderModelData(QSqlQuery &sqlquery, FolderItem *parent)
{
    Q_UNUSED(parent);

    QSqlRecord record = sqlquery.record();

    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int finished = record.indexOf("finished");
    int completed = record.indexOf("completed");
    int manga = record.indexOf("manga");
    int id = record.indexOf("id");
    int parentId = record.indexOf("parentId");

    while (sqlquery.next()) {
        QList<QVariant> data;

        data << sqlquery.value(name).toString();
        data << sqlquery.value(path).toString();
        data << sqlquery.value(finished).toBool();
        data << sqlquery.value(completed).toBool();
        data << sqlquery.value(manga).toBool();
        auto item = new FolderItem(data);

        item->id = sqlquery.value(id).toULongLong();
        //la inserci�n de hijos se hace de forma ordenada
        FolderItem *parent = items.value(sqlquery.value(parentId).toULongLong());
        if (parent != 0) //TODO if parent==0 the parent of item was removed from the DB and delete on cascade didn't work, ERROR.
            parent->appendChild(item);
        //se a�ade el item al map, de forma que se pueda encontrar como padre en siguientes iteraciones
        items.insert(item->id, item);
    }
}

QString FolderModel::getDatabase()
{
    return _databasePath;
}

QString FolderModel::getFolderPath(const QModelIndex &folder)
{
    if (!folder.isValid()) //root folder
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

            Folder f = DBHelper::loadFolder(item->id, db);
            f.setCompleted(status);
            DBHelper::update(f, db);
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    emit dataChanged(index(list.first().row(), FolderModel::Name), index(list.last().row(), FolderModel::Completed));
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

            Folder f = DBHelper::loadFolder(item->id, db);
            f.setFinished(status);
            DBHelper::update(f, db);
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    emit dataChanged(index(list.first().row(), FolderModel::Name), index(list.last().row(), FolderModel::Completed));
}

void FolderModel::updateFolderManga(const QModelIndexList &list, bool manga)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        foreach (QModelIndex mi, list) {
            auto item = static_cast<FolderItem *>(mi.internalPointer());

            std::function<void(FolderItem *, bool)> setManga;
            setManga = [&setManga](FolderItem *item, bool manga) -> void {
                item->setData(FolderModel::Manga, manga);

                for (auto child : item->children()) {
                    setManga(child, manga);
                }
            };

            setManga(item, manga);

            DBHelper::updateFolderTreeManga(item->id, db, manga);
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    emit dataChanged(index(list.first().row(), FolderModel::Name), index(list.last().row(), FolderModel::Manga));
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

void FolderModel::fetchMoreFromDB(const QModelIndex &parent)
{
    FolderItem *item;
    if (parent.isValid())
        item = static_cast<FolderItem *>(parent.internalPointer());
    else
        item = rootItem;

    //Remove all children
    if (item->childCount() > 0) {
        beginRemoveRows(parent, 0, item->childCount() - 1);
        item->clearChildren();
        endRemoveRows();
    }

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        QList<FolderItem *> items;
        QList<FolderItem *> nextLevelItems;

        QSqlQuery selectQuery(db);
        selectQuery.prepare("select * from folder where id <> 1 and parentId = :parentId order by parentId,name");

        items << item;
        bool firstLevelUpdated = false;
        while (items.size() > 0) {
            nextLevelItems.clear();
            foreach (FolderItem *item, items) {
                QLOG_DEBUG() << "ID " << item->id;
                selectQuery.bindValue(":parentId", item->id);

                selectQuery.exec();

                if (!firstLevelUpdated) {
                    //NO size support
                    int numResults = 0;
                    while (selectQuery.next())
                        numResults++;

                    if (!selectQuery.seek(-1))
                        selectQuery.exec();
                    //END no size support

                    beginInsertRows(parent, 0, numResults - 1);
                }

                updateFolderModelData(selectQuery, item);

                if (!firstLevelUpdated) {
                    endInsertRows();
                    firstLevelUpdated = true;
                }

                nextLevelItems << item->children();
            }

            items.clear();
            items = nextLevelItems;
        }
        connectionName = db.connectionName();
    }
    QLOG_DEBUG() << "item->childCount()-1" << item->childCount() - 1;

    QSqlDatabase::removeDatabase(connectionName);
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
    newFolder.setManga(parentItem->data(Columns::Manga).toBool());
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
    data << false; //finished
    data << true; //completed
    data << newFolder.isManga();

    auto item = new FolderItem(data);
    item->id = newFolder.id;

    beginInsertRows(parent, 0, 0); //TODO calculate the destRow before inserting the new child

    parentItem->appendChild(item);
    destRow = parentItem->children().indexOf(item); //TODO optimize this, appendChild should return the index of the new item
    items.insert(item->id, item);

    endInsertRows();

    return index(destRow, 0, parent);
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
        DBHelper::updateChildrenInfo(item->parent()->id, db);
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
        DBHelper::updateChildrenInfo(folderId, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

//PROXY

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
        delete rootItem; //TODO comprobar que se libera bien la memoria

    rootItem = root;

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QMap<unsigned long long, FolderItem *>::iterator i;
    for (i = filteredItems->begin(); i != filteredItems->end(); ++i)
        this->filteredItems.insert(i.key(), i.value());
#else
    this->filteredItems.insert(*filteredItems);
#endif

    endResetModel();

    delete filteredItems;
}

void FolderModelProxy::clear()
{
    filterEnabled = false;

    filteredItems.clear();

    QSortFilterProxyModel::invalidate();
}
