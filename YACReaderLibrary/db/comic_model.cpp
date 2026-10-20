
#include "comic_model.h"

#include "comic.h"
#include "comic_db.h"
#include "comic_item.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "overloaded.h"
#include "qnaturalsorting.h"
#include "reading_list_model.h"
#include "search_query.h"
#include "smart_list.h"
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

// the descriptive metadata, which every case has
const DescriptiveComicMetadata &rowMetadata(const ComicItem &item)
{
    return std::visit(overloaded {
                              [](const ComicItemCase::Comic &row) -> const DescriptiveComicMetadata & { return row.comic.info; },
                              [](const ComicItemCase::ComicInList &row) -> const DescriptiveComicMetadata & { return row.comic.info; },
                              [](const ComicItemCase::DeletedComicInList &row) -> const DescriptiveComicMetadata & { return row.info; },
                              [](const ComicItemCase::PlaceholderInList &row) -> const DescriptiveComicMetadata & { return row.placeholder; },
                      },
                      item);
}

// nullptr for a placeholder, which has no comic_info
const ComicInfo *rowInfo(const ComicItem &item)
{
    return std::visit(overloaded {
                              [](const ComicItemCase::Comic &row) -> const ComicInfo * { return &row.comic.info; },
                              [](const ComicItemCase::ComicInList &row) -> const ComicInfo * { return &row.comic.info; },
                              [](const ComicItemCase::DeletedComicInList &row) -> const ComicInfo * { return &row.info; },
                              [](const ComicItemCase::PlaceholderInList &) -> const ComicInfo * { return nullptr; },
                      },
                      item);
}

ComicInfo *rowInfo(ComicItem &item)
{
    return const_cast<ComicInfo *>(rowInfo(std::as_const(item)));
}

const Placeholder *rowPlaceholder(const ComicItem &item)
{
    const auto *row = std::get_if<ComicItemCase::PlaceholderInList>(&item);
    return row != nullptr ? &row->placeholder : nullptr;
}

// the comic of a row with a file
const ComicDB *rowComic(const ComicItem &item)
{
    return std::visit(overloaded {
                              [](const ComicItemCase::Comic &row) -> const ComicDB * { return &row.comic; },
                              [](const ComicItemCase::ComicInList &row) -> const ComicDB * { return &row.comic; },
                              [](const ComicItemCase::DeletedComicInList &) -> const ComicDB * { return nullptr; },
                              [](const ComicItemCase::PlaceholderInList &) -> const ComicDB * { return nullptr; },
                      },
                      item);
}

const ListEntry *rowEntry(const ComicItem &item)
{
    return std::visit(overloaded {
                              [](const ComicItemCase::Comic &) -> const ListEntry * { return nullptr; },
                              [](const ComicItemCase::ComicInList &row) -> const ListEntry * { return &row.entry; },
                              [](const ComicItemCase::DeletedComicInList &row) -> const ListEntry * { return &row.entry; },
                              [](const ComicItemCase::PlaceholderInList &row) -> const ListEntry * { return &row.entry; },
                      },
                      item);
}

// the file name of the row, or the last known one of a deleted comic; a placeholder has none
QString rowFileName(const ComicItem &item)
{
    return std::visit(overloaded {
                              [](const ComicItemCase::Comic &row) { return row.comic.file.fileName; },
                              [](const ComicItemCase::ComicInList &row) { return row.comic.file.fileName; },
                              [](const ComicItemCase::DeletedComicInList &row) { return row.entry.lastKnownFile ? row.entry.lastKnownFile->name : QString(); },
                              [](const ComicItemCase::PlaceholderInList &) { return QString(); },
                      },
                      item);
}

ComicRowId rowIdOf(const ComicItem &item)
{
    return std::visit(overloaded {
                              [](const ComicItemCase::Comic &row) -> ComicRowId { return row.comic.file.id; },
                              [](const ComicItemCase::ComicInList &row) -> ComicRowId { return row.entry.id; },
                              [](const ComicItemCase::DeletedComicInList &row) -> ComicRowId { return row.entry.id; },
                              [](const ComicItemCase::PlaceholderInList &row) -> ComicRowId { return row.entry.id; },
                      },
                      item);
}

