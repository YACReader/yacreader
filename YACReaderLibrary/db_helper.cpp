#include "db_helper.h"

#include "QsLog.h"
#include "comic_db.h"
#include "data_base_management.h"
#include "descriptive_comic_metadata.h"
#include "folder.h"
#include "library_item_types.h"
#include "list_entry.h"
#include "list_folder.h"
#include "overloaded.h"
#include "placeholder.h"
#include "qnaturalsorting.h"
#include "reading_list.h"
#include "smart_list.h"
#include "table_projection.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QMap>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QTextStream>

#include <algorithm>
#include <functional>
#include <utility>

using namespace YACReader;

// server

YACReaderLibraries DBHelper::getLibraries()
{
    YACReaderLibraries libraries;
    libraries.load();
    return libraries;
}

QList<Folder> DBHelper::getFolderSubfoldersFromLibrary(qulonglong libraryId, qulonglong folderId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<Folder> list;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        list = DBHelper::getFoldersFromParent(folderId, db, false);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return list;
}

QList<ComicDB> DBHelper::getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId)
{
    return DBHelper::getFolderComicsFromLibrary(libraryId, folderId, false);
}

QList<ComicDB> DBHelper::getFolderComicsFromLibraryForReading(qulonglong libraryId, qulonglong folderId)
{
    auto list = DBHelper::getFolderComicsFromLibrary(libraryId, folderId, false);

    std::sort(list.begin(), list.end(), [](const ComicDB &c1, const ComicDB &c2) {
        return comicNumberLessThan(c1.info.number, c1.file.fileName, c2.info.number, c2.file.fileName);
    });

    return list;
}

QList<ComicDB> DBHelper::getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId, bool sort)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<ComicDB> list;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        list = DBHelper::getComicsFromParent(folderId, db, sort);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return list;
}

std::optional<ComicDB> DBHelper::getComicInfo(qulonglong libraryId, qulonglong id)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    std::optional<ComicDB> comic;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        bool found;
        auto loaded = DBHelper::loadComic(id, db, found);
        if (found) {
            comic = loaded;
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return comic;
}

QList<ComicDB> DBHelper::getSiblings(qulonglong libraryId, qulonglong parentId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<ComicDB> comics;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        comics = DBHelper::getSortedComicsFromParent(parentId, db);
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
    return comics;
}

std::optional<Folder> DBHelper::getFolder(qulonglong libraryId, qulonglong id)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);

    std::optional<Folder> folder;
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        folder = DBHelper::loadFolder(id, db);
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
    return folder;
}

namespace {

QList<ComicDB> comicsFromQuery(QSqlQuery &query)
{
    QList<ComicDB> comics;
    const auto indexes = ComicDB::columnIndexes(query.record(), "c", "ci");
    while (query.next()) {
        comics.append(ComicDB::fromQuery(query, indexes));
    }
    return comics;
}

}

namespace {

struct EntryTable {
    const char *table;
    const char *parentColumn;
    ListKind listKind;
    bool repeats;
};

constexpr EntryTable readingListEntries { "reading_list_entry", "reading_list_id", ListKind::ReadingList, true };
constexpr EntryTable labelEntries { "label_entry", "label_id", ListKind::Label, false };

// a comic_info with several files shows the first one
QString listComicsQuery(const EntryTable &entries)
{
    return "SELECT " + ComicDB::selectList("c", "ci") + ", " + TableProjection::selectList("e", ListEntry::columns(entries.listKind)) +
            QString(" FROM %1 e "
                    "INNER JOIN comic c ON (c.id = (SELECT MIN(id) FROM comic WHERE comicInfoId = e.comic_info_id)) "
                    "INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                    "WHERE e.%2 = :list_id "
                    "ORDER BY e.ordering, e.id")
                    .arg(entries.table, entries.parentColumn);
}

// an entry refers to a comic_info (with or without a file) or to a placeholder
QString listEntriesQuery(const EntryTable &entries)
{
    return "SELECT " + ComicDB::selectList("c", "ci") + ", " + TableProjection::selectList("e", ListEntry::columns(entries.listKind)) + ", " +
            TableProjection::selectList("p", Placeholder::columns()) +
            QString(" FROM %1 e "
                    "LEFT JOIN comic_info ci ON (ci.id = e.comic_info_id) "
                    "LEFT JOIN comic c ON (c.id = (SELECT MIN(id) FROM comic WHERE comicInfoId = e.comic_info_id)) "
                    "LEFT JOIN placeholder p ON (p.id = e.placeholder_id) "
                    "WHERE e.%2 = :list_id "
                    "ORDER BY e.ordering, e.id")
                    .arg(entries.table, entries.parentColumn);
}

bool allOfKind(const QList<ListItemId> &entryIds, ListKind listKind)
{
    return std::all_of(entryIds.begin(), entryIds.end(), [listKind](const ListItemId &id) { return id.listKind == listKind; });
}

// a placeholder lives while an entry or a folder placement uses it
QString placeholderUnused()
{
    return QString("NOT EXISTS (SELECT 1 FROM %1 WHERE placeholder_id = p.id) "
                   "AND NOT EXISTS (SELECT 1 FROM %2 WHERE placeholder_id = p.id) "
                   "AND NOT EXISTS (SELECT 1 FROM folder_placeholder WHERE placeholder_id = p.id)")
            .arg(readingListEntries.table, labelEntries.table);
}

// deletes the placeholders of `placeholderIds` that nothing uses; gives their cover keys
std::optional<QStringList> deleteUnusedPlaceholders(const QSet<qulonglong> &placeholderIds, QSqlDatabase &db)
{
    QSqlQuery select(db);
    select.prepare("SELECT p.coverKey FROM placeholder p WHERE p.id = :id AND " + placeholderUnused());
    QSqlQuery remove(db);
    remove.prepare("DELETE FROM placeholder WHERE id = :id");

    QStringList coverKeys;
    for (const auto placeholderId : placeholderIds) {
        select.bindValue(":id", placeholderId);
        if (!select.exec()) {
            QLOG_ERROR() << "Unable to find unused placeholders:" << select.lastError().text();
            return std::nullopt;
        }
        if (!select.next()) {
            continue;
        }
        const auto coverKey = select.value(0);
        select.finish();

        remove.bindValue(":id", placeholderId);
        if (!remove.exec()) {
            QLOG_ERROR() << "Unable to delete an unused placeholder:" << remove.lastError().text();
            return std::nullopt;
        }
        if (!coverKey.isNull()) {
            coverKeys.append(coverKey.toString());
        }
    }
    return coverKeys;
}

// runs `removal` in a transaction and then deletes the placeholders of `placeholderIds` that nothing uses any more
std::optional<QStringList> removeWithPlaceholders(QSqlQuery &placeholderIds, const std::function<bool()> &removal, QSqlDatabase &db)
{
    db.transaction();

    QSet<qulonglong> ids;
    if (!placeholderIds.exec()) {
        QLOG_ERROR() << "Unable to read the placeholders of the removed entries:" << placeholderIds.lastError().text();
        db.rollback();
        return std::nullopt;
    }
    while (placeholderIds.next()) {
        ids.insert(placeholderIds.value(0).toULongLong());
    }
    placeholderIds.finish();

    if (!removal()) {
        db.rollback();
        return std::nullopt;
    }

    auto coverKeys = deleteUnusedPlaceholders(ids, db);
    if (!coverKeys) {
        db.rollback();
        return std::nullopt;
    }

    if (!db.commit()) {
        return std::nullopt;
    }
    return coverKeys;
}

std::optional<QStringList> removeEntries(const EntryTable &entries, qulonglong parentId, const QList<ListItemId> &entryIds, QSqlDatabase &db)
{
    if (!allOfKind(entryIds, entries.listKind)) {
        return std::nullopt;
    }

    QStringList ids;
    for (const auto &entryId : entryIds) {
        ids.append(QString::number(entryId.id));
    }

    QSqlQuery placeholderIds(db);
    placeholderIds.prepare(QString("SELECT DISTINCT placeholder_id FROM %1 WHERE placeholder_id IS NOT NULL AND %2 = :parent_id AND id IN (%3)")
                                   .arg(entries.table, entries.parentColumn, ids.join(",")));
    placeholderIds.bindValue(":parent_id", parentId);

    const auto removal = [&] {
        QSqlQuery query(db);
        query.prepare(QString("DELETE FROM %1 WHERE id = :id AND %2 = :parent_id").arg(entries.table, entries.parentColumn));

        for (const auto &entryId : entryIds) {
            query.bindValue(":id", entryId.id);
            query.bindValue(":parent_id", parentId);
            if (!query.exec()) {
                QLOG_ERROR() << "Unable to remove entries from" << entries.table << ":" << query.lastError().text();
                return false;
            }
        }
        return true;
    };
    return removeWithPlaceholders(placeholderIds, removal, db);
}

bool reorderEntries(const EntryTable &entries, qulonglong parentId, const QList<ListItemId> &entryIds, QSqlDatabase &db)
{
    if (!allOfKind(entryIds, entries.listKind) || QSet<ListItemId>(entryIds.begin(), entryIds.end()).size() != entryIds.size()) {
        return false;
    }

    db.transaction();

    QSqlQuery count(db);
    count.prepare(QString("SELECT COUNT(*) FROM %1 WHERE %2 = :parent_id").arg(entries.table, entries.parentColumn));
    count.bindValue(":parent_id", parentId);
    const bool allEntriesGiven = count.exec() && count.next() && count.value(0).toLongLong() == entryIds.size();
    count.finish();
    if (!allEntriesGiven) {
        db.rollback();
        return false;
    }

    QSqlQuery query(db);
    query.prepare(QString("UPDATE %1 SET ordering = :ordering WHERE id = :id AND %2 = :parent_id").arg(entries.table, entries.parentColumn));

    int ordering = 0;
    for (const auto &entryId : entryIds) {
        query.bindValue(":ordering", ordering++);
        query.bindValue(":id", entryId.id);
        query.bindValue(":parent_id", parentId);
        if (!query.exec() || query.numRowsAffected() != 1) {
            QLOG_ERROR() << "Unable to reorder entries of" << entries.table << ":" << query.lastError().text();
            db.rollback();
            return false;
        }
    }

    return db.commit();
}

bool appendComics(const EntryTable &entries, qulonglong parentId, const QList<ComicInfoId> &comicInfoIds, QSqlDatabase &db)
{
    db.transaction();

    QSqlQuery query(db);
    query.prepare(QString("INSERT INTO %1 (%2, ordering, comic_info_id) "
                          "SELECT :parent_id, "
                          "(SELECT COALESCE(MAX(ordering), -1) + 1 FROM %1 WHERE %2 = :parent_id), "
                          "ci.id "
                          "FROM comic_info ci "
                          "WHERE ci.id = :comic_info_id")
                          .arg(entries.table, entries.parentColumn) +
                  (entries.repeats ? QString() : QString(" AND NOT EXISTS (SELECT 1 FROM %1 WHERE %2 = :parent_id AND comic_info_id = ci.id)").arg(entries.table, entries.parentColumn)));

    for (const auto comicInfoId : comicInfoIds) {
        query.bindValue(":parent_id", parentId);
        query.bindValue(":comic_info_id", comicInfoId.value());
        if (!query.exec()) {
            QLOG_ERROR() << "Unable to append comics to" << entries.table << ":" << query.lastError().text();
            db.rollback();
            return false;
        }
    }

    return db.commit();
}

// appends what each row refers to, in order: the comic_info of a comic, the comic_info or placeholder of an entry, or
// a placeholder of a folder
bool appendRows(const EntryTable &entries, qulonglong parentId, const QList<ComicRowId> &rows, QSqlDatabase &db)
{
    const auto insert = [&entries](const QString &source) {
        auto statement = QString("INSERT INTO %1 (%2, ordering, comic_info_id, placeholder_id) "
                                 "SELECT :parent_id, "
                                 "(SELECT COALESCE(MAX(ordering), -1) + 1 FROM %1 WHERE %2 = :parent_id), "
                                 "s.comic_info_id, s.placeholder_id "
                                 "FROM (%3) s")
                                 .arg(entries.table, entries.parentColumn, source);
        if (!entries.repeats)
            statement += QString(" WHERE NOT EXISTS (SELECT 1 FROM %1 WHERE %2 = :parent_id AND comic_info_id IS s.comic_info_id AND placeholder_id IS s.placeholder_id)")
                                 .arg(entries.table, entries.parentColumn);
        return statement;
    };

    db.transaction();

    QSqlQuery fromComic(db);
    fromComic.prepare(insert("SELECT comicInfoId AS comic_info_id, NULL AS placeholder_id FROM comic WHERE id = :row_id"));
    QSqlQuery fromReadingListEntry(db);
    fromReadingListEntry.prepare(insert(QString("SELECT comic_info_id, placeholder_id FROM %1 WHERE id = :row_id").arg(readingListEntries.table)));
    QSqlQuery fromLabelEntry(db);
    fromLabelEntry.prepare(insert(QString("SELECT comic_info_id, placeholder_id FROM %1 WHERE id = :row_id").arg(labelEntries.table)));
    QSqlQuery fromPlaceholder(db);
    fromPlaceholder.prepare(insert("SELECT NULL AS comic_info_id, id AS placeholder_id FROM placeholder WHERE id = :row_id"));

    for (const auto &row : rows) {
        auto &query = std::visit(overloaded {
                                         [&](const ComicId &) -> QSqlQuery & { return fromComic; },
                                         [&](const ListItemId &entry) -> QSqlQuery & { return entry.listKind == ListKind::ReadingList ? fromReadingListEntry : fromLabelEntry; },
                                         [&](const PlaceholderId &) -> QSqlQuery & { return fromPlaceholder; },
                                 },
                                 row);
        query.bindValue(":parent_id", parentId);
        query.bindValue(":row_id", std::visit(overloaded {
                                                      [](const ComicId &comic) { return comic.value(); },
                                                      [](const ListItemId &entry) { return entry.id; },
                                                      [](const PlaceholderId &placeholder) { return placeholder.value(); },
                                              },
                                              row));
        if (!query.exec()) {
            QLOG_ERROR() << "Unable to append rows to" << entries.table << ":" << query.lastError().text();
            db.rollback();
            return false;
        }
    }

    return db.commit();
}

}

