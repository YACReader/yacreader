#ifndef DATABASE_HELPER_H
#define DATABASE_HELPER_H

#include "comic_db.h"
#include "db_helper.h"
#include "folder.h"

#include <QList>
#include <QSqlDatabase>
#include <QString>

#include <utility>

namespace YACReader {

//! A RAII class that adds or adopts a QSqlDatabase connection in its constructors
//! and removes this connection via QSqlDatabase::removeDatabase() in the destructor.
class DatabaseHolder
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    Q_DISABLE_COPY_MOVE(DatabaseHolder)
#else
    Q_DISABLE_COPY(DatabaseHolder)
#endif
public:
    explicit DatabaseHolder(const QString &path);
    explicit DatabaseHolder(const QSqlDatabase &db);

    QSqlDatabase &operator*() { return db; }
    QSqlDatabase *operator->() { return &db; }

private:
    class Remover
    {
    public:
        void setDb(const QSqlDatabase &db);
        ~Remover();

    private:
        QString connectionName;
    };

    // The order of the data members is important: remover must be destroyed after
    // db to avoid the Qt warning in log - "QSqlDatabasePrivate::removeDatabase:
    // connection '<name>' is still in use, all queries will cease to work."
    Remover remover;
    QSqlDatabase db;
};

//! A wrapper around DatabaseHolder that makes access to the managed QSqlDatabase
//! object a bit more verbose, but provides convenience forwarding functions for
//! all DBHelper's member functions that take a QSqlDatabase & argument.
class DatabaseHelper
{
public:
    //! @note Access to DatabaseHolder(const QSqlDatabase &db) is intentionally
    //! restricted to prevent its use by mistake. The QString constructor
    //! overload is much more common and should be used most of the time. Use
    //! DatabaseHolder directly if you need to load a database differently.
    explicit DatabaseHelper(const QString &path)
        : holder(path) { }

    QSqlDatabase &db() { return *holder; }

    // All public member functions below simply forward their arguments and db() to DBHelper.

    //objects management
    //deletes
    void removeFromDB(LibraryItem *item) { DBHelper::removeFromDB(item, db()); }
    void removeFromDB(Folder *folder) { DBHelper::removeFromDB(folder, db()); }
    void removeFromDB(ComicDB *comic) { DBHelper::removeFromDB(comic, db()); }
    void removeLabelFromDB(qulonglong id) { DBHelper::removeLabelFromDB(id, db()); }
    void removeListFromDB(qulonglong id) { DBHelper::removeListFromDB(id, db()); }
    //logic deletes
    void deleteComicsFromFavorites(const QList<ComicDB> &comicsList) { DBHelper::deleteComicsFromFavorites(comicsList, db()); }
    void deleteComicsFromLabel(const QList<ComicDB> &comicsList, qulonglong labelId) { DBHelper::deleteComicsFromLabel(comicsList, labelId, db()); }
    void deleteComicsFromReadingList(const QList<ComicDB> &comicsList, qulonglong readingListId) { DBHelper::deleteComicsFromReadingList(comicsList, readingListId, db()); }
    //inserts
    qulonglong insert(Folder *folder) { return DBHelper::insert(folder, db()); }
    qulonglong insert(ComicDB *comic) { return DBHelper::insert(comic, db()); }
    qulonglong insertLabel(const QString &name, YACReader::LabelColors color) { return DBHelper::insertLabel(name, color, db()); }
    qulonglong insertReadingList(const QString &name) { return DBHelper::insertReadingList(name, db()); }
    qulonglong insertReadingSubList(const QString &name, qulonglong parentId, int ordering) { return DBHelper::insertReadingSubList(name, parentId, ordering, db()); }
    void insertComicsInFavorites(const QList<ComicDB> &comicsList) { DBHelper::insertComicsInFavorites(comicsList, db()); }
    void insertComicsInLabel(const QList<ComicDB> &comicsList, qulonglong labelId) { DBHelper::insertComicsInLabel(comicsList, labelId, db()); }
    void insertComicsInReadingList(const QList<ComicDB> &comicsList, qulonglong readingListId) { DBHelper::insertComicsInReadingList(comicsList, readingListId, db()); }
    //updates
    void update(ComicDB *comics) { DBHelper::update(comics, db()); }
    void update(ComicInfo *comicInfo) { DBHelper::update(comicInfo, db()); }
    void updateRead(ComicInfo *comicInfo) { DBHelper::updateRead(comicInfo, db()); }
    void update(const Folder &folder) { DBHelper::update(folder, db()); }
    void updateChildrenInfo(const Folder &folder) { DBHelper::updateChildrenInfo(folder, db()); }
    void updateChildrenInfo(qulonglong folderId) { DBHelper::updateChildrenInfo(folderId, db()); }
    void updateChildrenInfo() { DBHelper::updateChildrenInfo(db()); }
    void updateReadingRemoteProgress(const ComicInfo &comicInfo) { DBHelper::updateReadingRemoteProgress(comicInfo, db()); }
    void renameLabel(qulonglong id, const QString &name) { DBHelper::renameLabel(id, name, db()); }
    void renameList(qulonglong id, const QString &name) { DBHelper::renameList(id, name, db()); }
    void reassignOrderToSublists(const QList<qulonglong> &ids) { DBHelper::reassignOrderToSublists(ids, db()); }
    void reassignOrderToComicsInFavorites(const QList<qulonglong> &comicIds) { DBHelper::reassignOrderToComicsInFavorites(comicIds, db()); }
    void reassignOrderToComicsInLabel(qulonglong labelId, const QList<qulonglong> &comicIds) { DBHelper::reassignOrderToComicsInLabel(labelId, comicIds, db()); }
    void reassignOrderToComicsInReadingList(qulonglong readingListId, const QList<qulonglong> &comicIds) { DBHelper::reassignOrderToComicsInReadingList(readingListId, comicIds, db()); }

    QList<LibraryItem *> getFoldersFromParent(qulonglong parentId, bool sort = true) { return DBHelper::getFoldersFromParent(parentId, db(), sort); }
    QList<ComicDB> getSortedComicsFromParent(qulonglong parentId) { return DBHelper::getSortedComicsFromParent(parentId, db()); }
    QList<LibraryItem *> getComicsFromParent(qulonglong parentId, bool sort = true) { return DBHelper::getComicsFromParent(parentId, db(), sort); }

    void updateFolderTreeManga(qulonglong id, bool manga) { DBHelper::updateFolderTreeManga(id, manga, db()); }

    //load
    Folder loadFolder(qulonglong id) { return DBHelper::loadFolder(id, db()); }
    Folder loadFolder(const QString &folderName, qulonglong parentId) { return DBHelper::loadFolder(folderName, parentId, db()); }
    ComicDB loadComic(qulonglong id) { return DBHelper::loadComic(id, db()); }
    ComicDB loadComic(QString cname, QString cpath, const QString &chash) { return DBHelper::loadComic(std::move(cname), std::move(cpath), chash, db()); }
    ComicInfo loadComicInfo(const QString &hash) { return DBHelper::loadComicInfo(hash, db()); }
    QList<QString> loadSubfoldersNames(qulonglong folderId) { return DBHelper::loadSubfoldersNames(folderId, db()); }
    //queries
    bool isFavoriteComic(qulonglong id) { return DBHelper::isFavoriteComic(id, db()); }

private:
    DatabaseHolder holder;
};
} // namespace YACReader

#endif // DATABASE_HELPER_H
