
#include "comic_model.h"

#include "comic.h"
#include "comic_db.h"
#include "comic_item.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "overloaded.h"
#include "qnaturalsorting.h"
#include "reading_list_model.h"
#include "yacreader_global_gui.h"

#include <QMap>
#include <QSet>
#include <QSqlRecord>
#include <QStringBuilder>
#include <QtDebug>
#include <QtGui>

#include <algorithm>
#include <utility>

#ifdef use_unarr
#include <unarr.h>
#endif

// ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read
#include "QsLog.h"

namespace {

const ComicDB &rowComic(const ComicItem &item)
{
    return std::visit(overloaded {
                              [](const ComicItemCase::Comic &row) -> const ComicDB & { return row.comic; },
                              [](const ComicItemCase::ComicInList &row) -> const ComicDB & { return row.comic; },
                      },
                      item);
}

ComicDB &rowComic(ComicItem &item)
{
    return std::visit(overloaded {
                              [](ComicItemCase::Comic &row) -> ComicDB & { return row.comic; },
                              [](ComicItemCase::ComicInList &row) -> ComicDB & { return row.comic; },
                      },
                      item);
}

bool sameRow(const ComicItem &a, const ComicItem &b)
{
    if (a.index() != b.index())
        return false;

    return std::visit(overloaded {
                              [&](const ComicItemCase::Comic &row) { return row.comic.file.id == std::get<ComicItemCase::Comic>(b).comic.file.id; },
                              [&](const ComicItemCase::ComicInList &row) { return row.entry.id == std::get<ComicItemCase::ComicInList>(b).entry.id; },
                      },
                      a);
}

bool sameEntry(const ComicItem &a, const ComicItem &b)
{
    const auto *entryA = std::get_if<ComicItemCase::ComicInList>(&a);
    const auto *entryB = std::get_if<ComicItemCase::ComicInList>(&b);
    return (entryA == nullptr) == (entryB == nullptr) && (entryA == nullptr || entryA->entry == entryB->entry);
}

bool sameValues(const ComicItem &a, const ComicItem &b)
{
    return sameRow(a, b) && sameEntry(a, b) && rowComic(a).file == rowComic(b).file && rowComic(a).info.hasSameValues(rowComic(b).info);
}

QVariant columnValue(const ComicDB &comic, int column)
{
    switch (column) {
    case ComicModel::Number:
        return comic.info.number;
    case ComicModel::Title:
        return comic.info.title;
    case ComicModel::FileName:
        return comic.file.fileName;
    case ComicModel::NumPages:
        return comic.info.numPages;
    case ComicModel::Id:
        return comic.file.id.value();
    case ComicModel::Parent_Id:
        return comic.file.folderId.value();
    case ComicModel::Path:
        return comic.file.path;
    case ComicModel::Hash:
        return comic.info.hash;
    case ComicModel::ReadColumn:
        return comic.info.read;
    case ComicModel::CurrentPage:
        return comic.info.currentPage;
    case ComicModel::Rating:
        return comic.info.rating;
    case ComicModel::HasBeenOpened:
        return qlonglong(comic.info.hasBeenOpened ? 1 : 0); // the views got the SQL integer
    case ComicModel::PublicationDate:
        return comic.info.date;
    case ComicModel::Added:
        return comic.info.added;
    case ComicModel::Type:
        return comic.info.type;
    case ComicModel::LastTimeOpened:
        return comic.info.lastTimeOpened;
    case ComicModel::Series:
        return comic.info.series;
    case ComicModel::Volume:
        return comic.info.volume;
    case ComicModel::StoryArc:
        return comic.info.storyArc;
    }
    return QVariant();
}

bool defaultFolderContentSortFunction(const ComicItem &c1, const ComicItem &c2)
{
    const auto &comic1 = rowComic(c1);
    const auto &comic2 = rowComic(c2);
    return comicNumberLessThan(comic1.info.number, comic1.file.fileName, comic2.info.number, comic2.file.fileName);
}

}

ComicModel::ComicModel(QObject *parent)
    : QAbstractItemModel(parent), showRecent(false), recentDays(1)

{
}

ComicModel::~ComicModel()
{
}

int ComicModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (_data.isEmpty())
        return 0;
    return ComicModel::Size + 1;
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
    QLOG_TRACE() << ">>>>>>>>>>>>>>dropMimeData ComicModel<<<<<<<<<<<<<<<<<" << parent << row << "," << column;

    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    const QList<qulonglong> comicIds = YACReader::mimeDataToComicsIds(data);
    if (comicIds.isEmpty())
        return false;

    if (row < 0 || row > _data.count())
        row = _data.count();

    QList<int> currentIndexes;
    int i;
    {
        const auto &currentData = _data;
        for (const auto id : comicIds) {
            i = 0;
            for (const auto &item : currentData) {
                if (rowComic(item).file.id.value() == id) {
                    currentIndexes << i;
                    break;
                }
                i++;
            }
        }
    }

    if (currentIndexes.size() != comicIds.size())
        return false;

    std::sort(currentIndexes.begin(), currentIndexes.end());

    if (currentIndexes.contains(row)) // no resorting
        return false;

    const int destinationRow = (row == -1 || row >= _data.length()) ? -1 : row;

    QList<int> newSorting;

    const auto &currentIndexList = currentIndexes;
    {
        for (i = 0; i < _data.length(); i++) {
            if (!currentIndexes.contains(i)) {

                if (i == destinationRow) {
                    for (const auto index : currentIndexList) {
                        newSorting << index;
                    }
                }

                newSorting << i;
            }
        }
    }

    if (destinationRow == -1) {
        for (const auto index : currentIndexList) {
            newSorting << index;
        }
    }

    QLOG_TRACE() << newSorting;

    int tempRow = row;

    for (const auto id : comicIds) {
        int i = 0;
        const auto dataSnapshot = _data;
        for (const auto &item : dataSnapshot) {
            if (rowComic(item).file.id.value() == id) {
                const bool skipElement = i == tempRow || i + 1 == tempRow;

                if (!skipElement) {
                    if (!beginMoveRows(parent, i, i, parent, tempRow))
                        return false;

                    if (i > tempRow)
                        _data.move(i, tempRow);
                    else
                        _data.move(i, tempRow - 1);

                    endMoveRows();
                }

                if (i > tempRow)
                    tempRow++;

                break;
            }
            i++;
        }
    }

    // TODO fix selection
    QList<qulonglong> allComicIds;
    QList<ListItemId> allEntryIds;
    for (const auto &item : std::as_const(_data)) {
        std::visit(overloaded {
                           [&allComicIds](const ComicItemCase::Comic &row) { allComicIds << row.comic.file.id.value(); },
                           [&allEntryIds](const ComicItemCase::ComicInList &row) { allEntryIds << row.entry.id; },
                   },
                   item);
    }
    bool reordered = true;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        switch (mode) {
        case Favorites:
            DBHelper::reasignOrderToComicsInFavorites(allComicIds, db);
            break;
        case Label:
            reordered = DBHelper::reorderLabelEntries(LabelId(sourceId), allEntryIds, db);
            break;
        case ReadingList:
            reordered = DBHelper::reorderReadingListEntries(ReadingListId(sourceId), allEntryIds, db);
            break;
        case Folder:
        case Reading:
        case Recent:
        case SearchResult:
            break;
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    // the rows show the stored order again
    if (!reordered) {
        reload();
        return false;
    }

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
    for (const auto &index : indexes) {
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
    roles[CurrentPageRole] = "current_page";
    roles[RatingRole] = "rating";
    roles[HasBeenOpenedRole] = "has_been_opened";
    roles[CoverPathRole] = "cover_path";
    roles[PublicationDate] = "date";
    roles[ReadableTitle] = "readable_title";
    roles[AddedRole] = "added_date";
    roles[TypeRole] = "type";
    roles[ShowRecentRole] = "show_recent";
    roles[RecentRangeRole] = "recent_range";
    roles[SizeRole] = "size";
    roles[SeriesRole] = "series";
    roles[VolumeRole] = "volume";
    roles[StoryArcRole] = "story_arc";

    return roles;
}

QVariant ComicModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

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
        case ComicModel::Size:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::CurrentPage:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    // TODO check here if any view is asking for TableModel::Roles
    // these roles will be used from QML/GridView

    const auto &comic = rowComic(_data.at(index.row()));
    const auto value = [&comic](int column) { return columnValue(comic, column); };

    auto sizeString = [=] {
        auto bytes = value(ComicModel::Hash).toString().right(value(ComicModel::Hash).toString().length() - 40).toULongLong();

        QStringList units = { "B", "KB", "MB", "GB", "TB" };
        int i;
        double outputSize = bytes;
        for (i = 0; i < units.size() - 1; i++) {
            if (outputSize < 1024) {
                break;
            }
            outputSize = outputSize / 1024;
        }
        return QString("%1 %2").arg(outputSize, 0, 'f', 2).arg(units[i]);
    };

    if (role == NumberRole)
        return value(Number);
    else if (role == TitleRole)
        return value(Title).isNull() ? value(FileName) : value(Title);
    else if (role == ReadableTitle) {
        QString title;
        if (!value(Number).isNull()) {
            title = title % "#" % value(Number).toString() % " ";
        }
        return QVariant(title % (value(Title).isNull() ? value(FileName).toString() : value(Title).toString()));
    } else if (role == FileNameRole)
        return value(FileName);
    else if (role == RatingRole)
        return value(Rating);
    else if (role == CoverPathRole) {
        auto coverUrl = getCoverUrlPathForComicHash(value(Hash).toString());
        const auto revision = coverRevisions.value(comic.info.id.value());
        if (revision > 0)
            coverUrl.setQuery(QStringLiteral("revision=%1").arg(revision));
        return coverUrl;
    } else if (role == NumPagesRole)
        return value(NumPages);
    else if (role == CurrentPageRole)
        return value(CurrentPage);
    else if (role == ReadColumnRole)
        return value(ReadColumn).toBool();
    else if (role == HasBeenOpenedRole)
        return value(HasBeenOpened);
    else if (role == IdRole)
        return value(Id);
    else if (role == HashRole)
        return value(Hash);
    else if (role == PublicationDateRole)
        return QVariant(localizedDate(value(PublicationDate).toString()));
    else if (role == AddedRole)
        return value(Added);
    else if (role == TypeRole)
        return value(Type);
    else if (role == ShowRecentRole)
        return showRecent;
    else if (role == RecentRangeRole)
        return recentDays * 86400;
    else if (role == SizeRole)
        return sizeString();
    else if (role == SeriesRole)
        return value(Series);
    else if (role == VolumeRole)
        return value(Volume);
    else if (role == StoryArcRole)
        return value(StoryArc);

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() == ComicModel::Hash)
        return value(ComicModel::Hash).toString();
    if (index.column() == ComicModel::Size)
        return sizeString();
    if (index.column() == ComicModel::ReadColumn)
        return (value(ComicModel::CurrentPage).toInt() == value(ComicModel::NumPages).toInt() || value(ComicModel::ReadColumn).toBool()) ? QVariant(tr("yes")) : QVariant(tr("no"));
    if (index.column() == ComicModel::CurrentPage)
        return value(ComicModel::HasBeenOpened).toBool() ? value(index.column()) : QVariant("-");

    if (index.column() == ComicModel::Rating)
        return QVariant();

    if (index.column() == ComicModel::PublicationDate) {
        return QVariant(localizedDate(value(PublicationDate).toString()));
    }

    return value(index.column());
}