std::optional<QStringList> DBHelper::removeLabelEntries(LabelId labelId, const QList<ListItemId> &entryIds, QSqlDatabase &db)
{
    return removeEntries(labelEntries, labelId.value(), entryIds, db);
}

std::optional<QStringList> DBHelper::removeReadingListEntries(ReadingListId readingListId, const QList<ListItemId> &entryIds, QSqlDatabase &db)
{
    return removeEntries(readingListEntries, readingListId.value(), entryIds, db);
}

bool DBHelper::reorderLabelEntries(LabelId labelId, const QList<ListItemId> &entryIds, QSqlDatabase &db)
{
    return reorderEntries(labelEntries, labelId.value(), entryIds, db);
}

bool DBHelper::reorderReadingListEntries(ReadingListId readingListId, const QList<ListItemId> &entryIds, QSqlDatabase &db)
{
    return reorderEntries(readingListEntries, readingListId.value(), entryIds, db);
}

bool DBHelper::appendComicsToLabel(LabelId labelId, const QList<ComicInfoId> &comicInfoIds, QSqlDatabase &db)
{
    return appendComics(labelEntries, labelId.value(), comicInfoIds, db);
}

bool DBHelper::appendComicsToReadingList(ReadingListId readingListId, const QList<ComicInfoId> &comicInfoIds, QSqlDatabase &db)
{
    return appendComics(readingListEntries, readingListId.value(), comicInfoIds, db);
}

bool DBHelper::appendRowsToLabel(LabelId labelId, const QList<ComicRowId> &rows, QSqlDatabase &db)
{
    return appendRows(labelEntries, labelId.value(), rows, db);
}

bool DBHelper::appendRowsToReadingList(ReadingListId readingListId, const QList<ComicRowId> &rows, QSqlDatabase &db)
{
    return appendRows(readingListEntries, readingListId.value(), rows, db);
}

const QString &DBHelper::listComicsQuery(ListKind listKind)
{
    static const QString readingListQuery = ::listComicsQuery(readingListEntries);
    static const QString labelQuery = ::listComicsQuery(labelEntries);

    switch (listKind) {
    case ListKind::ReadingList:
        return readingListQuery;
    case ListKind::Label:
        return labelQuery;
    }
    return readingListQuery;
}

const QString &DBHelper::listEntriesQuery(ListKind listKind)
{
    static const QString readingListQuery = ::listEntriesQuery(readingListEntries);
    static const QString labelQuery = ::listEntriesQuery(labelEntries);

    switch (listKind) {
    case ListKind::ReadingList:
        return readingListQuery;
    case ListKind::Label:
        return labelQuery;
    }
    return readingListQuery;
}

QString DBHelper::getLibraryName(int id)
{
    return getLibraries().getName(id);
}

QList<ComicDB> DBHelper::getLabelComics(qulonglong libraryId, qulonglong labelId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);

    QList<ComicDB> list;
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        QSqlQuery selectQuery(db);
        selectQuery.setForwardOnly(true);
        selectQuery.prepare(listComicsQuery(ListKind::Label));
        selectQuery.bindValue(":list_id", labelId);
        selectQuery.exec();

        list = comicsFromQuery(selectQuery);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return list;
}

QList<ComicDB> DBHelper::getFavorites(qulonglong libraryId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QList<ComicDB> list;

    const int FAV_ID = 1;
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT " + ComicDB::selectList("c", "ci") + " "
                                                                         "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                                                                         "INNER JOIN comic_default_reading_list cdrl ON (c.id == cdrl.comic_id) "
                                                                         "WHERE cdrl.default_reading_list_id = :parentDefaultListId "
                                                                         "ORDER BY cdrl.ordering");
        selectQuery.bindValue(":parentDefaultListId", FAV_ID);
        selectQuery.exec();

        list = comicsFromQuery(selectQuery);

        connectionName = db.connectionName();
    }
    // TODO ?
    QSqlDatabase::removeDatabase(connectionName);

    return list;
}

QList<ComicDB> DBHelper::getReading(qulonglong libraryId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QList<ComicDB> list;
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT " + ComicDB::selectList("c", "ci") + " "
                                                                         "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                                                                         "WHERE ci.hasBeenOpened = 1 AND ci.read = 0 "
                                                                         "ORDER BY ci.lastTimeOpened DESC");
        selectQuery.exec();

        list = comicsFromQuery(selectQuery);
        connectionName = db.connectionName();
    }
    // TODO ?
    QSqlDatabase::removeDatabase(connectionName);

    return list;
}

QList<ReadingList> DBHelper::getReadingLists(qulonglong libraryId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<ReadingList> list;

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        QSqlQuery selectQuery("SELECT " + TableProjection::selectList("rl", ReadingList::columns()) +
                                      " FROM reading_list rl WHERE rl.parentId IS NULL ORDER BY rl.name DESC",
                              db);

        selectQuery.exec();

        const auto indexes = ReadingList::columnIndexes(selectQuery.record(), "rl");
        while (selectQuery.next()) {
            const auto item = ReadingList::fromQuery(selectQuery, indexes);

            if (list.isEmpty()) {
                list.append(item);
            } else {
                int i = 0;
                while (i < list.length() && naturalSortLessThanCI(list.at(i).name, item.name))
                    i++;
                list.insert(i, item);
            }
        }
        connectionName = db.connectionName();
    }
    // TODO ?
    QSqlDatabase::removeDatabase(connectionName);

    return list;
}

QList<ComicDB> DBHelper::getReadingListFullContent(qulonglong libraryId, qulonglong readingListId, QList<ListItemId> *entryIds)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QList<ComicDB> list;
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        QList<qulonglong> ids;
        ids << readingListId;

        QSqlQuery subfolders(db);
        subfolders.prepare("SELECT id "
                           "FROM reading_list "
                           "WHERE parentId = :parentId "
                           "ORDER BY ordering ASC");
        subfolders.bindValue(":parentId", readingListId);
        subfolders.exec();
        while (subfolders.next())
            ids << subfolders.value(0).toULongLong();

        QSqlQuery selectQuery(db);
        selectQuery.setForwardOnly(true);
        selectQuery.prepare(listComicsQuery(ListKind::ReadingList));
        for (const auto id : std::as_const(ids)) {
            selectQuery.bindValue(":list_id", id);
            selectQuery.exec();

            if (entryIds == nullptr) {
                list.append(comicsFromQuery(selectQuery));
                continue;
            }
            const auto comicIndexes = ComicDB::columnIndexes(selectQuery.record(), "c", "ci");
            const auto entryIndexes = ListEntry::columnIndexes(selectQuery.record(), "e", ListKind::ReadingList);
            while (selectQuery.next()) {
                list.append(ComicDB::fromQuery(selectQuery, comicIndexes));
                entryIds->append(ListEntry::fromQuery(selectQuery, entryIndexes, ListKind::ReadingList).id);
            }
        }
        connectionName = db.connectionName();
    }

    // TODO ?
    QSqlDatabase::removeDatabase(connectionName);

    return list;
}

// objects management
// deletes
void DBHelper::removeFromDB(DiskItem &item, QSqlDatabase &db)
{
    std::visit(overloaded {
                       [&db](Folder &folder) { DBHelper::removeFromDB(&folder, db); },
                       [&db](ComicDB &comic) { DBHelper::removeFromDB(&comic, db); },
               },
               item);
}
void DBHelper::removeFromDB(Folder *folder, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM folder WHERE id = :id");
    query.bindValue(":id", folder->id.value());
    query.exec();
}
void DBHelper::removeFromDB(ComicDB *comic, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM comic WHERE id = :id");
    query.bindValue(":id", comic->file.id.value());
    query.exec();
}

namespace {

// the tables of one kind of list folders
struct FolderTable {
    const char *table;
    const char *listTable;
};

FolderTable folderTable(ListFolderKind kind)
{
    switch (kind) {
    case ListFolderKind::ReadingList:
        return { "reading_list_folder", "reading_list" };
    case ListFolderKind::SmartList:
        return { "smart_list_folder", "smart_list" };
    }
    return { "reading_list_folder", "reading_list" };
}

bool rowExists(const QString &table, qulonglong id, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare(QString("SELECT 1 FROM %1 WHERE id = :id").arg(table));
    query.bindValue(":id", id);
    return query.exec() && query.next();
}

// a top level reading list or a smart list goes to the end of `folderId` (nothing: the top level)
bool moveListToFolder(ListFolderKind kind, qulonglong listId, const std::optional<qulonglong> &folderId, QSqlDatabase &db)
{
    const auto tables = folderTable(kind);
    const QString topLevelOnly = kind == ListFolderKind::ReadingList ? " AND parentId IS NULL" : "";
    const QVariant folder = folderId ? QVariant(*folderId) : QVariant();

    db.transaction();
    QSqlQuery list(db);
    list.prepare(QString("SELECT 1 FROM %1 WHERE id = :id%2").arg(tables.listTable, topLevelOnly));
    list.bindValue(":id", listId);
    bool moved = list.exec() && list.next() && (!folderId || rowExists(tables.table, *folderId, db));
    list.finish();

    if (moved) {
        QSqlQuery move(db);
        move.prepare(QString("UPDATE %1 SET folder_id = :folder_id, "
                             "ordering = (SELECT COALESCE(MAX(ordering) + 1, 0) FROM %1 WHERE folder_id IS :folder_id AND id <> :id%2) "
                             "WHERE id = :id")
                             .arg(tables.listTable, topLevelOnly));
        move.bindValue(":folder_id", folder);
        move.bindValue(":id", listId);
        moved = move.exec() && move.numRowsAffected() == 1;
        if (!moved)
            QLOG_ERROR() << "Unable to move a list to a folder:" << move.lastError().text();
    }

    if (moved && db.commit())
        return true;
    db.rollback();
    return false;
}

}

std::optional<QStringList> DBHelper::removeLabelFromDB(qulonglong id, QSqlDatabase &db)
{
    QSqlQuery placeholderIds(db);
    placeholderIds.prepare("SELECT DISTINCT placeholder_id FROM label_entry WHERE placeholder_id IS NOT NULL AND label_id = :id");
    placeholderIds.bindValue(":id", id);

    const auto removal = [&] {
        QSqlQuery query(db);
        query.prepare("DELETE FROM label WHERE id = :id");
        query.bindValue(":id", id);
        if (!query.exec()) {
            QLOG_ERROR() << "Unable to remove a label:" << query.lastError().text();
            return false;
        }
        return true;
    };
    return removeWithPlaceholders(placeholderIds, removal, db);
}

