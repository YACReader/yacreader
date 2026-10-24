#ifndef DB_HELPER_H
#define DB_HELPER_H

class QString;
#include "comic_record.h"
#include "descriptive_comic_metadata.h"
#include "library_item_types.h"
#include "yacreader_global.h"

#include <QList>
#include <QMap>
#include <QStringList>
#include <QUuid>
#include <QVariantMap>

#include <optional>

class ComicDB;
struct Folder;
struct Label;
struct Placeholder;
struct SmartList;
struct ListFolder;
class DescriptiveMetadataPatch;
class QSqlDatabase;
class ComicInfo;
class QSqlRecord;
class QSqlQuery;
class YACReaderLibraries;
struct ReadingList;

// a Comic Vine issue of a volume that becomes a missing comic of a folder
struct MissingIssue {
    QString comicVineId;
    DescriptiveMetadataPatch patch;
    std::optional<QString> coverKey;
};

struct ComicProgressUpdate {
    ComicId comicId;
    ComicInfo info;
};

class DBHelper
{
public:
    // server
    static YACReaderLibraries getLibraries();
    static QList<Folder> getFolderSubfoldersFromLibrary(qulonglong libraryId, qulonglong folderId);
    static QList<ComicDB> getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId);
    static QList<ComicDB> getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId, bool sort);
    static QList<ComicDB> getFolderComicsFromLibraryForReading(qulonglong libraryId, qulonglong folderId);
    static std::optional<ComicDB> getComicInfo(qulonglong libraryId, qulonglong id);
    static QList<ComicDB> getSiblings(qulonglong libraryId, qulonglong parentId);
    static std::optional<Folder> getFolder(qulonglong libraryId, qulonglong id);
    static QString getLibraryName(int id);
    static QList<ComicDB> getLabelComics(qulonglong libraryId, qulonglong labelId);
    static QList<ComicDB> getFavorites(qulonglong libraryId);
    static QList<ComicDB> getReading(qulonglong libraryId);
    static QList<ReadingList> getReadingLists(qulonglong libraryId);
    // the comics of the reading list and of its child lists; entryIds receives the entry of each comic, in the same order
    static QList<ComicDB> getReadingListFullContent(qulonglong libraryId, qulonglong readingListId, QList<ListItemId> *entryIds = nullptr);
    // the available comics of a list with their entries, in list order; binds :list_id
    static const QString &listComicsQuery(ListKind listKind);
    // every entry of a list that refers to a comic_info, with its first file when it has one (c_id is NULL without a file); binds :list_id
    static const QString &listEntriesQuery(ListKind listKind);

    // objects management
    // deletes
    static void removeFromDB(DiskItem &item, QSqlDatabase &db);
    static void removeFromDB(Folder *folder, QSqlDatabase &db);
    static void removeFromDB(ComicDB *comic, QSqlDatabase &db);
    // removals of entries, lists, labels, and folder placements also delete the placeholders that nothing uses any more
    // (no entry and no placement); on success they give the cover keys of those placeholders, and the caller deletes the
    // files after the commit (removePlaceholderCovers)
    static std::optional<QStringList> removeLabelFromDB(qulonglong id, QSqlDatabase &db);
    // also removes the child lists
    static std::optional<QStringList> removeListFromDB(qulonglong id, QSqlDatabase &db);
    static void removePlaceholderCovers(const QString &libraryDataPath, const QStringList &coverKeys);
    // no transaction; for removals that cascade to folder placements (folder deletes)
    static std::optional<QStringList> removeUnusedPlaceholders(QSqlDatabase &db);
    // placing a placeholder again in the same folder changes nothing
    static bool placePlaceholderInFolder(PlaceholderId placeholderId, FolderId folderId, QSqlDatabase &db);
    // No transaction. Inserts each placeholder and places it in the folder.
    static bool insertPlaceholdersInFolder(const QList<Placeholder> &placeholders, FolderId folderId, QSqlDatabase &db);
    static std::optional<QStringList> removePlaceholdersFromFolder(const QList<PlaceholderId> &placeholderIds, FolderId folderId, QSqlDatabase &db);
    // No transaction. Each issue is expected in the folder: the placeholder of that issue is reused, else a new one is
    // made, and the patch is written to it; nothing when a comic of the folder is that issue. A cover key becomes the
    // cover of a placeholder that has none. Gives the cover keys that no placeholder took; nothing when a write fails
    // or an issue has no Comic Vine id.
    static std::optional<QStringList> placeIssuesInFolder(const QList<MissingIssue> &issues, FolderId folderId, QSqlDatabase &db);
    // logic deletes
    static void deleteComicsFromFavorites(const QList<ComicDB> &comicsList, QSqlDatabase &db);
    static void deleteComicsFromReading(const QList<ComicDB> &comicsList, QSqlDatabase &db);
    // entry functions reject an entry id of the other list kind and change nothing
    static std::optional<QStringList> removeLabelEntries(LabelId labelId, const QList<ListItemId> &entryIds, QSqlDatabase &db);
    static std::optional<QStringList> removeReadingListEntries(ReadingListId readingListId, const QList<ListItemId> &entryIds, QSqlDatabase &db);
    static bool reorderLabelEntries(LabelId labelId, const QList<ListItemId> &entryIds, QSqlDatabase &db);
    static bool reorderReadingListEntries(ReadingListId readingListId, const QList<ListItemId> &entryIds, QSqlDatabase &db);
    // a label skips the comic_info it already has; a reading list adds it again
    static bool appendComicsToLabel(LabelId labelId, const QList<ComicInfoId> &comicInfoIds, QSqlDatabase &db);
    static bool appendComicsToReadingList(ReadingListId readingListId, const QList<ComicInfoId> &comicInfoIds, QSqlDatabase &db);
    // appends, in order, what each row refers to (the comic_info of a comic, the comic_info or placeholder of an entry); labels skip repeats
    static bool appendRowsToLabel(LabelId labelId, const QList<ComicRowId> &rows, QSqlDatabase &db);
    static bool appendRowsToReadingList(ReadingListId readingListId, const QList<ComicRowId> &rows, QSqlDatabase &db);
    // inserts
    static qulonglong insert(Folder *folder, QSqlDatabase &db);
    static qulonglong insert(ComicDB *comic, QSqlDatabase &db, bool insertAllInfo);
    static qulonglong insertLabel(const QString &name, YACReader::LabelColors color, QSqlDatabase &db);
    // the placeholder gets a new id; `placeholder.id` is not used
    static std::optional<PlaceholderId> insertPlaceholder(const Placeholder &placeholder, QSqlDatabase &db);
    static qulonglong insertReadingList(const QString &name, QSqlDatabase &db);
    // at the end of the order
    static std::optional<SmartListId> insertSmartList(const QString &name, const QString &query, QSqlDatabase &db);
    static qulonglong insertReadingSubList(const QString &name, qulonglong parentId, int ordering, QSqlDatabase &db);
    static void insertComicsInFavorites(const QList<ComicDB> &comicsList, QSqlDatabase &db);
    // updates
    static void update(qulonglong libraryId, ComicInfo &comicInfo);
    static void update(ComicDB *comics, QSqlDatabase &db);
    static bool update(ComicInfo *comicInfo, QSqlDatabase &db);
    static bool updateAdded(ComicInfo *comicInfo, QSqlDatabase &db);
    // writes only the fields in the patch, and marks the comic_info as edited
    static bool updateComicInfoMetadata(ComicInfoId id, const DescriptiveMetadataPatch &patch, QSqlDatabase &db);
    static bool updatePlaceholderMetadata(PlaceholderId id, const DescriptiveMetadataPatch &patch, QSqlDatabase &db);
    // NULL when the placeholder has no cover; fails when the placeholder is gone
    static bool updatePlaceholderCover(PlaceholderId id, const std::optional<QString> &coverKey, QSqlDatabase &db);
    // a type set by the user, so the comic_info is marked as edited
    static bool updateComicInfoType(ComicInfoId id, YACReader::FileType type, QSqlDatabase &db);
    // writes the cover page, the cover size, and the custom-cover columns of `info`; a cover set by the user, so the
    // comic_info is marked as edited
    static bool updateComicInfoCover(const ComicInfo &info, QSqlDatabase &db);
    static void update(const Folder &folder, QSqlDatabase &db);
    static void propagateFolderUpdatesToParent(const Folder &folder, QSqlDatabase &db);
    static Folder updateChildrenInfo(qulonglong folderId, QSqlDatabase &db);
    static void updateChildrenInfo(QSqlDatabase &db);
    static void updateProgress(qulonglong libraryId, ComicId comicId, const ComicInfo &comicInfo);
    static void updateImageFilters(qulonglong libraryId, ComicId comicId, const ComicInfo &comicInfo);
    // targeted updates, they only write the columns that change; unread matches ComicModel::setComicsRead(Unread)
    static bool setComicRead(qulonglong comicId, bool read, QSqlDatabase &db);
    static bool setComicType(qulonglong comicId, YACReader::FileType type, QSqlDatabase &db);
    static bool setFolderFinished(qulonglong folderId, bool finished, QSqlDatabase &db);
    static bool setFolderCompleted(qulonglong folderId, bool completed, QSqlDatabase &db);
    static void setComicAsReading(qulonglong libraryId, ComicId comicId);
    static void updateReadingRemoteProgress(const ComicInfo &comicInfo, QSqlDatabase &db);
    static QMap<qulonglong, QList<ComicDB>> updateFromRemoteClient(const QMap<qulonglong, QList<ComicProgressUpdate>> &comics, bool clientSendsHasBeenOpened, bool clientSendsImageFilters);
    // returns the libraries where at least one comic was updated
    static QList<QUuid> updateFromRemoteClientWithHash(const QList<ComicInfo> &comics);
    static void renameLabel(qulonglong id, const QString &name, QSqlDatabase &db);
    // fails when the smart list is gone
    static bool updateSmartList(SmartListId id, const QString &name, const QString &query, QSqlDatabase &db);
    static bool removeSmartList(SmartListId id, QSqlDatabase &db);

    // list folders: one function for both kinds; a folder only holds lists and folders of its own kind
    // at the end of the folders of its parent; nothing as parent is the top level
    static std::optional<ListFolderId> insertListFolder(ListFolderKind kind, const std::optional<qulonglong> &parentId, const QString &name, QSqlDatabase &db);
    static bool renameListFolder(const ListFolderId &id, const QString &name, QSqlDatabase &db);
    // refuses the folder itself and the folders below it as the new parent
    static bool moveListFolder(const ListFolderId &id, const std::optional<qulonglong> &parentId, QSqlDatabase &db);
    // a child reading list is never in a folder
    static bool moveReadingListToFolder(ReadingListId id, const std::optional<qulonglong> &folderId, QSqlDatabase &db);
    static bool moveSmartListToFolder(SmartListId id, const std::optional<qulonglong> &folderId, QSqlDatabase &db);
    // with everything in it; on success the cover keys of the placeholders that nothing uses any more
    static std::optional<QStringList> removeListFolder(const ListFolderId &id, QSqlDatabase &db);
    static void renameList(qulonglong id, const QString &name, QSqlDatabase &db);
    static bool renameFolder(qulonglong id, const QString &name, const QString &oldPath, const QString &newPath, QSqlDatabase &db, QString *error = nullptr);
    static bool moveComic(qulonglong comicId, qulonglong newParentId, const QString &newFileName, const QString &newRelativePath, QSqlDatabase &db);
    static qulonglong ensureFolderPath(const QString &relativePath, QSqlDatabase &db, QList<qulonglong> *createdFolderIds = nullptr);
    static void syncFolderAddedFromContents(const QList<qulonglong> &folderIds, QSqlDatabase &db);
    static void removeEmptyFolderPaths(const QStringList &relativePaths, QSqlDatabase &db, QList<QVariantMap> *removedRows = nullptr);
    static bool restoreFolderRows(const QList<QVariantMap> &rows, QSqlDatabase &db);
    static void removeEmptyFolderRows(const QList<qulonglong> &folderIds, QSqlDatabase &db);
    static void reasignOrderToSublists(QList<qulonglong> ids, QSqlDatabase &db);
    static void reasignOrderToComicsInFavorites(QList<qulonglong> comicIds, QSqlDatabase &db);
    // returns the hashes of the removed rows, so their covers can be removed after the commit
    static QStringList removeUnusedComicInfo(QSqlDatabase &db);
    // no transaction, the scanner calls it in its own; entries stay while another file still uses the old comic_info
    static bool moveEntriesToComicInfo(ComicInfoId oldComicInfoId, ComicInfoId newComicInfoId, QSqlDatabase &db);

    static QList<Folder> getFoldersFromParent(qulonglong parentId, QSqlDatabase &db, bool sort = true);
    // in the order of the folders tree
    static std::optional<QList<Folder>> loadSubfolders(FolderId parentId, QSqlDatabase &db);
    static QList<ComicDB> getSortedComicsFromParent(qulonglong parentId, QSqlDatabase &db);
    static QList<ComicDB> getComicsFromParent(qulonglong parentId, QSqlDatabase &db, bool sort = true);
    static QList<Label> getLabels(qulonglong libraryId);

    static bool updateFolderTreeType(qulonglong id, QSqlDatabase &db, YACReader::FileType type);
    static void updateDBType(QSqlDatabase &db, YACReader::FileType type);

    // load
    static std::optional<Folder> loadFolder(qulonglong id, QSqlDatabase &db);
    static std::optional<Folder> loadFolder(const QString &folderName, qulonglong parentId, QSqlDatabase &db);
    // reads the next row of a query that selects the Folder projection with alias "f"; for one row, loops use Folder::fromQuery
    static std::optional<Folder> readFolderFromQuery(QSqlQuery &query);
    static ComicDB loadComic(qulonglong id, QSqlDatabase &db, bool &found);
    // in the order of `ids`; a missing comic has only its id, as in loadComic
    static QList<ComicDB> loadComics(const QList<qulonglong> &ids, QSqlDatabase &db);
    static ComicDB loadComic(QString cname, QString cpath, QString chash, QSqlDatabase &database);
    static std::optional<ComicInfo> loadComicInfo(QString hash, QSqlDatabase &db);
    static std::optional<ComicInfo> loadComicInfo(ComicInfoId id, QSqlDatabase &db);
    static std::optional<Placeholder> loadPlaceholder(PlaceholderId id, QSqlDatabase &db);
    // nothing if the query fails
    static std::optional<QList<Placeholder>> loadFolderPlaceholders(FolderId folderId, QSqlDatabase &db);
    static std::optional<SmartList> loadSmartList(SmartListId id, QSqlDatabase &db);
    // in their order; nothing if the query fails
    static std::optional<QList<SmartList>> loadSmartLists(QSqlDatabase &db);
    // in their order (ordering, then id); nothing if the query fails
    static std::optional<QList<ListFolder>> loadListFolders(ListFolderKind kind, QSqlDatabase &db);
    // the hash of the first comic of each list, as its view shows it (its own entries, then its child lists); a list
    // without comics has none; nothing if the query fails
    static std::optional<QHash<qulonglong, QString>> readingListCoverHashes(const QList<ReadingListId> &ids, QSqlDatabase &db);
    // queries
    static bool isFavoriteComic(qulonglong id, QSqlDatabase &db);
    // the record as it is now; nothing when a comic with a file lost that file or the file has a different comic_info,
    // or when the comic_info of a deleted comic no longer exists
    static std::optional<ComicRecord> loadRecord(const ComicRecord &record, QSqlDatabase &db);

    // library
    static QVariantMap getLibraryInfoData(QUuid id);
    static QString getLibraryInfo(QUuid id);
};

#endif