bool supportsAction(const ComicItem &item, ComicAction action)
{
    const auto withFile = [action] {
        switch (action) {
        case ComicAction::Open:
        case ComicAction::OpenContainingFolder:
        case ComicAction::ChangeFile:
        case ComicAction::SaveCover:
        case ComicAction::SetReadState:
        case ComicAction::SetType:
        case ComicAction::EditMetadata:
        case ComicAction::DeleteMetadata:
        case ComicAction::SearchComicVine:
        case ComicAction::AssignNumbers:
        case ComicAction::SetCustomCover:
        case ComicAction::AddToFavorites:
        case ComicAction::AddToList:
            return true;
        case ComicAction::RemoveFromList:
        case ComicAction::MatchToLibraryComic:
            return false;
        }
        return false;
    };

    return std::visit(overloaded {
                              [&](const ComicItemCase::Comic &) { return withFile(); },
                              [&](const ComicItemCase::ComicInList &) { return action == ComicAction::RemoveFromList || withFile(); },
                              [action](const ComicItemCase::DeletedComicInList &) {
                                  switch (action) {
                                  case ComicAction::SaveCover:
                                  case ComicAction::SetReadState:
                                  case ComicAction::SetType:
                                  case ComicAction::EditMetadata:
                                  case ComicAction::DeleteMetadata:
                                  case ComicAction::AssignNumbers:
                                  case ComicAction::SearchComicVine:
                                  case ComicAction::SetCustomCover:
                                  case ComicAction::AddToList:
                                  case ComicAction::RemoveFromList:
                                  case ComicAction::MatchToLibraryComic:
                                      return true;
                                  // Favorites is keyed by comic id
                                  case ComicAction::AddToFavorites:
                                  case ComicAction::Open:
                                  case ComicAction::OpenContainingFolder:
                                  case ComicAction::ChangeFile:
                                      return false;
                                  }
                                  return false;
                              },
                              [action](const ComicItemCase::PlaceholderInList &row) {
                                  switch (action) {
                                  case ComicAction::SaveCover:
                                      return row.placeholder.coverKey.has_value();
                                  case ComicAction::EditMetadata:
                                  case ComicAction::SearchComicVine:
                                  case ComicAction::SetCustomCover:
                                  case ComicAction::AssignNumbers:
                                  case ComicAction::AddToList:
                                  case ComicAction::RemoveFromList:
                                  case ComicAction::MatchToLibraryComic:
                                      return true;
                                  // no file, reading state, type, or rating
                                  case ComicAction::Open:
                                  case ComicAction::OpenContainingFolder:
                                  case ComicAction::ChangeFile:
                                  case ComicAction::SetReadState:
                                  case ComicAction::SetType:
                                  case ComicAction::DeleteMetadata:
                                  case ComicAction::AddToFavorites:
                                      return false;
                                  }
                                  return false;
                              },
                      },
                      item);
}

bool sameRow(const ComicItem &a, const ComicItem &b)
{
    return rowIdOf(a) == rowIdOf(b);
}

bool sameValues(const ComicItem &a, const ComicItem &b)
{
    if (a.index() != b.index() || !sameRow(a, b))
        return false;

    const auto *entryA = rowEntry(a);
    const auto *entryB = rowEntry(b);
    if (entryA != nullptr && *entryA != *entryB)
        return false;

    const auto *comicA = rowComic(a);
    const auto *comicB = rowComic(b);
    if (comicA != nullptr && comicA->file != comicB->file)
        return false;

    if (const auto *placeholderA = rowPlaceholder(a))
        return placeholderA->hasSameValues(*rowPlaceholder(b));
    return rowInfo(a)->hasSameValues(*rowInfo(b));
}

