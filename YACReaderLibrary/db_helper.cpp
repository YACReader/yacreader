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
    if(comicInfo == nullptr)
        return;

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
    comic.info.read = comic.info.read || comic.info.currentPage == comic.info.numPages;

    DBHelper::updateReadingRemoteProgress(comic.info,db);

    db.close();
    QSqlDatabase::removeDatabase(libraryPath);
}

void DBHelper::updateReadingRemoteProgress(const ComicInfo &comicInfo, QSqlDatabase &db)
{
    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info SET "
                            "read = :read, "
                            "currentPage = :currentPage, "
                            "hasBeenOpened = :hasBeenOpened, "
                            "rating = :rating"
                            " WHERE id = :id ");

    updateComicInfo.bindValue(":read", comicInfo.read?1:0);
    updateComicInfo.bindValue(":currentPage", comicInfo.currentPage);
    updateComicInfo.bindValue(":hasBeenOpened", comicInfo.hasBeenOpened?1:0);
    updateComicInfo.bindValue(":id", comicInfo.id);
    updateComicInfo.bindValue(":rating", comicInfo.rating);
    updateComicInfo.exec();

    updateComicInfo.clear();
}


void DBHelper::updateFromRemoteClient(qulonglong libraryId,const ComicInfo & comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath+"/.yacreaderlibrary");

    ComicDB comic = DBHelper::loadComic(comicInfo.id,db);

    if(comic.info.hash == comicInfo.hash)
    {
        if(comicInfo.currentPage > 0)
        {
            if(comic.info.currentPage == comic.info.numPages)
                comic.info.read = true;

            comic.info.currentPage = comicInfo.currentPage;

            comic.info.hasBeenOpened = true;
        }

        if(comicInfo.rating > 0)
            comic.info.rating = comicInfo.rating;

        DBHelper::updateReadingRemoteProgress(comic.info,db);
    }

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
        QLOG_TRACE() << updateOrdering.lastError().databaseText() << "-" << updateOrdering.lastError().driverText();
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
    QSqlQuery getNumComicsInFavoritesQuery("SELECT count(*) FROM comic_default_reading_list WHERE default_reading_list_id = 1;",db);
    getNumComicsInFavoritesQuery.next();

    int numComics = getNumComicsInFavoritesQuery.value(0).toInt();

    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic_default_reading_list (default_reading_list_id, comic_id, ordering) "
                   "VALUES (1, :comic_id, :ordering)");

    foreach(ComicDB comic, comicsList)
    {
        query.bindValue(":comic_id", comic.id);
        query.bindValue(":ordering", numComics++);
        query.exec();
    }

    db.commit();
}

void DBHelper::insertComicsInLabel(const QList<ComicDB> &comicsList, qulonglong labelId, QSqlDatabase &db)
{
    QSqlQuery getNumComicsInFavoritesQuery(QString("SELECT count(*) FROM comic_label WHERE label_id = %1;").arg(labelId) ,db);
    getNumComicsInFavoritesQuery.next();

    int numComics = getNumComicsInFavoritesQuery.value(0).toInt();

    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic_label (label_id, comic_id, ordering) "
                   "VALUES (:label_id, :comic_id, :ordering)");

    foreach(ComicDB comic, comicsList)
    {
        query.bindValue(":label_id", labelId);
        query.bindValue(":comic_id", comic.id);
        query.bindValue(":ordering", numComics++);
        query.exec();
    }

    db.commit();
}

