
#include <QtGui>
#include <QtDebug>
#include <limits>

#include "comic_item.h"
#include "comic_model.h"
#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "comic_db.h"
#include "db_helper.h"
#include "query_parser.h"
#include "reading_list_model.h"

// ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read
#include "QsLog.h"

ComicModel::ComicModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

ComicModel::ComicModel(QSqlQuery &sqlquery, QObject *parent)
    : QAbstractItemModel(parent)
{
    setupModelData(sqlquery);
}

ComicModel::~ComicModel()
{
    qDeleteAll(_data);
}

int ComicModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (_data.isEmpty())
        return 0;
    return _data.first()->columnCount();
}

bool ComicModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    if (!enableResorting)
        return false;
    return data->formats().contains(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat);
}

// TODO: optimize this method (seriously)
bool ComicModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{

    QAbstractItemModel::dropMimeData(data, action, row, column, parent);
    QLOG_TRACE() << ">>>>>>>>>>>>>>dropMimeData ComicModel<<<<<<<<<<<<<<<<<" << parent << row << "," << column;

    if (!data->formats().contains(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat))
        return false;

    QList<qulonglong> comicIds = YACReader::mimeDataToComicsIds(data);
    QList<int> currentIndexes;
    int i;
    foreach (qulonglong id, comicIds) {
        i = 0;
        foreach (ComicItem *item, _data) {
            if (item->data(Id) == id) {
                currentIndexes << i;
                break;
            }
            i++;
        }
    }

    std::sort(currentIndexes.begin(), currentIndexes.end());
    QList<ComicItem *> resortedData;

    if (currentIndexes.contains(row)) // no resorting
        return false;

    ComicItem *destinationItem;
    if (row == -1 || row >= _data.length())
        destinationItem = 0;
    else
        destinationItem = _data.at(row);

    QList<int> newSorting;

    i = 0;
    foreach (ComicItem *item, _data) {
        if (!currentIndexes.contains(i)) {

            if (item == destinationItem) {
                foreach (int index, currentIndexes) {
                    resortedData << _data.at(index);
                    newSorting << index;
                }
            }

            resortedData << item;
            newSorting << i;
        }

        i++;
    }

    if (destinationItem == 0) {
        foreach (int index, currentIndexes) {
            resortedData << _data.at(index);
            newSorting << index;
        }
    }

    QLOG_TRACE() << newSorting;

    int tempRow = row;

    if (tempRow < 0)
        tempRow = _data.count();

    foreach (qulonglong id, comicIds) {
        int i = 0;
        foreach (ComicItem *item, _data) {
            if (item->data(Id) == id) {
                beginMoveRows(parent, i, i, parent, tempRow);

                bool skipElement = i == tempRow || i + 1 == tempRow;

                if (!skipElement) {
                    if (i > tempRow)
                        _data.move(i, tempRow);
                    else
                        _data.move(i, tempRow - 1);
                }

                endMoveRows();

                if (i > tempRow)
                    tempRow++;

                break;
            }
            i++;
        }
    }

    // TODO fix selection
    QList<qulonglong> allComicIds;
    foreach (ComicItem *item, _data) {
        allComicIds << item->data(Id).toULongLong();
    }
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        switch (mode) {
        case Favorites:
            DBHelper::reasignOrderToComicsInFavorites(allComicIds, db);
            break;
        case Label:
            DBHelper::reasignOrderToComicsInLabel(sourceId, allComicIds, db);
            break;
        case ReadingList:
            DBHelper::reasignOrderToComicsInReadingList(sourceId, allComicIds, db);
            break;
        default:
            break;
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    // endMoveRows();

    emit resortedIndexes(newSorting);
    int destSelectedIndex = row < 0 ? _data.length() : row;

    if (destSelectedIndex > currentIndexes.at(0))
        emit newSelectedIndex(index(qMax(0, destSelectedIndex - 1), 0, parent));
    else
        emit newSelectedIndex(index(qMax(0, destSelectedIndex), 0, parent));

    return true;
}

bool ComicModel::canBeResorted()
{
    return enableResorting;
}

QMimeData *ComicModel::mimeData(const QModelIndexList &indexes) const
{
    // custom model data
    // application/yacreader-comics-ids + list of ids in a QByteArray
    QList<qulonglong> ids;
    foreach (QModelIndex index, indexes) {
        QLOG_DEBUG() << "dragging : " << index.data(IdRole).toULongLong();
        ids << index.data(IdRole).toULongLong();
    }

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << ids; // serialize the list of identifiers

    auto mimeData = new QMimeData();
    mimeData->setData(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat, data);

    return mimeData;
}

QStringList ComicModel::mimeTypes() const
{
    QLOG_DEBUG() << "mimeTypes";
    QStringList list;
    list << YACReader::YACReaderLibrarComiscSelectionMimeDataFormat;
    return list;
}

QHash<int, QByteArray> ComicModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[NumberRole] = "number";
    roles[TitleRole] = "title";
    roles[FileNameRole] = "file_name";
    roles[NumPagesRole] = "num_pages";
    roles[IdRole] = "id";
    roles[Parent_IdRole] = "parent_id";
    roles[PathRole] = "path";
    roles[HashRole] = "hash";
    roles[ReadColumnRole] = "read_column";
    roles[IsBisRole] = "is_bis";
    roles[CurrentPageRole] = "current_page";
    roles[RatingRole] = "rating";
    roles[HasBeenOpenedRole] = "has_been_opened";
    roles[CoverPathRole] = "cover_path";

    return roles;
}