std::optional<QStringList> DBHelper::removeListFromDB(qulonglong id, QSqlDatabase &db)
{
    QSqlQuery placeholderIds(db);
    placeholderIds.prepare("WITH RECURSIVE lists(id) AS (SELECT :id UNION ALL SELECT r.id FROM reading_list r INNER JOIN lists l ON r.parentId = l.id) "
                           "SELECT DISTINCT placeholder_id FROM reading_list_entry WHERE placeholder_id IS NOT NULL AND reading_list_id IN (SELECT id FROM lists)");
    placeholderIds.bindValue(":id", id);

    const auto removal = [&] {
        QSqlQuery query(db);
        query.prepare("DELETE FROM reading_list WHERE id = :id");
        query.bindValue(":id", id);
        if (!query.exec()) {
            QLOG_ERROR() << "Unable to remove a reading list:" << query.lastError().text();
            return false;
        }
        return true;
    };
    return removeWithPlaceholders(placeholderIds, removal, db);
}

void DBHelper::removePlaceholderCovers(const QString &libraryDataPath, const QStringList &coverKeys)
{
    for (const auto &coverKey : coverKeys) {
        QFile::remove(LibraryPaths::placeholderCoverPathFromDataPath(libraryDataPath, coverKey));
    }
}

std::optional<QStringList> DBHelper::removeUnusedPlaceholders(QSqlDatabase &db)
{
    QSqlQuery select(db);
    select.setForwardOnly(true);
    if (!select.exec("SELECT p.id FROM placeholder p WHERE " + placeholderUnused())) {
        QLOG_ERROR() << "Unable to find unused placeholders:" << select.lastError().text();
        return std::nullopt;
    }
    QSet<qulonglong> ids;
    while (select.next()) {
        ids.insert(select.value(0).toULongLong());
    }
    select.finish();
    return deleteUnusedPlaceholders(ids, db);
}