void DBHelper::insertComicsInReadingList(const QList<ComicDB> &comicsList, qulonglong readingListId, QSqlDatabase &db)
{
    QSqlQuery getNumComicsInFavoritesQuery("SELECT count(*) FROM comic_reading_list;",db);
    getNumComicsInFavoritesQuery.next();

    int numComics = getNumComicsInFavoritesQuery.value(0).toInt();

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

    QSqlRecord record = selectQuery.record();

    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int id = record.indexOf("id");

    Folder * currentItem;
	while (selectQuery.next()) 
	{
		//TODO sort by sort indicator and name
        currentItem = new Folder(selectQuery.value(id).toULongLong(),parentId,selectQuery.value(name).toString(),selectQuery.value(path).toString());
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

    selectQuery.setForwardOnly(true);
    selectQuery.prepare("select * from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
    selectQuery.bindValue(":parentId", parentId);
	selectQuery.exec();

    QSqlRecord record = selectQuery.record();

    int id = record.indexOf("id");
    //int parentIdIndex = record.indexOf("parentId");
    int fileName = record.indexOf("fileName");
    int path = record.indexOf("path");

    int hash = record.indexOf("hash");
    int comicInfoId = record.indexOf("comicInfoId");
    int read = record.indexOf("read");
    int edited = record.indexOf("edited");

    //new 7.0 fields
    int hasBeenOpened = record.indexOf("hasBeenOpened");
    int currentPage = record.indexOf("currentPage");
    int bookmark1 = record.indexOf("bookmark1");
    int bookmark2 = record.indexOf("bookmark2");
    int bookmark3 = record.indexOf("bookmark3");
    int brightness = record.indexOf("brightness");
    int contrast = record.indexOf("contrast");
    int gamma = record.indexOf("gamma");
    int rating = record.indexOf("rating");
    //--

    int title = record.indexOf("title");
    int numPages = record.indexOf("numPages");

    int coverPage = record.indexOf("coverPage");

    int number = record.indexOf("number");
    int isBis = record.indexOf("isBis");
    int count = record.indexOf("count");

    int volume = record.indexOf("volume");
    int storyArc = record.indexOf("storyArc");
    int arcNumber = record.indexOf("arcNumber");
    int arcCount = record.indexOf("arcCount");

    int genere = record.indexOf("genere");

    int writer = record.indexOf("writer");
    int penciller = record.indexOf("penciller");
    int inker = record.indexOf("inker");
    int colorist = record.indexOf("colorist");
    int letterer = record.indexOf("letterer");
    int coverArtist = record.indexOf("coverArtist");

    int date = record.indexOf("date");
    int publisher = record.indexOf("publisher");
    int format = record.indexOf("format");
    int color = record.indexOf("color");
    int ageRating = record.indexOf("ageRating");

    int synopsis = record.indexOf("synopsis");
    int characters = record.indexOf("characters");
    int notes = record.indexOf("notes");

    int comicVineID = record.indexOf("comicVineID");

	ComicDB currentItem;
	while (selectQuery.next()) 
	{
        currentItem.id = selectQuery.value(id).toULongLong();
        currentItem.parentId = parentId;//selectQuery.value(parentId).toULongLong();
        currentItem.name = selectQuery.value(fileName).toString();
        currentItem.path = selectQuery.value(path).toString();

        currentItem.info.hash = selectQuery.value(hash).toString();
        currentItem.info.id = selectQuery.value(comicInfoId).toULongLong();
        currentItem.info.read = selectQuery.value(read).toBool();
        currentItem.info.edited = selectQuery.value(edited).toBool();

        //new 7.0 fields
        currentItem.info.hasBeenOpened = selectQuery.value(hasBeenOpened).toBool();
        currentItem.info.currentPage = selectQuery.value(currentPage).toInt();
        currentItem.info.bookmark1 = selectQuery.value(bookmark1).toInt();
        currentItem.info.bookmark2 = selectQuery.value(bookmark2).toInt();
        currentItem.info.bookmark3 = selectQuery.value(bookmark3).toInt();
        currentItem.info.brightness = selectQuery.value(brightness).toInt();
        currentItem.info.contrast = selectQuery.value(contrast).toInt();
        currentItem.info.gamma = selectQuery.value(gamma).toInt();
        currentItem.info.rating = selectQuery.value(rating).toInt();
        //--

        currentItem.info.title = selectQuery.value(title);
        currentItem.info.numPages = selectQuery.value(numPages);

        currentItem.info.coverPage = selectQuery.value(coverPage);

        currentItem.info.number = selectQuery.value(number);
        currentItem.info.isBis = selectQuery.value(isBis);
        currentItem.info.count = selectQuery.value(count);

        currentItem.info.volume = selectQuery.value(volume);
        currentItem.info.storyArc = selectQuery.value(storyArc);
        currentItem.info.arcNumber = selectQuery.value(arcNumber);
        currentItem.info.arcCount = selectQuery.value(arcCount);

        currentItem.info.genere = selectQuery.value(genere);

        currentItem.info.writer = selectQuery.value(writer);
        currentItem.info.penciller = selectQuery.value(penciller);
        currentItem.info.inker = selectQuery.value(inker);
        currentItem.info.colorist = selectQuery.value(colorist);
        currentItem.info.letterer = selectQuery.value(letterer);
        currentItem.info.coverArtist = selectQuery.value(coverArtist);

        currentItem.info.date = selectQuery.value(date);
        currentItem.info.publisher = selectQuery.value(publisher);
        currentItem.info.format = selectQuery.value(format);
        currentItem.info.color = selectQuery.value(color);
        currentItem.info.ageRating = selectQuery.value(ageRating);

        currentItem.info.synopsis = selectQuery.value(synopsis);
        currentItem.info.characters = selectQuery.value(characters);
        currentItem.info.notes = selectQuery.value(notes);

        currentItem.info.comicVineID = selectQuery.value(comicVineID);

        currentItem.info.existOnDb = true;

        list.append(currentItem);
    }

    std::sort(list.begin(), list.end(), [](const ComicDB&c1, const ComicDB&c2)
    {
        if(c1.info.number.isNull() && c2.info.number.isNull())
        {
            return naturalSortLessThanCI(c1.name, c2.name);
        }
        else
        {
            if (c1.info.number.isNull() == false && c2.info.number.isNull() == false)
            {
                return c1.info.number.toInt() < c2.info.number.toInt();
            }
            else
            {
                return c2.info.number.isNull();
            }
        }
    });

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

    QSqlRecord record = selectQuery.record();

    int id = record.indexOf("id");

	ComicDB * currentItem;
	while (selectQuery.next()) 
	{
		currentItem = new ComicDB();
        currentItem->id = selectQuery.value(id).toULongLong();
        currentItem->parentId = selectQuery.value(1).toULongLong();
        currentItem->name = selectQuery.value(2).toString();
        currentItem->path = selectQuery.value(3).toString();
        currentItem->info = DBHelper::loadComicInfo(selectQuery.value(4).toString(),db);

        list.append(currentItem);
	}

    if (sort)
    {
        std::sort(list.begin(), list.end(), [](const LibraryItem * c1, const LibraryItem * c2){
            return c1->name.localeAwareCompare(c2->name) < 0;
        });
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

    QSqlRecord record = query.record();

    int parentId = record.indexOf("parentId");
    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int finished = record.indexOf("finished");
    int completed = record.indexOf("completed");

	if(query.next())
	{
        folder.parentId = query.value(parentId).toULongLong();
        folder.name = query.value(name).toString();
        folder.path = query.value(path).toString();
        folder.knownId = true;
        //new 7.1
        folder.setFinished(query.value(finished).toBool());
        folder.setCompleted(query.value(completed).toBool());
	}

    return folder;
}

Folder DBHelper::loadFolder(const QString &folderName, qulonglong parentId, QSqlDatabase &db)
{
    Folder folder;

    QSqlQuery query(db);
    query.prepare("SELECT * FROM folder WHERE parentId = :parentId AND name = :folderName");
    query.bindValue(":parentId",parentId);
    query.bindValue(":folderName", folderName);
    query.exec();

    QSqlRecord record = query.record();

    int id = record.indexOf("id");
    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int finished = record.indexOf("finished");
    int completed = record.indexOf("completed");

    folder.parentId = parentId;
    if(query.next())
    {
        folder.id = query.value(id).toULongLong();
        folder.name = query.value(name).toString();
        folder.path = query.value(path).toString();
        folder.knownId = true;
        //new 7.1
        folder.setFinished(query.value(finished).toBool());
        folder.setCompleted(query.value(completed).toBool());
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

    QSqlRecord record = selectQuery.record();

    int parentId = record.indexOf("parentId");
    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int hash = record.indexOf("hash");

	comic.id = id;
	if(selectQuery.next())
	{
        comic.parentId = selectQuery.value(parentId).toULongLong();
        comic.name = selectQuery.value(name).toString();
        comic.path = selectQuery.value(path).toString();
        comic.info = DBHelper::loadComicInfo(selectQuery.value(hash).toString(),db);
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

    QSqlRecord record = findComicInfo.record();

    int id = record.indexOf("id");
    int read = record.indexOf("read");
    int edited = record.indexOf("edited");

    //new 7.0 fields
    int hasBeenOpened = record.indexOf("hasBeenOpened");
    int currentPage = record.indexOf("currentPage");
    int bookmark1 = record.indexOf("bookmark1");
    int bookmark2 = record.indexOf("bookmark2");
    int bookmark3 = record.indexOf("bookmark3");
    int brightness = record.indexOf("brightness");
    int contrast = record.indexOf("contrast");
    int gamma = record.indexOf("gamma");
    int rating = record.indexOf("rating");
    //--

    int title = record.indexOf("title");
    int numPages = record.indexOf("numPages");

    int coverPage = record.indexOf("coverPage");

    int number = record.indexOf("number");
    int isBis = record.indexOf("isBis");
    int count = record.indexOf("count");

    int volume = record.indexOf("volume");
    int storyArc = record.indexOf("storyArc");
    int arcNumber = record.indexOf("arcNumber");
    int arcCount = record.indexOf("arcCount");

    int genere = record.indexOf("genere");

    int writer = record.indexOf("writer");
    int penciller = record.indexOf("penciller");
    int inker = record.indexOf("inker");
    int colorist = record.indexOf("colorist");
    int letterer = record.indexOf("letterer");
    int coverArtist = record.indexOf("coverArtist");

    int date = record.indexOf("date");
    int publisher = record.indexOf("publisher");
    int format = record.indexOf("format");
    int color = record.indexOf("color");
    int ageRating = record.indexOf("ageRating");

    int synopsis = record.indexOf("synopsis");
    int characters = record.indexOf("characters");
    int notes = record.indexOf("notes");

    int comicVineID = record.indexOf("comicVineID");

	if(findComicInfo.next())
	{
		comicInfo.hash = hash;
        comicInfo.id = findComicInfo.value(id).toULongLong();
        comicInfo.read = findComicInfo.value(read).toBool();
        comicInfo.edited = findComicInfo.value(edited).toBool();

		//new 7.0 fields
        comicInfo.hasBeenOpened = findComicInfo.value(hasBeenOpened).toBool();
        comicInfo.currentPage = findComicInfo.value(currentPage).toInt();
        comicInfo.bookmark1 = findComicInfo.value(bookmark1).toInt();
        comicInfo.bookmark2 = findComicInfo.value(bookmark2).toInt();
        comicInfo.bookmark3 = findComicInfo.value(bookmark3).toInt();
        comicInfo.brightness = findComicInfo.value(brightness).toInt();
        comicInfo.contrast = findComicInfo.value(contrast).toInt();
        comicInfo.gamma = findComicInfo.value(gamma).toInt();
        comicInfo.rating = findComicInfo.value(rating).toInt();
		//--

        comicInfo.title = findComicInfo.value(title);
        comicInfo.numPages = findComicInfo.value(numPages);

        comicInfo.coverPage = findComicInfo.value(coverPage);

        comicInfo.number = findComicInfo.value(number);
        comicInfo.isBis = findComicInfo.value(isBis);
        comicInfo.count = findComicInfo.value(count);

        comicInfo.volume = findComicInfo.value(volume);
        comicInfo.storyArc = findComicInfo.value(storyArc);
        comicInfo.arcNumber = findComicInfo.value(arcNumber);
        comicInfo.arcCount = findComicInfo.value(arcCount);

        comicInfo.genere = findComicInfo.value(genere);

        comicInfo.writer = findComicInfo.value(writer);
        comicInfo.penciller = findComicInfo.value(penciller);
        comicInfo.inker = findComicInfo.value(inker);
        comicInfo.colorist = findComicInfo.value(colorist);
        comicInfo.letterer = findComicInfo.value(letterer);
        comicInfo.coverArtist = findComicInfo.value(coverArtist);

        comicInfo.date = findComicInfo.value(date);
        comicInfo.publisher = findComicInfo.value(publisher);
        comicInfo.format = findComicInfo.value(format);
        comicInfo.color = findComicInfo.value(color);
        comicInfo.ageRating = findComicInfo.value(ageRating);

        comicInfo.synopsis = findComicInfo.value(synopsis);
        comicInfo.characters = findComicInfo.value(characters);
        comicInfo.notes = findComicInfo.value(notes);

        comicInfo.comicVineID = findComicInfo.value(comicVineID);

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

    int name = selectQuery.record().indexOf("name");

    while(selectQuery.next()){
        result << selectQuery.value(name).toString();
    }
    return result;
}

bool DBHelper::isFavoriteComic(qulonglong id, QSqlDatabase &db)
{
    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT * FROM comic_default_reading_list cl WHERE cl.comic_id = :comic_id AND cl.default_reading_list_id = 1");
    selectQuery.bindValue(":comic_id", id);
    selectQuery.exec();

    if(selectQuery.next())
    {
        return true;
    }

    return false;
}
