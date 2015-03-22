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

#include <limits>

#include "library_item.h"
#include "comic_db.h"
#include "data_base_management.h"
#include "folder.h"
#include "yacreader_libraries.h"

#include "qnaturalsorting.h"

#include "QsLog.h"
//server

YACReaderLibraries DBHelper::getLibraries()
{
	YACReaderLibraries libraries;
	libraries.load();
	return libraries;
}
QList<LibraryItem *> DBHelper::getFolderSubfoldersFromLibrary(qulonglong libraryId, qulonglong folderId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
	QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");
	
	QList<LibraryItem *> list = DBHelper::getFoldersFromParent(folderId,db,false);
	
	db.close();
	QSqlDatabase::removeDatabase(libraryPath);
	return list;
}
QList<LibraryItem *> DBHelper::getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId)
{
    return DBHelper::getFolderComicsFromLibrary(libraryId, folderId, false);
}

QList<LibraryItem *> DBHelper::getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId, bool sort)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");

    QList<LibraryItem *> list = DBHelper::getComicsFromParent(folderId,db,sort);

    db.close();
    QSqlDatabase::removeDatabase(libraryPath);
    return list;
}
qulonglong DBHelper::getParentFromComicFolderId(qulonglong libraryId, qulonglong id)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
	QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");

	Folder f = DBHelper::loadFolder(id,db);

	db.close();
	QSqlDatabase::removeDatabase(libraryPath);
	return f.parentId;
}
ComicDB DBHelper::getComicInfo(qulonglong libraryId, qulonglong id)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
	QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");

	ComicDB comic = DBHelper::loadComic(id,db);

	db.close();
	QSqlDatabase::removeDatabase(libraryPath);
	return comic;
}

QList<ComicDB> DBHelper::getSiblings(qulonglong libraryId, qulonglong parentId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
	QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");

	QList<ComicDB> comics =  DBHelper::getSortedComicsFromParent(parentId,db);
	db.close();
	QSqlDatabase::removeDatabase(libraryPath);
	return comics;
}

