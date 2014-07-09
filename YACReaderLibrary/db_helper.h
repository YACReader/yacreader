#ifndef DB_HELPER_H
#define DB_HELPER_H

class QString;
#include <QMap>
#include <QList>

class ComicDB;
class Folder;
class LibraryItem;
class QSqlDatabase;
class ComicInfo;
class QSqlRecord;
class QSqlQuery;
class YACReaderLibraries;

class DBHelper
{
public:
	//server
	static	YACReaderLibraries getLibraries();
	static	QList<LibraryItem *> getFolderContentFromLibrary(const QString & libraryName, qulonglong folderId);
	static	QList<LibraryItem *> getFolderComicsFromLibrary(const QString & libraryName, qulonglong folderId);
	static	qulonglong getParentFromComicFolderId(const QString & libraryName, qulonglong id);
	static	ComicDB getComicInfo(const QString & libraryName, qulonglong id);
	static  QList<ComicDB> getSiblings(const QString & libraryName, qulonglong parentId);
	static	QString getFolderName(const QString & libraryName, qulonglong id);
	static  QList<QString> getLibrariesNames();
	static  QString getLibraryName(int id);

	//objects management
	//deletes
	static void removeFromDB(LibraryItem * item, QSqlDatabase & db);
	static void removeFromDB(Folder * folder, QSqlDatabase & db);
	static void removeFromDB(ComicDB * comic, QSqlDatabase & db);
	//inserts
	static qulonglong insert(Folder * folder, QSqlDatabase & db);
	static qulonglong insert(ComicDB * comic, QSqlDatabase & db);
	//updates
	static void update(const QString & libraryName, ComicInfo & comicInfo);
	static void update(ComicDB * comics, QSqlDatabase & db);
	static void update(ComicInfo * comicInfo, QSqlDatabase & db);
	static void updateRead(ComicInfo * comicInfo, QSqlDatabase & db);
    static void update(const Folder & folder, QSqlDatabase & db);
	//queries
	static QList<LibraryItem *> getFoldersFromParent(qulonglong parentId, QSqlDatabase & db, bool sort = true);
	static QList<ComicDB> getSortedComicsFromParent(qulonglong parentId, QSqlDatabase & db);
	static QList<LibraryItem *> getComicsFromParent(qulonglong parentId, QSqlDatabase & db, bool sort = true);
	//load
	static Folder loadFolder(qulonglong id, QSqlDatabase & db);
	static ComicDB loadComic(qulonglong id, QSqlDatabase & db);
    static ComicDB loadComic(QString cname, QString cpath, QString chash, QSqlDatabase & database);
	static ComicInfo loadComicInfo(QString hash, QSqlDatabase & db);
    static QList<QString> loadSubfoldersNames(qulonglong folderId, QSqlDatabase & db);
};

#endif