QVariant ComicModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    /*if (index.column() == TableModel::Rating && role == Qt::DecorationRole)
        {
                TableItem *item = static_cast<TableItem*>(index.internalPointer());
                return QPixmap(QString(":/images/rating%1.png").arg(item->data(index.column()).toInt()));
        }*/

    if (role == Qt::DecorationRole) {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) // TODO obtener esto de la query
        {
        case ComicModel::Number:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::NumPages:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::Hash:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::CurrentPage:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    // TODO check here if any view is asking for TableModel::Roles
    // these roles will be used from QML/GridView

    auto item = static_cast<ComicItem *>(index.internalPointer());

    if (role == NumberRole)
        return item->data(Number);
    else if (role == TitleRole)
        return item->data(Title).isNull() ? item->data(FileName) : item->data(Title);
    else if (role == FileNameRole)
        return item->data(FileName);
    else if (role == RatingRole)
        return item->data(Rating);
    else if (role == CoverPathRole)
        return getCoverUrlPathForComicHash(item->data(Hash).toString());
    else if (role == NumPagesRole)
        return item->data(NumPages);
    else if (role == CurrentPageRole)
        return item->data(CurrentPage);
    else if (role == ReadColumnRole)
        return item->data(ReadColumn).toBool();
    else if (role == HasBeenOpenedRole)
        return item->data(ComicModel::HasBeenOpened);
    else if (role == IdRole)
        return item->data(Id);

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() == ComicModel::Hash)
        return QString::number(item->data(index.column()).toString().right(item->data(index.column()).toString().length() - 40).toInt() / 1024.0 / 1024.0, 'f', 2) + "Mb";
    if (index.column() == ComicModel::ReadColumn)
        return (item->data(ComicModel::CurrentPage).toInt() == item->data(ComicModel::NumPages).toInt() || item->data(ComicModel::ReadColumn).toBool()) ? QVariant(tr("yes")) : QVariant(tr("no"));
    if (index.column() == ComicModel::CurrentPage)
        return item->data(ComicModel::HasBeenOpened).toBool() ? item->data(index.column()) : QVariant("-");

    if (index.column() == ComicModel::Rating)
        return QVariant();

    return item->data(index.column());
}

Qt::ItemFlags ComicModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};
    if (index.column() == ComicModel::Rating)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

