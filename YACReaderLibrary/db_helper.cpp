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
#include <QSqlRecord>
#include <QSqlQuery>

#include "library_item.h"
#include "comic_db.h"
#include "data_base_management.h"
#include "folder.h"

#include "qnaturalsorting.h"

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

	QList<LibraryItem *> list = DBHelper::getComicsFromParent(folderId,db,false);

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

	ComicDB comic = DBHelper::loadComic(id,db);

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
QList<QString> DBHelper::getLibrariesNames()
{
	QStringList names = getLibraries().keys();
	qSort(names.begin(),names.end(),naturalSortLessThanCI);
	return names;
}
QString DBHelper::getLibraryName(int id)
{
	QStringList names = getLibrariesNames();
	if(names.isEmpty())
		return "";
	if(id>=0 && id<names.count())
		return names.at(id);
	else
		return names.at(0);
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

//updates
void DBHelper::update(ComicDB * comic, QSqlDatabase & db)
{
	//do nothing
}

void DBHelper::update(ComicInfo * comicInfo, QSqlDatabase & db)
{
	QSqlQuery updateComicInfo(db);
	updateComicInfo.prepare("UPDATE comic_info SET "
		"title = :title,"
		
		"coverPage = :coverPage,"
		"numPages = :numPages,"

		"number = :number,"
		"isBis = :isBis,"
		"count = :count,"

		"volume = :volume,"
		"storyArc = :storyArc,"
		"arcNumber = :arcNumber,"
		"arcCount = :arcCount,"

		"genere = :genere,"
		
		"writer = :writer,"
		"penciller = :penciller,"
		"inker = :inker,"
		"colorist = :colorist,"
		"letterer = :letterer,"
		"coverArtist = :coverArtist,"

		"date = :date,"
		"publisher = :publisher,"
		"format = :format,"
		"color = :color,"
		"ageRating = :ageRating,"

		"synopsis = :synopsis,"
		"characters = :characters,"
		"notes = :notes,"
		
		"read = :read,"
		"edited = :edited"
		
		" WHERE id = :id ");
	bindField(":title",comicInfo->title,updateComicInfo);

	bindField(":coverPage",comicInfo->coverPage,updateComicInfo);
	bindField(":numPages",comicInfo->numPages,updateComicInfo);

	bindField(":number",comicInfo->number,updateComicInfo);
	bindField(":isBis",comicInfo->isBis,updateComicInfo);
	bindField(":count",comicInfo->count,updateComicInfo);

	bindField(":volume",comicInfo->volume,updateComicInfo);
	bindField(":storyArc",comicInfo->storyArc,updateComicInfo);
	bindField(":arcNumber",comicInfo->arcNumber,updateComicInfo);
	bindField(":arcCount",comicInfo->arcCount,updateComicInfo);

	bindField(":genere",comicInfo->genere,updateComicInfo);

	bindField(":writer",comicInfo->writer,updateComicInfo);
	bindField(":penciller",comicInfo->penciller,updateComicInfo);
	bindField(":inker",comicInfo->inker,updateComicInfo);
	bindField(":colorist",comicInfo->colorist,updateComicInfo);
	bindField(":letterer",comicInfo->letterer,updateComicInfo);
	bindField(":coverArtist",comicInfo->coverArtist,updateComicInfo);

	bindField(":date",comicInfo->date,updateComicInfo);
	bindField(":publisher",comicInfo->publisher,updateComicInfo);
	bindField(":format",comicInfo->format,updateComicInfo);
	bindField(":color",comicInfo->color,updateComicInfo);
	bindField(":ageRating",comicInfo->ageRating,updateComicInfo);

	bindField(":synopsis",comicInfo->synopsis,updateComicInfo);
	bindField(":characters",comicInfo->characters,updateComicInfo);
	bindField(":notes",comicInfo->notes,updateComicInfo);

	updateComicInfo.bindValue(":read", comicInfo->read?1:0);
	updateComicInfo.bindValue(":id", comicInfo->id);
	updateComicInfo.bindValue(":edited", comicInfo->edited?1:0);
	updateComicInfo.exec();
}

void DBHelper::updateRead(ComicInfo * comicInfo, QSqlDatabase & db)
{
	QSqlQuery findComicInfo(db);
	findComicInfo.prepare("UPDATE comic_info SET "
		"read = :read"
		" WHERE id = :id ");

	findComicInfo.bindValue(":read", comicInfo->read?1:0);
	findComicInfo.bindValue(":id", comicInfo->id);
	findComicInfo.exec();
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

qulonglong DBHelper::insert(ComicDB * comic, QSqlDatabase & db)
{
	if(!comic->info.existOnDb)
	{
		QSqlQuery comicInfoInsert(db);
		comicInfoInsert.prepare("INSERT INTO comic_info (hash,numPages) "
			"VALUES (:hash,:numPages)");
		comicInfoInsert.bindValue(":hash", comic->info.hash);
		comicInfoInsert.bindValue(":numPages", *(comic->info.numPages));
		comicInfoInsert.exec();
		comic->info.id =comicInfoInsert.lastInsertId().toULongLong();
		comic->_hasCover = false;
	}
	else
		comic->_hasCover = true;
	
	QSqlQuery query(db);
	query.prepare("INSERT INTO comic (parentId, comicInfoId, fileName, path) "
                   "VALUES (:parentId,:comicInfoId,:name, :path)");
    query.bindValue(":parentId", comic->parentId);
	query.bindValue(":comicInfoId", comic->info.id);
    query.bindValue(":name", comic->name);
	query.bindValue(":path", comic->path);
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
QList<LibraryItem *> DBHelper::getComicsFromParent(qulonglong parentId, QSqlDatabase & db, bool sort)
{
		QList<LibraryItem *> list;

	QSqlQuery selectQuery(db);
	selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
	selectQuery.bindValue(":parentId", parentId);
	selectQuery.exec();

	ComicDB * currentItem;
	while (selectQuery.next()) 
	{
		QList<QVariant> data;
		QSqlRecord record = selectQuery.record();
		for(int i=0;i<record.count();i++)
			data << record.value(i);

		currentItem = new ComicDB();
		currentItem->id = record.value("id").toULongLong();
		currentItem->parentId = record.value(1).toULongLong();
		currentItem->name = record.value(2).toString();
		currentItem->path = record.value(3).toString();
		currentItem->info = DBHelper::loadComicInfo(record.value(4).toString(),db);
		int lessThan = 0;
		if(list.isEmpty() || !sort)
			list.append(currentItem);
		else
		{
			ComicDB * last = static_cast<ComicDB *>(list.back());
			QString nameLast = last->name; 
			QString nameCurrent = currentItem->name;
			QList<LibraryItem *>::iterator i;
			i = list.end();
			i--;
			while ((0 > (lessThan = nameCurrent.localeAwareCompare(nameLast))) && i != list.begin())  //se usa la misma ordenación que en QDir
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
	//selectQuery.finish();
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

ComicDB DBHelper::loadComic(qulonglong id, QSqlDatabase & db)
{
	ComicDB comic;

	QSqlQuery selectQuery(db); 
	selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.id = :id");
	selectQuery.bindValue(":id", id);
	selectQuery.exec();
	comic.id = id;
	if(selectQuery.next())
	{
		QSqlRecord record = selectQuery.record();
		//id = record.value("id").toULongLong();
		comic.parentId = record.value("parentId").toULongLong();
		comic.name = record.value("name").toString();
		comic.path = record.value("path").toString();
		comic.info = DBHelper::loadComicInfo(record.value("hash").toString(),db);
	}

	return comic;
}

ComicDB DBHelper::loadComic(qulonglong cparentId, QString cname, QString cpath, QString chash, QSqlDatabase & database)
{
	ComicDB comic;

	comic.parentId = cparentId;
	comic.name = cname;
	comic.path = cpath;

	comic.info = DBHelper::loadComicInfo(chash,database);

	if(!comic.info.existOnDb)
	{
		comic.info.hash = chash;
		comic.info.coverPage = new int(1);
		comic._hasCover = false;
	}
	else
		comic._hasCover = true;

	return comic;
}

ComicInfo DBHelper::loadComicInfo(QString hash, QSqlDatabase & db)
{
	ComicInfo comicInfo;

	QSqlQuery findComicInfo(db);
	findComicInfo.prepare("SELECT * FROM comic_info WHERE hash = :hash");
	findComicInfo.bindValue(":hash", hash);
	findComicInfo.exec();


	if(findComicInfo.next())
	{
		comicInfo.hash = hash;
		QSqlRecord record = findComicInfo.record();

		comicInfo.hash = hash;
		comicInfo.id = record.value("id").toULongLong();
		comicInfo.read = record.value("read").toBool();
		comicInfo.edited = record.value("edited").toBool();

		setField("title",comicInfo.title,record);
		setField("numPages",comicInfo.numPages,record);

		setField("coverPage",comicInfo.coverPage,record);

		setField("number",comicInfo.number,record);
		setField("isBis",comicInfo.isBis,record);
		setField("count",comicInfo.count,record);

		setField("volume",comicInfo.volume,record);
		setField("storyArc",comicInfo.storyArc,record);
		setField("arcNumber",comicInfo.arcNumber,record);
		setField("arcCount",comicInfo.arcCount,record);

		setField("genere",comicInfo.genere,record);

		setField("writer",comicInfo.writer,record);
		setField("penciller",comicInfo.penciller,record);
		setField("inker",comicInfo.inker,record);
		setField("colorist",comicInfo.colorist,record);
		setField("letterer",comicInfo.letterer,record);
		setField("coverArtist",comicInfo.coverArtist,record);

		setField("date",comicInfo.date,record);
		setField("publisher",comicInfo.publisher,record);
		setField("format",comicInfo.format,record);
		setField("color",comicInfo.color,record);
		setField("ageRating",comicInfo.ageRating,record);

		setField("synopsis",comicInfo.synopsis,record);
		setField("characters",comicInfo.characters,record);
		setField("notes",comicInfo.notes,record);

		comicInfo.existOnDb = true;
	}
	else
		comicInfo.existOnDb = false;

	return comicInfo;
}

void DBHelper::setField(const QString & name, QString * & field, QSqlRecord & record)
{
	if(!record.value(name).isNull())
	{
		field = new QString();
		*field = record.value(name).toString();
	}
}

void DBHelper::setField(const QString & name, int * & field, QSqlRecord & record)
{
	if(!record.value(name).isNull())
	{
		field = new int;
		*field = record.value(name).toInt();
	}
}

void DBHelper::setField(const QString & name, bool * & field, QSqlRecord & record)
{
	if(!record.value(name).isNull())
	{
		field = new bool;
		*field = record.value(name).toBool();
	}
}


void DBHelper::bindField(const QString & name, QString * field, QSqlQuery & query)
{
	if(field != NULL)
	{
		query.bindValue(name,*field);
	}
}

void DBHelper::bindField(const QString & name, int * field, QSqlQuery & query)
{
	if(field != NULL)
	{
		query.bindValue(name,*field);
	}
}

void DBHelper::bindField(const QString & name, bool * field, QSqlQuery & query)
{
	if(field != NULL)
	{
		query.bindValue(name,*field);
	}
}