QString DBHelper::getFolderName(qulonglong libraryId, qulonglong id)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
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
	QStringList names = getLibraries().getNames();
	qSort(names.begin(),names.end(),naturalSortLessThanCI);
	return names;
}
QString DBHelper::getLibraryName(int id)
{
	return getLibraries().getName(id);
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

void DBHelper::removeLabelFromDB(qulonglong id, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM label WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();
}

void DBHelper::removeListFromDB(qulonglong id, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM reading_list WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();
}

void DBHelper::deleteComicsFromFavorites(const QList<ComicDB> &comicsList, QSqlDatabase &db)
{
    db.transaction();

    QLOG_DEBUG() << "deleteComicsFromFavorites----------------------------------";

    QSqlQuery query(db);
    query.prepare("DELETE FROM comic_default_reading_list WHERE comic_id = :comic_id AND default_reading_list_id = 1");
    foreach(ComicDB comic, comicsList)
    {
        query.bindValue(":comic_id", comic.id);
        query.exec();
    }

    db.commit();
}

void DBHelper::deleteComicsFromLabel(const QList<ComicDB> &comicsList, qulonglong labelId, QSqlDatabase &db)
{
    db.transaction();

    QLOG_DEBUG() << "deleteComicsFromLabel----------------------------------";

    QSqlQuery query(db);
    query.prepare("DELETE FROM comic_label WHERE comic_id = :comic_id AND label_id = :label_id");
    foreach(ComicDB comic, comicsList)
    {
        query.bindValue(":comic_id", comic.id);
        query.bindValue(":label_id", labelId);
        query.exec();

        QLOG_DEBUG() << "cid = " << comic.id << "lid = " << labelId;
        QLOG_DEBUG() << query.lastError().databaseText() << "-" << query.lastError().driverText();
    }

    db.commit();
}

void DBHelper::deleteComicsFromReadingList(const QList<ComicDB> &comicsList, qulonglong readingListId, QSqlDatabase &db)
{
    db.transaction();

    QLOG_DEBUG() << "deleteComicsFromReadingList----------------------------------";

    QSqlQuery query(db);
    query.prepare("DELETE FROM comic_reading_list WHERE comic_id = :comic_id AND reading_list_id = :reading_list_id");
    foreach(ComicDB comic, comicsList)
    {
        query.bindValue(":comic_id", comic.id);
        query.bindValue(":reading_list_id", readingListId);
        query.exec();
    }

    db.commit();
}

//updates
void DBHelper::update(ComicDB * comic, QSqlDatabase & db)
{
	Q_UNUSED(comic)
	Q_UNUSED(db)
	//do nothing
}

void DBHelper::update(qulonglong libraryId, ComicInfo & comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
	QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");

	DBHelper::update(&comicInfo,db);

	db.close();
	QSqlDatabase::removeDatabase(libraryPath);
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
		"edited = :edited,"
		//new 7.0 fields
		"hasBeenOpened = :hasBeenOpened,"

		"currentPage = :currentPage,"
		"bookmark1 = :bookmark1,"
		"bookmark2 = :bookmark2,"
		"bookmark3 = :bookmark3,"
		"brightness = :brightness,"
		"contrast = :contrast, "
		"gamma = :gamma,"
        "rating = :rating,"

        //new 7.1 fields
        "comicVineID = :comicVineID"
		//--
		" WHERE id = :id ");

    updateComicInfo.bindValue(":title",comicInfo->title);

    updateComicInfo.bindValue(":coverPage", comicInfo->coverPage);
    updateComicInfo.bindValue(":numPages", comicInfo->numPages);

    updateComicInfo.bindValue(":number", comicInfo->number);
    updateComicInfo.bindValue(":isBis", comicInfo->isBis);
    updateComicInfo.bindValue(":count", comicInfo->count);

    updateComicInfo.bindValue(":volume", comicInfo->volume);
    updateComicInfo.bindValue(":storyArc", comicInfo->storyArc);
	updateComicInfo.bindValue(":arcNumber",comicInfo->arcNumber);
	updateComicInfo.bindValue(":arcCount",comicInfo->arcCount);

	updateComicInfo.bindValue(":genere",comicInfo->genere);

	updateComicInfo.bindValue(":writer",comicInfo->writer);
	updateComicInfo.bindValue(":penciller",comicInfo->penciller);
	updateComicInfo.bindValue(":inker",comicInfo->inker);
	updateComicInfo.bindValue(":colorist",comicInfo->colorist);
	updateComicInfo.bindValue(":letterer",comicInfo->letterer);
	updateComicInfo.bindValue(":coverArtist",comicInfo->coverArtist);

	updateComicInfo.bindValue(":date",comicInfo->date);
	updateComicInfo.bindValue(":publisher",comicInfo->publisher);
	updateComicInfo.bindValue(":format",comicInfo->format);
	updateComicInfo.bindValue(":color",comicInfo->color);
	updateComicInfo.bindValue(":ageRating",comicInfo->ageRating);

	updateComicInfo.bindValue(":synopsis",comicInfo->synopsis);
	updateComicInfo.bindValue(":characters",comicInfo->characters);
	updateComicInfo.bindValue(":notes",comicInfo->notes);

    bool read = comicInfo->read || comicInfo->currentPage == comicInfo->numPages.toInt(); //if current page is the las page, the comic is read(completed)
    comicInfo->read = read;
    updateComicInfo.bindValue(":read", read?1:0);
	updateComicInfo.bindValue(":id", comicInfo->id);
	updateComicInfo.bindValue(":edited", comicInfo->edited?1:0);

	updateComicInfo.bindValue(":hasBeenOpened", comicInfo->hasBeenOpened?1:0);
	updateComicInfo.bindValue(":currentPage", comicInfo->currentPage);
	updateComicInfo.bindValue(":bookmark1", comicInfo->bookmark1);
	updateComicInfo.bindValue(":bookmark2", comicInfo->bookmark2);
	updateComicInfo.bindValue(":bookmark3", comicInfo->bookmark3);
	updateComicInfo.bindValue(":brightness", comicInfo->brightness);
	updateComicInfo.bindValue(":contrast", comicInfo->contrast);
	updateComicInfo.bindValue(":gamma", comicInfo->gamma);
	updateComicInfo.bindValue(":rating", comicInfo->rating);

    updateComicInfo.bindValue(":comicVineID", comicInfo->comicVineID);

	updateComicInfo.exec();
}

void DBHelper::updateRead(ComicInfo * comicInfo, QSqlDatabase & db)
{
    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info SET "
		"read = :read"
		" WHERE id = :id ");

    updateComicInfo.bindValue(":read", comicInfo->read?1:0);
    updateComicInfo.bindValue(":id", comicInfo->id);
    updateComicInfo.exec();
}

void DBHelper::update(const Folder & folder, QSqlDatabase &db)
{
    QSqlQuery updateFolderInfo(db);
    updateFolderInfo.prepare("UPDATE folder SET "
            "finished = :finished, "
            "completed = :completed "
            "WHERE id = :id ");
    updateFolderInfo.bindValue(":finished", folder.isFinished()?1:0);
    updateFolderInfo.bindValue(":completed", folder.isCompleted()?1:0);
    updateFolderInfo.bindValue(":id", folder.id);
    updateFolderInfo.exec();
}

