#ifndef DB_HELPER_H
#define DB_HELPER_H

class QString;
class LibraryItem;
class ComicDB;
#include <QMap>
#include <QList>

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
};

#endif