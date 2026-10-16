#ifndef DB_HELPER_H
#define DB_HELPER_H

class QString;
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
class QSqlDatabase;
class ComicInfo;
class QSqlRecord;
class QSqlQuery;
class YACReaderLibraries;
struct ReadingList;

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
    static QList<ComicDB> getReadingListFullContent(qulonglong libraryId, qulonglong readingListId);
    // the available comics of a list with their entries, in list order; binds :list_id
    static const QString &listComicsQuery(ListKind listKind);

    // objects management
    // deletes
    static void removeFromDB(DiskItem &item, QSqlDatabase &db);
    static void removeFromDB(Folder *folder, QSqlDatabase &db);
    static void removeFromDB(ComicDB *comic, QSqlDatabase &db);
    static void removeLabelFromDB(qulonglong id, QSqlDatabase &db);
    static void removeListFromDB(qulonglong id, QSqlDatabase &db);
    // logic deletes
    static void deleteComicsFromFavorites(const QList<ComicDB> &comicsList, QSqlDatabase &db);
    static void deleteComicsFromReading(const QList<ComicDB> &comicsList, QSqlDatabase &db);
    // entry functions reject an entry id of the other list kind and change nothing
    static bool removeLabelEntries(LabelId labelId, const QList<ListItemId> &entryIds, QSqlDatabase &db);
    static bool removeReadingListEntries(ReadingListId readingListId, const QList<ListItemId> &entryIds, QSqlDatabase &db);
    static bool reorderLabelEntries(LabelId labelId, const QList<ListItemId> &entryIds, QSqlDatabase &db);
    static bool reorderReadingListEntries(ReadingListId readingListId, const QList<ListItemId> &entryIds, QSqlDatabase &db);
    // a label skips the comic_info it already has; a reading list adds it again
    static bool appendComicsToLabel(LabelId labelId, const QList<ComicInfoId> &comicInfoIds, QSqlDatabase &db);
    static bool appendComicsToReadingList(ReadingListId readingListId, const QList<ComicInfoId> &comicInfoIds, QSqlDatabase &db);
    // inserts
    static qulonglong insert(Folder *folder, QSqlDatabase &db);
    static qulonglong insert(ComicDB *comic, QSqlDatabase &db, bool insertAllInfo);
    static qulonglong insertLabel(const QString &name, YACReader::LabelColors color, QSqlDatabase &db);
    static qulonglong insertReadingList(const QString &name, QSqlDatabase &db);
    static qulonglong insertReadingSubList(const QString &name, qulonglong parentId, int ordering, QSqlDatabase &db);
    static void insertComicsInFavorites(const QList<ComicDB> &comicsList, QSqlDatabase &db);
    // updates
    static void update(qulonglong libraryId, ComicInfo &comicInfo);
    static void update(ComicDB *comics, QSqlDatabase &db);
    static bool update(ComicInfo *comicInfo, QSqlDatabase &db);
    static bool updateAdded(ComicInfo *comicInfo, QSqlDatabase &db);
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
    static void updateComicsInfo(QList<ComicDB> &comics, const QString &databasePath);
    // returns the hashes of the removed rows, so their covers can be removed after the commit
    static QStringList removeUnusedComicInfo(QSqlDatabase &db);
    // no transaction, the scanner calls it in its own; entries stay while another file still uses the old comic_info
    static bool moveEntriesToComicInfo(ComicInfoId oldComicInfoId, ComicInfoId newComicInfoId, QSqlDatabase &db);

    static QList<Folder> getFoldersFromParent(qulonglong parentId, QSqlDatabase &db, bool sort = true);
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
    // queries
    static bool isFavoriteComic(qulonglong id, QSqlDatabase &db);

    // library
    static QVariantMap getLibraryInfoData(QUuid id);
    static QString getLibraryInfo(QUuid id);
};

#endif