QVariant ComicModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) // TODO obtener esto de la query
        {
        case ComicModel::Number:
            return QVariant(QString("#"));
        case ComicModel::Title:
            return QVariant(QString(tr("Title")));
        case ComicModel::FileName:
            return QVariant(QString(tr("File Name")));
        case ComicModel::NumPages:
            return QVariant(QString(tr("Pages")));
        case ComicModel::Hash:
            return QVariant(QString(tr("Size")));
        case ComicModel::ReadColumn:
            return QVariant(QString(tr("Read")));
        case ComicModel::CurrentPage:
            return QVariant(QString(tr("Current Page")));
        case ComicModel::Rating:
            return QVariant(QString(tr("Rating")));
        }
    }

    if (orientation == Qt::Horizontal && role == Qt::TextAlignmentRole) {
        switch (section) // TODO obtener esto de la query
        {
        case ComicModel::Number:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::NumPages:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::Hash:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::CurrentPage:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (orientation == Qt::Vertical && role == Qt::DecorationRole) {
        QString fileName = _data.value(section)->data(ComicModel::FileName).toString();
        QFileInfo fi(fileName);
        QString ext = fi.suffix();

        if (ext.compare("cbr", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/comicRar.png"));
        else if (ext.compare("cbz", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/comicZip.png"));
        else if (ext.compare("pdf", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/pdf.png"));
        else if (ext.compare("tar", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/tar.png"));
        else if (ext.compare("zip", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/zip.png"));
        else if (ext.compare("rar", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/rar.png"));
#ifndef use_unarr
        else if (ext.compare("7z", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/7z.png"));
        else if (ext.compare("cb7", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/comic7z.png"));
#endif
        else if (ext.compare("cbt", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/comicTar.png"));
    }

    return QVariant();
}

QModelIndex ComicModel::index(int row, int column, const QModelIndex &parent)
        const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, _data.at(row));
}

QModelIndex ComicModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int ComicModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return _data.count();

    return 0;
}

QStringList ComicModel::getPaths(const QString &_source)
{
    QStringList paths;
    QString source = _source + "/.yacreaderlibrary/covers/";
    QList<ComicItem *>::ConstIterator itr;
    for (itr = _data.constBegin(); itr != _data.constEnd(); itr++) {
        QString hash = (*itr)->data(ComicModel::Hash).toString();
        paths << source + hash + ".jpg";
    }

    return paths;
}

void ComicModel::setupFolderModelData(unsigned long long int folderId, const QString &databasePath)
{
    enableResorting = false;
    mode = Folder;
    sourceId = folderId;

    beginResetModel();
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "WHERE c.parentId = :parentId");
        selectQuery.bindValue(":parentId", folderId);
        selectQuery.exec();
        setupModelData(selectQuery);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    endResetModel();
}

void ComicModel::setupLabelModelData(unsigned long long parentLabel, const QString &databasePath)
{
    enableResorting = true;
    mode = Label;
    sourceId = parentLabel;

    beginResetModel();
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "INNER JOIN comic_label cl ON (c.id == cl.comic_id) "
                            "WHERE cl.label_id = :parentLabelId "
                            "ORDER BY cl.ordering");
        selectQuery.bindValue(":parentLabelId", parentLabel);
        selectQuery.exec();
        setupModelDataForList(selectQuery);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    endResetModel();
}

void ComicModel::setupReadingListModelData(unsigned long long parentReadingList, const QString &databasePath)
{
    mode = ReadingList;
    sourceId = parentReadingList;

    beginResetModel();
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QList<qulonglong> ids;
        ids << parentReadingList;

        QSqlQuery subfolders(db);
        subfolders.prepare("SELECT id "
                           "FROM reading_list "
                           "WHERE parentId = :parentId "
                           "ORDER BY ordering ASC");
        subfolders.bindValue(":parentId", parentReadingList);
        subfolders.exec();
        while (subfolders.next())
            ids << subfolders.record().value(0).toULongLong();

        enableResorting = ids.length() == 1; // only resorting if no sublists exist

        foreach (qulonglong id, ids) {
            QSqlQuery selectQuery(db);
            selectQuery.prepare("SELECT ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened "
                                "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                                "INNER JOIN comic_reading_list crl ON (c.id == crl.comic_id) "
                                "WHERE crl.reading_list_id = :parentReadingList "
                                "ORDER BY crl.ordering");
            selectQuery.bindValue(":parentReadingList", id);
            selectQuery.exec();

            // TODO, extra information is needed (resorting)
            QList<ComicItem *> tempData = _data;
            _data.clear();

            setupModelDataForList(selectQuery);

            _data = tempData << _data;
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    endResetModel();
}

void ComicModel::setupFavoritesModelData(const QString &databasePath)
{
    enableResorting = true;
    mode = Favorites;
    sourceId = -1;

    beginResetModel();
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "INNER JOIN comic_default_reading_list cdrl ON (c.id == cdrl.comic_id) "
                            "WHERE cdrl.default_reading_list_id = :parentDefaultListId "
                            "ORDER BY cdrl.ordering");
        selectQuery.bindValue(":parentDefaultListId", 1);
        selectQuery.exec();
        setupModelDataForList(selectQuery);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    endResetModel();
}

void ComicModel::setupReadingModelData(const QString &databasePath)
{
    enableResorting = false;
    mode = Reading;
    sourceId = -1;

    beginResetModel();
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "WHERE ci.hasBeenOpened = 1 AND ci.read = 0 "
                            "ORDER BY ci.lastTimeOpened DESC");
        selectQuery.exec();

        setupModelDataForList(selectQuery);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    endResetModel();
}

void ComicModel::setupAllModelData(const QString &databasePath)
{
    enableResorting = false;
    mode = Reading; // TODO: Investigate if this mode can be used
    sourceId = -1;

    beginResetModel();
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.isBis,ci.currentPage,ci.rating,ci.hasBeenOpened "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "ORDER BY ci.lastTimeOpened DESC");
        selectQuery.exec();

        setupModelDataForList(selectQuery);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    endResetModel();
}

void ComicModel::setModelData(QList<ComicItem *> *data, const QString &databasePath)
{
    _databasePath = databasePath;

    beginResetModel();

    qDeleteAll(_data);

    _data.clear();

    _data.append(*data);

    endResetModel();

    emit searchNumResults(_data.length());

    delete data;
}

QString ComicModel::getComicPath(QModelIndex mi)
{
    if (mi.isValid())
        return _data.at(mi.row())->data(ComicModel::Path).toString();
    return "";
}

void ComicModel::setupModelData(QSqlQuery &sqlquery)
{
    int numColumns = sqlquery.record().count();

    while (sqlquery.next()) {
        QList<QVariant> data;

        for (int i = 0; i < numColumns; i++)
            data << sqlquery.value(i);

        _data.append(new ComicItem(data));
    }

    std::sort(_data.begin(), _data.end(), [](const ComicItem *c1, const ComicItem *c2) {
        if (c1->data(ComicModel::Number).isNull() && c2->data(ComicModel::Number).isNull()) {
            return naturalSortLessThanCI(c1->data(ComicModel::FileName).toString(), c2->data(ComicModel::FileName).toString());
        } else {
            if (c1->data(ComicModel::Number).isNull() == false && c2->data(ComicModel::Number).isNull() == false) {
                return c1->data(ComicModel::Number).toInt() < c2->data(ComicModel::Number).toInt();
            } else {
                return c2->data(ComicModel::Number).isNull();
            }
        }
    });
}

// comics are sorted by "ordering", the sorting is done in the sql query
void ComicModel::setupModelDataForList(QSqlQuery &sqlquery)
{
    int numColumns = sqlquery.record().count();

    while (sqlquery.next()) {
        QList<QVariant> data;
        for (int i = 0; i < numColumns; i++)
            data << sqlquery.value(i);

        _data.append(new ComicItem(data));
    }
}

ComicDB ComicModel::getComic(const QModelIndex &mi)
{
    ComicDB c;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        c = DBHelper::loadComic(_data.at(mi.row())->data(ComicModel::Id).toULongLong(), db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return c;
}

ComicDB ComicModel::_getComic(const QModelIndex &mi)
{
    ComicDB c;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        c = DBHelper::loadComic(_data.at(mi.row())->data(ComicModel::Id).toULongLong(), db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return c;
}

QVector<YACReaderComicReadStatus> ComicModel::getReadList()
{
    int numComics = _data.count();
    QVector<YACReaderComicReadStatus> readList(numComics);
    for (int i = 0; i < numComics; i++) {
        if (_data.value(i)->data(ComicModel::ReadColumn).toBool())
            readList[i] = YACReader::Read;
        else if (_data.value(i)->data(ComicModel::CurrentPage).toInt() == _data.value(i)->data(ComicModel::NumPages).toInt())
            readList[i] = YACReader::Read;
        else if (_data.value(i)->data(ComicModel::HasBeenOpened).toBool())
            readList[i] = YACReader::Opened;
        else
            readList[i] = YACReader::Unread;
    }
    return readList;
}
// TODO untested, this method is no longer used
QVector<YACReaderComicReadStatus> ComicModel::setAllComicsRead(YACReaderComicReadStatus read)
{
    return setComicsRead(persistentIndexList(), read);
}

QList<ComicDB> ComicModel::getAllComics()
{
    QList<ComicDB> comics;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();

        int numComics = _data.count();
        for (int i = 0; i < numComics; i++) {
            comics.append(DBHelper::loadComic(_data.value(i)->data(ComicModel::Id).toULongLong(), db));
        }

        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return comics;
}

QList<ComicDB> ComicModel::getComics(QList<QModelIndex> list)
{
    QList<ComicDB> comics;
    for (auto itr = list.constBegin(); itr != list.constEnd(); itr++) {
        comics.append(_getComic(*itr));
    }
    return comics;
}
// TODO
QVector<YACReaderComicReadStatus> ComicModel::setComicsRead(QList<QModelIndex> list, YACReaderComicReadStatus read)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        foreach (QModelIndex mi, list) {
            if (read == YACReader::Read) {
                _data.value(mi.row())->setData(ComicModel::ReadColumn, QVariant(true));
                ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(ComicModel::Id).toULongLong(), db);
                c.info.read = true;
                DBHelper::update(&(c.info), db);
            }
            if (read == YACReader::Unread) {
                _data.value(mi.row())->setData(ComicModel::ReadColumn, QVariant(false));
                _data.value(mi.row())->setData(ComicModel::CurrentPage, QVariant(1));
                _data.value(mi.row())->setData(ComicModel::HasBeenOpened, QVariant(false));
                ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(ComicModel::Id).toULongLong(), db);
                c.info.read = false;
                c.info.currentPage = 1;
                c.info.hasBeenOpened = false;
                c.info.lastTimeOpened.setValue(QVariant());
                DBHelper::update(&(c.info), db);
            }
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    emit dataChanged(index(list.first().row(), ComicModel::ReadColumn), index(list.last().row(), ComicModel::HasBeenOpened), QVector<int>() << ReadColumnRole << CurrentPageRole << HasBeenOpenedRole);

    return getReadList();
}

void ComicModel::setComicsManga(QList<QModelIndex> list, bool isManga)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        foreach (QModelIndex mi, list) {
            ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(ComicModel::Id).toULongLong(), db);
            c.info.manga = isManga;
            DBHelper::update(&(c.info), db);
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

qint64 ComicModel::asignNumbers(QList<QModelIndex> list, int startingNumber)
{
    qint64 idFirst;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        idFirst = _data.value(list[0].row())->data(ComicModel::Id).toULongLong();
        int i = 0;
        foreach (QModelIndex mi, list) {
            ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(ComicModel::Id).toULongLong(), db);
            c.info.number = startingNumber + i;
            c.info.edited = true;
            DBHelper::update(&(c.info), db);
            i++;
        }
        db.commit();
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);

    return idFirst;
}
QModelIndex ComicModel::getIndexFromId(quint64 id)
{
    QList<ComicItem *>::ConstIterator itr;
    int i = 0;
    for (itr = _data.constBegin(); itr != _data.constEnd(); itr++) {
        if ((*itr)->data(ComicModel::Id).toULongLong() == id)
            break;
        i++;
    }

    return index(i, 0);
}

// TODO completely inefficiently
QList<QModelIndex> ComicModel::getIndexesFromIds(const QList<qulonglong> &comicIds)
{
    QList<QModelIndex> comicsIndexes;

    foreach (qulonglong id, comicIds)
        comicsIndexes << getIndexFromId(id);

    return comicsIndexes;
}

void ComicModel::startTransaction()
{
    auto dbTransaction = DataBaseManagement::loadDatabase(_databasePath);
    _databaseConnection = dbTransaction.connectionName();
    dbTransaction.transaction();
}

void ComicModel::finishTransaction()
{
    {
        QSqlDatabase::database(_databaseConnection).commit();
    }
    QSqlDatabase::removeDatabase(_databaseConnection);
}

void ComicModel::removeInTransaction(int row)
{
    auto dbTransaction = QSqlDatabase::database(_databaseConnection);
    ComicDB c = DBHelper::loadComic(_data.at(row)->data(ComicModel::Id).toULongLong(), dbTransaction);

    DBHelper::removeFromDB(&c, dbTransaction);
    beginRemoveRows(QModelIndex(), row, row);
    removeRow(row);
    delete _data.at(row);
    _data.removeAt(row);

    endRemoveRows();
}

void ComicModel::remove(int row)
{
    removeInTransaction(row);
}

void ComicModel::reload(const ComicDB &comic)
{
    int row = 0;
    bool found = false;
    foreach (ComicItem *item, _data) {
        if (item->data(ComicModel::Id).toULongLong() == comic.id) {
            found = true;
            item->setData(ComicModel::ReadColumn, comic.info.read);
            item->setData(ComicModel::CurrentPage, comic.info.currentPage);
            item->setData(ComicModel::HasBeenOpened, true);
            break;
        }
        row++;
    }
    if (found)
        emit dataChanged(index(row, ReadColumn), index(row, HasBeenOpened), QVector<int>() << ReadColumnRole << CurrentPageRole << HasBeenOpenedRole);
}

void ComicModel::resetComicRating(const QModelIndex &mi)
{
    ComicDB comic = getComic(mi);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        comic.info.rating = 0;
        _data[mi.row()]->setData(ComicModel::Rating, 0);
        DBHelper::update(&(comic.info), db);

        emit dataChanged(mi, mi);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

QUrl ComicModel::getCoverUrlPathForComicHash(const QString &hash) const
{
    return QUrl("file:" + _databasePath + "/covers/" + hash + ".jpg");
}

void ComicModel::addComicsToFavorites(const QList<qulonglong> &comicIds)
{
    addComicsToFavorites(getIndexesFromIds(comicIds));
}

void ComicModel::addComicsToFavorites(const QList<QModelIndex> &comicsList)
{
    QList<ComicDB> comics = getComics(comicsList);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        DBHelper::insertComicsInFavorites(comics, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void ComicModel::addComicsToLabel(const QList<qulonglong> &comicIds, qulonglong labelId)
{
    addComicsToLabel(getIndexesFromIds(comicIds), labelId);
}

void ComicModel::addComicsToLabel(const QList<QModelIndex> &comicsList, qulonglong labelId)
{
    QList<ComicDB> comics = getComics(comicsList);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        DBHelper::insertComicsInLabel(comics, labelId, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void ComicModel::addComicsToReadingList(const QList<qulonglong> &comicIds, qulonglong readingListId)
{
    addComicsToReadingList(getIndexesFromIds(comicIds), readingListId);
}

void ComicModel::addComicsToReadingList(const QList<QModelIndex> &comicsList, qulonglong readingListId)
{
    QList<ComicDB> comics = getComics(comicsList);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        DBHelper::insertComicsInReadingList(comics, readingListId, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void ComicModel::deleteComicsFromFavorites(const QList<QModelIndex> &comicsList)
{
    QList<ComicDB> comics = getComics(comicsList);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        DBHelper::deleteComicsFromFavorites(comics, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (mode == Favorites)
        deleteComicsFromModel(comicsList);
}

void ComicModel::deleteComicsFromReading(const QList<QModelIndex> &comicsList)
{
    QList<ComicDB> comics = getComics(comicsList);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        DBHelper::deleteComicsFromReading(comics, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (mode == Reading)
        deleteComicsFromModel(comicsList);
}

void ComicModel::deleteComicsFromSpecialList(const QList<QModelIndex> &comicsList, qulonglong specialListId)
{
    auto type = (ReadingListModel::TypeSpecialList)specialListId;

    // TODO: Check if it should support "All" path
    switch (type) {
    case ReadingListModel::TypeSpecialList::Reading:
        deleteComicsFromReading(comicsList);
        break;
    case ReadingListModel::TypeSpecialList::Favorites:
        deleteComicsFromFavorites(comicsList);
        break;
    }
}

void ComicModel::deleteComicsFromLabel(const QList<QModelIndex> &comicsList, qulonglong labelId)
{
    QList<ComicDB> comics = getComics(comicsList);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        DBHelper::deleteComicsFromLabel(comics, labelId, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    deleteComicsFromModel(comicsList);
}

void ComicModel::deleteComicsFromReadingList(const QList<QModelIndex> &comicsList, qulonglong readingListId)
{
    QList<ComicDB> comics = getComics(comicsList);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        DBHelper::deleteComicsFromReadingList(comics, readingListId, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    deleteComicsFromModel(comicsList);
}

void ComicModel::deleteComicsFromModel(const QList<QModelIndex> &comicsList)
{
    QListIterator<QModelIndex> it(comicsList);
    it.toBack();
    while (it.hasPrevious()) {
        int row = it.previous().row();
        beginRemoveRows(QModelIndex(), row, row);
        _data.removeAt(row);
        endRemoveRows();
    }

    if (_data.isEmpty())
        emit isEmpty();
}

bool ComicModel::isFavorite(const QModelIndex &index)
{
    bool isFavorite;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        isFavorite = DBHelper::isFavoriteComic(_data[index.row()]->data(Id).toLongLong(), db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return isFavorite;
}

void ComicModel::updateRating(int rating, QModelIndex mi)
{
    ComicDB comic = getComic(mi);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        // TODO optimize update

        comic.info.rating = rating;
        _data[mi.row()]->setData(ComicModel::Rating, rating);
        DBHelper::update(&(comic.info), db);

        emit dataChanged(mi, mi);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}