bool DBHelper::placePlaceholderInFolder(PlaceholderId placeholderId, FolderId folderId, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("INSERT OR IGNORE INTO folder_placeholder (folder_id, placeholder_id) VALUES (:folder_id, :placeholder_id)");
    query.bindValue(":folder_id", folderId.value());
    query.bindValue(":placeholder_id", placeholderId.value());
    if (!query.exec()) {
        QLOG_ERROR() << "Unable to place a placeholder in a folder:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DBHelper::insertPlaceholdersInFolder(const QList<Placeholder> &placeholders, FolderId folderId, QSqlDatabase &db)
{
    QSqlQuery insert(db);
    insert.prepare("INSERT INTO placeholder (" + TableProjection::insertColumns(Placeholder::columns()) + ") VALUES (" + TableProjection::insertValues(Placeholder::columns()) + ")");
    QSqlQuery place(db);
    place.prepare("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (:folder_id, :placeholder_id)");

    for (const auto &placeholder : placeholders) {
        placeholder.bindTo(insert);
        if (!insert.exec()) {
            QLOG_ERROR() << "Unable to insert a placeholder:" << insert.lastError().text();
            return false;
        }
        place.bindValue(":folder_id", folderId.value());
        place.bindValue(":placeholder_id", insert.lastInsertId());
        if (!place.exec()) {
            QLOG_ERROR() << "Unable to place a placeholder in a folder:" << place.lastError().text();
            return false;
        }
    }
    return true;
}

std::optional<QStringList> DBHelper::placeIssuesInFolder(const QList<MissingIssue> &issues, FolderId folderId, QSqlDatabase &db)
{
    QStringList unusedCovers;
    if (issues.isEmpty())
        return unusedCovers;

    QStringList parameters;
    for (qsizetype i = 0; i < issues.size(); i++) {
        if (issues.at(i).comicVineId.trimmed().isEmpty())
            return std::nullopt;
        parameters.append(QString(":id%1").arg(i));
    }

    QSqlQuery inFolder(db);
    inFolder.setForwardOnly(true);
    inFolder.prepare("SELECT ci.comicVineID FROM comic c INNER JOIN comic_info ci ON ci.id = c.comicInfoId WHERE c.parentId = :folder_id AND ci.comicVineID IS NOT NULL");
    inFolder.bindValue(":folder_id", folderId.value());
    if (!inFolder.exec()) {
        QLOG_ERROR() << "Unable to find the Comic Vine ids of the comics of a folder:" << inFolder.lastError().text();
        return std::nullopt;
    }
    QSet<QString> comicsInFolder;
    while (inFolder.next())
        comicsInFolder.insert(inFolder.value(0).toString());

    // the placeholder of each issue is the one with the lowest id
    QSqlQuery existing(db);
    existing.setForwardOnly(true);
    existing.prepare("SELECT comicVineID, id, coverKey FROM placeholder WHERE comicVineID IN (" + parameters.join(", ") + ") ORDER BY id");
    for (qsizetype i = 0; i < issues.size(); i++)
        existing.bindValue(parameters.at(i), issues.at(i).comicVineId);
    if (!existing.exec()) {
        QLOG_ERROR() << "Unable to find the placeholders of the issues:" << existing.lastError().text();
        return std::nullopt;
    }
    struct Found {
        PlaceholderId id;
        bool hasCover;
    };
    QHash<QString, Found> placeholders;
    while (existing.next()) {
        const auto comicVineId = existing.value(0).toString();
        if (!placeholders.contains(comicVineId))
            placeholders.insert(comicVineId, { PlaceholderId(existing.value(1).toULongLong()), !existing.value(2).isNull() });
    }

    for (const auto &issue : issues) {
        if (comicsInFolder.contains(issue.comicVineId)) {
            if (issue.coverKey)
                unusedCovers.append(*issue.coverKey);
            continue;
        }

        auto found = placeholders.find(issue.comicVineId);
        if (found == placeholders.end()) {
            const auto inserted = insertPlaceholder(Placeholder(), db);
            if (!inserted)
                return std::nullopt;
            found = placeholders.insert(issue.comicVineId, { *inserted, false });
        }

        if (!updatePlaceholderMetadata(found->id, issue.patch, db))
            return std::nullopt;
        if (issue.coverKey && !found->hasCover) {
            if (!updatePlaceholderCover(found->id, issue.coverKey, db))
                return std::nullopt;
            found->hasCover = true;
        } else if (issue.coverKey) {
            unusedCovers.append(*issue.coverKey);
        }
        if (!placePlaceholderInFolder(found->id, folderId, db))
            return std::nullopt;
    }
    return unusedCovers;
}

std::optional<QStringList> DBHelper::removePlaceholdersFromFolder(const QList<PlaceholderId> &placeholderIds, FolderId folderId, QSqlDatabase &db)
{
    QStringList ids;
    for (const auto &placeholderId : placeholderIds) {
        ids.append(QString::number(placeholderId.value()));
    }

    QSqlQuery placed(db);
    placed.prepare(QString("SELECT placeholder_id FROM folder_placeholder WHERE folder_id = :folder_id AND placeholder_id IN (%1)").arg(ids.join(",")));
    placed.bindValue(":folder_id", folderId.value());

    const auto removal = [&] {
        QSqlQuery query(db);
        query.prepare("DELETE FROM folder_placeholder WHERE folder_id = :folder_id AND placeholder_id = :placeholder_id");
        for (const auto &placeholderId : placeholderIds) {
            query.bindValue(":folder_id", folderId.value());
            query.bindValue(":placeholder_id", placeholderId.value());
            if (!query.exec()) {
                QLOG_ERROR() << "Unable to remove placeholders from a folder:" << query.lastError().text();
                return false;
            }
        }
        return true;
    };
    return removeWithPlaceholders(placed, removal, db);
}

void DBHelper::deleteComicsFromFavorites(const QList<ComicDB> &comicsList, QSqlDatabase &db)
{
    db.transaction();

    QLOG_DEBUG() << "deleteComicsFromFavorites----------------------------------";

    QSqlQuery query(db);
    query.prepare("DELETE FROM comic_default_reading_list WHERE comic_id = :comic_id AND default_reading_list_id = 1");
    for (const auto &comic : comicsList) {
        query.bindValue(":comic_id", comic.file.id.value());
        query.exec();
    }

    db.commit();
}

// a.k.a set comics as unread by reverting the conditions used to load the comics -> void ComicModel::setupReadingModelData(const QString &databasePath)
void DBHelper::deleteComicsFromReading(const QList<ComicDB> &comicsList, QSqlDatabase &db)
{
    db.transaction();

    QLOG_DEBUG() << "deleteComicsFromReading----------------------------------";

    for (auto comic : comicsList) {
        comic.info.hasBeenOpened = false;
        comic.info.currentPage = 0; // update sets hasBeenOpened to true if currentPage > 0;
        DBHelper::update(&comic.info, db);
    }

    db.commit();
}

// updates
void DBHelper::update(ComicDB *comic, QSqlDatabase &db)
{
    Q_UNUSED(comic)
    Q_UNUSED(db)
    // do nothing
}

void DBHelper::update(qulonglong libraryId, ComicInfo &comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        DBHelper::update(&comicInfo, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

bool DBHelper::update(ComicInfo *comicInfo, QSqlDatabase &db)
{
    if (comicInfo == nullptr)
        return false;

    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info SET " + DescriptiveComicMetadata::assignments().join(",") + "," +
                            "coverPage = :coverPage,"
                            "numPages = :numPages,"

                            "read = :read,"
                            "edited = :edited,"
                            // new 7.0 fields
                            "hasBeenOpened = :hasBeenOpened,"

                            "currentPage = :currentPage,"
                            "bookmark1 = :bookmark1,"
                            "bookmark2 = :bookmark2,"
                            "bookmark3 = :bookmark3,"
                            "brightness = :brightness,"
                            "contrast = :contrast, "
                            "gamma = :gamma,"
                            "rating = :rating,"

                            // new 9.5 fields
                            "lastTimeOpened = :lastTimeOpened,"

                            "coverSizeRatio = :coverSizeRatio,"
                            "originalCoverSize = :originalCoverSize,"
                            //--

                            // new 9.8 fields
                            // "manga = :manga," removed in 9.13
                            //--

                            // new 9.13 fields
                            "added = :added,"
                            "type = :type,"

                            // new 9.16 fields
                            "imageFiltersJson = :imageFiltersJson,"
                            "lastTimeImageFiltersSet = :lastTimeImageFiltersSet,"
                            "lastTimeCoverSet = :lastTimeCoverSet,"
                            "usesExternalCover = :usesExternalCover,"
                            "lastTimeMetadataSet = :lastTimeMetadataSet"

                            //--
                            " WHERE id = :id");

    comicInfo->bindTo(updateComicInfo);

    updateComicInfo.bindValue(":coverPage", comicInfo->coverPage);
    updateComicInfo.bindValue(":numPages", comicInfo->numPages);

    bool read = comicInfo->read || comicInfo->currentPage == comicInfo->numPages.toInt(); // if current page is the las page, the comic is read(completed)
    comicInfo->read = read;
    updateComicInfo.bindValue(":read", read ? 1 : 0);
    updateComicInfo.bindValue(":id", comicInfo->id.value());
    updateComicInfo.bindValue(":edited", comicInfo->edited ? 1 : 0);

    updateComicInfo.bindValue(":hasBeenOpened", comicInfo->hasBeenOpened ? 1 : 0 || comicInfo->currentPage > 1);
    updateComicInfo.bindValue(":currentPage", comicInfo->currentPage);
    updateComicInfo.bindValue(":bookmark1", comicInfo->bookmark1);
    updateComicInfo.bindValue(":bookmark2", comicInfo->bookmark2);
    updateComicInfo.bindValue(":bookmark3", comicInfo->bookmark3);
    updateComicInfo.bindValue(":brightness", comicInfo->brightness);
    updateComicInfo.bindValue(":contrast", comicInfo->contrast);
    updateComicInfo.bindValue(":gamma", comicInfo->gamma);
    updateComicInfo.bindValue(":rating", comicInfo->rating);

    updateComicInfo.bindValue(":lastTimeOpened", comicInfo->lastTimeOpened);

    updateComicInfo.bindValue(":coverSizeRatio", comicInfo->coverSizeRatio);
    updateComicInfo.bindValue(":originalCoverSize", comicInfo->originalCoverSize);

    updateComicInfo.bindValue(":added", comicInfo->added);
    auto intType = static_cast<int>(comicInfo->type.value<YACReader::FileType>());
    updateComicInfo.bindValue(":type", intType);

    updateComicInfo.bindValue(":imageFiltersJson", comicInfo->imageFiltersJson);
    updateComicInfo.bindValue(":lastTimeImageFiltersSet", comicInfo->lastTimeImageFiltersSet);
    updateComicInfo.bindValue(":lastTimeCoverSet", comicInfo->lastTimeCoverSet);
    updateComicInfo.bindValue(":usesExternalCover", comicInfo->usesExternalCover);
    updateComicInfo.bindValue(":lastTimeMetadataSet", comicInfo->lastTimeMetadataSet);

    const bool success = updateComicInfo.exec();

    QLOG_INFO() << updateComicInfo.lastError().databaseText();
    QLOG_INFO() << updateComicInfo.lastError().text();
    QLOG_INFO() << updateComicInfo.lastQuery();

    return success;
}

bool DBHelper::updateComicInfoMetadata(ComicInfoId id, const DescriptiveMetadataPatch &patch, QSqlDatabase &db)
{
    if (patch.isEmpty())
        return true;

    QSqlQuery query(db);
    query.prepare("UPDATE comic_info SET " + patch.assignments().join(", ") + ", edited = 1 WHERE id = :id");
    patch.bindTo(query);
    query.bindValue(":id", id.value());
    if (!query.exec()) {
        QLOG_ERROR() << "Unable to update the metadata of a comic:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DBHelper::updatePlaceholderMetadata(PlaceholderId id, const DescriptiveMetadataPatch &patch, QSqlDatabase &db)
{
    if (patch.isEmpty())
        return true;

    QSqlQuery query(db);
    query.prepare("UPDATE placeholder SET " + patch.assignments().join(", ") + " WHERE id = :id");
    patch.bindTo(query);
    query.bindValue(":id", id.value());
    if (!query.exec()) {
        QLOG_ERROR() << "Unable to update the metadata of a placeholder:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DBHelper::updatePlaceholderCover(PlaceholderId id, const std::optional<QString> &coverKey, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("UPDATE placeholder SET coverKey = :coverKey WHERE id = :id");
    query.bindValue(":coverKey", coverKey ? QVariant(*coverKey) : QVariant());
    query.bindValue(":id", id.value());
    if (!query.exec() || query.numRowsAffected() != 1) {
        QLOG_ERROR() << "Unable to update the cover of a placeholder:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DBHelper::updateComicInfoType(ComicInfoId id, YACReader::FileType type, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("UPDATE comic_info SET type = :type, edited = 1 WHERE id = :id");
    query.bindValue(":type", static_cast<int>(type));
    query.bindValue(":id", id.value());
    if (!query.exec()) {
        QLOG_ERROR() << "Unable to update the type of a comic:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DBHelper::updateComicInfoCover(const ComicInfo &info, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("UPDATE comic_info SET coverPage = :coverPage, originalCoverSize = :originalCoverSize, coverSizeRatio = :coverSizeRatio, "
                  "lastTimeCoverSet = :lastTimeCoverSet, usesExternalCover = :usesExternalCover, edited = 1 WHERE id = :id");
    query.bindValue(":coverPage", info.coverPage);
    query.bindValue(":originalCoverSize", info.originalCoverSize);
    query.bindValue(":coverSizeRatio", info.coverSizeRatio);
    query.bindValue(":lastTimeCoverSet", info.lastTimeCoverSet);
    query.bindValue(":usesExternalCover", info.usesExternalCover);
    query.bindValue(":id", info.id.value());
    if (!query.exec()) {
        QLOG_ERROR() << "Unable to update the cover of a comic:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DBHelper::updateAdded(ComicInfo *comicInfo, QSqlDatabase &db)
{
    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info SET "
                            "added = :added"
                            " WHERE id = :id ");

    updateComicInfo.bindValue(":added", comicInfo->added);
    updateComicInfo.bindValue(":id", comicInfo->id.value());
    return updateComicInfo.exec();
}

void DBHelper::update(const Folder &folder, QSqlDatabase &db)
{
    QSqlQuery updateFolderInfo(db);

    updateFolderInfo.prepare("UPDATE folder SET " + TableProjection::assignments(Folder::columns()) + " WHERE id = :id");

    folder.bindTo(updateFolderInfo);
    updateFolderInfo.bindValue(":id", folder.id.value());

    updateFolderInfo.exec();
}

void DBHelper::propagateFolderUpdatesToParent(const Folder &folder, QSqlDatabase &db)
{
    auto currentParentId = folder.parentId.value();
    auto currentId = folder.id.value();
    while (currentParentId != 1 && currentParentId != 0) { // currentParentId is 0 if `folder` is the root folder
        const auto f = loadFolder(currentParentId, db);
        currentId = currentParentId;
        currentParentId = f ? f->parentId.value() : 0;
    }

    if (currentId != folder.id.value()) {
        updateChildrenInfo(currentId, db);
    }
}

Folder DBHelper::updateChildrenInfo(qulonglong folderId, QSqlDatabase &db)
{
    auto folder = loadFolder(folderId, db).value_or(Folder());
    folder.id = FolderId(folderId);
    const auto subfolders = DBHelper::getFoldersFromParent(folderId, db, true);
    const auto comics = DBHelper::getComicsFromParent(folderId, db, true);

    QList<Folder> updatedSubfolders;
    for (const auto &sf : subfolders) {
        updatedSubfolders.append(updateChildrenInfo(sf.id.value(), db));
    }

    QString coverHash = "";

    if (!comics.isEmpty()) {
        coverHash = comics.first().info.hash;
    } else {
        for (const auto &f : updatedSubfolders) {
            auto firstChildHash = f.firstChildHash;
            if (!firstChildHash.isEmpty()) {
                coverHash = firstChildHash;
                break;
            }
        }
    }

    if (folder.numChildren == subfolders.count() + comics.count() && folder.firstChildHash == coverHash) {
        return folder;
    }

    folder.numChildren = subfolders.count() + comics.count();
    folder.firstChildHash = coverHash;

    QSqlQuery updateFolderInfo(db);
    updateFolderInfo.prepare("UPDATE folder SET "
                             "numChildren = :numChildren, "
                             "firstChildHash = :firstChildHash "
                             "WHERE id = :id ");
    updateFolderInfo.bindValue(":numChildren", folder.numChildren);
    updateFolderInfo.bindValue(":firstChildHash", folder.firstChildHash);
    updateFolderInfo.bindValue(":id", folderId);
    updateFolderInfo.exec();

    return folder;
}

void DBHelper::updateChildrenInfo(QSqlDatabase &db)
{
    // measure time
    auto start = std::chrono::high_resolution_clock::now();

    QSqlQuery selectQuery(db); // TODO check
    selectQuery.prepare("SELECT id FROM folder f WHERE f.parentId = 1 AND f.id <> 1");
    selectQuery.exec();

    while (selectQuery.next()) {
        DBHelper::updateChildrenInfo(selectQuery.value(0).toULongLong(), db);
    }

    auto end = std::chrono::high_resolution_clock::now();

    QString time = QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    QString message = "updateChildrenInfo took " + time + "ms";
    QLOG_INFO() << message;
}

void DBHelper::updateProgress(qulonglong libraryId, ComicId comicId, const ComicInfo &comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        bool found;
        ComicDB comic = DBHelper::loadComic(comicId.value(), db, found);
        comic.info.currentPage = comicInfo.currentPage;
        comic.info.hasBeenOpened = comicInfo.currentPage > 0 || comic.info.hasBeenOpened;
        comic.info.read = comic.info.read || comic.info.currentPage == comic.info.numPages;

        DBHelper::updateReadingRemoteProgress(comic.info, db);

        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
}

void DBHelper::updateImageFilters(qulonglong libraryId, ComicId comicId, const ComicInfo &comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        bool found;
        ComicDB comic = DBHelper::loadComic(comicId.value(), db, found);

        if (found && comic.info.lastTimeImageFiltersSet.toULongLong() < comicInfo.lastTimeImageFiltersSet.toULongLong()) {
            QSqlQuery updateComicInfo(db);
            updateComicInfo.prepare("UPDATE comic_info SET "
                                    "lastTimeImageFiltersSet = :lastTimeImageFiltersSet, "
                                    "imageFiltersJson = :imageFiltersJson "
                                    " WHERE id = :id ");

            updateComicInfo.bindValue(":lastTimeImageFiltersSet", comicInfo.lastTimeImageFiltersSet.toULongLong());
            updateComicInfo.bindValue(":imageFiltersJson", comicInfo.imageFiltersJson);
            updateComicInfo.bindValue(":id", comic.info.id.value());

            auto ret = updateComicInfo.exec();
            if (!ret) {
                QLOG_ERROR() << "Error updating image filters for comic ID:" << comicId.value();
                QLOG_ERROR() << updateComicInfo.lastError().databaseText();
                QLOG_ERROR() << updateComicInfo.lastError().text();
            }
        }

        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
}

void DBHelper::setComicAsReading(qulonglong libraryId, ComicId comicId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        bool found;
        ComicDB comic = DBHelper::loadComic(comicId.value(), db, found);
        comic.info.hasBeenOpened = true;
        comic.info.read = comic.info.read || comic.info.currentPage == comic.info.numPages;

        DBHelper::updateReadingRemoteProgress(comic.info, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void DBHelper::updateReadingRemoteProgress(const ComicInfo &comicInfo, QSqlDatabase &db)
{
    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info SET "
                            "read = :read, "
                            "currentPage = :currentPage, "
                            "hasBeenOpened = :hasBeenOpened, "
                            "lastTimeOpened = :lastTimeOpened, "
                            "rating = :rating"
                            " WHERE id = :id ");

    updateComicInfo.bindValue(":read", comicInfo.read ? 1 : 0);
    updateComicInfo.bindValue(":currentPage", comicInfo.currentPage);
    updateComicInfo.bindValue(":hasBeenOpened", comicInfo.hasBeenOpened ? 1 : 0);
    updateComicInfo.bindValue(":lastTimeOpened", QDateTime::currentSecsSinceEpoch());
    updateComicInfo.bindValue(":id", comicInfo.id.value());
    updateComicInfo.bindValue(":rating", comicInfo.rating);
    updateComicInfo.exec();

    updateComicInfo.clear();
}

QMap<qulonglong, QList<ComicDB>> DBHelper::updateFromRemoteClient(const QMap<qulonglong, QList<ComicProgressUpdate>> &comics, bool clientSendsHasBeenOpened, bool clientSendsImageFilters)
{
    QMap<qulonglong, QList<ComicDB>> moreRecentComics;

    const auto libraryIds = comics.keys();
    for (const auto libraryId : libraryIds) {
        QList<ComicDB> libraryMoreRecentComics;

        QString libraryPath = DBHelper::getLibraries().getPath(libraryId);

        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

            if (!db.isValid()) {
                QLOG_ERROR() << "updateFromRemoteClient: could not open database for library" << libraryPath;
                continue;
            }

            db.transaction();

            QSqlQuery updateComicInfo(db);
            updateComicInfo.prepare("UPDATE comic_info SET "
                                    "read = :read, "
                                    "currentPage = :currentPage, "
                                    "hasBeenOpened = :hasBeenOpened, "
                                    "lastTimeOpened = :lastTimeOpened, "
                                    "rating = :rating, "
                                    "imageFiltersJson = :imageFiltersJson, "
                                    "lastTimeImageFiltersSet = :lastTimeImageFiltersSet "
                                    " WHERE id = :id ");

            const auto &updates = comics.constFind(libraryId).value();
            for (const auto &update : updates) {
                const auto &comicInfo = update.info;
                bool found;
                ComicDB comic = DBHelper::loadComic(update.comicId.value(), db, found);

                if (comic.info.hash == comicInfo.hash) {
                    bool isMoreRecent = false;

                    // completion takes precedence over lastTimeOpened, if we just want to synchronize the lastest status we should use only lastTimeOpened
                    if ((comic.info.currentPage > 1 && comic.info.currentPage > comicInfo.currentPage) || (comic.info.read && !comicInfo.read)) {
                        isMoreRecent = true;
                    }

                    if (comic.info.hasBeenOpened && comic.info.currentPage > comicInfo.currentPage) {
                        isMoreRecent = true;
                    }

                    if (comic.info.lastTimeOpened.toULongLong() > 0 && comicInfo.lastTimeOpened.toULongLong() == 0) {
                        isMoreRecent = true;
                    }

                    if (clientSendsImageFilters) {
                        if (comic.info.lastTimeImageFiltersSet.toULongLong() > comicInfo.lastTimeImageFiltersSet.toULongLong()) {
                            isMoreRecent = true;
                        }
                    }

                    comic.info.currentPage = qMax(comic.info.currentPage, comicInfo.currentPage);

                    if (comic.info.currentPage == comic.info.numPages)
                        comic.info.read = true;

                    comic.info.read = comic.info.read || comicInfo.read;

                    if (clientSendsHasBeenOpened) {
                        comic.info.hasBeenOpened = comic.info.hasBeenOpened || comicInfo.hasBeenOpened; // android
                    } else {
                        comic.info.hasBeenOpened = comic.info.hasBeenOpened || comicInfo.currentPage > 0; // ios (legacy)
                    }

                    if (comic.info.lastTimeOpened.toULongLong() < comicInfo.lastTimeOpened.toULongLong() && comicInfo.lastTimeOpened.toULongLong() > 0)
                        comic.info.lastTimeOpened = comicInfo.lastTimeOpened;

                    if (comicInfo.rating > 0)
                        comic.info.rating = comicInfo.rating;

                    if (clientSendsImageFilters) {
                        if (comic.info.lastTimeImageFiltersSet.toULongLong() < comicInfo.lastTimeImageFiltersSet.toULongLong()) {
                            comic.info.imageFiltersJson = comicInfo.imageFiltersJson;
                            comic.info.lastTimeImageFiltersSet = comicInfo.lastTimeImageFiltersSet;
                        }
                    }

                    updateComicInfo.bindValue(":read", comic.info.read ? 1 : 0);
                    updateComicInfo.bindValue(":currentPage", comic.info.currentPage);
                    updateComicInfo.bindValue(":hasBeenOpened", comic.info.hasBeenOpened ? 1 : 0);
                    updateComicInfo.bindValue(":lastTimeOpened", comic.info.lastTimeOpened);
                    updateComicInfo.bindValue(":id", comic.info.id.value());
                    updateComicInfo.bindValue(":rating", comic.info.rating);
                    updateComicInfo.bindValue(":imageFiltersJson", comic.info.imageFiltersJson);
                    updateComicInfo.bindValue(":lastTimeImageFiltersSet", comic.info.lastTimeImageFiltersSet);
                    updateComicInfo.exec();

                    if (isMoreRecent) {
                        libraryMoreRecentComics.append(comic);
                    }
                }
            }

            if (!libraryMoreRecentComics.isEmpty()) {
                moreRecentComics[libraryId] = libraryMoreRecentComics;
            }

            db.commit();
            connectionName = db.connectionName();
        }

        QSqlDatabase::removeDatabase(connectionName);
    }

    return moreRecentComics;
}

QList<QUuid> DBHelper::updateFromRemoteClientWithHash(const QList<ComicInfo> &comics)
{
    QList<QUuid> updatedLibraries;
    const YACReaderLibraries libraries = DBHelper::getLibraries();
    const QStringList names = libraries.getNames();

    for (const auto &name : names) {
        bool libraryUpdated = false;
        QString libraryPath = DBHelper::getLibraries().getPath(libraries.getId(name));
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

            if (!db.isValid()) {
                QLOG_ERROR() << "updateFromRemoteClientWithHash: could not open database for library" << libraryPath;
                continue;
            }

            db.transaction();

            QSqlQuery updateComicInfo(db);
            updateComicInfo.prepare("UPDATE comic_info SET "
                                    "read = :read, "
                                    "currentPage = :currentPage, "
                                    "hasBeenOpened = :hasBeenOpened, "
                                    "lastTimeOpened = :lastTimeOpened, "
                                    "rating = :rating"
                                    " WHERE id = :id ");

            for (const auto &comicInfo : comics) {
                const auto storedInfo = loadComicInfo(comicInfo.hash, db);

                if (!storedInfo) {
                    continue;
                }

                ComicInfo info = *storedInfo;

                if (comicInfo.currentPage > 0) {
                    info.currentPage = comicInfo.currentPage;

                    if (info.currentPage == info.numPages)
                        info.read = true;

                    info.hasBeenOpened = true;

                    if (info.lastTimeOpened.toULongLong() < comicInfo.lastTimeOpened.toULongLong())
                        info.lastTimeOpened = comicInfo.lastTimeOpened;
                }

                if (comicInfo.rating > 0) {
                    info.rating = comicInfo.rating;
                }

                updateComicInfo.bindValue(":read", info.read ? 1 : 0);
                updateComicInfo.bindValue(":currentPage", info.currentPage);
                updateComicInfo.bindValue(":hasBeenOpened", info.hasBeenOpened ? 1 : 0);
                updateComicInfo.bindValue(":lastTimeOpened", QDateTime::currentSecsSinceEpoch());
                updateComicInfo.bindValue(":id", info.id.value());
                updateComicInfo.bindValue(":rating", info.rating);
                updateComicInfo.exec();
                libraryUpdated = true;
            }

            db.commit();
            connectionName = db.connectionName();
        }
        QSqlDatabase::removeDatabase(connectionName);

        if (libraryUpdated) {
            updatedLibraries.append(libraries.getUuid(name));
        }
    }

    return updatedLibraries;
}

std::optional<ListFolderId> DBHelper::insertListFolder(ListFolderKind kind, const std::optional<qulonglong> &parentId, const QString &name, QSqlDatabase &db)
{
    const auto tables = folderTable(kind);
    if (parentId && !rowExists(tables.table, *parentId, db))
        return std::nullopt;

    QSqlQuery insert(db);
    insert.prepare(QString("INSERT INTO %1 (parent_id, name, ordering) "
                           "VALUES (:parent_id, :name, (SELECT COALESCE(MAX(ordering) + 1, 0) FROM %1 WHERE parent_id IS :parent_id))")
                           .arg(tables.table));
    insert.bindValue(":parent_id", parentId ? QVariant(*parentId) : QVariant());
    insert.bindValue(":name", name);
    if (!insert.exec()) {
        QLOG_ERROR() << "Unable to insert a list folder:" << insert.lastError().text();
        return std::nullopt;
    }
    return ListFolderId { kind, insert.lastInsertId().toULongLong() };
}

bool DBHelper::renameListFolder(const ListFolderId &id, const QString &name, QSqlDatabase &db)
{
    QSqlQuery update(db);
    update.prepare(QString("UPDATE %1 SET name = :name WHERE id = :id").arg(folderTable(id.kind).table));
    update.bindValue(":name", name);
    update.bindValue(":id", id.id);
    if (!update.exec() || update.numRowsAffected() != 1) {
        QLOG_ERROR() << "Unable to rename a list folder:" << update.lastError().text();
        return false;
    }
    return true;
}

bool DBHelper::moveListFolder(const ListFolderId &id, const std::optional<qulonglong> &parentId, QSqlDatabase &db)
{
    const auto tables = folderTable(id.kind);
    const QVariant parent = parentId ? QVariant(*parentId) : QVariant();

    db.transaction();
    bool moved = rowExists(tables.table, id.id, db) && (!parentId || rowExists(tables.table, *parentId, db));
    if (moved && parentId) {
        // the new parent cannot be the folder or one of the folders below it; the driver binds qulonglong as text, and
        // the columns of a CTE have no type to convert it
        QSqlQuery below(db);
        below.prepare(QString("WITH RECURSIVE subtree(id) AS (SELECT CAST(:id AS INTEGER) UNION ALL SELECT f.id FROM %1 f INNER JOIN subtree s ON f.parent_id = s.id) "
                              "SELECT 1 FROM subtree WHERE id = CAST(:parent_id AS INTEGER)")
                              .arg(tables.table));
        below.bindValue(":id", id.id);
        below.bindValue(":parent_id", *parentId);
        moved = below.exec() && !below.next();
    }

    if (moved) {
        QSqlQuery move(db);
        move.prepare(QString("UPDATE %1 SET parent_id = :parent_id, "
                             "ordering = (SELECT COALESCE(MAX(ordering) + 1, 0) FROM %1 WHERE parent_id IS :parent_id AND id <> :id) "
                             "WHERE id = :id")
                             .arg(tables.table));
        move.bindValue(":parent_id", parent);
        move.bindValue(":id", id.id);
        moved = move.exec() && move.numRowsAffected() == 1;
        if (!moved)
            QLOG_ERROR() << "Unable to move a list folder:" << move.lastError().text();
    }

    if (moved && db.commit())
        return true;
    db.rollback();
    return false;
}

bool DBHelper::moveReadingListToFolder(ReadingListId id, const std::optional<qulonglong> &folderId, QSqlDatabase &db)
{
    return moveListToFolder(ListFolderKind::ReadingList, id.value(), folderId, db);
}

bool DBHelper::moveSmartListToFolder(SmartListId id, const std::optional<qulonglong> &folderId, QSqlDatabase &db)
{
    return moveListToFolder(ListFolderKind::SmartList, id.value(), folderId, db);
}

std::optional<QStringList> DBHelper::removeListFolder(const ListFolderId &id, QSqlDatabase &db)
{
    const auto tables = folderTable(id.kind);

    // the placeholders of the entries of every reading list below the folder, child lists included
    QSqlQuery placeholderIds(db);
    if (id.kind == ListFolderKind::ReadingList) {
        placeholderIds.prepare("WITH RECURSIVE folders(id) AS (SELECT CAST(:id AS INTEGER) UNION ALL SELECT f.id FROM reading_list_folder f INNER JOIN folders s ON f.parent_id = s.id), "
                               "lists(id) AS (SELECT r.id FROM reading_list r WHERE r.folder_id IN (SELECT id FROM folders) "
                               "UNION ALL SELECT r.id FROM reading_list r INNER JOIN lists l ON r.parentId = l.id) "
                               "SELECT DISTINCT placeholder_id FROM reading_list_entry WHERE placeholder_id IS NOT NULL AND reading_list_id IN (SELECT id FROM lists)");
        placeholderIds.bindValue(":id", id.id);
    } else {
        placeholderIds.prepare("SELECT NULL WHERE 0");
    }

    // the foreign keys delete the folders, lists, child lists, and entries below it
    const auto removal = [&] {
        QSqlQuery query(db);
        query.prepare(QString("DELETE FROM %1 WHERE id = :id").arg(tables.table));
        query.bindValue(":id", id.id);
        if (!query.exec() || query.numRowsAffected() != 1) {
            QLOG_ERROR() << "Unable to remove a list folder:" << query.lastError().text();
            return false;
        }
        return true;
    };
    return removeWithPlaceholders(placeholderIds, removal, db);
}

bool DBHelper::updateSmartList(SmartListId id, const QString &name, const QString &query, QSqlDatabase &db)
{
    QSqlQuery update(db);
    update.prepare("UPDATE smart_list SET name = :name, query = :query WHERE id = :id");
    update.bindValue(":name", name);
    update.bindValue(":query", query);
    update.bindValue(":id", id.value());
    if (!update.exec() || update.numRowsAffected() != 1) {
        QLOG_ERROR() << "Unable to update a smart list:" << update.lastError().text();
        return false;
    }
    return true;
}

bool DBHelper::removeSmartList(SmartListId id, QSqlDatabase &db)
{
    QSqlQuery remove(db);
    remove.prepare("DELETE FROM smart_list WHERE id = :id");
    remove.bindValue(":id", id.value());
    if (!remove.exec() || remove.numRowsAffected() != 1) {
        QLOG_ERROR() << "Unable to remove a smart list:" << remove.lastError().text();
        return false;
    }
    return true;
}

void DBHelper::renameLabel(qulonglong id, const QString &name, QSqlDatabase &db)
{
    QSqlQuery renameLabelQuery(db);
    renameLabelQuery.prepare("UPDATE label SET "
                             "name = :name "
                             "WHERE id = :id");
    renameLabelQuery.bindValue(":name", name);
    renameLabelQuery.bindValue(":id", id);
    renameLabelQuery.exec();

    QLOG_DEBUG() << renameLabelQuery.lastError().databaseText();
}

void DBHelper::renameList(qulonglong id, const QString &name, QSqlDatabase &db)
{
    QSqlQuery renameLabelQuery(db);
    renameLabelQuery.prepare("UPDATE reading_list SET "
                             "name = :name "
                             "WHERE id = :id");
    renameLabelQuery.bindValue(":name", name);
    renameLabelQuery.bindValue(":id", id);
    renameLabelQuery.exec();
}

void DBHelper::reasignOrderToSublists(QList<qulonglong> ids, QSqlDatabase &db)
{
    QSqlQuery updateOrdering(db);
    updateOrdering.prepare("UPDATE reading_list SET "
                           "ordering = :ordering "
                           "WHERE id = :id");
    db.transaction();
    int order = 0;
    const auto &readingListIds = ids;
    for (const auto id : readingListIds) {
        updateOrdering.bindValue(":ordering", order++);
        updateOrdering.bindValue(":id", id);
        updateOrdering.exec();
    }

    db.commit();
}

void DBHelper::reasignOrderToComicsInFavorites(QList<qulonglong> comicIds, QSqlDatabase &db)
{
    QSqlQuery updateOrdering(db);
    updateOrdering.prepare("UPDATE comic_default_reading_list SET "
                           "ordering = :ordering "
                           "WHERE comic_id = :comic_id AND default_reading_list_id = 1");
    db.transaction();
    int order = 0;
    const auto &favoriteComicIds = comicIds;
    for (const auto id : favoriteComicIds) {
        updateOrdering.bindValue(":ordering", order++);
        updateOrdering.bindValue(":comic_id", id);
        updateOrdering.exec();
    }

    db.commit();
}

bool DBHelper::moveEntriesToComicInfo(ComicInfoId oldComicInfoId, ComicInfoId newComicInfoId, QSqlDatabase &db)
{
    for (const auto table : { "reading_list_entry", "label_entry" }) {
        QSqlQuery move(db);
        move.prepare(QString("UPDATE %1 SET comic_info_id = :new_comic_info_id "
                             "WHERE comic_info_id = :old_comic_info_id "
                             "AND NOT EXISTS (SELECT 1 FROM comic WHERE comicInfoId = :old_comic_info_id)")
                             .arg(table));
        move.bindValue(":new_comic_info_id", newComicInfoId.value());
        move.bindValue(":old_comic_info_id", oldComicInfoId.value());
        if (!move.exec()) {
            QLOG_ERROR() << "Unable to move the entries of" << table << "to the new comic info:" << move.lastError().text();
            return false;
        }
    }

    return true;
}

QStringList DBHelper::removeUnusedComicInfo(QSqlDatabase &db)
{
    static const QString unused = "NOT EXISTS (SELECT 1 FROM comic WHERE comicInfoId = comic_info.id) "
                                  "AND NOT EXISTS (SELECT 1 FROM reading_list_entry WHERE comic_info_id = comic_info.id) "
                                  "AND NOT EXISTS (SELECT 1 FROM label_entry WHERE comic_info_id = comic_info.id)";

    QStringList hashes;
    QSqlQuery selectUnused(db);
    selectUnused.setForwardOnly(true);
    if (!selectUnused.exec("SELECT hash FROM comic_info WHERE " + unused)) {
        QLOG_ERROR() << "Error getting unused comic info:" << selectUnused.lastError().text();
        return { };
    }
    while (selectUnused.next()) {
        hashes.append(selectUnused.value(0).toString());
    }

    QSqlQuery deleteUnused(db);
    if (!deleteUnused.exec("DELETE FROM comic_info WHERE " + unused)) {
        QLOG_ERROR() << "Error purging info from comic_info:" << deleteUnused.lastError().text();
        return { };
    }

    return hashes;
}

bool DBHelper::renameFolder(qulonglong id, const QString &name, const QString &oldPath, const QString &newPath, QSqlDatabase &db, QString *error)
{
    if (error != nullptr)
        error->clear();

    const auto execute = [error](QSqlQuery &query) {
        if (query.exec())
            return true;

        if (error != nullptr)
            *error = query.lastError().text();
        return false;
    };

    QSqlQuery renameFolder(db);
    renameFolder.prepare("UPDATE folder SET name = :name, path = :newPath WHERE id = :id AND path = :oldPath");
    renameFolder.bindValue(":name", name);
    renameFolder.bindValue(":newPath", newPath);
    renameFolder.bindValue(":id", id);
    renameFolder.bindValue(":oldPath", oldPath);
    if (!execute(renameFolder) || renameFolder.numRowsAffected() != 1) {
        if (error != nullptr && error->isEmpty())
            *error = QCoreApplication::translate("DBHelper", "The folder entry could not be found in the library database.");
        return false;
    }

    QSqlQuery updateSubfolders(db);
    updateSubfolders.prepare("UPDATE folder SET path = :newPath || substr(path, length(:oldPath) + 1) "
                             "WHERE substr(path, 1, length(:oldPath) + 1) = :oldPath || '/'");
    updateSubfolders.bindValue(":newPath", newPath);
    updateSubfolders.bindValue(":oldPath", oldPath);
    if (!execute(updateSubfolders))
        return false;

    QSqlQuery updateComics(db);
    updateComics.prepare("UPDATE comic SET path = :newPath || substr(path, length(:oldPath) + 1) "
                         "WHERE substr(path, 1, length(:oldPath) + 1) = :oldPath || '/'");
    updateComics.bindValue(":newPath", newPath);
    updateComics.bindValue(":oldPath", oldPath);
    return execute(updateComics);
}

bool DBHelper::moveComic(qulonglong comicId, qulonglong newParentId, const QString &newFileName, const QString &newRelativePath, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("UPDATE comic SET parentId = :parentId, fileName = :fileName, path = :path WHERE id = :id");
    query.bindValue(":parentId", newParentId);
    query.bindValue(":fileName", newFileName);
    query.bindValue(":path", newRelativePath);
    query.bindValue(":id", comicId);

    return query.exec() && query.numRowsAffected() == 1;
}

qulonglong DBHelper::ensureFolderPath(const QString &relativePath, QSqlDatabase &db, QList<qulonglong> *createdFolderIds)
{
    const auto segments = relativePath.split('/', Qt::SkipEmptyParts);

    qulonglong parentId = 1;
    auto inheritedType = DBHelper::loadFolder(parentId, db).value_or(Folder()).type;
    QString currentPath;

    for (const auto &segment : segments) {
        currentPath += '/' + segment;

        const auto existing = DBHelper::loadFolder(segment, parentId, db);
        if (existing) {
            parentId = existing->id.value();
            inheritedType = existing->type;
            continue;
        }

        Folder folder;
        folder.name = segment;
        folder.path = currentPath;
        folder.parentId = FolderId(parentId);
        folder.type = inheritedType;

        parentId = DBHelper::insert(&folder, db);
        if (createdFolderIds != nullptr)
            createdFolderIds->append(parentId);
    }

    return parentId;
}

void DBHelper::syncFolderAddedFromContents(const QList<qulonglong> &folderIds, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("UPDATE folder SET added = COALESCE("
                  "(SELECT MIN(ci.added) FROM comic c INNER JOIN comic_info ci ON c.comicInfoId = ci.id WHERE c.parentId = folder.id), added) "
                  "WHERE id = :id");

    for (const auto id : folderIds) {
        query.bindValue(":id", id);
        if (!query.exec()) {
            QLOG_ERROR() << "syncFolderAddedFromContents: update failed for folder" << id << query.lastError().text();
        }
    }
}

void DBHelper::removeEmptyFolderPaths(const QStringList &relativePaths, QSqlDatabase &db, QList<QVariantMap> *removedRows)
{
    QSqlQuery select(db);
    select.prepare("SELECT * FROM folder WHERE path = :path AND id <> 1"
                   " AND NOT EXISTS (SELECT 1 FROM comic WHERE comic.parentId = folder.id)"
                   " AND NOT EXISTS (SELECT 1 FROM folder AS child WHERE child.parentId = folder.id)");

    QSqlQuery remove(db);
    remove.prepare("DELETE FROM folder WHERE id = :id");

    for (const auto &path : relativePaths) {
        select.bindValue(":path", path);
        if (!select.exec()) {
            QLOG_ERROR() << "removeEmptyFolderPaths: select failed for" << path << select.lastError().text();
            continue;
        }

        if (!select.next())
            continue;

        const auto record = select.record();

        QVariantMap row;
        for (int i = 0; i < record.count(); ++i)
            row.insert(record.fieldName(i), record.value(i));

        remove.bindValue(":id", row.value(QStringLiteral("id")));
        if (!remove.exec()) {
            QLOG_ERROR() << "removeEmptyFolderPaths: delete failed for" << path << remove.lastError().text();
            continue;
        }

        if (removedRows != nullptr)
            removedRows->append(row);
    }
}

void DBHelper::removeEmptyFolderRows(const QList<qulonglong> &folderIds, QSqlDatabase &db)
{
    QSqlQuery remove(db);
    remove.prepare("DELETE FROM folder WHERE id = :id AND id <> 1"
                   " AND NOT EXISTS (SELECT 1 FROM comic WHERE comic.parentId = folder.id)"
                   " AND NOT EXISTS (SELECT 1 FROM folder AS child WHERE child.parentId = folder.id)");

    for (const auto id : folderIds) {
        remove.bindValue(":id", id);
        if (!remove.exec()) {
            QLOG_ERROR() << "removeEmptyFolderRows: delete failed for folder" << id << remove.lastError().text();
        }
    }
}

bool DBHelper::restoreFolderRows(const QList<QVariantMap> &rows, QSqlDatabase &db)
{
    // A child cannot be inserted before its parent, because parentId is a foreign
    // key into the same table.
    auto ordered = rows;
    std::sort(ordered.begin(), ordered.end(), [](const QVariantMap &a, const QVariantMap &b) {
        return a.value(QStringLiteral("path")).toString().count(QLatin1Char('/')) < b.value(QStringLiteral("path")).toString().count(QLatin1Char('/'));
    });

    bool success = true;

    for (const auto &row : std::as_const(ordered)) {
        if (row.value(QStringLiteral("id")).toULongLong() == 0)
            continue;

        QStringList columns;
        QStringList placeholders;
        for (auto it = row.constBegin(); it != row.constEnd(); ++it) {
            columns << it.key();
            placeholders << QLatin1Char(':') + it.key();
        }

        QSqlQuery insert(db);
        insert.prepare(QStringLiteral("INSERT OR IGNORE INTO folder (%1) VALUES (%2)")
                               .arg(columns.join(QStringLiteral(", ")), placeholders.join(QStringLiteral(", "))));

        for (auto it = row.constBegin(); it != row.constEnd(); ++it)
            insert.bindValue(QLatin1Char(':') + it.key(), it.value());

        if (!insert.exec()) {
            QLOG_ERROR() << "restoreFolderRows: insert failed for"
                         << row.value(QStringLiteral("path")).toString() << insert.lastError().text();
            success = false;
        }
    }

    return success;
}

// inserts
qulonglong DBHelper::insert(Folder *folder, QSqlDatabase &db)
{
    auto added = QDateTime::currentSecsSinceEpoch();
    folder->added = added;

    QSqlQuery query(db);
    query.prepare("INSERT INTO folder (parentId, name, path, added, type) "
                  "VALUES (:parentId, :name, :path, :added, :type)");
    query.bindValue(":parentId", folder->parentId.value());
    query.bindValue(":name", folder->name);
    query.bindValue(":path", folder->path);
    query.bindValue(":added", added);
    auto intType = static_cast<int>(folder->type);
    query.bindValue(":type", intType);
    query.exec();

    return query.lastInsertId().toULongLong();
}

qulonglong DBHelper::insert(ComicDB *comic, QSqlDatabase &db, bool insertAllInfo)
{
    auto added = QDateTime::currentSecsSinceEpoch();

    if (comic->info.id == ComicInfoId()) {
        QSqlQuery comicInfoInsert(db);

        comicInfoInsert.prepare("INSERT INTO comic_info (hash,numPages,coverSizeRatio,originalCoverSize,added,type) "
                                "VALUES (:hash,:numPages,:coverSizeRatio,:originalCoverSize,:added,:type)");
        comicInfoInsert.bindValue(":hash", comic->info.hash);
        comicInfoInsert.bindValue(":numPages", comic->info.numPages);
        comicInfoInsert.bindValue(":coverSizeRatio", comic->info.coverSizeRatio);
        comicInfoInsert.bindValue(":originalCoverSize", comic->info.originalCoverSize);
        comicInfoInsert.bindValue(":added", added);
        auto intType = static_cast<int>(comic->info.type.value<YACReader::FileType>());
        comicInfoInsert.bindValue(":type", intType);
        comicInfoInsert.exec();
        comic->info.id = ComicInfoId(comicInfoInsert.lastInsertId().toULongLong());
        comic->info.added = added;

        if (insertAllInfo) {
            DBHelper::update(&(comic->info), db); // TODO use insert to insert all the info values, the common binding need to be extracted and shared between update and insert
        }
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic (parentId, comicInfoId, fileName, path) "
                  "VALUES (:parentId,:comicInfoId,:name, :path)");
    query.bindValue(":parentId", comic->file.folderId.value());
    query.bindValue(":comicInfoId", comic->info.id.value());
    query.bindValue(":name", comic->file.fileName);
    query.bindValue(":path", comic->file.path);
    query.exec();

    // loop through parents and update their updated field
    // TODO: use stored procedures
    QSqlQuery updateFolder(db);
    updateFolder.prepare("UPDATE folder SET "
                         "updated = :updated "
                         "WHERE id = :id ");
    auto currentParentId = comic->file.folderId.value();
    while (currentParentId != 1 && currentParentId != 0) {
        updateFolder.bindValue(":updated", added);
        updateFolder.bindValue(":id", currentParentId);
        updateFolder.exec();

        const auto f = loadFolder(currentParentId, db);
        currentParentId = f ? f->parentId.value() : 0;
    }
    //----

    return query.lastInsertId().toULongLong();
}

qulonglong DBHelper::insertLabel(const QString &name, YACReader::LabelColors color, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO label (name, color, ordering) "
                  "VALUES (:name, :color, :ordering)");
    query.bindValue(":name", name);
    query.bindValue(":color", YACReader::colorToName(color));
    query.bindValue(":ordering", color);
    query.exec();
    return query.lastInsertId().toULongLong();
}

std::optional<PlaceholderId> DBHelper::insertPlaceholder(const Placeholder &placeholder, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO placeholder (" + TableProjection::insertColumns(Placeholder::columns()) + ") VALUES (" + TableProjection::insertValues(Placeholder::columns()) + ")");
    placeholder.bindTo(query);
    if (!query.exec()) {
        QLOG_ERROR() << "Unable to insert a placeholder:" << query.lastError().text();
        return std::nullopt;
    }
    return PlaceholderId(query.lastInsertId().toULongLong());
}

qulonglong DBHelper::insertReadingList(const QString &name, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO reading_list (name) "
                  "VALUES (:name)");
    query.bindValue(":name", name);
    query.exec();
    return query.lastInsertId().toULongLong();
}

std::optional<SmartListId> DBHelper::insertSmartList(const QString &name, const QString &query, QSqlDatabase &db)
{
    QSqlQuery insert(db);
    insert.prepare("INSERT INTO smart_list (name, query, ordering) "
                   "VALUES (:name, :query, (SELECT COALESCE(MAX(ordering) + 1, 0) FROM smart_list))");
    insert.bindValue(":name", name);
    insert.bindValue(":query", query);
    if (!insert.exec()) {
        QLOG_ERROR() << "Unable to insert a smart list:" << insert.lastError().text();
        return std::nullopt;
    }
    return SmartListId(insert.lastInsertId().toULongLong());
}

qulonglong DBHelper::insertReadingSubList(const QString &name, qulonglong parentId, int ordering, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO reading_list (name, parentId, ordering) "
                  "VALUES (:name, :parentId, :ordering)");
    query.bindValue(":name", name);
    query.bindValue(":parentId", parentId);
    query.bindValue(":ordering", ordering);
    query.exec();
    return query.lastInsertId().toULongLong();
}

void DBHelper::insertComicsInFavorites(const QList<ComicDB> &comicsList, QSqlDatabase &db)
{
    QSqlQuery getNumComicsInFavoritesQuery("SELECT count(*) FROM comic_default_reading_list WHERE default_reading_list_id = 1;", db);
    getNumComicsInFavoritesQuery.next();

    int numComics = getNumComicsInFavoritesQuery.value(0).toInt();

    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic_default_reading_list (default_reading_list_id, comic_id, ordering) "
                  "VALUES (1, :comic_id, :ordering)");

    for (const auto &comic : comicsList) {
        query.bindValue(":comic_id", comic.file.id.value());
        query.bindValue(":ordering", numComics++);
        query.exec();
    }

    QLOG_TRACE() << query.lastError();

    db.commit();
}

// queries
QList<Folder> DBHelper::getFoldersFromParent(qulonglong parentId, QSqlDatabase &db, bool sort)
{
    QList<Folder> list;

    QSqlQuery selectQuery(db); // TODO check
    selectQuery.prepare("SELECT " + TableProjection::selectList("f", Folder::columns()) + " FROM folder f WHERE f.parentId = :parentId and f.id <> 1");
    selectQuery.bindValue(":parentId", parentId);
    selectQuery.exec();

    const auto indexes = Folder::columnIndexes(selectQuery.record(), "f");
    while (selectQuery.next()) {
        // TODO sort by sort indicator and name
        const auto currentItem = Folder::fromQuery(selectQuery, indexes);

        int lessThan = 0;

        if (list.isEmpty() || !sort)
            list.append(currentItem);
        else {
            QString nameLast = list.back().name;
            QString nameCurrent = currentItem.name;
            QList<Folder>::iterator i;
            i = list.end();
            i--;
            while ((0 > (lessThan = naturalCompare(nameCurrent, nameLast, Qt::CaseInsensitive))) && i != list.begin()) {
                i--;
                nameLast = i->name;
            }
            if (lessThan >= 0) // si se ha encontrado un elemento menor que current, se inserta justo después
                list.insert(++i, currentItem);
            else
                list.insert(i, currentItem);
        }
    }

    return list;
}

QList<ComicDB> DBHelper::getSortedComicsFromParent(qulonglong parentId, QSqlDatabase &db)
{
    QList<ComicDB> list;

    QSqlQuery selectQuery(db);

    selectQuery.setForwardOnly(true);
    selectQuery.prepare("SELECT " + ComicDB::selectList("c", "ci") + " FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) WHERE c.parentId = :parentId");
    selectQuery.bindValue(":parentId", parentId);
    selectQuery.exec();

    list = comicsFromQuery(selectQuery);

    std::sort(list.begin(), list.end(), [](const ComicDB &c1, const ComicDB &c2) {
        return comicNumberLessThan(c1.info.number, c1.file.fileName, c2.info.number, c2.file.fileName);
    });

    // selectQuery.finish();
    return list;
}
QList<ComicDB> DBHelper::getComicsFromParent(qulonglong parentId, QSqlDatabase &db, bool sort)
{
    QList<ComicDB> list;

    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT " + ComicDB::selectList("c", "ci") + " FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) WHERE c.parentId = :parentId");
    selectQuery.bindValue(":parentId", parentId);
    selectQuery.exec();

    list = comicsFromQuery(selectQuery);

    if (sort) {
        std::sort(list.begin(), list.end(), [](const ComicDB &c1, const ComicDB &c2) {
            return c1.file.fileName.localeAwareCompare(c2.file.fileName) < 0;
        });
    }

    return list;
}

QList<Label> DBHelper::getLabels(qulonglong libraryId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<Label> labels;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        QSqlQuery selectQuery("SELECT " + TableProjection::selectList("l", Label::columns()) +
                                      " FROM label l ORDER BY l.ordering, l.name",
                              db); // TODO add some kind of

        const auto indexes = Label::columnIndexes(selectQuery.record(), "l");
        while (selectQuery.next()) {
            const auto item = Label::fromQuery(selectQuery, indexes);

            if (labels.isEmpty()) {
                labels << item;
            } else {
                int i = 0;

                while (i < labels.count() && (labels.at(i).ordering < item.ordering))
                    i++;

                if (i < labels.count()) {
                    if (labels.at(i).ordering == item.ordering) // sort by name
                    {
                        while (i < labels.count() && labels.at(i).ordering == item.ordering && naturalSortLessThanCI(labels.at(i).name, item.name))
                            i++;
                    }
                }
                if (i >= labels.count()) {
                    labels << item;
                } else {
                    labels.insert(i, item);
                }
            }
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return labels;
}

bool DBHelper::updateFolderTreeType(qulonglong id, QSqlDatabase &db, YACReader::FileType type)
{
    QSqlQuery updateFolderQuery(db);
    updateFolderQuery.prepare("UPDATE folder "
                              "SET type = :type "
                              "WHERE id = :id");
    updateFolderQuery.bindValue(":type", static_cast<int>(type));
    updateFolderQuery.bindValue(":id", id);
    bool success = updateFolderQuery.exec();

    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info "
                            "SET type = :type "
                            "WHERE id IN (SELECT ci.id FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) WHERE c.parentId = :parentId)");
    updateComicInfo.bindValue(":type", static_cast<int>(type));
    updateComicInfo.bindValue(":parentId", id);
    success = updateComicInfo.exec() && success;

    QSqlQuery getSubFoldersQuery(db);
    getSubFoldersQuery.prepare("SELECT id FROM folder WHERE parentId = :parentId AND id <> 1"); // do not select the root folder
    getSubFoldersQuery.bindValue(":parentId", id);
    success = getSubFoldersQuery.exec() && success;

    int childFolderIdPos = getSubFoldersQuery.record().indexOf("id");

    while (getSubFoldersQuery.next()) {
        success = updateFolderTreeType(getSubFoldersQuery.value(childFolderIdPos).toULongLong(), db, type) && success;
    }

    return success;
}

bool DBHelper::setComicRead(qulonglong comicId, bool read, QSqlDatabase &db)
{
    QSqlQuery updateComicInfo(db);
    if (read) {
        updateComicInfo.prepare("UPDATE comic_info SET "
                                "read = 1 "
                                "WHERE id = (SELECT comicInfoId FROM comic WHERE id = :id)");
    } else {
        updateComicInfo.prepare("UPDATE comic_info SET "
                                "read = 0, "
                                "currentPage = 1, "
                                "hasBeenOpened = 0, "
                                "lastTimeOpened = NULL "
                                "WHERE id = (SELECT comicInfoId FROM comic WHERE id = :id)");
    }
    updateComicInfo.bindValue(":id", comicId);

    if (!updateComicInfo.exec()) {
        QLOG_ERROR() << "setComicRead failed for comic" << comicId << ":" << updateComicInfo.lastError().text();
        return false;
    }

    return true;
}

bool DBHelper::setComicType(qulonglong comicId, YACReader::FileType type, QSqlDatabase &db)
{
    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info SET "
                            "type = :type "
                            "WHERE id = (SELECT comicInfoId FROM comic WHERE id = :id)");
    updateComicInfo.bindValue(":type", static_cast<int>(type));
    updateComicInfo.bindValue(":id", comicId);

    if (!updateComicInfo.exec()) {
        QLOG_ERROR() << "setComicType failed for comic" << comicId << ":" << updateComicInfo.lastError().text();
        return false;
    }

    return true;
}

bool DBHelper::setFolderFinished(qulonglong folderId, bool finished, QSqlDatabase &db)
{
    QSqlQuery updateFolder(db);
    updateFolder.prepare("UPDATE folder SET finished = :finished WHERE id = :id");
    updateFolder.bindValue(":finished", finished ? 1 : 0);
    updateFolder.bindValue(":id", folderId);

    if (!updateFolder.exec()) {
        QLOG_ERROR() << "setFolderFinished failed for folder" << folderId << ":" << updateFolder.lastError().text();
        return false;
    }

    return true;
}

bool DBHelper::setFolderCompleted(qulonglong folderId, bool completed, QSqlDatabase &db)
{
    QSqlQuery updateFolder(db);
    updateFolder.prepare("UPDATE folder SET completed = :completed WHERE id = :id");
    updateFolder.bindValue(":completed", completed ? 1 : 0);
    updateFolder.bindValue(":id", folderId);

    if (!updateFolder.exec()) {
        QLOG_ERROR() << "setFolderCompleted failed for folder" << folderId << ":" << updateFolder.lastError().text();
        return false;
    }

    return true;
}

void DBHelper::updateDBType(QSqlDatabase &db, YACReader::FileType type)
{
    QSqlQuery updateFolderQuery(db);
    updateFolderQuery.prepare("UPDATE folder "
                              "SET type = :type");
    updateFolderQuery.bindValue(":type", static_cast<int>(type));
    updateFolderQuery.exec();

    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info "
                            "SET type = :type");
    updateComicInfo.bindValue(":type", static_cast<int>(type));
    updateComicInfo.exec();
}

