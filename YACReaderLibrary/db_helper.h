#ifndef DB_HELPER_H
#define DB_HELPER_H

class QString;
#include <QMap>
#include <QList>
#include "yacreader_global.h"

class ComicDB;
class Folder;
class LibraryItem;
class Label;
class QSqlDatabase;
class ComicInfo;
class QSqlRecord;
class QSqlQuery;
class YACReaderLibraries;
class ReadingList;

class DBHelper
{
public:
    //server
    static YACReaderLibraries getLibraries();
    static QList<LibraryItem *> getFolderSubfoldersFromLibrary(qulonglong libraryId, qulonglong folderId);
    static QList<LibraryItem *> getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId);
    static QList<LibraryItem *> getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId, bool sort);
    static quint32 getNumChildrenFromFolder(qulonglong libraryId, qulonglong folderId);
    static qulonglong getParentFromComicFolderId(qulonglong libraryId, qulonglong id);
    static ComicDB getComicInfo(qulonglong libraryId, qulonglong id);
    static QList<ComicDB> getSiblings(qulonglong libraryId, qulonglong parentId);
    static QString getFolderName(qulonglong libraryId, qulonglong id);
    static QList<QString> getLibrariesNames();
    static QString getLibraryName(int id);
    static QList<ComicDB> getLabelComics(qulonglong libraryId, qulonglong labelId);
    static QList<ComicDB> getFavorites(qulonglong libraryId);
    static QList<ComicDB> getReading(qulonglong libraryId);
    static QList<ReadingList> getReadingLists(qulonglong libraryId);
    static QList<ComicDB> getReadingListFullContent(qulonglong libraryId, qulonglong readingListId);

    //objects management
    //deletes
    static void removeFromDB(LibraryItem *item, QSqlDatabase &db);
    static void removeFromDB(Folder *folder, QSqlDatabase &db);
    static void removeFromDB(ComicDB *comic, QSqlDatabase &db);
    static void removeLabelFromDB(qulonglong id, QSqlDatabase &db);
    static void removeListFromDB(qulonglong id, QSqlDatabase &db);
    //logic deletes
    static void deleteComicsFromFavorites(const QList<ComicDB> &comicsList, QSqlDatabase &db);
    static void deleteComicsFromLabel(const QList<ComicDB> &comicsList, qulonglong labelId, QSqlDatabase &db);
    static void deleteComicsFromReadingList(const QList<ComicDB> &comicsList, qulonglong readingListId, QSqlDatabase &db);
    //inserts
    static qulonglong insert(Folder *folder, QSqlDatabase &db);
    static qulonglong insert(ComicDB *comic, QSqlDatabase &db);
    static qulonglong insertLabel(const QString &name, YACReader::LabelColors color, QSqlDatabase &db);
    static qulonglong insertReadingList(const QString &name, QSqlDatabase &db);
    static qulonglong insertReadingSubList(const QString &name, qulonglong parentId, int ordering, QSqlDatabase &db);
    static void insertComicsInFavorites(const QList<ComicDB> &comicsList, QSqlDatabase &db);
    static void insertComicsInLabel(const QList<ComicDB> &comicsList, qulonglong labelId, QSqlDatabase &db);
    static void insertComicsInReadingList(const QList<ComicDB> &comicsList, qulonglong readingListId, QSqlDatabase &db);
    //updates
    static void update(qulonglong libraryId, ComicInfo &comicInfo);
    static void update(ComicDB *comics, QSqlDatabase &db);
    static void update(ComicInfo *comicInfo, QSqlDatabase &db);
    static void updateRead(ComicInfo *comicInfo, QSqlDatabase &db);
    static void update(const Folder &folder, QSqlDatabase &db);
    static void updateChildrenInfo(const Folder &folder, QSqlDatabase &db);
    static void updateChildrenInfo(qulonglong folderId, QSqlDatabase &db);
    static void updateChildrenInfo(QSqlDatabase &db);
    static void updateProgress(qulonglong libraryId, const ComicInfo &comicInfo);
    static void setComicAsReading(qulonglong libraryId, const ComicInfo &comicInfo);
    static void updateFromRemoteClient(qulonglong libraryId, const ComicInfo &comicInfo);
    static void updateFromRemoteClientWithHash(const ComicInfo &comicInfo);
    static void updateReadingRemoteProgress(const ComicInfo &comicInfo, QSqlDatabase &db);
    static QMap<qulonglong, QList<ComicDB>> updateFromRemoteClient(const QMap<qulonglong, QList<ComicInfo>> &comics);
    static void updateFromRemoteClientWithHash(const QList<ComicInfo> &comics);
    static void renameLabel(qulonglong id, const QString &name, QSqlDatabase &db);
    static void renameList(qulonglong id, const QString &name, QSqlDatabase &db);
    static void reasignOrderToSublists(QList<qulonglong> ids, QSqlDatabase &db);
    static void reasignOrderToComicsInFavorites(QList<qulonglong> comicIds, QSqlDatabase &db);
    static void reasignOrderToComicsInLabel(qulonglong labelId, QList<qulonglong> comicIds, QSqlDatabase &db);
    static void reasignOrderToComicsInReadingList(qulonglong readingListId, QList<qulonglong> comicIds, QSqlDatabase &db);

    static QList<LibraryItem *> getFoldersFromParent(qulonglong parentId, QSqlDatabase &db, bool sort = true);
    static QList<ComicDB> getSortedComicsFromParent(qulonglong parentId, QSqlDatabase &db);
    static QList<LibraryItem *> getComicsFromParent(qulonglong parentId, QSqlDatabase &db, bool sort = true);
    static QList<Label> getLabels(qulonglong libraryId);

    //load
    static Folder loadFolder(qulonglong id, QSqlDatabase &db);
    static Folder loadFolder(const QString &folderName, qulonglong parentId, QSqlDatabase &db);
    static ComicDB loadComic(qulonglong id, QSqlDatabase &db);
    static ComicDB loadComic(QString cname, QString cpath, QString chash, QSqlDatabase &database);
    static ComicInfo loadComicInfo(QString hash, QSqlDatabase &db);
    static QList<QString> loadSubfoldersNames(qulonglong folderId, QSqlDatabase &db);
    //queries
    static bool isFavoriteComic(qulonglong id, QSqlDatabase &db);
};

#endif