// what kind of row this is and where it is; a role that the case does not have is nothing to show
QVariant identityValue(const ComicItem &item, int role)
{
    return std::visit(overloaded {
                              [role](const ComicItemCase::Comic &) -> QVariant {
                                  switch (role) {
                                  case ComicModel::KindRole:
                                      return QStringLiteral("Comic");
                                  case ComicModel::AvailableRole:
                                      return true;
                                  }
                                  return { };
                              },
                              [role](const ComicItemCase::ComicInList &row) -> QVariant {
                                  switch (role) {
                                  case ComicModel::KindRole:
                                      return QStringLiteral("ComicInList");
                                  case ComicModel::AvailableRole:
                                      return true;
                                  case ComicModel::ListItemIdRole:
                                      return row.entry.id.id;
                                  case ComicModel::ListIdRole:
                                      return row.entry.listId;
                                  }
                                  return { };
                              },
                              [role](const ComicItemCase::DeletedComicInList &row) -> QVariant {
                                  switch (role) {
                                  case ComicModel::KindRole:
                                      return QStringLiteral("DeletedComicInList");
                                  case ComicModel::AvailableRole:
                                      return false;
                                  case ComicModel::LastKnownNameRole:
                                      return row.entry.lastKnownFile ? QVariant(row.entry.lastKnownFile->name) : QVariant();
                                  case ComicModel::ListItemIdRole:
                                      return row.entry.id.id;
                                  case ComicModel::ListIdRole:
                                      return row.entry.listId;
                                  }
                                  return { };
                              },
                              [role](const ComicItemCase::PlaceholderInList &row) -> QVariant {
                                  switch (role) {
                                  case ComicModel::KindRole:
                                      return QStringLiteral("PlaceholderInList");
                                  case ComicModel::AvailableRole:
                                      return false;
                                  case ComicModel::ListItemIdRole:
                                      return row.entry.id.id;
                                  case ComicModel::ListIdRole:
                                      return row.entry.listId;
                                  }
                                  return { };
                              },
                      },
                      item);
}

// the file columns; a deleted comic shows its last known file and has no id or folder
QVariant fileColumnValue(const ComicItem &item, int column)
{
    const auto fromFile = [column](const ComicFile &file) -> QVariant {
        switch (column) {
        case ComicModel::FileName:
            return file.fileName;
        case ComicModel::Id:
            return file.id.value();
        case ComicModel::Parent_Id:
            return file.folderId.value();
        case ComicModel::Path:
            return file.path;
        }
        return { };
    };

    return std::visit(overloaded {
                              [&](const ComicItemCase::Comic &row) { return fromFile(row.comic.file); },
                              [&](const ComicItemCase::ComicInList &row) { return fromFile(row.comic.file); },
                              [column](const ComicItemCase::DeletedComicInList &row) -> QVariant {
                                  if (!row.entry.lastKnownFile)
                                      return { };
                                  switch (column) {
                                  case ComicModel::FileName:
                                      return row.entry.lastKnownFile->name;
                                  case ComicModel::Path:
                                      return row.entry.lastKnownFile->path;
                                  }
                                  return { };
                              },
                              [](const ComicItemCase::PlaceholderInList &) -> QVariant { return { }; },
                      },
                      item);
}

// a placeholder has the descriptive columns only
QVariant columnValue(const ComicItem &item, int column)
{
    const auto &metadata = rowMetadata(item);
    switch (column) {
    case ComicModel::Number:
        return metadata.number;
    case ComicModel::Title:
        return metadata.title;
    case ComicModel::PublicationDate:
        return metadata.date;
    case ComicModel::Series:
        return metadata.series;
    case ComicModel::Volume:
        return metadata.volume;
    case ComicModel::StoryArc:
        return metadata.storyArc;
    case ComicModel::FileName:
    case ComicModel::Id:
    case ComicModel::Parent_Id:
    case ComicModel::Path:
        return fileColumnValue(item, column);
    }

    const auto *infoOfRow = rowInfo(item);
    if (infoOfRow == nullptr)
        return { };
    const auto &info = *infoOfRow;
    switch (column) {
    case ComicModel::NumPages:
        return info.numPages;
    case ComicModel::Hash:
        return info.hash;
    case ComicModel::ReadColumn:
        return info.read;
    case ComicModel::CurrentPage:
        return info.currentPage;
    case ComicModel::Rating:
        return info.rating;
    case ComicModel::HasBeenOpened:
        return qlonglong(info.hasBeenOpened ? 1 : 0); // the views got the SQL integer
    case ComicModel::Added:
        return info.added;
    case ComicModel::Type:
        return info.type;
    case ComicModel::LastTimeOpened:
        return info.lastTimeOpened;
    }
    return QVariant();
}

bool defaultFolderContentSortFunction(const ComicItem &c1, const ComicItem &c2)
{
    return comicNumberLessThan(rowMetadata(c1).number, rowFileName(c1), rowMetadata(c2).number, rowFileName(c2));
}

