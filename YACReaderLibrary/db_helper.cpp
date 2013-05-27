#include "db_helper.h"

#include <QMap>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QTextStream>
#include <QSqlDatabase>

#include "library_item.h"
#include "comic_db.h"
#include "data_base_management.h"
#include "folder.h"

//server

//TODO optimizar, evitar que se tenga que leer en cada petición el archivo
//conservar un QDateTime stático que compruebe si libraries.yacr ha sido modificado
//libraries debe ser una variable estática
static QDateTime lastModified;
static QMap<QString,QString> libraries;

QMap<QString,QString> DBHelper::getLibraries()
{
	QFileInfo fi(QCoreApplication::applicationDirPath()+"/libraries.yacr");
	if(fi.lastModified() == lastModified)
		return libraries;
	
	lastModified = fi.lastModified();
	libraries.clear();
	QFile f(QCoreApplication::applicationDirPath()+"/libraries.yacr");
	f.open(QIODevice::ReadOnly);
	QTextStream txtS(&f);
	QString content = txtS.readAll();
	QStringList lines = content.split('\n');
	QString line,name;
	int i=0;
	foreach(line,lines)
	{
		if((i%2)==0)
		{
			name = line;
		}
		else
		{
			//sólo se agregan las bibliotecas realmente disponibles
			QSqlDatabase db = DataBaseManagement::loadDatabase(line.trimmed()+"/.yacreaderlibrary");
			if(db.isValid())
				libraries.insert(name.trimmed(),line.trimmed());
			db.close();
		}
		i++;
	}
	f.close();
	return libraries;
}
QList<LibraryItem *> DBHelper::getFolderContentFromLibrary(const QString & libraryName, qulonglong folderId)
{
	QString libraryPath = DBHelper::getLibraries().value(libraryName);
	QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");
	
	QList<LibraryItem *> list = Folder::getFoldersFromParent(folderId,db,false);
	
	db.close();
	QSqlDatabase::removeDatabase(libraryPath);
	return list;
}
QList<LibraryItem *> DBHelper::getFolderComicsFromLibrary(const QString & libraryName, qulonglong folderId)
{
	QString libraryPath = DBHelper::getLibraries().value(libraryName);
	QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");

	QList<LibraryItem *> list = ComicDB::getComicsFromParent(folderId,db,false);

	db.close();
	QSqlDatabase::removeDatabase(libraryPath);
	return list;
}
qulonglong DBHelper::getParentFromComicFolderId(const QString & libraryName, qulonglong id)
{
	QString libraryPath = DBHelper::getLibraries().value(libraryName);
	QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");

	Folder f(id,db);

	db.close();
	QSqlDatabase::removeDatabase(libraryPath);
	return f.parentId;
}
ComicDB DBHelper::getComicInfo(const QString & libraryName, qulonglong id)
{
	QString libraryPath = DBHelper::getLibraries().value(libraryName);
	QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");

	ComicDB comic;
	comic.load(id,db);

	db.close();
	QSqlDatabase::removeDatabase(libraryPath);
	return comic;
}
QString DBHelper::getFolderName(const QString & libraryName, qulonglong id)
{
	QString libraryPath = DBHelper::getLibraries().value(libraryName);
	QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");

	QString name="";

	{
		QSqlQuery selectQuery(db); //TODO check
		selectQuery.prepare("SELECT name FROM folder WHERE id = :id");
		selectQuery.bindValue(":id", id);
		selectQuery.exec();

		if(selectQuery.next()) 
		{
			QSqlRecord record = selectQuery.record();
			name = record.value(0).toString();
		}
	}

	db.close();
	QSqlDatabase::removeDatabase(libraryPath);
	return name;
}

//objects management
void DBHelper::removeFromDB(LibraryItem * item, QSqlDatabase & db)
{
	if(item->isDir())
		DBHelper::removeFromDB(dynamic_cast<Folder *>(item),db);
	else
		DBHelper::removeFromDB(dynamic_cast<ComicDB *>(item),db);
}
void DBHelper::removeFromDB(Folder * folder, QSqlDatabase & db)
{
	QSqlQuery query(db);
	query.prepare("DELETE FROM folder WHERE id = :id");
    query.bindValue(":id", folder->id);
	query.exec();
}
void DBHelper::removeFromDB(ComicDB * comic, QSqlDatabase & db)
{
	QSqlQuery query(db);
	query.prepare("DELETE FROM comic WHERE id = :id");
    query.bindValue(":id", comic->id);
	query.exec();
}