void DBHelper::updateProgress(qulonglong libraryId, const ComicInfo &comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");

    ComicDB comic = DBHelper::loadComic(comicInfo.id,db);
    comic.info.currentPage = comicInfo.currentPage;
    comic.info.hasBeenOpened = true;

    DBHelper::update(&comic.info,db);

    db.close();
    QSqlDatabase::removeDatabase(libraryPath);
}

void DBHelper::renameLabel(qulonglong id, const QString &name, QSqlDatabase &db)
{
    QSqlQuery renameLabelQuery(db);
    renameLabelQuery.prepare("UPDATE label SET "
                             "name = :name "
                             "WHERE id = :id");
    renameLabelQuery.bindValue(":name", name);
    renameLabelQuery.bindValue(":id", id);
    renameLabelQuery.exec();

    QLOG_DEBUG() << renameLabelQuery.lastError().databaseText();
}

void DBHelper::renameList(qulonglong id, const QString &name, QSqlDatabase &db)
{
    QSqlQuery renameLabelQuery(db);
    renameLabelQuery.prepare("UPDATE reading_list SET "
                             "name = :name "
                             "WHERE id = :id");
    renameLabelQuery.bindValue(":name", name);
    renameLabelQuery.bindValue(":id", id);
    renameLabelQuery.exec();
}

void DBHelper::reasignOrderToSublists(QList<qulonglong> ids, QSqlDatabase &db)
{
    QSqlQuery updateOrdering(db);
    updateOrdering.prepare("UPDATE reading_list SET "
                           "ordering = :ordering "
                           "WHERE id = :id");
    db.transaction();
    int order = 0;
    foreach(qulonglong id, ids)
    {
        updateOrdering.bindValue(":ordering",order++);
        updateOrdering.bindValue(":id", id);
        updateOrdering.exec();
    }

    db.commit();
}

void DBHelper::reasignOrderToComicsInFavorites(QList<qulonglong> comicIds, QSqlDatabase &db)
{
    QSqlQuery updateOrdering(db);
    updateOrdering.prepare("UPDATE comic_default_reading_list SET "
                           "ordering = :ordering "
                           "WHERE comic_id = :comic_id AND default_reading_list_id = 0");
    db.transaction();
    int order = 0;
    foreach(qulonglong id, comicIds)
    {
        updateOrdering.bindValue(":ordering",order++);
        updateOrdering.bindValue(":comic_id", id);
        updateOrdering.exec();
    }

    db.commit();
}

void DBHelper::reasignOrderToComicsInLabel(qulonglong labelId, QList<qulonglong> comicIds, QSqlDatabase &db)
{
    QSqlQuery updateOrdering(db);
    updateOrdering.prepare("UPDATE comic_label SET "
                           "ordering = :ordering "
                           "WHERE comic_id = :comic_id AND label_id = :label_id");
    db.transaction();
    int order = 0;
    foreach(qulonglong id, comicIds)
    {
        updateOrdering.bindValue(":ordering",order++);
        updateOrdering.bindValue(":comic_id", id);
        updateOrdering.bindValue(":label_id", labelId);
        updateOrdering.exec();
    }

    db.commit();
}

void DBHelper::reasignOrderToComicsInReadingList(qulonglong readingListId, QList<qulonglong> comicIds, QSqlDatabase &db)
{
    QSqlQuery updateOrdering(db);
    updateOrdering.prepare("UPDATE comic_reading_list SET "
                           "ordering = :ordering "
                           "WHERE comic_id = :comic_id AND reading_list_id = :reading_list_id");
    db.transaction();
    int order = 0;
    foreach(qulonglong id, comicIds)
    {
        updateOrdering.bindValue(":ordering",order++);
        updateOrdering.bindValue(":comic_id", id);
        updateOrdering.bindValue(":reading_list_id", readingListId);
        updateOrdering.exec();
        QLOG_INFO() << updateOrdering.lastError().databaseText() << "-" << updateOrdering.lastError().driverText();
    }

    db.commit();
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
        comicInfoInsert.bindValue(":numPages", comic->info.numPages);
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

qulonglong DBHelper::insertLabel(const QString &name, YACReader::LabelColors color, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO label (name, color, ordering) "
                   "VALUES (:name, :color, :ordering)");
    query.bindValue(":name", name);
    query.bindValue(":color", YACReader::colorToName(color));
    query.bindValue(":ordering", color);
    query.exec();
    return query.lastInsertId().toULongLong();
}

