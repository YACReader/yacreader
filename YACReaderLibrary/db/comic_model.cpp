
#include "comic_model.h"

#include "comic.h"
#include "comic_db.h"
#include "comic_item.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "qnaturalsorting.h"
#include "reading_list_model.h"
#include "yacreader_global_gui.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlRecord>
#include <QStringBuilder>
#include <QtDebug>
#include <QtGui>

#include <utility>

#ifdef use_unarr
#include <unarr.h>
#endif

// ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read
#include "QsLog.h"

auto defaultFolderContentSortFunction = [](const ComicItem *c1, const ComicItem *c2) {
    return comicNumberLessThan(c1->data(ComicModel::Number), c1->data(ComicModel::FileName).toString(),
                               c2->data(ComicModel::Number), c2->data(ComicModel::FileName).toString());
};

ComicModel::ComicModel(QObject *parent)
    : QAbstractItemModel(parent), showRecent(false), recentDays(1)

{
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
    return _data.first()->columnCount() + 1 /* + the number of calculated columns */;
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
            for (auto *item : currentData) {
                if (item->data(Id) == id) {
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
    QList<ComicItem *> resortedData;

    if (currentIndexes.contains(row)) // no resorting
        return false;

    ComicItem *destinationItem;
    if (row == -1 || row >= _data.length())
        destinationItem = 0;
    else
        destinationItem = _data.at(row);

    QList<int> newSorting;

    const auto &currentIndexList = currentIndexes;
    {
        const auto &currentData = _data;
        i = 0;
        for (auto *item : currentData) {
            if (!currentIndexes.contains(i)) {

                if (item == destinationItem) {
                    for (const auto index : currentIndexList) {
                        resortedData << _data.at(index);
                        newSorting << index;
                    }
                }

                resortedData << item;
                newSorting << i;
            }

            i++;
        }
    }

    if (destinationItem == 0) {
        for (const auto index : currentIndexList) {
            resortedData << _data.at(index);
            newSorting << index;
        }
    }

    QLOG_TRACE() << newSorting;

    int tempRow = row;

    for (const auto id : comicIds) {
        int i = 0;
        const auto dataSnapshot = _data;
        for (auto *item : dataSnapshot) {
            if (item->data(Id) == id) {
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
    const auto &allData = _data;
    for (auto *item : allData) {
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
        case Folder:
        case Reading:
        case Recent:
        case SearchResult:
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
    roles[IsPlaceholderRole] = "is_placeholder";

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

    auto item = static_cast<ComicItem *>(index.internalPointer());

    auto sizeString = [=] {
        auto bytes = item->data(ComicModel::Hash).toString().right(item->data(ComicModel::Hash).toString().length() - 40).toULongLong();

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
        return item->data(Number);
    else if (role == TitleRole)
        return item->data(Title).isNull() ? item->data(FileName) : item->data(Title);
    else if (role == ReadableTitle) {
        QString title;
        if (!item->data(Number).isNull()) {
            title = title % "#" % item->data(Number).toString() % " ";
        }
        return QVariant(title % (item->data(Title).isNull() ? item->data(FileName).toString() : item->data(Title).toString()));
    } else if (role == FileNameRole)
        return item->data(FileName);
    else if (role == RatingRole)
        return item->data(Rating);
    else if (role == CoverPathRole) {
        auto coverUrl = getCoverUrlPathForComicHash(item->data(Hash).toString());
        const auto revision = coverRevisions.value(item->data(Id).toULongLong());
        if (revision > 0)
            coverUrl.setQuery(QStringLiteral("revision=%1").arg(revision));
        return coverUrl;
    } else if (role == NumPagesRole)
        return item->data(NumPages);
    else if (role == CurrentPageRole)
        return item->data(CurrentPage);
    else if (role == ReadColumnRole)
        return item->data(ReadColumn).toBool();
    else if (role == HasBeenOpenedRole)
        return item->data(HasBeenOpened);
    else if (role == IdRole)
        return item->data(Id);
    else if (role == HashRole)
        return item->data(Hash);
    else if (role == PublicationDateRole)
        return QVariant(localizedDate(item->data(PublicationDate).toString()));
    else if (role == AddedRole)
        return item->data(Added);
    else if (role == TypeRole)
        return item->data(Type);
    else if (role == ShowRecentRole)
        return showRecent;
    else if (role == RecentRangeRole)
        return recentDays * 86400;
    else if (role == SizeRole)
        return sizeString();
    else if (role == SeriesRole)
        return item->data(Series);
    else if (role == VolumeRole)
        return item->data(Volume);
    else if (role == StoryArcRole)
        return item->data(StoryArc);
    else if (role == IsPlaceholderRole)
        return item->data(Id).toLongLong() < 0;

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() == ComicModel::Hash)
        return item->data(ComicModel::Hash).toString();
    if (index.column() == ComicModel::Size)
        return sizeString();
    if (index.column() == ComicModel::ReadColumn)
        return (item->data(ComicModel::CurrentPage).toInt() == item->data(ComicModel::NumPages).toInt() || item->data(ComicModel::ReadColumn).toBool()) ? QVariant(tr("yes")) : QVariant(tr("no"));
    if (index.column() == ComicModel::CurrentPage)
        return item->data(ComicModel::HasBeenOpened).toBool() ? item->data(index.column()) : QVariant("-");

    if (index.column() == ComicModel::Rating)
        return QVariant();

    if (index.column() == ComicModel::PublicationDate) {
        return QVariant(localizedDate(item->data(PublicationDate).toString()));
    }

    return item->data(index.column());
}

Qt::ItemFlags ComicModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return { };
    const auto item = static_cast<ComicItem *>(index.internalPointer());
    if (item->data(Id).toLongLong() < 0)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
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
        QString fileName = _data.value(section)->data(ComicModel::FileName).toString();
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
    QList<ComicItem *>::ConstIterator itr;
    for (itr = _data.constBegin(); itr != _data.constEnd(); itr++) {
        QString hash = (*itr)->data(ComicModel::Hash).toString();
        paths << LibraryPaths::coverPath(_source, hash);
    }

    return paths;
}

#define COMIC_MODEL_QUERY_FIELDS "ci.number,ci.title,c.fileName,ci.numPages,c.id,c.parentId,c.path,ci.hash,ci.read,ci.currentPage,ci.rating,ci.hasBeenOpened,ci.date,ci.added,ci.type,ci.lastTimeOpened,ci.series,ci.volume,ci.storyArc"

namespace {
QString likePattern(const QString &value)
{
    return QStringLiteral("%") + value.trimmed() + QStringLiteral("%");
}

bool addTextRule(const QJsonObject &rules, const QString &key, const QStringList &columns, QStringList &where, QHash<QString, QVariant> &binds)
{
    const QString value = rules.value(key).toString().trimmed();
    if (value.isEmpty())
        return false;

    QStringList matches;
    const QString bindName = QStringLiteral(":%1").arg(key);
    for (const auto &column : columns)
        matches << QStringLiteral("COALESCE(%1,'') LIKE %2 COLLATE NOCASE").arg(column, bindName);
    where << QStringLiteral("(%1)").arg(matches.join(QStringLiteral(" OR ")));
    binds.insert(bindName, likePattern(value));
    return true;
}

QString smartListSortOrder(const QJsonObject &rules)
{
    const QString direction = rules.value(QStringLiteral("sortDirection")).toString(QStringLiteral("asc")) == QStringLiteral("desc")
            ? QStringLiteral("DESC")
            : QStringLiteral("ASC");
    const QString textDirection = QStringLiteral("COLLATE NOCASE ") + direction;
    const QString sortField = rules.value(QStringLiteral("sortField")).toString(QStringLiteral("series"));

    if (sortField == QStringLiteral("title"))
        return QStringLiteral("COALESCE(ci.title,'') %1, c.fileName %1").arg(textDirection);
    if (sortField == QStringLiteral("fileName"))
        return QStringLiteral("c.fileName %1").arg(textDirection);
    if (sortField == QStringLiteral("year"))
        return QStringLiteral("CAST(substr(COALESCE(ci.date,''), -4) AS INTEGER) %1, COALESCE(ci.series,'') %2, CAST(ci.number AS REAL) %1, ci.number %2, c.fileName %2")
                .arg(direction, textDirection);
    if (sortField == QStringLiteral("added"))
        return QStringLiteral("COALESCE(ci.added,0) %1, COALESCE(ci.series,'') %2, CAST(ci.number AS REAL) %1, ci.number %2, c.fileName %2")
                .arg(direction, textDirection);
    if (sortField == QStringLiteral("lastOpened"))
        return QStringLiteral("COALESCE(ci.lastTimeOpened,0) %1, COALESCE(ci.series,'') %2, CAST(ci.number AS REAL) %1, ci.number %2, c.fileName %2")
                .arg(direction, textDirection);
    if (sortField == QStringLiteral("rating"))
        return QStringLiteral("COALESCE(ci.rating,0) %1, COALESCE(ci.series,'') %2, CAST(ci.number AS REAL) %1, ci.number %2, c.fileName %2")
                .arg(direction, textDirection);
    if (sortField == QStringLiteral("publisher"))
        return QStringLiteral("COALESCE(ci.publisher,'') %1, COALESCE(ci.series,'') %1, CAST(ci.number AS REAL) %2, ci.number %1, c.fileName %1")
                .arg(textDirection, direction);

    return QStringLiteral("COALESCE(ci.series,'') %1, CAST(ci.number AS REAL) %2, ci.number %1, c.fileName %1")
            .arg(textDirection, direction);
}

bool prepareSmartListQuery(QSqlQuery &query, const QString &rulesJson)
{
    const auto document = QJsonDocument::fromJson(rulesJson.toUtf8());
    const QJsonObject rules = document.object();
    QStringList where;
    QHash<QString, QVariant> binds;

    addTextRule(rules, QStringLiteral("series"), { QStringLiteral("ci.series"), QStringLiteral("ci.alternateSeries") }, where, binds);
    addTextRule(rules, QStringLiteral("title"), { QStringLiteral("ci.title") }, where, binds);
    addTextRule(rules, QStringLiteral("publisher"), { QStringLiteral("ci.publisher"), QStringLiteral("ci.imprint") }, where, binds);
    addTextRule(rules, QStringLiteral("storyArc"), { QStringLiteral("ci.storyArc"), QStringLiteral("ci.seriesGroup") }, where, binds);
    addTextRule(rules, QStringLiteral("path"), { QStringLiteral("c.path"), QStringLiteral("c.fileName") }, where, binds);
    addTextRule(rules, QStringLiteral("tag"), { QStringLiteral("ci.tags"), QStringLiteral("ci.characters"), QStringLiteral("ci.teams"), QStringLiteral("ci.locations"), QStringLiteral("ci.mainCharacterOrTeam") }, where, binds);
    addTextRule(rules, QStringLiteral("creator"), { QStringLiteral("ci.writer"), QStringLiteral("ci.penciller"), QStringLiteral("ci.inker"), QStringLiteral("ci.colorist"), QStringLiteral("ci.letterer"), QStringLiteral("ci.coverArtist"), QStringLiteral("ci.editor") }, where, binds);

    const int readState = rules.value(QStringLiteral("readState")).toInt(0);
    if (readState == 1)
        where << QStringLiteral("COALESCE(ci.read,0) = 0");
    else if (readState == 2)
        where << QStringLiteral("COALESCE(ci.read,0) <> 0");

    const int openedState = rules.value(QStringLiteral("openedState")).toInt(0);
    if (openedState == 1)
        where << QStringLiteral("COALESCE(ci.hasBeenOpened,0) = 0");
    else if (openedState == 2)
        where << QStringLiteral("COALESCE(ci.hasBeenOpened,0) <> 0");

    const int ratingMin = rules.value(QStringLiteral("ratingMin")).toInt(0);
    if (ratingMin > 0) {
        where << QStringLiteral("COALESCE(ci.rating,0) >= :ratingMin");
        binds.insert(QStringLiteral(":ratingMin"), ratingMin);
    }

    const int type = rules.value(QStringLiteral("type")).toInt(-1);
    if (type >= 0) {
        where << QStringLiteral("COALESCE(ci.type,0) = :type");
        binds.insert(QStringLiteral(":type"), type);
    }

    const int yearFrom = rules.value(QStringLiteral("yearFrom")).toInt(0);
    if (yearFrom > 0) {
        where << QStringLiteral("CAST(substr(COALESCE(ci.date,''), -4) AS INTEGER) >= :yearFrom");
        binds.insert(QStringLiteral(":yearFrom"), yearFrom);
    }

    const int yearTo = rules.value(QStringLiteral("yearTo")).toInt(0);
    if (yearTo > 0) {
        where << QStringLiteral("CAST(substr(COALESCE(ci.date,''), -4) AS INTEGER) <= :yearTo");
        binds.insert(QStringLiteral(":yearTo"), yearTo);
    }

    const int addedWithinDays = rules.value(QStringLiteral("addedWithinDays")).toInt(0);
    if (addedWithinDays > 0) {
        where << QStringLiteral("COALESCE(ci.added,0) >= :addedAfter");
        binds.insert(QStringLiteral(":addedAfter"), QDateTime::currentSecsSinceEpoch() - qint64(addedWithinDays) * 86400);
    }

    const int openedWithinDays = rules.value(QStringLiteral("openedWithinDays")).toInt(0);
    if (openedWithinDays > 0) {
        where << QStringLiteral("COALESCE(ci.lastTimeOpened,0) >= :openedAfter");
        binds.insert(QStringLiteral(":openedAfter"), QDateTime::currentSecsSinceEpoch() - qint64(openedWithinDays) * 86400);
    }

    const QString whereClause = where.isEmpty() ? QString() : QStringLiteral("WHERE ") + where.join(QStringLiteral(" AND "));
    const QString orderClause = smartListSortOrder(rules);
    query.prepare(QStringLiteral("SELECT " COMIC_MODEL_QUERY_FIELDS " "
                                 "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                                 "%1 ORDER BY %2")
                          .arg(whereClause, orderClause));
    for (auto it = binds.constBegin(); it != binds.constEnd(); ++it)
        query.bindValue(it.key(), it.value());
    return true;
}
}

QList<ComicItem *> ComicModel::createFolderModelData(unsigned long long folderId, const QString &databasePath) const
{
    QList<ComicItem *> modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT " COMIC_MODEL_QUERY_FIELDS " "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "WHERE c.parentId = :parentId");
        selectQuery.bindValue(":parentId", folderId);
        selectQuery.exec();

        modelData = createModelData(selectQuery);

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
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;

    takeData(createFolderModelData(folderId, databasePath));

    endResetModel();
}

QList<ComicItem *> ComicModel::createLabelModelData(unsigned long long parentLabel, const QString &databasePath) const
{
    QList<ComicItem *> modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT " COMIC_MODEL_QUERY_FIELDS " "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "INNER JOIN comic_label cl ON (c.id == cl.comic_id) "
                            "WHERE cl.label_id = :parentLabelId "
                            "ORDER BY cl.ordering");
        selectQuery.bindValue(":parentLabelId", parentLabel);
        selectQuery.exec();
        modelData = createModelDataForList(selectQuery);
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
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;

    takeData(createLabelModelData(parentLabel, databasePath));

    endResetModel();
}

QList<ComicItem *> ComicModel::createReadingListData(unsigned long long parentReadingList, const QString &databasePath, bool &enableResorting) const
{
    QList<ComicItem *> modelData;

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

        const auto &readingListIds = ids;
        DBHelper::ensureReadingListEntries(db);
        db.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS reading_list_smart (reading_list_id INTEGER PRIMARY KEY, rules_json TEXT NOT NULL, "
                               "FOREIGN KEY(reading_list_id) REFERENCES reading_list(id) ON DELETE CASCADE)"));
        for (const auto id : readingListIds) {
            DBHelper::relinkMissingReadingListEntries(db, id);
            QSqlQuery selectQuery(db);
            QSqlQuery smartList(db);
            smartList.prepare("SELECT rules_json FROM reading_list_smart WHERE reading_list_id = :id");
            smartList.bindValue(":id", id);
            const bool isSmartList = smartList.exec() && smartList.next();
            if (isSmartList) {
                enableResorting = false;
                prepareSmartListQuery(selectQuery, smartList.value(0).toString());
                selectQuery.exec();
                modelData << createModelDataForList(selectQuery);
                continue;
            }

            QSqlQuery tableCheck(db);
            const bool hasCblTables = tableCheck.exec("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = 'cbl_reading_list_meta'")
                    && tableCheck.next();
            bool isImportedCbl = false;
            if (hasCblTables) {
                QSqlQuery importedList(db);
                importedList.prepare("SELECT 1 FROM cbl_reading_list_meta WHERE reading_list_id = :id");
                importedList.bindValue(":id", id);
                isImportedCbl = importedList.exec() && importedList.next();
            }

            if (isImportedCbl) {
                selectQuery.prepare(
                        "SELECT COALESCE(ci.number,e.number), ci.title, "
                        "CASE WHEN c.id IS NULL THEN '[MISSING]' ELSE c.fileName END, "
                        "COALESCE(ci.numPages,0), CASE WHEN c.id IS NULL THEN -e.id ELSE c.id END, c.parentId, c.path, ci.hash, "
                        "COALESCE(ci.read,0), COALESCE(ci.currentPage,0), COALESCE(ci.rating,0), COALESCE(ci.hasBeenOpened,0), "
                        "COALESCE(ci.date,e.year), ci.added, ci.type, ci.lastTimeOpened, COALESCE(ci.series,e.series), COALESCE(ci.volume,e.volume), ci.storyArc "
                        "FROM cbl_reading_list_entry e "
                        "LEFT JOIN comic c ON c.id = e.comic_id "
                        "LEFT JOIN comic_info ci ON c.comicInfoId = ci.id "
                        "WHERE e.reading_list_id = :parentReadingList ORDER BY e.ordering");
            } else {
                selectQuery.prepare(
                        "SELECT COALESCE(ci.number,e.number), ci.title, "
                        "CASE WHEN c.id IS NULL THEN '[MISSING]' ELSE c.fileName END, "
                        "COALESCE(ci.numPages,0), CASE WHEN c.id IS NULL THEN -e.id ELSE c.id END, c.parentId, c.path, ci.hash, "
                        "COALESCE(ci.read,0), COALESCE(ci.currentPage,0), COALESCE(ci.rating,0), COALESCE(ci.hasBeenOpened,0), "
                        "COALESCE(ci.date,e.year), ci.added, ci.type, ci.lastTimeOpened, COALESCE(ci.series,e.series), COALESCE(ci.volume,e.volume), ci.storyArc "
                        "FROM reading_list_entry e "
                        "LEFT JOIN comic c ON c.id = e.comic_id "
                        "LEFT JOIN comic_info ci ON c.comicInfoId = ci.id "
                        "WHERE e.reading_list_id = :parentReadingList ORDER BY e.ordering");
            }
            selectQuery.bindValue(":parentReadingList", id);
            selectQuery.exec();

            modelData << createModelDataForList(selectQuery);
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
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;

    takeData(createReadingListData(parentReadingList, databasePath, enableResorting));

    endResetModel();
}

QList<ComicItem *> ComicModel::createFavoritesModelData(const QString &databasePath) const
{
    QList<ComicItem *> modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT " COMIC_MODEL_QUERY_FIELDS " "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "INNER JOIN comic_default_reading_list cdrl ON (c.id == cdrl.comic_id) "
                            "WHERE cdrl.default_reading_list_id = :parentDefaultListId "
                            "ORDER BY cdrl.ordering");
        selectQuery.bindValue(":parentDefaultListId", 1);
        selectQuery.exec();
        modelData = createModelDataForList(selectQuery);
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
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;

    takeData(createFavoritesModelData(databasePath));

    endResetModel();
}

QList<ComicItem *> ComicModel::createReadingModelData(const QString &databasePath) const
{
    QList<ComicItem *> modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT " COMIC_MODEL_QUERY_FIELDS " "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "WHERE ci.hasBeenOpened = 1 AND ci.read = 0 "
                            "ORDER BY ci.lastTimeOpened DESC");
        selectQuery.exec();

        modelData = createModelDataForList(selectQuery);
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
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;

    takeData(createReadingModelData(databasePath));

    endResetModel();
}

QList<ComicItem *> ComicModel::createRecentModelData(const QString &databasePath) const
{
    QList<ComicItem *> modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT " COMIC_MODEL_QUERY_FIELDS " "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "WHERE ci.added > :limit "
                            "ORDER BY ci.added DESC");
        selectQuery.bindValue(":limit", QDateTime::currentDateTime().addDays(-recentDays).toSecsSinceEpoch());
        selectQuery.exec();

        modelData = createModelDataForList(selectQuery);
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
    qDeleteAll(_data);
    _data.clear();

    _databasePath = databasePath;

    takeData(createRecentModelData(databasePath));

    endResetModel();
}

void ComicModel::setModelData(QList<ComicItem *> *data, const QString &databasePath)
{
    enableResorting = false;
    mode = SearchResult;
    sourceId = -1;

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

QList<ComicItem *> ComicModel::createModelData(QSqlQuery &sqlquery) const
{
    QList<ComicItem *> modelData;

    int numColumns = sqlquery.record().count();

    while (sqlquery.next()) {
        QList<QVariant> data;

        for (int i = 0; i < numColumns; i++)
            data << sqlquery.value(i);

        modelData.append(new ComicItem(data));
    }

    std::sort(modelData.begin(), modelData.end(), defaultFolderContentSortFunction);

    return modelData;
}

// the sorting is done in the sql query
QList<ComicItem *> ComicModel::createModelDataForList(QSqlQuery &sqlquery) const
{
    QList<ComicItem *> modelData;

    int numColumns = sqlquery.record().count();

    while (sqlquery.next()) {
        QList<QVariant> data;
        for (int i = 0; i < numColumns; i++)
            data << sqlquery.value(i);

        modelData.append(new ComicItem(data));
    }

    return modelData;
}

void ComicModel::takeData(const QList<ComicItem *> &data)
{
    qDeleteAll(_data);
    _data = data;
}

void ComicModel::takeUpdatedData(const QList<ComicItem *> &updatedData, std::function<bool(ComicItem *, ComicItem *)> comparator)
{
    int length = _data.size();
    int lengthUpdated = updatedData.size();

    int i; // index of the internal data
    int j; // index of the updated children
    for (i = 0, j = 0; i < length && j < lengthUpdated;) {
        auto comic = _data.at(i);
        auto updatedComic = updatedData.at(j);

        auto sameComic = comic->data(ComicModel::Id) == updatedComic->data(ComicModel::Id);
        if (sameComic) {
            if (comic->getData() != updatedComic->getData()) {
                auto modelIndexToUpdate = index(i, 0, QModelIndex());

                comic->setData(updatedComic->getData());

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

        delete _data.at(k);
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
        c = DBHelper::loadComic(_data.at(mi.row())->data(ComicModel::Id).toULongLong(), db, found);
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
        bool found;
        c = DBHelper::loadComic(_data.at(mi.row())->data(ComicModel::Id).toULongLong(), db, found);
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
            bool found;
            comics.append(DBHelper::loadComic(_data.value(i)->data(ComicModel::Id).toULongLong(), db, found));
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
    const auto &comicIndexes = list;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        for (const auto &mi : comicIndexes) {
            if (read == YACReader::Read) {
                _data.value(mi.row())->setData(ComicModel::ReadColumn, QVariant(true));
                bool found;
                ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(ComicModel::Id).toULongLong(), db, found);
                c.info.read = true;
                DBHelper::update(&(c.info), db);
            }
            if (read == YACReader::Unread) {
                _data.value(mi.row())->setData(ComicModel::ReadColumn, QVariant(false));
                _data.value(mi.row())->setData(ComicModel::CurrentPage, QVariant(1));
                _data.value(mi.row())->setData(ComicModel::HasBeenOpened, QVariant(false));
                bool found;
                ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(ComicModel::Id).toULongLong(), db, found);
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

void ComicModel::setComicsType(QList<QModelIndex> list, FileType type)
{
    QString connectionName = "";
    const auto &comicIndexes = list;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        for (const auto &mi : comicIndexes) {
            bool found;
            ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(ComicModel::Id).toULongLong(), db, found);
            c.info.type = QVariant::fromValue(type);
            DBHelper::update(&(c.info), db);
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    for (const auto &mi : comicIndexes) {
        _data.value(mi.row())->setData(ComicModel::Type, QVariant::fromValue(type));
    }

    emit dataChanged(index(list.first().row(), ComicModel::Type), index(list.last().row(), ComicModel::Type), QVector<int>() << TypeRole);
}

qint64 ComicModel::asignNumbers(QList<QModelIndex> list, int startingNumber)
{
    qint64 idFirst;
    QString connectionName = "";
    const auto &comicIndexes = list;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        idFirst = _data.value(list[0].row())->data(ComicModel::Id).toULongLong();
        int i = 0;
        for (const auto &mi : comicIndexes) {
            bool found;
            ComicDB c = DBHelper::loadComic(_data.value(mi.row())->data(ComicModel::Id).toULongLong(), db, found);
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
    ComicDB c = DBHelper::loadComic(_data.at(row)->data(ComicModel::Id).toULongLong(), dbTransaction, found);

    DBHelper::removeFromDB(&c, dbTransaction);
    beginRemoveRows(QModelIndex(), row, row);

    removeRow(row);
    delete _data.at(row);
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
        takeUpdatedData(createReadingModelData(_databasePath), [](const ComicItem *c1, const ComicItem *c2) {
            return c1->data(ComicModel::LastTimeOpened).toDateTime() > c2->data(ComicModel::LastTimeOpened).toDateTime();
        });
        break;
    case Recent:
        takeUpdatedData(createRecentModelData(_databasePath), [](const ComicItem *c1, const ComicItem *c2) {
            return c1->data(ComicModel::Added).toDateTime() > c2->data(ComicModel::Added).toDateTime();
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
    int row = 0;
    bool found = false;
    const auto &currentData = _data;
    for (auto *item : currentData) {
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

void ComicModel::notifyCoverChange(const ComicDB &comic)
{
    auto it = std::find_if(_data.begin(), _data.end(), [comic](ComicItem *item) { return item->data(ComicModel::Id).toULongLong() == comic.id; });
    if (it == _data.end())
        return;

    // Keep cover changes non-structural. Removing and reinserting the row makes
    // views adjust their selection and scroll position before the edit refresh can
    // capture them. Changing the URL also makes QML reload the image even though
    // the underlying cover file path is unchanged.
    ++coverRevisions[comic.id];
    const auto itemIndex = std::distance(_data.begin(), it);
    emit dataChanged(index(itemIndex, 0), index(itemIndex, columnCount() - 1), { CoverPathRole });
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
        comicIds.append(comic.id);
    emit favoritesChanged(comicIds);
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

    QList<qulonglong> comicIds;
    comicIds.reserve(comics.size());
    for (const auto &comic : std::as_const(comics))
        comicIds.append(comic.id);
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
        _data[mi.row()]->setData(ComicModel::Rating, rating);
        DBHelper::update(&(comic.info), db);

        emit dataChanged(mi, mi);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
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
