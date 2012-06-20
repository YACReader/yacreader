#include "comic.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

//-----------------------------------------------------------------------------
//COMIC------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Comic::Comic()
{

}

Comic::Comic(qulonglong cparentId, QString cname, QString cpath, QString chash, QSqlDatabase & database)
{
	parentId = cparentId;
	name = cname;
	path = cpath;

	if(!info.load(chash,database))
	{
		info.hash = chash;
		_hasCover = false;
	}
	else
		_hasCover = true;
}

QList<LibraryItem *> Comic::getComicsFromParent(qulonglong parentId, QSqlDatabase & db)
{
	QList<LibraryItem *> list;

	QSqlQuery selectQuery(db); //TODO check
	selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
	selectQuery.bindValue(":parentId", parentId);
	selectQuery.exec();

	Comic * currentItem;
	while (selectQuery.next()) 
	{
		QList<QVariant> data;
		QSqlRecord record = selectQuery.record();
		for(int i=0;i<record.count();i++)
			data << record.value(i);
		//TODO sort by sort indicator and name
		currentItem = new Comic();
		currentItem->id = record.value("id").toLongLong();
		currentItem->parentId = record.value(1).toLongLong();
		currentItem->name = record.value(2).toString();
		currentItem->path = record.value(3).toString();
		currentItem->info.load(record.value(4).toString(),db);
		int lessThan = 0;
		if(list.isEmpty())
			list.append(currentItem);
		else
		{
			Comic * last = static_cast<Comic *>(list.back());
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

	return list;
}

bool Comic::load(qulonglong id, QSqlDatabase & db)
{

	QSqlQuery selectQuery(db); //TODO check
	selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.id = :id");
	selectQuery.bindValue(":id", id);
	selectQuery.exec();

	if(selectQuery.next())
	{
		QSqlRecord record = selectQuery.record();
		id = record.value("id").toLongLong();
		parentId = record.value("parentId").toLongLong();
		name = record.value("name").toString();
		info.load(record.value("hash").toString(),db);
		return true;
	}
	return false;

}

qulonglong Comic::insert(QSqlDatabase & db)
{
	//TODO comprobar si ya hay comic info con ese hash
	//TODO cambiar por info.insert(db)

	if(!info.existOnDb)
	{
		QSqlQuery comicInfoInsert(db);
		comicInfoInsert.prepare("INSERT INTO comic_info (hash) "
			"VALUES (:hash)");
		comicInfoInsert.bindValue(":hash", info.hash);
		comicInfoInsert.exec();
		info.id =comicInfoInsert.lastInsertId().toLongLong();
		_hasCover = false;
	}
	else
		_hasCover = true; //TODO check on disk...
	
	QSqlQuery query(db);
	query.prepare("INSERT INTO comic (parentId, comicInfoId, fileName, path) "
                   "VALUES (:parentId,:comicInfoId,:name, :path)");
    query.bindValue(":parentId", parentId);
	query.bindValue(":comicInfoId", info.id);
    query.bindValue(":name", name);
	query.bindValue(":path", path);
	query.exec();
	return query.lastInsertId().toLongLong();
}

void Comic::update(QSqlDatabase & db)
{

}

void Comic::removeFromDB(QSqlDatabase & db)
{
	QSqlQuery query(db);
	query.prepare("DELETE FROM comic WHERE id = :id");
    query.bindValue(":id", id);
	query.exec();
}

bool Comic::isDir()
{
	return false;
}

//-----------------------------------------------------------------------------
//COMIC_INFO-------------------------------------------------------------------
//-----------------------------------------------------------------------------
ComicInfo::ComicInfo()
	:existOnDb(false)
{

}

bool ComicInfo::load(QString hash, QSqlDatabase & db)
{
	QSqlQuery findComicInfo(db);
	findComicInfo.prepare("SELECT * FROM comic_info WHERE hash = :hash");
	findComicInfo.bindValue(":hash", hash);
	findComicInfo.exec();


	if(findComicInfo.next())
	{
	QSqlRecord record = findComicInfo.record();

	this->hash = hash;
	this->id = record.value("id").toLongLong();
	this->title = record.value("title").toString();
	this->read = record.value("read").toBool();
	this->edited = record.value("edited").toBool();
	existOnDb = true;
	return true;
	}

	existOnDb = false;
	return false;
}

qulonglong ComicInfo::insert(QSqlDatabase & db)
{
	return 0;
}
void ComicInfo::removeFromDB(QSqlDatabase & db)
{

}
void ComicInfo::update(QSqlDatabase & db)
{
	//db.open();
	QSqlQuery findComicInfo(db);
	findComicInfo.prepare("UPDATE comic_info SET title = :title, read = :read WHERE id = :id ");
	findComicInfo.bindValue(":title", title);
	findComicInfo.bindValue(":read", read?1:0);
	findComicInfo.bindValue(":id", id);
	findComicInfo.exec();
	//db.close();
}