std::optional<Folder> DBHelper::loadFolder(qulonglong id, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("SELECT " + TableProjection::selectList("f", Folder::columns()) + " FROM folder f WHERE f.id = :id");
    query.bindValue(":id", id);
    query.exec();

    return DBHelper::readFolderFromQuery(query);
}

std::optional<Folder> DBHelper::loadFolder(const QString &folderName, qulonglong parentId, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("SELECT " + TableProjection::selectList("f", Folder::columns()) + " FROM folder f WHERE f.parentId = :parentId AND f.name = :folderName");
    query.bindValue(":parentId", parentId);
    query.bindValue(":folderName", folderName);
    query.exec();

    return DBHelper::readFolderFromQuery(query);
}

std::optional<Folder> DBHelper::readFolderFromQuery(QSqlQuery &query)
{
    if (query.next()) {
        return Folder::fromRecord(query.record(), "f");
    }

    return std::nullopt;
}

ComicDB DBHelper::loadComic(qulonglong id, QSqlDatabase &db, bool &found)
{
    ComicDB comic;

    static const QString sql = "SELECT " + ComicDB::selectList("c", "ci") + " FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) WHERE c.id = :id";
    QSqlQuery selectQuery(db);
    selectQuery.prepare(sql);
    selectQuery.bindValue(":id", id);
    selectQuery.exec();

    comic.file.id = ComicId(id);
    if (selectQuery.next()) {
        comic = ComicDB::fromRecord(selectQuery.record(), "c", "ci");
        found = true;
    } else {
        found = false;
    }

    return comic;
}