// -1, 0, or 1 in natural order without case; an empty text is first when `emptyFirst`, else last
int compareNatural(const QString &a, const QString &b, bool emptyFirst)
{
    if (a.isEmpty() != b.isEmpty())
        return a.isEmpty() == emptyFirst ? -1 : 1;
    if (naturalSortLessThanCI(a, b))
        return -1;
    return naturalSortLessThanCI(b, a) ? 1 : 0;
}

// search results and smart lists: each series stays together, in volume and number order
bool searchResultLessThan(const ComicItem &c1, const ComicItem &c2)
{
    const auto &a = rowMetadata(c1);
    const auto &b = rowMetadata(c2);
    if (const auto order = compareNatural(a.series.toString().trimmed(), b.series.toString().trimmed(), false); order != 0)
        return order < 0;
    if (const auto order = compareNatural(a.volume.toString().trimmed(), b.volume.toString().trimmed(), true); order != 0)
        return order < 0;
    if (comicNumberLessThan(a.number, QString(), b.number, QString()))
        return true;
    if (comicNumberLessThan(b.number, QString(), a.number, QString()))
        return false;
    if (const auto order = compareNatural(rowFileName(c1), rowFileName(c2), true); order != 0)
        return order < 0;
    const auto *comic1 = rowComic(c1);
    const auto *comic2 = rowComic(c2);
    return compareNatural(comic1 != nullptr ? comic1->file.path : QString(), comic2 != nullptr ? comic2->file.path : QString(), true) < 0;
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
    return ComicModel::State + 1;
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

    // inside a list the drag names its entries; Favorites rows are comics
    QList<ComicRowId> rowIds;
    if (const auto entryIds = YACReader::mimeDataToListEntries(data); !entryIds.isEmpty()) {
        for (const auto &entryId : entryIds)
            rowIds.append(entryId);
    } else {
        for (const auto id : YACReader::mimeDataToComicsIds(data))
            rowIds.append(ComicId(id));
    }
    if (rowIds.isEmpty())
        return false;

    if (row < 0 || row > _data.count())
        row = _data.count();

    QList<int> currentIndexes;
    int i;
    for (const auto &rowId : std::as_const(rowIds)) {
        const auto current = indexOf(rowId);
        if (!current.isValid())
            return false;
        currentIndexes << current.row();
    }

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

    for (const auto &rowId : std::as_const(rowIds)) {
        int i = 0;
        const auto dataSnapshot = _data;
        for (const auto &item : dataSnapshot) {
            if (rowIdOf(item) == rowId) {
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
                           [&allEntryIds](const ComicItemCase::DeletedComicInList &row) { allEntryIds << row.entry.id; },
                           [&allEntryIds](const ComicItemCase::PlaceholderInList &row) { allEntryIds << row.entry.id; },
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
        case SmartList:
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

// comic ids for Favorites, file moves, and the drop checks, and the entries of a list; the rows of the drag are the entries, or else the comics
QMimeData *ComicModel::mimeData(const QModelIndexList &indexes) const
{
    QList<qulonglong> ids;
    QList<ListItemId> entryIds;
    for (const auto &index : indexes) {
        const auto &item = _data.at(index.row());
        if (const auto *comic = rowComic(item))
            ids << comic->file.id.value();
        if (const auto *entry = rowEntry(item))
            entryIds << entry->id;
    }

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << ids; // serialize the list of identifiers

    auto mimeData = new QMimeData();
    mimeData->setData(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat, data);
    if (!entryIds.isEmpty())
        YACReader::setListEntriesMimeData(mimeData, entryIds);

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
    roles[KindRole] = "kind";
    roles[AvailableRole] = "available";
    roles[LastKnownNameRole] = "last_known_name";
    roles[ListItemIdRole] = "list_item_id";
    roles[ListIdRole] = "list_id";

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

    const auto &item = _data.at(index.row());
    const auto *placeholder = rowPlaceholder(item);
    const auto value = [&item](int column) { return columnValue(item, column); };
    // only a missing comic can have no title, series, number, or file name
    const auto orMissingComic = [this](const QString &title) { return title.isEmpty() ? tr("Missing comic") : title; };
    const auto title = [&] { return orMissingComic(titleOrFileName(rowMetadata(item), rowFileName(item))); };

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
        return title();
    else if (role == ReadableTitle) {
        const auto &metadata = rowMetadata(item);
        // without a title, the fallback already has the number
        if (!metadata.title.toString().trimmed().isEmpty() && !metadata.number.toString().trimmed().isEmpty())
            return QString(QStringLiteral("#") % metadata.number.toString() % QStringLiteral(" ") % metadata.title.toString());
        return title();
    } else if (role == FileNameRole)
        return rowComic(item) ? value(FileName) : QVariant();
    else if (role == RatingRole)
        return value(Rating);
    else if (role == CoverPathRole) {
        if (placeholder != nullptr) {
            const auto coverPath = placeholder->coverKey ? LibraryPaths::placeholderCoverPathFromDataPath(_databasePath, *placeholder->coverKey) : QString();
            return coverPath.isEmpty() || !QFileInfo::exists(coverPath) ? QUrl(QStringLiteral("qrc:/images/unavailableCover.png")) : QUrl::fromLocalFile(coverPath);
        }
        auto coverUrl = getCoverUrlPathForComicHash(value(Hash).toString());
        if (!rowComic(item) && !QFileInfo::exists(coverUrl.toLocalFile()))
            return QUrl(QStringLiteral("qrc:/images/unavailableCover.png"));
        const auto revision = coverRevisions.value(rowInfo(item)->id.value());
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
        return placeholder != nullptr ? QVariant() : QVariant(sizeString());
    else if (role == SeriesRole)
        return value(Series);
    else if (role == VolumeRole)
        return value(Volume);
    else if (role == StoryArcRole)
        return value(StoryArc);
    else if (role == KindRole || role == AvailableRole || role == LastKnownNameRole || role == ListItemIdRole || role == ListIdRole)
        return identityValue(item, role);

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() == ComicModel::Hash)
        return value(ComicModel::Hash);
    if (index.column() == ComicModel::Title)
        return title();
    // a placeholder shows "-" for the values it does not have
    if (placeholder != nullptr) {
        switch (index.column()) {
        case ComicModel::FileName:
        case ComicModel::NumPages:
        case ComicModel::CurrentPage:
        case ComicModel::Size:
            return QStringLiteral("-");
        case ComicModel::ReadColumn:
            return QVariant();
        }
    }
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
    if (index.column() == ComicModel::Rating && supports(index, ComicAction::SetReadState))
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
        const QString fileName = (section >= 0 && section < _data.size()) ? rowFileName(_data.at(section)) : QString();
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
        QString path;
        if (const auto *placeholder = rowPlaceholder(item))
            path = placeholder->coverKey ? LibraryPaths::placeholderCoverPathFromDataPath(LibraryPaths::libraryDataPath(_source), *placeholder->coverKey) : QString();
        else
            path = LibraryPaths::coverPath(_source, rowInfo(item)->hash);
        if (rowComic(item) == nullptr && (path.isEmpty() || !QFileInfo::exists(path)))
            path = QStringLiteral(":/images/unavailableCover.png");
        paths << path;
    }

    return paths;
}

QList<bool> ComicModel::getUnavailableList() const
{
    QList<bool> unavailable;
    unavailable.reserve(_data.size());
    for (const auto &item : std::as_const(_data))
        unavailable.append(rowComic(item) == nullptr);
    return unavailable;
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
        selectQuery.prepare(DBHelper::listEntriesQuery(ListKind::Label));
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
        selectQuery.prepare(DBHelper::listEntriesQuery(ListKind::ReadingList));
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

// a smart list that no longer exists has no rows
QList<ComicItem> ComicModel::createSmartListModelData(unsigned long long smartListId, const QString &databasePath) const
{
    QList<ComicItem> modelData;

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        if (const auto list = DBHelper::loadSmartList(SmartListId(smartListId), db)) {
            try {
                auto query = allComicsSearchQuery(db, list->query);
                modelData = sortedForSearch(createItems(query));
            } catch (const std::exception &) {
            }
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return modelData;
}

void ComicModel::setupSmartListModelData(unsigned long long smartListId, const QString &databasePath)
{
    enableResorting = false;
    mode = SmartList;
    sourceId = smartListId;

    beginResetModel();
    _data.clear();

    _databasePath = databasePath;

    takeData(createSmartListModelData(smartListId, databasePath));

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
        if (const auto *comic = rowComic(_data.at(mi.row())))
            return comic->file.path;
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
    const auto placeholderIndexes = Placeholder::columnIndexes(record, "p");
    const auto fileId = indexes.file.first();
    const auto placeholderId = placeholderIndexes.first();
    while (query.next()) {
        auto entry = ListEntry::fromQuery(query, entryIndexes, listKind);
        if (!query.value(placeholderId).isNull())
            items.append(ComicItemCase::PlaceholderInList { Placeholder::fromQuery(query, placeholderIndexes), std::move(entry) });
        else if (query.value(fileId).isNull())
            items.append(ComicItemCase::DeletedComicInList { ComicInfo::fromQuery(query, indexes.info), std::move(entry) });
        else
            items.append(ComicItemCase::ComicInList { ComicDB::fromQuery(query, indexes), std::move(entry) });
    }
    return items;
}

QList<ComicItem> ComicModel::sortedForFolder(QList<ComicItem> items)
{
    std::sort(items.begin(), items.end(), defaultFolderContentSortFunction);
    return items;
}

QList<ComicItem> ComicModel::sortedForSearch(QList<ComicItem> items)
{
    std::stable_sort(items.begin(), items.end(), searchResultLessThan);
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

std::optional<ComicDB> ComicModel::getComic(const QModelIndex &mi)
{
    const auto *rowFile = rowComic(_data.at(mi.row()));
    if (rowFile == nullptr)
        return std::nullopt;

    ComicDB c;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        bool found;
        c = DBHelper::loadComic(rowFile->file.id.value(), db, found);
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
        const auto *infoOfRow = rowInfo(_data.at(i));
        if (infoOfRow == nullptr) {
            readList[i] = YACReader::Unread;
            continue;
        }
        const auto &info = *infoOfRow;
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
QList<ComicDB> ComicModel::getComics(QList<QModelIndex> list)
{
    QList<ComicDB> comics;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        QList<qulonglong> ids;
        ids.reserve(list.size());
        for (const auto &mi : std::as_const(list)) {
            if (const auto *comic = rowComic(_data.at(mi.row())))
                ids.append(comic->file.id.value());
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
        const auto *comic = rowComic(item);
        if (comic != nullptr && !comic->info.read) {
            return *comic;
        }
    }
    return std::nullopt;
}

ComicRecord ComicModel::record(const QModelIndex &index) const
{
    return std::visit(overloaded {
                              [](const ComicItemCase::Comic &row) -> ComicRecord { return ComicRecordCase::Comic { row.comic }; },
                              [](const ComicItemCase::ComicInList &row) -> ComicRecord { return ComicRecordCase::Comic { row.comic }; },
                              [](const ComicItemCase::DeletedComicInList &row) -> ComicRecord { return ComicRecordCase::DeletedComic { row.info, row.entry.lastKnownFile }; },
                              [](const ComicItemCase::PlaceholderInList &row) -> ComicRecord { return ComicRecordCase::Placeholder { row.placeholder }; },
                      },
                      _data.at(index.row()));
}

std::optional<ListEntry> ComicModel::listEntry(const QModelIndex &index) const
{
    const auto *entry = index.isValid() ? rowEntry(_data.at(index.row())) : nullptr;
    return entry != nullptr ? std::optional(*entry) : std::nullopt;
}

bool ComicModel::hasUnavailableRows() const
{
    return std::any_of(_data.cbegin(), _data.cend(), [](const ComicItem &item) { return rowComic(item) == nullptr; });
}

bool ComicModel::supports(const QModelIndex &index, ComicAction action) const
{
    if (!index.isValid())
        return false;
    // Favorites and Reading remove by comic; Recent is read only
    if (action == ComicAction::RemoveFromList && (mode == Favorites || mode == Reading))
        return true;
    return supportsAction(_data.at(index.row()), action);
}

QModelIndexList ComicModel::supportedIndexes(const QModelIndexList &indexes, ComicAction action) const
{
    QModelIndexList supported;
    for (const auto &index : indexes) {
        if (supports(index, action))
            supported.append(index);
    }
    return supported;
}

void ComicModel::deleteMetadata(const QModelIndexList &indexes)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        QSet<ComicInfoId> done;
        for (const auto &mi : indexes) {
            const auto *info = rowInfo(_data.at(mi.row()));
            if (info == nullptr)
                continue;
            const auto id = info->id;
            if (done.contains(id))
                continue;
            done.insert(id);
            if (auto info = DBHelper::loadComicInfo(id, db)) {
                info->deleteMetadata();
                DBHelper::update(&*info, db);
            }
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    reload();
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
            const auto *rowComicInfo = rowInfo(_data.at(mi.row()));
            if (rowComicInfo == nullptr)
                continue;
            const auto id = rowComicInfo->id;
            if (read == YACReader::Read) {
                if (auto info = DBHelper::loadComicInfo(id, db)) {
                    info->read = true;
                    DBHelper::update(&*info, db);
                }
            }
            if (read == YACReader::Unread) {
                if (auto info = DBHelper::loadComicInfo(id, db)) {
                    info->read = false;
                    info->currentPage = 1;
                    info->hasBeenOpened = false;
                    info->lastTimeOpened.setValue(QVariant());
                    DBHelper::update(&*info, db);
                }
            }
            if (read == YACReader::Read || read == YACReader::Unread)
                changed.append(id);
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
            const auto *rowComicInfo = rowInfo(_data.at(mi.row()));
            if (rowComicInfo == nullptr)
                continue;
            const auto id = rowComicInfo->id;
            if (auto info = DBHelper::loadComicInfo(id, db)) {
                info->type = QVariant::fromValue(type);
                DBHelper::update(&*info, db);
            }
            changed.append(id);
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
        auto *info = rowInfo(_data[row]);
        if (info == nullptr || info->id != id)
            continue;
        change(*info);
        emit dataChanged(index(row, firstColumn), index(row, lastColumn), roles);
    }
}

// each comic_info or placeholder gets one number, at its first appearance in `list`
ComicRowId ComicModel::asignNumbers(QList<QModelIndex> list, int startingNumber)
{
    ComicRowId idFirst;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        db.transaction();
        idFirst = rowIdOf(_data.at(list[0].row()));
        QSet<ComicInfoId> numberedComics;
        QSet<PlaceholderId> numberedPlaceholders;
        auto number = startingNumber;
        for (const auto &mi : std::as_const(list)) {
            const auto &item = _data.at(mi.row());
            DescriptiveMetadataPatch patch;
            patch.set(&DescriptiveComicMetadata::number, QString::number(number));
            if (const auto *placeholder = rowPlaceholder(item)) {
                if (numberedPlaceholders.contains(placeholder->id))
                    continue;
                numberedPlaceholders.insert(placeholder->id);
                DBHelper::updatePlaceholderMetadata(placeholder->id, patch, db);
            } else {
                const auto id = rowInfo(item)->id;
                if (numberedComics.contains(id))
                    continue;
                numberedComics.insert(id);
                DBHelper::updateComicInfoMetadata(id, patch, db);
            }
            number++;
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
        const auto *comic = rowComic(item);
        if (comic != nullptr && comic->file.id.value() == id)
            break;
        i++;
    }

    return index(i, 0);
}

ComicRowId ComicModel::rowId(const QModelIndex &index) const
{
    return rowIdOf(_data.at(index.row()));
}

QModelIndex ComicModel::indexOf(const ComicRowId &rowId) const
{
    for (int row = 0; row < _data.size(); row++) {
        if (rowIdOf(_data.at(row)) == rowId)
            return index(row, 0);
    }
    return { };
}

QModelIndexList ComicModel::indexesOf(const QList<ComicRowId> &rowIds) const
{
    QHash<ComicRowId, int> rows;
    rows.reserve(_data.size());
    for (int row = _data.size() - 1; row >= 0; row--)
        rows.insert(rowIdOf(_data.at(row)), row);

    QModelIndexList indexes;
    indexes.reserve(rowIds.size());
    for (const auto &rowId : rowIds) {
        const auto row = rows.find(rowId);
        indexes.append(row == rows.end() ? QModelIndex() : index(*row, 0));
    }
    return indexes;
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
    const auto *rowFile = rowComic(_data.at(row));
    if (rowFile == nullptr)
        return;

    auto dbTransaction = QSqlDatabase::database(_databaseConnection);
    bool found;
    ComicDB c = DBHelper::loadComic(rowFile->file.id.value(), dbTransaction, found);

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
            return rowInfo(c1)->lastTimeOpened.toLongLong() > rowInfo(c2)->lastTimeOpened.toLongLong();
        });
        break;
    case Recent:
        takeUpdatedData(createRecentModelData(_databasePath), [](const ComicItem &c1, const ComicItem &c2) {
            return rowInfo(c1)->added.toLongLong() > rowInfo(c2)->added.toLongLong();
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
    case SmartList:
        takeUpdatedData(createSmartListModelData(sourceId, _databasePath), searchResultLessThan);
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
    const auto *info = rowInfo(_data.at(mi.row()));
    if (info == nullptr)
        return;
    const auto id = info->id;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        if (auto info = DBHelper::loadComicInfo(id, db)) {
            info->rating = 0;
            DBHelper::update(&*info, db);
        }

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    updateRowsOf(id, [](ComicInfo &info) { info.rating = 0; }, 0, columnCount() - 1, { });
}

void ComicModel::notifyCoverChange(ComicInfoId id)
{
    // Keep cover changes non-structural. Removing and reinserting the row makes
    // views adjust their selection and scroll position before the edit refresh can
    // capture them. Changing the URL also makes QML reload the image even though
    // the underlying cover file path is unchanged.
    ++coverRevisions[id.value()];
    updateRowsOf(id, [](ComicInfo &) { }, 0, columnCount() - 1, { CoverPathRole });
}

void ComicModel::notifyPlaceholderCoverChange(PlaceholderId id, const std::optional<QString> &coverKey)
{
    for (int row = 0; row < _data.size(); row++) {
        auto *item = std::get_if<ComicItemCase::PlaceholderInList>(&_data[row]);
        if (item == nullptr || item->placeholder.id != id)
            continue;
        item->placeholder.coverKey = coverKey;
        emit dataChanged(index(row, 0), index(row, columnCount() - 1), { CoverPathRole });
    }
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

void ComicModel::addComicsToLabel(const QList<QModelIndex> &comicsList, qulonglong labelId)
{
    QList<ComicRowId> rows;
    rows.reserve(comicsList.size());
    for (const auto &index : comicsList)
        rows.append(rowIdOf(_data.at(index.row())));

    addComicsToLabel(rows, labelId);
}

void ComicModel::addComicsToLabel(const QList<ComicRowId> &rows, qulonglong labelId)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        DBHelper::appendRowsToLabel(LabelId(labelId), rows, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void ComicModel::addComicsToReadingList(const QList<QModelIndex> &comicsList, qulonglong readingListId)
{
    QList<ComicRowId> rows;
    rows.reserve(comicsList.size());
    for (const auto &index : comicsList)
        rows.append(rowIdOf(_data.at(index.row())));

    addComicsToReadingList(rows, readingListId);
}

void ComicModel::addComicsToReadingList(const QList<ComicRowId> &rows, qulonglong readingListId)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        DBHelper::appendRowsToReadingList(ReadingListId(readingListId), rows, db);
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
        if (const auto *entry = rowEntry(_data.at(index.row())); entry != nullptr && !rows.contains(index.row())) {
            rows.insert(index.row());
            entryIdsByList[{ entry->id.listKind, entry->listId }].append(entry->id);
        }
    }

    bool removed = true;
    QStringList placeholderCovers;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);

        for (auto it = entryIdsByList.cbegin(); it != entryIdsByList.cend(); ++it) {
            const auto [listKind, listId] = it.key();
            std::optional<QStringList> covers;
            switch (listKind) {
            case ListKind::ReadingList:
                covers = DBHelper::removeReadingListEntries(ReadingListId(listId), it.value(), db);
                break;
            case ListKind::Label:
                covers = DBHelper::removeLabelEntries(LabelId(listId), it.value(), db);
                break;
            }
            if (covers)
                placeholderCovers.append(*covers);
            else
                removed = false;
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    DBHelper::removePlaceholderCovers(_databasePath, placeholderCovers);

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

        const auto *comic = rowComic(_data.at(index.row()));
        isFavorite = comic != nullptr && DBHelper::isFavoriteComic(comic->file.id.value(), db);
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
    const auto *info = rowInfo(_data.at(mi.row()));
    if (info == nullptr)
        return;
    const auto id = info->id;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(_databasePath);
        // TODO optimize update

        if (auto info = DBHelper::loadComicInfo(id, db)) {
            info->rating = rating;
            DBHelper::update(&*info, db);
        }

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    updateRowsOf(id, [rating](ComicInfo &info) { info.rating = rating; }, 0, columnCount() - 1, { });
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
