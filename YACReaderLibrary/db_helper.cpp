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
	
	QList<LibraryItem *> list = DBHelper::getFoldersFromParent(folderId,db,false);
	
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

	Folder f = DBHelper::loadFolder(id,db);

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
//deletes
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

//inserts
qulonglong DBHelper::insert(Folder * folder, QSqlDatabase & db)
{
	QSqlQuery query(db);
	query.prepare("INSERT INTO folder (parentId, name, path) "
                   "VALUES (:parentId, :name, :path)");
    query.bindValue(":parentId", folder->parentId);
    query.bindValue(":name", folder->name);
	query.bindValue(":path", folder->path);
	query.exec();
	return query.lastInsertId().toULongLong();
}

//queries
QList<LibraryItem *> DBHelper::getFoldersFromParent(qulonglong parentId, QSqlDatabase & db, bool sort)
{
	QList<LibraryItem *> list;

	QSqlQuery selectQuery(db); //TODO check
	selectQuery.prepare("SELECT * FROM folder WHERE parentId = :parentId and id <> 1");
	selectQuery.bindValue(":parentId", parentId);
	selectQuery.exec();

	Folder * currentItem;
	while (selectQuery.next()) 
	{
		QList<QVariant> data;
		QSqlRecord record = selectQuery.record();
		for(int i=0;i<record.count();i++)
			data << record.value(i);
		//TODO sort by sort indicator and name
		currentItem = new Folder(record.value("id").toULongLong(),record.value("parentId").toULongLong(),record.value("name").toString(),record.value("path").toString());
		int lessThan = 0;

		if(list.isEmpty() || !sort)
			list.append(currentItem);
		else
		{
			Folder * last = static_cast<Folder *>(list.back());
			QString nameLast = last->name; 
			QString nameCurrent = currentItem->name;
			QList<LibraryItem *>::iterator i;
			i = list.end();
			i--;
			while ((0 > (lessThan = nameCurrent.localeAwareCompare(nameLast))) && i != list.begin())
			{
				i--;
				nameLast = (*i)->name;
			}
			if(lessThan>0) //si se ha encontrado un elemento menor que current, se inserta justo después
				list.insert(++i,currentItem);
			else
				list.insert(i,currentItem);

		}
	}

	return list;
}

//loads
Folder DBHelper::loadFolder(qulonglong id, QSqlDatabase & db)
{
	Folder folder;

	QSqlQuery query(db);
	query.prepare("SELECT * FROM folder WHERE id = :id");
	query.bindValue(":id",id);
	query.exec();
	folder.id = id;
	folder.parentId = 0;
	if(query.next())
	{
		QSqlRecord record = query.record();
		folder.parentId = record.value("parentId").toULongLong();
		folder.name = record.value("name").toString();
		folder.path = record.value("path").toString();
	}

	return folder;
}