QList<ComicDB> DBHelper::loadComics(const QList<qulonglong> &ids, QSqlDatabase &db)
{
    QList<ComicDB> comics;
    if (ids.isEmpty()) {
        return comics;
    }

    QStringList idList;
    idList.reserve(ids.size());
    for (const auto id : ids) {
        idList.append(QString::number(id));
    }

    QSqlQuery selectQuery(db);
    selectQuery.setForwardOnly(true);
    selectQuery.prepare("SELECT " + ComicDB::selectList("c", "ci") + " FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) WHERE c.id IN (" + idList.join(',') + ")");
    selectQuery.exec();

    QHash<qulonglong, ComicDB> loaded;
    loaded.reserve(ids.size());
    const auto indexes = ComicDB::columnIndexes(selectQuery.record(), "c", "ci");
    while (selectQuery.next()) {
        auto comic = ComicDB::fromQuery(selectQuery, indexes);
        loaded.insert(comic.file.id.value(), comic);
    }

    comics.reserve(ids.size());
    for (const auto id : ids) {
        const auto it = loaded.constFind(id);
        if (it != loaded.constEnd()) {
            comics.append(it.value());
        } else {
            ComicDB missing;
            missing.file.id = ComicId(id);
            comics.append(missing);
        }
    }

    return comics;
}