Qt::ItemFlags ComicModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return { };
    if (index.column() == ComicModel::Rating)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

QVariant ComicModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ComicModel::Number:
            return QVariant(QString("#"));
        case ComicModel::Title:
            return QVariant(QString(tr("Title")));
        case ComicModel::FileName:
            return QVariant(QString(tr("File Name")));
        case ComicModel::NumPages:
            return QVariant(QString(tr("Pages")));
        case ComicModel::Size:
            return QVariant(QString(tr("Size")));
        case ComicModel::ReadColumn:
            return QVariant(QString(tr("Read")));
        case ComicModel::CurrentPage:
            return QVariant(QString(tr("Current Page")));
        case ComicModel::PublicationDate:
            return QVariant(QString(tr("Publication Date")));
        case ComicModel::Rating:
            return QVariant(QString(tr("Rating")));
        case ComicModel::Series:
            return QVariant(QString(tr("Series")));
        case ComicModel::Volume:
            return QVariant(QString(tr("Volume")));
        case ComicModel::StoryArc:
            return QVariant(QString(tr("Story Arc")));
        }
    }

    if (orientation == Qt::Horizontal && role == Qt::TextAlignmentRole) {
        switch (section) {
        case ComicModel::Number:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::NumPages:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::Size:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case ComicModel::CurrentPage:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (orientation == Qt::Vertical && role == Qt::DecorationRole) {
        const QString fileName = (section >= 0 && section < _data.size()) ? rowComic(_data.at(section)).file.fileName : QString();
        QFileInfo fi(fileName);
        QString ext = fi.suffix();

        if (ext.compare("cbr", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/comicRar.png"));
        else if (ext.compare("cbz", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/comicZip.png"));
        else if (Comic::fileIsEpub(fileName))
            return QVariant(QIcon(":/images/comicZip.png"));
        else if (ext.compare("pdf", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/pdf.png"));
        else if (ext.compare("tar", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/tar.png"));
        else if (ext.compare("zip", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/zip.png"));
        else if (ext.compare("rar", Qt::CaseInsensitive) == 0)
            return QVariant(QIcon(":/images/rar.png"));
#if !defined(use_unarr) || (UNARR_API_VERSION >= 110)
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

    return createIndex(row, column);
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
    for (const auto &item : std::as_const(_data)) {
        paths << LibraryPaths::coverPath(_source, rowComic(item).info.hash);
    }

    return paths;
}

QList<ComicItem> ComicModel::createFolderModelData(unsigned long long folderId, const QString &databasePath) const
{
    QList<ComicItem> modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.setForwardOnly(true);
        selectQuery.prepare("SELECT " + ComicDB::selectList("c", "ci") + " "
                                                                         "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                                                                         "WHERE c.parentId = :parentId");
        selectQuery.bindValue(":parentId", folderId);
        selectQuery.exec();

        modelData = createItems(selectQuery);
        std::sort(modelData.begin(), modelData.end(), defaultFolderContentSortFunction);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return modelData;
}

void ComicModel::setupFolderModelData(unsigned long long int folderId, const QString &databasePath)
{
    enableResorting = false;
    mode = Folder;
    sourceId = folderId;

    beginResetModel();
    _data.clear();

    _databasePath = databasePath;

    takeData(createFolderModelData(folderId, databasePath));

    endResetModel();
}

QList<ComicItem> ComicModel::createLabelModelData(unsigned long long parentLabel, const QString &databasePath) const
{
    QList<ComicItem> modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.setForwardOnly(true);
        selectQuery.prepare(DBHelper::listComicsQuery(ListKind::Label));
        selectQuery.bindValue(":list_id", parentLabel);
        selectQuery.exec();
        modelData = createItems(selectQuery, ListKind::Label);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return modelData;
}

void ComicModel::setupLabelModelData(unsigned long long parentLabel, const QString &databasePath)
{
    enableResorting = true;
    mode = Label;
    sourceId = parentLabel;

    beginResetModel();
    _data.clear();

    _databasePath = databasePath;

    takeData(createLabelModelData(parentLabel, databasePath));

    endResetModel();
}

QList<ComicItem> ComicModel::createReadingListData(unsigned long long parentReadingList, const QString &databasePath, bool &enableResorting) const
{
    QList<ComicItem> modelData;

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

        QSqlQuery selectQuery(db);
        selectQuery.setForwardOnly(true);
        selectQuery.prepare(DBHelper::listComicsQuery(ListKind::ReadingList));
        for (const auto id : std::as_const(ids)) {
            selectQuery.bindValue(":list_id", id);
            selectQuery.exec();

            modelData << createItems(selectQuery, ListKind::ReadingList);
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return modelData;
}

void ComicModel::setupReadingListModelData(unsigned long long parentReadingList, const QString &databasePath)
{
    mode = ReadingList;
    sourceId = parentReadingList;

    beginResetModel();
    _data.clear();

    _databasePath = databasePath;

    takeData(createReadingListData(parentReadingList, databasePath, enableResorting));

    endResetModel();
}

QList<ComicItem> ComicModel::createFavoritesModelData(const QString &databasePath) const
{
    QList<ComicItem> modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.setForwardOnly(true);
        selectQuery.prepare("SELECT " + ComicDB::selectList("c", "ci") + " "
                                                                         "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                                                                         "INNER JOIN comic_default_reading_list cdrl ON (c.id == cdrl.comic_id) "
                                                                         "WHERE cdrl.default_reading_list_id = :parentDefaultListId "
                                                                         "ORDER BY cdrl.ordering");
        selectQuery.bindValue(":parentDefaultListId", 1);
        selectQuery.exec();
        modelData = createItems(selectQuery);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return modelData;
}

void ComicModel::setupFavoritesModelData(const QString &databasePath)
{
    enableResorting = true;
    mode = Favorites;
    sourceId = -1;

    beginResetModel();
    _data.clear();

    _databasePath = databasePath;

    takeData(createFavoritesModelData(databasePath));

    endResetModel();
}

QList<ComicItem> ComicModel::createReadingModelData(const QString &databasePath) const
{
    QList<ComicItem> modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.setForwardOnly(true);
        selectQuery.prepare("SELECT " + ComicDB::selectList("c", "ci") + " "
                                                                         "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                                                                         "WHERE ci.hasBeenOpened = 1 AND ci.read = 0 "
                                                                         "ORDER BY ci.lastTimeOpened DESC");
        selectQuery.exec();

        modelData = createItems(selectQuery);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return modelData;
}

void ComicModel::setupReadingModelData(const QString &databasePath)
{
    enableResorting = false;
    mode = Reading;
    sourceId = -1;

    beginResetModel();
    _data.clear();

    _databasePath = databasePath;

    takeData(createReadingModelData(databasePath));

    endResetModel();
}

QList<ComicItem> ComicModel::createRecentModelData(const QString &databasePath) const
{
    QList<ComicItem> modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.setForwardOnly(true);
        selectQuery.prepare("SELECT " + ComicDB::selectList("c", "ci") + " "
                                                                         "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                                                                         "WHERE ci.added > :limit "
                                                                         "ORDER BY ci.added DESC");
        selectQuery.bindValue(":limit", QDateTime::currentDateTime().addDays(-recentDays).toSecsSinceEpoch());
        selectQuery.exec();

        modelData = createItems(selectQuery);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return modelData;
}

void ComicModel::setupRecentModelData(const QString &databasePath)
{
    enableResorting = false;
    mode = Recent;
    sourceId = -1;

    beginResetModel();
    _data.clear();

    _databasePath = databasePath;

    takeData(createRecentModelData(databasePath));

    endResetModel();
}

void ComicModel::setModelData(QList<ComicItem> *data, const QString &databasePath)
{
    enableResorting = false;
    mode = SearchResult;
    sourceId = -1;

    _databasePath = databasePath;

    beginResetModel();

    _data = *data;

    endResetModel();

    delete data;
}

QString ComicModel::getComicPath(QModelIndex mi)
{
    if (mi.isValid())
        return rowComic(_data.at(mi.row())).file.path;
    return "";
}

QList<ComicItem> ComicModel::createItems(QSqlQuery &query)
{
    QList<ComicItem> items;
    const auto indexes = ComicDB::columnIndexes(query.record(), "c", "ci");
    while (query.next()) {
        items.append(ComicItemCase::Comic { ComicDB::fromQuery(query, indexes) });
    }
    return items;
}

// the sorting of lists is done in the sql query
QList<ComicItem> ComicModel::createItems(QSqlQuery &query, ListKind listKind)
{
    QList<ComicItem> items;
    const auto record = query.record();
    const auto indexes = ComicDB::columnIndexes(record, "c", "ci");
    const auto entryIndexes = ListEntry::columnIndexes(record, "e", listKind);
    while (query.next()) {
        items.append(ComicItemCase::ComicInList { ComicDB::fromQuery(query, indexes), ListEntry::fromQuery(query, entryIndexes, listKind) });
    }
    return items;
}

QList<ComicItem> ComicModel::sortedForFolder(QList<ComicItem> items)
{
    std::sort(items.begin(), items.end(), defaultFolderContentSortFunction);
    return items;
}

void ComicModel::takeData(const QList<ComicItem> &data)
{
    _data = data;
}

void ComicModel::takeUpdatedData(const QList<ComicItem> &updatedData, std::function<bool(const ComicItem &, const ComicItem &)> comparator)
{
    int length = _data.size();
    int lengthUpdated = updatedData.size();

    int i; // index of the internal data
    int j; // index of the updated children
    for (i = 0, j = 0; i < length && j < lengthUpdated;) {
        const auto &comic = _data.at(i);
        const auto &updatedComic = updatedData.at(j);

        if (sameRow(comic, updatedComic)) {
            if (!sameValues(comic, updatedComic)) {
                auto modelIndexToUpdate = index(i, 0, QModelIndex());

                _data[i] = updatedComic;

                emit dataChanged(modelIndexToUpdate, modelIndexToUpdate);
            }

            i++;
            j++;
            continue;
        }

        auto lessThan = comparator(comic, updatedComic);

        // comic added
        if (!lessThan) {
            beginInsertRows(QModelIndex(), i, i);

            _data.insert(i, updatedComic);

            endInsertRows();

            i++;
            j++;
            length++;
            continue;
        }

        // comic removed
        if (lessThan) {
            beginRemoveRows(QModelIndex(), i, i);

            _data.removeAt(i);

            endRemoveRows();

            length--;
            continue;
        }
    }

    // add remaining comics
    for (; j < lengthUpdated; j++) {
        beginInsertRows(QModelIndex(), i, i);

        _data.append(updatedData.at(j));

        endInsertRows();

        i++;
    }

    // remove remaining comics
    for (int k = length - 1; k >= i; k--) {
        beginRemoveRows(QModelIndex(), k, k);

        _data.removeAt(k);

        endRemoveRows();
    }
}

ComicDB ComicModel::getComic(const QModelIndex &mi)
{
    ComicDB c;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        bool found;
        c = DBHelper::loadComic(rowComic(_data.at(mi.row())).file.id.value(), db, found);
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
        const auto &info = rowComic(_data.at(i)).info;
        if (info.read)
            readList[i] = YACReader::Read;
        else if (info.currentPage == info.numPages.toInt())
            readList[i] = YACReader::Read;
        else if (info.hasBeenOpened)
            readList[i] = YACReader::Opened;
        else
            readList[i] = YACReader::Unread;
    }
    return readList;
}
QList<ComicDB> ComicModel::getAllComics()
{
    QList<ComicDB> comics;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        QList<qulonglong> ids;
        ids.reserve(_data.size());
        for (const auto &item : std::as_const(_data)) {
            ids.append(rowComic(item).file.id.value());
        }
        comics = DBHelper::loadComics(ids, db);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return comics;
}

QList<ComicDB> ComicModel::getComics(QList<QModelIndex> list)
{
    QList<ComicDB> comics;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        QList<qulonglong> ids;
        ids.reserve(list.size());
        for (const auto &mi : std::as_const(list)) {
            ids.append(rowComic(_data.at(mi.row())).file.id.value());
        }
        comics = DBHelper::loadComics(ids, db);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return comics;
}

std::optional<ComicDB> ComicModel::firstUnreadComic() const
{
    for (const auto &item : _data) {
        const auto &comic = rowComic(item);
        if (!comic.info.read) {
            return comic;
        }
    }
    return std::nullopt;
}
// TODO
QVector<YACReaderComicReadStatus> ComicModel::setComicsRead(QList<QModelIndex> list, YACReaderComicReadStatus read)
{
    QString connectionName = "";
    const auto &comicIndexes = list;
    QList<ComicInfoId> changed;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        for (const auto &mi : comicIndexes) {
            if (read == YACReader::Read) {
                bool found;
                ComicDB c = DBHelper::loadComic(rowComic(_data.at(mi.row())).file.id.value(), db, found);
                c.info.read = true;
                DBHelper::update(&(c.info), db);
            }
            if (read == YACReader::Unread) {
                bool found;
                ComicDB c = DBHelper::loadComic(rowComic(_data.at(mi.row())).file.id.value(), db, found);
                c.info.read = false;
                c.info.currentPage = 1;
                c.info.hasBeenOpened = false;
                c.info.lastTimeOpened.setValue(QVariant());
                DBHelper::update(&(c.info), db);
            }
            if (read == YACReader::Read || read == YACReader::Unread)
                changed.append(rowComic(_data.at(mi.row())).info.id);
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    for (const auto id : std::as_const(changed)) {
        updateRowsOf(id, [read](ComicInfo &info) {
            info.read = read == YACReader::Read;
            if (read == YACReader::Unread) {
                info.currentPage = 1;
                info.hasBeenOpened = false;
            } }, ComicModel::ReadColumn, ComicModel::HasBeenOpened, { ReadColumnRole, CurrentPageRole, HasBeenOpenedRole });
    }

    return getReadList();
}

void ComicModel::setComicsType(QList<QModelIndex> list, FileType type)
{
    QString connectionName = "";
    const auto &comicIndexes = list;
    QList<ComicInfoId> changed;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        for (const auto &mi : comicIndexes) {
            bool found;
            ComicDB c = DBHelper::loadComic(rowComic(_data.at(mi.row())).file.id.value(), db, found);
            c.info.type = QVariant::fromValue(type);
            DBHelper::update(&(c.info), db);
            changed.append(rowComic(_data.at(mi.row())).info.id);
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    for (const auto id : std::as_const(changed)) {
        updateRowsOf(id, [type](ComicInfo &info) { info.type = QVariant::fromValue(type); }, ComicModel::Type, ComicModel::Type, { TypeRole });
    }
}

void ComicModel::updateRowsOf(ComicInfoId id, const std::function<void(ComicInfo &)> &change, int firstColumn, int lastColumn, const QList<int> &roles)
{
    for (int row = 0; row < _data.size(); row++) {
        auto &comic = rowComic(_data[row]);
        if (comic.info.id != id)
            continue;
        change(comic.info);
        emit dataChanged(index(row, firstColumn), index(row, lastColumn), roles);
    }
}

qint64 ComicModel::asignNumbers(QList<QModelIndex> list, int startingNumber)
{
    qint64 idFirst;
    QString connectionName = "";
    const auto &comicIndexes = list;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        idFirst = rowComic(_data.at(list[0].row())).file.id.value();
        int i = 0;
        for (const auto &mi : comicIndexes) {
            bool found;
            ComicDB c = DBHelper::loadComic(rowComic(_data.at(mi.row())).file.id.value(), db, found);
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
    int i = 0;
    for (const auto &item : std::as_const(_data)) {
        if (rowComic(item).file.id.value() == id)
            break;
        i++;
    }

    return index(i, 0);
}

// TODO completely inefficiently
QList<QModelIndex> ComicModel::getIndexesFromIds(const QList<qulonglong> &comicIds)
{
    QList<QModelIndex> comicsIndexes;

    for (const auto id : comicIds)
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
    bool found;
    ComicDB c = DBHelper::loadComic(rowComic(_data.at(row)).file.id.value(), dbTransaction, found);

    DBHelper::removeFromDB(&c, dbTransaction);
    beginRemoveRows(QModelIndex(), row, row);

    removeRow(row);
    _data.removeAt(row);

    endRemoveRows();
}

void ComicModel::reloadContinueReading()
{
    setupReadingModelData(_databasePath);
}

// The `comparator` passed to `takeUpdatedData` is used to determine if a row has been removed or added
void ComicModel::reload()
{
    switch (mode) {
    case Folder:
        takeUpdatedData(createFolderModelData(sourceId, _databasePath), defaultFolderContentSortFunction);
        break;
    case Favorites:
        setupFavoritesModelData(_databasePath); // TODO we need a comparator
        break;
    case Reading:
        takeUpdatedData(createReadingModelData(_databasePath), [](const ComicItem &c1, const ComicItem &c2) {
            return rowComic(c1).info.lastTimeOpened.toLongLong() > rowComic(c2).info.lastTimeOpened.toLongLong();
        });
        break;
    case Recent:
        takeUpdatedData(createRecentModelData(_databasePath), [](const ComicItem &c1, const ComicItem &c2) {
            return rowComic(c1).info.added.toLongLong() > rowComic(c2).info.added.toLongLong();
        });
        break;
    case Label:
        setupLabelModelData(sourceId, _databasePath); // TODO we need a comparator
        break;
    case ReadingList:
        setupReadingListModelData(sourceId, _databasePath); // TODO we need a comparator
        break;
    case SearchResult:
        // TODO: reload search results, we don't have a way to recreate the query in this class
        break;
    }
}

void ComicModel::remove(int row)
{
    removeInTransaction(row);
}

void ComicModel::reload(const ComicDB &comic)
{
    updateRowsOf(comic.info.id, [&comic](ComicInfo &info) {
        info.read = comic.info.read;
        info.currentPage = comic.info.currentPage;
        info.hasBeenOpened = true; }, ReadColumn, HasBeenOpened, { ReadColumnRole, CurrentPageRole, HasBeenOpenedRole });
}

void ComicModel::resetComicRating(const QModelIndex &mi)
{
    ComicDB comic = getComic(mi);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        comic.info.rating = 0;
        DBHelper::update(&(comic.info), db);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    updateRowsOf(rowComic(_data.at(mi.row())).info.id, [](ComicInfo &info) { info.rating = 0; }, 0, columnCount() - 1, { });
}

void ComicModel::notifyCoverChange(const ComicDB &comic)
{
    // Keep cover changes non-structural. Removing and reinserting the row makes
    // views adjust their selection and scroll position before the edit refresh can
    // capture them. Changing the URL also makes QML reload the image even though
    // the underlying cover file path is unchanged.
    ++coverRevisions[comic.info.id.value()];
    updateRowsOf(comic.info.id, [](ComicInfo &) { }, 0, columnCount() - 1, { CoverPathRole });
}

QUrl ComicModel::getCoverUrlPathForComicHash(const QString &hash) const
{
    auto coverPath = LibraryPaths::coverPathFromLibraryDataPath(_databasePath, hash);
    return QUrl::fromLocalFile(coverPath);
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

    QList<qulonglong> comicIds;
    comicIds.reserve(comics.size());
    for (const auto &comic : std::as_const(comics))
        comicIds.append(comic.file.id.value());
    emit favoritesChanged(comicIds);
}

void ComicModel::addComicsToLabel(const QList<qulonglong> &comicIds, qulonglong labelId)
{
    addComicsToLabel(getIndexesFromIds(comicIds), labelId);
}

void ComicModel::addComicsToLabel(const QList<QModelIndex> &comicsList, qulonglong labelId)
{
    QList<ComicInfoId> comicInfoIds;
    comicInfoIds.reserve(comicsList.size());
    for (const auto &index : comicsList)
        comicInfoIds.append(rowComic(_data.at(index.row())).info.id);

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        DBHelper::appendComicsToLabel(LabelId(labelId), comicInfoIds, db);
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
    QList<ComicInfoId> comicInfoIds;
    comicInfoIds.reserve(comicsList.size());
    for (const auto &index : comicsList)
        comicInfoIds.append(rowComic(_data.at(index.row())).info.id);

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        DBHelper::appendComicsToReadingList(ReadingListId(readingListId), comicInfoIds, db);
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

    QList<qulonglong> comicIds;
    comicIds.reserve(comics.size());
    for (const auto &comic : std::as_const(comics))
        comicIds.append(comic.file.id.value());
    emit favoritesChanged(comicIds);

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

    switch (type) {
    case ReadingListModel::TypeSpecialList::Reading:
        deleteComicsFromReading(comicsList);
        break;
    case ReadingListModel::TypeSpecialList::Favorites:
        deleteComicsFromFavorites(comicsList);
        break;
    case ReadingListModel::TypeSpecialList::Recent:
        // do nothing, recent is read only
        break;
    }
}

// a reading list also shows the rows of its child lists, so each row is removed from the list that holds it
void ComicModel::deleteComicsFromLists(const QList<QModelIndex> &comicsList)
{
    QSet<int> rows;
    QMap<std::pair<ListKind, qulonglong>, QList<ListItemId>> entryIdsByList;
    for (const auto &index : comicsList) {
        if (const auto *row = std::get_if<ComicItemCase::ComicInList>(&_data.at(index.row())); row != nullptr && !rows.contains(index.row())) {
            rows.insert(index.row());
            entryIdsByList[{ row->entry.id.listKind, row->entry.listId }].append(row->entry.id);
        }
    }

    bool removed = true;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        for (auto it = entryIdsByList.cbegin(); it != entryIdsByList.cend(); ++it) {
            const auto [listKind, listId] = it.key();
            switch (listKind) {
            case ListKind::ReadingList:
                removed = DBHelper::removeReadingListEntries(ReadingListId(listId), it.value(), db) && removed;
                break;
            case ListKind::Label:
                removed = DBHelper::removeLabelEntries(LabelId(listId), it.value(), db) && removed;
                break;
            }
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!removed) {
        reload();
        return;
    }

    QList<int> sortedRows(rows.begin(), rows.end());
    std::sort(sortedRows.begin(), sortedRows.end());
    QList<QModelIndex> removedRows;
    for (const auto row : std::as_const(sortedRows))
        removedRows.append(index(row, 0));
    deleteComicsFromModel(removedRows);
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

        isFavorite = DBHelper::isFavoriteComic(rowComic(_data.at(index.row())).file.id.value(), db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return isFavorite;
}

void ComicModel::setShowRecent(bool showRecent)
{
    if (this->showRecent == showRecent)
        return;

    this->showRecent = showRecent;

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), { ComicModel::ShowRecentRole });
}

void ComicModel::setRecentRange(int days)
{
    if (this->recentDays == days)
        return;

    this->recentDays = days;

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), { ComicModel::RecentRangeRole });

    if (mode == ComicModel::Recent)
        reload();
}

void ComicModel::updateRating(int rating, QModelIndex mi)
{
    ComicDB comic = getComic(mi);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        // TODO optimize update

        comic.info.rating = rating;
        DBHelper::update(&(comic.info), db);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    updateRowsOf(rowComic(_data.at(mi.row())).info.id, [rating](ComicInfo &info) { info.rating = rating; }, 0, columnCount() - 1, { });
}

QString ComicModel::localizedDate(const QString &dbDate) const
{
    auto dateComponents = dbDate.split("/");

    if (dateComponents.length() == 3) {
        auto dayString = dateComponents[0];
        auto monthString = dateComponents[1];
        auto yearString = dateComponents[2];

        auto hasDay = !dayString.isEmpty();
        auto hasMonth = !monthString.isEmpty();
        auto hasYear = !yearString.isEmpty();

        auto day = hasDay ? dayString.toInt() : 1;
        auto month = hasMonth ? monthString.toInt() : 1;
        auto year = hasYear ? yearString.toInt() : 1;

        auto locale = QLocale();
        auto date = QDate(year, month, day);
        auto dateTime = QDateTime();
        dateTime.setDate(date);

        if (hasDay && hasMonth && hasYear) {
            return locale.toString(dateTime, locale.dateFormat(QLocale::ShortFormat));
        }
    }

    return dbDate; // TODO
}
