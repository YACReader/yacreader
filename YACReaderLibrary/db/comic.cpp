#include "comic.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

Comic::Comic()
{

}

Comic::Comic(qulonglong cparentId, qulonglong ccomicInfoId, QString cname, QString cpath, QString chash)
		:comicInfoId(ccomicInfoId),hash(chash)
{
	parentId = cparentId;
	name = cname;
	path = cpath;
}

qulonglong Comic::insert(QSqlDatabase & db)
{
	//TODO comprobar si ya hay comic info con ese hash
	QSqlQuery comicInfoInsert(db);
	comicInfoInsert.prepare("INSERT INTO comic_info (hash) "
		"VALUES (:hash)");
	comicInfoInsert.bindValue(":hash", hash);
	 comicInfoInsert.exec();
	 qulonglong comicInfoId =comicInfoInsert.lastInsertId().toLongLong();

	QSqlQuery query(db);
	query.prepare("INSERT INTO comic (parentId, comicInfoId, fileName, path) "
                   "VALUES (:parentId,:comicInfoId,:name, :path)");
    query.bindValue(":parentId", parentId);
	query.bindValue(":comicInfoId", comicInfoId);
    query.bindValue(":name", name);
	query.bindValue(":path", path);
	query.exec();
	return query.lastInsertId().toLongLong();
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
		currentItem->id = record.value(0).toLongLong();
		currentItem->parentId = record.value(1).toLongLong();
		currentItem->name = record.value(2).toString();
		currentItem->hash = record.value(3).toString();
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