qulonglong DBHelper::insertReadingList(const QString &name, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO reading_list (name) "
                   "VALUES (:name)");
    query.bindValue(":name", name);
    query.exec();
    return query.lastInsertId().toULongLong();
}

qulonglong DBHelper::insertReadingSubList(const QString &name, qulonglong parentId, int ordering, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO reading_list (name, parentId, ordering) "
                   "VALUES (:name, :parentId, :ordering)");
    query.bindValue(":name", name);
    query.bindValue(":parentId", parentId);
    query.bindValue(":ordering", ordering);
    query.exec();
    return query.lastInsertId().toULongLong();
}

void DBHelper::insertComicsInFavorites(const QList<ComicDB> &comicsList, QSqlDatabase &db)
{
    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic_default_reading_list (default_reading_list_id, comic_id) "
                   "VALUES (1, :comic_id)");

    foreach(ComicDB comic, comicsList)
    {
        query.bindValue(":comic_id", comic.id);
        //query.bindValue(":order", numComics++);
        query.exec();
    }

    db.commit();
}

void DBHelper::insertComicsInLabel(const QList<ComicDB> &comicsList, qulonglong labelId, QSqlDatabase &db)
{
    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic_label (label_id, comic_id) "
                   "VALUES (:label_id, :comic_id)");

    foreach(ComicDB comic, comicsList)
    {
        query.bindValue(":label_id", labelId);
        query.bindValue(":comic_id", comic.id);
        query.exec();
    }

    db.commit();
}

void DBHelper::insertComicsInReadingList(const QList<ComicDB> &comicsList, qulonglong readingListId, QSqlDatabase &db)
{
    QSqlQuery getNumComicsInFavoritesQuery("SELECT count(*) from comic_reading_list;",db);
    getNumComicsInFavoritesQuery.next();
    QSqlRecord record = getNumComicsInFavoritesQuery.record();
    int numComics = record.value(0).toInt();

    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic_reading_list (reading_list_id, comic_id, ordering) "
                   "VALUES (:reading_list_id, :comic_id, :ordering)");

    foreach(ComicDB comic, comicsList)
    {
        query.bindValue(":reading_list_id", readingListId);
        query.bindValue(":comic_id", comic.id);
        query.bindValue(":ordering", numComics++);
        query.exec();
    }

    db.commit();
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
			while ((0 > (lessThan = naturalSortLessThanCI(nameCurrent,nameLast))) && i != list.begin())
			{
				i--;
				nameLast = (*i)->name;
			}
			if(lessThan>=0) //si se ha encontrado un elemento menor que current, se inserta justo después
				list.insert(++i,currentItem);
			else
				list.insert(i,currentItem);

		}
	}

	return list;
}

QList<ComicDB> DBHelper::getSortedComicsFromParent(qulonglong parentId, QSqlDatabase & db)
{
	
	QList <ComicDB> list;

	QSqlQuery selectQuery(db);
	selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
	selectQuery.bindValue(":parentId", parentId);
	selectQuery.exec();

	ComicDB currentItem;
	while (selectQuery.next()) 
	{
		QList<QVariant> data;
		QSqlRecord record = selectQuery.record();
		for(int i=0;i<record.count();i++)
			data << record.value(i);

		currentItem.id = record.value("id").toULongLong();
		currentItem.parentId = record.value(1).toULongLong();
		currentItem.name = record.value(2).toString();
		currentItem.path = record.value(3).toString();
		currentItem.info = DBHelper::loadComicInfo(record.value(4).toString(),db);
		int lessThan = 0;
		if(list.isEmpty())
            list.append(currentItem);
		else
		{
			ComicDB last = static_cast<ComicDB>(list.back());
			QString nameLast = last.name; 
			QString nameCurrent = currentItem.name;

			int numberLast,numberCurrent;
			int max = (std::numeric_limits<int>::max)();
			numberLast = numberCurrent = max; //TODO change by std limit

            if(!last.info.number.isNull())
                numberLast = last.info.number.toInt();

            if(!currentItem.info.number.isNull())
                numberCurrent = currentItem.info.number.toInt();

			QList<ComicDB>::iterator i;
			i = list.end();
			i--;

			if(numberCurrent != max)
			{
				while ((lessThan =numberCurrent < numberLast) && i != list.begin())
				{
					i--;
					numberLast = max;

                    if(!(*i).info.number.isNull())
                        numberLast = (*i).info.number.toInt();
				}
			}
			else
			{
				while ((lessThan = naturalSortLessThanCI(nameCurrent,nameLast)) && i != list.begin() && numberLast == max)
				{
					i--;
                    nameLast = (*i).name;
					numberLast = max;

                    if(!(*i).info.number.isNull())
                        numberLast = (*i).info.number.toInt();
				}

			}
			if(!lessThan) //si se ha encontrado un elemento menor que current, se inserta justo después
			{
				if(numberCurrent != max)
				{
					if(numberCurrent == numberLast)
                        if(currentItem.info.isBis.toBool())
						{
							list.insert(++i,currentItem);
						}
						else
							list.insert(i,currentItem);
					else
						list.insert(++i,currentItem);
				}
				else
					list.insert(++i,currentItem);
			}
			else
			{
				list.insert(i,currentItem);
			}

		}
	}
	//selectQuery.finish();
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
        folder.knownId = true;
        //new 7.1
        folder.setFinished(record.value("finished").toBool());
        folder.setCompleted(record.value("completed").toBool());
	}

    return folder;
}

