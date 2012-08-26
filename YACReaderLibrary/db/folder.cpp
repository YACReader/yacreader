
#include "folder.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

qulonglong Folder::insert(QSqlDatabase & db)
{
	QSqlQuery query(db);
	query.prepare("INSERT INTO folder (parentId, name, path) "
                   "VALUES (:parentId, :name, :path)");
    query.bindValue(":parentId", parentId);
    query.bindValue(":name", name);
	query.bindValue(":path", path);
	query.exec();
	return query.lastInsertId().toULongLong();
}

QList<LibraryItem *> Folder::getFoldersFromParent(qulonglong parentId, QSqlDatabase & db)
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
		if(list.isEmpty())
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

void Folder::removeFromDB(QSqlDatabase & db)
{
	QSqlQuery query(db);
	query.prepare("DELETE FROM folder WHERE id = :id");
    query.bindValue(":id", id);
	query.exec();
}