ComicDB DBHelper::loadComic(QString cname, QString cpath, QString chash, QSqlDatabase &database)
{
    ComicDB comic;

    // comic.parentId = cparentId;
    comic.file.fileName = cname;
    comic.file.path = cpath;

    if (auto info = DBHelper::loadComicInfo(chash, database)) {
        comic.info = *info;
    } else {
        comic.info.hash = chash;
        comic.info.coverPage = 1;
    }

    return comic;
}

std::optional<ComicInfo> DBHelper::loadComicInfo(QString hash, QSqlDatabase &db)
{
    static const QString sql = "SELECT " + TableProjection::selectList("ci", ComicInfo::columns()) + " FROM comic_info ci WHERE ci.hash = :hash";
    QSqlQuery findComicInfo(db);
    findComicInfo.prepare(sql);
    findComicInfo.bindValue(":hash", hash);
    findComicInfo.exec();

    if (findComicInfo.next()) {
        return ComicInfo::fromRecord(findComicInfo.record(), "ci");
    }

    return std::nullopt;
}

std::optional<Placeholder> DBHelper::loadPlaceholder(PlaceholderId id, QSqlDatabase &db)
{
    static const QString sql = "SELECT " + TableProjection::selectList("p", Placeholder::columns()) + " FROM placeholder p WHERE p.id = :id";
    QSqlQuery query(db);
    query.prepare(sql);
    query.bindValue(":id", id.value());
    query.exec();

    if (query.next()) {
        return Placeholder::fromQuery(query, Placeholder::columnIndexes(query.record(), "p"));
    }

    return std::nullopt;
}