Folder DBHelper::loadFolder(const QString &folderName, qulonglong parentId, QSqlDatabase &db)
{
    Folder folder;

    QLOG_DEBUG() << "Looking for folder with name = " << folderName << " and parent " << parentId;

    QSqlQuery query(db);
    query.prepare("SELECT * FROM folder WHERE parentId = :parentId AND name = :folderName");
    query.bindValue(":parentId",parentId);
    query.bindValue(":folderName", folderName);
    query.exec();

    folder.parentId = parentId;
    if(query.next())
    {
        QSqlRecord record = query.record();
        folder.id = record.value("id").toULongLong();
        folder.name = record.value("name").toString();
        folder.path = record.value("path").toString();
        folder.knownId = true;
        //new 7.1
        folder.setFinished(record.value("finished").toBool());
        folder.setCompleted(record.value("completed").toBool());
        QLOG_DEBUG() << "FOUND!!";
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

ComicDB DBHelper::loadComic(QString cname, QString cpath, QString chash, QSqlDatabase & database)
{
	ComicDB comic;

    //comic.parentId = cparentId;
	comic.name = cname;
	comic.path = cpath;

	comic.info = DBHelper::loadComicInfo(chash,database);

	if(!comic.info.existOnDb)
	{
		comic.info.hash = chash;
        comic.info.coverPage = 1;
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

		//new 7.0 fields
		comicInfo.hasBeenOpened = record.value("hasBeenOpened").toBool();
		comicInfo.currentPage = record.value("currentPage").toInt();
		comicInfo.bookmark1 = record.value("bookmark1").toInt();
		comicInfo.bookmark2 = record.value("bookmark2").toInt();
		comicInfo.bookmark3 = record.value("bookmark3").toInt();
		comicInfo.brightness = record.value("brightness").toInt();
		comicInfo.contrast = record.value("contrast").toInt();
		comicInfo.gamma = record.value("gamma").toInt();
		comicInfo.rating = record.value("rating").toInt();
		//--

        comicInfo.title = record.value("title");
        comicInfo.numPages = record.value("numPages");

        comicInfo.coverPage = record.value("coverPage");

        comicInfo.number = record.value("number");
        comicInfo.isBis = record.value("isBis");
        comicInfo.count = record.value("count");

        comicInfo.volume = record.value("volume");
        comicInfo.storyArc = record.value("storyArc");
        comicInfo.arcNumber = record.value("arcNumber");
        comicInfo.arcCount = record.value("arcCount");

        comicInfo.genere = record.value("genere");

        comicInfo.writer = record.value("writer");
        comicInfo.penciller = record.value("penciller");
        comicInfo.inker = record.value("inker");
        comicInfo.colorist = record.value("colorist");
        comicInfo.letterer = record.value("letterer");
        comicInfo.coverArtist = record.value("coverArtist");

        comicInfo.date = record.value("date");
        comicInfo.publisher = record.value("publisher");
        comicInfo.format = record.value("format");
        comicInfo.color = record.value("color");
        comicInfo.ageRating = record.value("ageRating");

        comicInfo.synopsis = record.value("synopsis");
        comicInfo.characters = record.value("characters");
        comicInfo.notes = record.value("notes");

        comicInfo.comicVineID = record.value("comicVineID");

		comicInfo.existOnDb = true;
	}
	else
		comicInfo.existOnDb = false;

    return comicInfo;
}

QList<QString> DBHelper::loadSubfoldersNames(qulonglong folderId, QSqlDatabase &db)
{
    QList<QString> result;
    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT name FROM folder WHERE parentId = :parentId AND id <> 1"); //do not select the root folder
    selectQuery.bindValue(":parentId", folderId);
    selectQuery.exec();
    while(selectQuery.next()){
        result << selectQuery.record().value("name").toString();
    }
    return result;
}
