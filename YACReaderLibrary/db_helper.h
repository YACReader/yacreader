#ifndef DB_HELPER_H
#define DB_HELPER_H

class QString;
#include <QMap>
#include <QList>

class ComicDB;
class Folder;
class LibraryItem;
class QSqlDatabase;

class DBHelper
{
public:
	//server
	static	QMap<QString,QString> getLibraries();
	static	QList<LibraryItem *> getFolderContentFromLibrary(const QString & libraryName, qulonglong folderId);
	static	QList<LibraryItem *> getFolderComicsFromLibrary(const QString & libraryName, qulonglong folderId);
	static	qulonglong getParentFromComicFolderId(const QString & libraryName, qulonglong id);
	static	ComicDB getComicInfo(const QString & libraryName, qulonglong id);
	static	QString getFolderName(const QString & libraryName, qulonglong id);

	//objects management
	static void removeFromDB(LibraryItem * item, QSqlDatabase & db);
	static void removeFromDB(Folder * folder, QSqlDatabase & db);
	static void removeFromDB(ComicDB * comic, QSqlDatabase & db);
};

#endif