std::optional<QList<Placeholder>> DBHelper::loadFolderPlaceholders(FolderId folderId, QSqlDatabase &db)
{
    static const QString sql = "SELECT " + TableProjection::selectList("p", Placeholder::columns()) +
            " FROM folder_placeholder fp INNER JOIN placeholder p ON p.id = fp.placeholder_id WHERE fp.folder_id = :folder_id ORDER BY p.id";
    QSqlQuery query(db);
    query.setForwardOnly(true);
    query.prepare(sql);
    query.bindValue(":folder_id", folderId.value());
    if (!query.exec()) {
        QLOG_ERROR() << "Unable to load the placeholders of a folder:" << query.lastError().text();
        return std::nullopt;
    }

    QList<Placeholder> placeholders;
    const auto columns = Placeholder::columnIndexes(query.record(), "p");
    while (query.next()) {
        placeholders.append(Placeholder::fromQuery(query, columns));
    }
    return placeholders;
}

std::optional<SmartList> DBHelper::loadSmartList(SmartListId id, QSqlDatabase &db)
{
    static const QString sql = "SELECT " + TableProjection::selectList("s", SmartList::columns()) + " FROM smart_list s WHERE s.id = :id";
    QSqlQuery query(db);
    query.prepare(sql);
    query.bindValue(":id", id.value());
    if (query.exec() && query.next())
        return SmartList::fromQuery(query, SmartList::columnIndexes(query.record(), "s"));
    return std::nullopt;
}

std::optional<QList<SmartList>> DBHelper::loadSmartLists(QSqlDatabase &db)
{
    static const QString sql = "SELECT " + TableProjection::selectList("s", SmartList::columns()) + " FROM smart_list s ORDER BY s.ordering, s.id";
    QSqlQuery query(db);
    query.setForwardOnly(true);
    if (!query.exec(sql)) {
        QLOG_ERROR() << "Unable to load the smart lists:" << query.lastError().text();
        return std::nullopt;
    }

    QList<SmartList> lists;
    const auto indexes = SmartList::columnIndexes(query.record(), "s");
    while (query.next())
        lists.append(SmartList::fromQuery(query, indexes));
    return lists;
}

std::optional<QList<ListFolder>> DBHelper::loadListFolders(ListFolderKind kind, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.setForwardOnly(true);
    if (!query.exec("SELECT " + TableProjection::selectList("f", ListFolder::columns()) + QString(" FROM %1 f ORDER BY f.ordering, f.id").arg(folderTable(kind).table))) {
        QLOG_ERROR() << "Unable to load the list folders:" << query.lastError().text();
        return std::nullopt;
    }

    QList<ListFolder> folders;
    const auto indexes = ListFolder::columnIndexes(query.record(), "f");
    while (query.next())
        folders.append(ListFolder::fromQuery(query, indexes, kind));
    return folders;
}

std::optional<QHash<qulonglong, QString>> DBHelper::readingListCoverHashes(const QList<ReadingListId> &ids, QSqlDatabase &db)
{
    QHash<qulonglong, QString> hashes;
    if (ids.isEmpty())
        return hashes;

    QStringList idTexts;
    for (const auto &id : ids)
        idTexts.append(QString::number(id.value()));

    QSqlQuery query(db);
    query.setForwardOnly(true);
    if (!query.exec(QString("SELECT rl.id, (SELECT ci.hash FROM reading_list_entry e "
                            "INNER JOIN comic_info ci ON (ci.id = e.comic_info_id) "
                            "INNER JOIN reading_list l ON (l.id = e.reading_list_id) "
                            "WHERE l.id = rl.id OR l.parentId = rl.id "
                            "ORDER BY l.parentId IS NOT NULL, l.ordering, e.ordering, e.id LIMIT 1) "
                            "FROM reading_list rl WHERE rl.id IN (%1)")
                            .arg(idTexts.join(',')))) {
        QLOG_ERROR() << "Unable to load the covers of reading lists:" << query.lastError().text();
        return std::nullopt;
    }
    while (query.next()) {
        if (!query.value(1).isNull())
            hashes.insert(query.value(0).toULongLong(), query.value(1).toString());
    }
    return hashes;
}

std::optional<ComicInfo> DBHelper::loadComicInfo(ComicInfoId id, QSqlDatabase &db)
{
    static const QString sql = "SELECT " + TableProjection::selectList("ci", ComicInfo::columns()) + " FROM comic_info ci WHERE ci.id = :id";
    QSqlQuery findComicInfo(db);
    findComicInfo.prepare(sql);
    findComicInfo.bindValue(":id", id.value());
    findComicInfo.exec();

    if (findComicInfo.next()) {
        return ComicInfo::fromRecord(findComicInfo.record(), "ci");
    }

    return std::nullopt;
}

std::optional<ComicRecord> DBHelper::loadRecord(const ComicRecord &record, QSqlDatabase &db)
{
    if (const auto *comic = recordComic(record)) {
        bool found = false;
        auto loaded = loadComic(comic->file.id.value(), db, found);
        if (!found || loaded.info.id != comic->info.id)
            return std::nullopt;
        return ComicRecordCase::Comic { std::move(loaded) };
    }

    if (const auto *placeholder = recordPlaceholder(record)) {
        auto loaded = loadPlaceholder(placeholder->id, db);
        if (!loaded)
            return std::nullopt;
        return ComicRecordCase::Placeholder { std::move(*loaded) };
    }

    const auto &deleted = std::get<ComicRecordCase::DeletedComic>(record);
    auto info = loadComicInfo(deleted.info.id, db);
    if (!info)
        return std::nullopt;
    return ComicRecordCase::DeletedComic { std::move(*info), deleted.lastKnownFile };
}

bool DBHelper::isFavoriteComic(qulonglong id, QSqlDatabase &db)
{
    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT * FROM comic_default_reading_list cl WHERE cl.comic_id = :comic_id AND cl.default_reading_list_id = 1");
    selectQuery.bindValue(":comic_id", id);
    selectQuery.exec();

    if (selectQuery.next()) {
        return true;
    }

    return false;
}

QVariantMap DBHelper::getLibraryInfoData(QUuid id)
{
    const QString libraryPath = DBHelper::getLibraries().getPath(id);
    QVariantMap info {
        { QStringLiteral("path"), libraryPath },
    };

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        connectionName = db.connectionName();

        // num folders
        auto foldersQuery = db.exec("SELECT COUNT(*) FROM folder WHERE id <> 1");
        foldersQuery.next();
        info.insert(QStringLiteral("folderCount"), foldersQuery.value(0));

        // num comics
        auto comicsQuery = db.exec("SELECT COUNT(*) FROM comic");
        comicsQuery.next();
        info.insert(QStringLiteral("comicCount"), comicsQuery.value(0));

        // num read comics
        auto readComicsQuery = db.exec("SELECT count(*) FROM comic c INNER JOIN comic_info ci ON c.comicInfoId = ci.id WHERE ci.read = 1");
        readComicsQuery.next();
        info.insert(QStringLiteral("readComicCount"), readComicsQuery.value(0));
    }
    QSqlDatabase::removeDatabase(connectionName);

    return info;
}

QString DBHelper::getLibraryInfo(QUuid id)
{
    const auto libraryInfo = getLibraryInfoData(id);
    QString info = "<b>Library path</b>:<br/>" + libraryInfo.value(QStringLiteral("path")).toString() + "<br/><br/>";
    info += "<b>Number of folders</b>:<br/>" + libraryInfo.value(QStringLiteral("folderCount")).toString() + "<br/><br/>";
    info += "<b>Number of comics</b>:<br/>" + libraryInfo.value(QStringLiteral("comicCount")).toString() + "<br/><br/>";
    info += "<b>Number of read comics</b>:<br/>" + libraryInfo.value(QStringLiteral("readComicCount")).toString() + "<br/><br/>";
    return info;
}
