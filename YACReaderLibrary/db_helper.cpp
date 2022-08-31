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

#include <algorithm>
#include <limits>

#include "reading_list.h"
#include "library_item.h"
#include "comic_db.h"
#include "data_base_management.h"
#include "folder.h"
#include "yacreader_libraries.h"

#include "qnaturalsorting.h"

#include "QsLog.h"
// server

YACReaderLibraries DBHelper::getLibraries()
{
    YACReaderLibraries libraries;
    libraries.load();
    return libraries;
}

QList<LibraryItem *> DBHelper::getFolderSubfoldersFromLibrary(qulonglong libraryId, qulonglong folderId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<LibraryItem *> list;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");
        list = DBHelper::getFoldersFromParent(folderId, db, false);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return list;
}

QList<LibraryItem *> DBHelper::getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId)
{
    return DBHelper::getFolderComicsFromLibrary(libraryId, folderId, false);
}

QList<LibraryItem *> DBHelper::getFolderComicsFromLibraryForReading(qulonglong libraryId, qulonglong folderId)
{
    auto list = DBHelper::getFolderComicsFromLibrary(libraryId, folderId, false);

    std::sort(list.begin(), list.end(), [](LibraryItem *i1, LibraryItem *i2) {
        auto c1 = static_cast<ComicDB *>(i1);
        auto c2 = static_cast<ComicDB *>(i2);

        if (c1->info.number.isNull() && c2->info.number.isNull()) {
            return naturalSortLessThanCI(c1->name, c2->name);
        } else {
            if (c1->info.number.isNull() == false && c2->info.number.isNull() == false) {
                return c1->info.number.toInt() < c2->info.number.toInt();
            } else {
                return c2->info.number.isNull();
            }
        }
    });

    return list;
}

QList<LibraryItem *> DBHelper::getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId, bool sort)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<LibraryItem *> list;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");
        list = DBHelper::getComicsFromParent(folderId, db, sort);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return list;
}

quint32 DBHelper::getNumChildrenFromFolder(qulonglong libraryId, qulonglong folderId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    quint32 result = 0;
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");

        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT count(*) FROM folder WHERE parentId = :parentId and id <> 1");
        selectQuery.bindValue(":parentId", folderId);
        selectQuery.exec();

        result += selectQuery.record().value(0).toULongLong();

        selectQuery.prepare("SELECT count(*) FROM comic c WHERE c.parentId = :parentId");
        selectQuery.bindValue(":parentId", folderId);
        selectQuery.exec();

        result += selectQuery.record().value(0).toULongLong();
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);

    return result;
}

qulonglong DBHelper::getParentFromComicFolderId(qulonglong libraryId, qulonglong id)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    Folder f;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");

        f = DBHelper::loadFolder(id, db);
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
    return f.parentId;
}
ComicDB DBHelper::getComicInfo(qulonglong libraryId, qulonglong id)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    ComicDB comic;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");

        comic = DBHelper::loadComic(id, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return comic;
}

QList<ComicDB> DBHelper::getSiblings(qulonglong libraryId, qulonglong parentId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<ComicDB> comics;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");
        comics = DBHelper::getSortedComicsFromParent(parentId, db);
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
    return comics;
}

QString DBHelper::getFolderName(qulonglong libraryId, qulonglong id)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);

    QString name = "";
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");
        QSqlQuery selectQuery(db); // TODO check
        selectQuery.prepare("SELECT name FROM folder WHERE id = :id");
        selectQuery.bindValue(":id", id);
        selectQuery.exec();

        if (selectQuery.next()) {
            name = selectQuery.value(0).toString();
        }
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
    return name;
}
QList<QString> DBHelper::getLibrariesNames()
{
    auto names = getLibraries().getNames();
    std::sort(names.begin(), names.end(), naturalSortLessThanCI);
    return names;
}
QString DBHelper::getLibraryName(int id)
{
    return getLibraries().getName(id);
}

QList<ComicDB> DBHelper::getLabelComics(qulonglong libraryId, qulonglong labelId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);

    QList<ComicDB> list;
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT c.id,c.fileName,ci.title,ci.currentPage,ci.numPages,ci.hash,ci.read,ci.coverSizeRatio "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "INNER JOIN comic_label cl ON (c.id == cl.comic_id) "
                            "WHERE cl.label_id = :parentLabelId "
                            "ORDER BY cl.ordering");
        selectQuery.bindValue(":parentLabelId", labelId);
        selectQuery.exec();

        while (selectQuery.next()) {
            ComicDB comic;

            comic.id = selectQuery.value(0).toULongLong();
            comic.parentId = labelId;
            comic.name = selectQuery.value(1).toString();
            comic.info.title = selectQuery.value(2).toString();
            comic.info.currentPage = selectQuery.value(3).toInt();
            comic.info.numPages = selectQuery.value(4).toInt();
            comic.info.hash = selectQuery.value(5).toString();
            comic.info.read = selectQuery.value(6).toBool();
            comic.info.coverSizeRatio = selectQuery.value(7).toFloat();

            list.append(comic);
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return list;
}

QList<ComicDB> DBHelper::getFavorites(qulonglong libraryId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QList<ComicDB> list;

    const int FAV_ID = 1;
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT c.id,c.fileName,ci.title,ci.currentPage,ci.numPages,ci.hash,ci.read,ci.coverSizeRatio "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "INNER JOIN comic_default_reading_list cdrl ON (c.id == cdrl.comic_id) "
                            "WHERE cdrl.default_reading_list_id = :parentDefaultListId "
                            "ORDER BY cdrl.ordering");
        selectQuery.bindValue(":parentDefaultListId", FAV_ID);
        selectQuery.exec();

        while (selectQuery.next()) {
            ComicDB comic;

            comic.id = selectQuery.value(0).toULongLong();
            comic.parentId = FAV_ID;
            comic.name = selectQuery.value(1).toString();
            comic.info.title = selectQuery.value(2).toString();
            comic.info.currentPage = selectQuery.value(3).toInt();
            comic.info.numPages = selectQuery.value(4).toInt();
            comic.info.hash = selectQuery.value(5).toString();
            comic.info.read = selectQuery.value(6).toBool();
            comic.info.coverSizeRatio = selectQuery.value(7).toFloat();

            list.append(comic);
        }

        connectionName = db.connectionName();
    }
    // TODO ?
    QSqlDatabase::removeDatabase(connectionName);

    return list;
}

QList<ComicDB> DBHelper::getReading(qulonglong libraryId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QList<ComicDB> list;
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT c.id,c.parentId,c.fileName,ci.title,ci.currentPage,ci.numPages,ci.hash,ci.read,ci.coverSizeRatio "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "WHERE ci.hasBeenOpened = 1 AND ci.read = 0 "
                            "ORDER BY ci.lastTimeOpened DESC");
        selectQuery.exec();

        while (selectQuery.next()) {
            ComicDB comic;

            comic.id = selectQuery.value(0).toULongLong();
            comic.parentId = selectQuery.value(1).toULongLong();
            comic.name = selectQuery.value(2).toString();
            comic.info.title = selectQuery.value(3).toString();
            comic.info.currentPage = selectQuery.value(4).toInt();
            comic.info.numPages = selectQuery.value(5).toInt();
            comic.info.hash = selectQuery.value(6).toString();
            comic.info.read = selectQuery.value(7).toBool();
            comic.info.coverSizeRatio = selectQuery.value(8).toFloat();

            list.append(comic);
        }
        connectionName = db.connectionName();
    }
    // TODO ?
    QSqlDatabase::removeDatabase(connectionName);

    return list;
}

QList<ReadingList> DBHelper::getReadingLists(qulonglong libraryId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<ReadingList> list;

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");

        QSqlQuery selectQuery("SELECT * from reading_list WHERE parentId IS NULL ORDER BY name DESC", db);

        selectQuery.exec();

        QSqlRecord record = selectQuery.record();

        int name = record.indexOf("name");
        int id = record.indexOf("id");
        int ordering = record.indexOf("ordering");

        while (selectQuery.next()) {
            ReadingList item(selectQuery.value(name).toString(), selectQuery.value(id).toLongLong(), selectQuery.value(ordering).toInt());

            if (list.isEmpty()) {
                list.append(item);
            } else {
                int i = 0;
                while (i < list.length() && naturalSortLessThanCI(list.at(i).getName(), item.getName()))
                    i++;
                list.insert(i, item);
            }
        }
        connectionName = db.connectionName();
    }
    // TODO ?
    QSqlDatabase::removeDatabase(connectionName);

    return list;
}

QList<ComicDB> DBHelper::getReadingListFullContent(qulonglong libraryId, qulonglong readingListId, bool getFullComicInfoFields)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QList<ComicDB> list;
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");
        QList<qulonglong> ids;
        ids << readingListId;

        QSqlQuery subfolders(db);
        subfolders.prepare("SELECT id "
                           "FROM reading_list "
                           "WHERE parentId = :parentId "
                           "ORDER BY ordering ASC");
        subfolders.bindValue(":parentId", readingListId);
        subfolders.exec();
        while (subfolders.next())
            ids << subfolders.value(0).toULongLong();

        foreach (qulonglong id, ids) {
            QSqlQuery selectQuery(db);

            QString params;
            if (getFullComicInfoFields) {
                params = "*";
            } else {
                params = "c.id,c.parentId,c.fileName,c.path,ci.title,ci.currentPage,ci.numPages,ci.hash,ci.read,ci.coverSizeRatio,ci.number";
            }

            selectQuery.prepare("SELECT " + params + " "
                                                     "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                                                     "INNER JOIN comic_reading_list crl ON (c.id == crl.comic_id) "
                                                     "WHERE crl.reading_list_id = :parentReadingList "
                                                     "ORDER BY crl.ordering");
            selectQuery.bindValue(":parentReadingList", id);
            selectQuery.exec();

            auto record = selectQuery.record();

            int idComicIndex = record.indexOf("id");
            int parentIdIndex = record.indexOf("parentId");
            int fileName = record.indexOf("fileName");
            int path = record.indexOf("path");

            while (selectQuery.next()) {
                ComicDB comic;

                if (getFullComicInfoFields) {
                    comic.id = selectQuery.value(idComicIndex).toULongLong();
                    comic.parentId = selectQuery.value(parentIdIndex).toULongLong();
                    comic.name = selectQuery.value(fileName).toString();
                    comic.path = selectQuery.value(path).toString();

                    comic.info = getComicInfoFromQuery(selectQuery, "comicInfoId");
                } else {
                    comic.id = selectQuery.value(0).toULongLong();
                    comic.parentId = selectQuery.value(1).toULongLong();
                    comic.name = selectQuery.value(2).toString();
                    comic.path = selectQuery.value(3).toString();

                    comic.info.title = selectQuery.value(4).toString();
                    comic.info.currentPage = selectQuery.value(5).toInt();
                    comic.info.numPages = selectQuery.value(6).toInt();
                    comic.info.hash = selectQuery.value(7).toString();
                    comic.info.read = selectQuery.value(8).toBool();
                    comic.info.coverSizeRatio = selectQuery.value(9).toFloat();
                    comic.info.number = selectQuery.value(9).toInt();
                }

                list.append(comic);
            }
        }
        connectionName = db.connectionName();
    }

    // TODO ?
    QSqlDatabase::removeDatabase(connectionName);

    return list;
}

// objects management
// deletes
void DBHelper::removeFromDB(LibraryItem *item, QSqlDatabase &db)
{
    if (item->isDir())
        DBHelper::removeFromDB(dynamic_cast<Folder *>(item), db);
    else
        DBHelper::removeFromDB(dynamic_cast<ComicDB *>(item), db);
}
void DBHelper::removeFromDB(Folder *folder, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM folder WHERE id = :id");
    query.bindValue(":id", folder->id);
    query.exec();
}
void DBHelper::removeFromDB(ComicDB *comic, QSqlDatabase &db)
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
    foreach (ComicDB comic, comicsList) {
        query.bindValue(":comic_id", comic.id);
        query.exec();
    }

    db.commit();
}

// a.k.a set comics as unread by reverting the conditions used to load the comics -> void ComicModel::setupReadingModelData(const QString &databasePath)
void DBHelper::deleteComicsFromReading(const QList<ComicDB> &comicsList, QSqlDatabase &db)
{
    db.transaction();

    QLOG_DEBUG() << "deleteComicsFromReading----------------------------------";

    for (auto comic : comicsList) {
        comic.info.hasBeenOpened = false;
        comic.info.currentPage = 0; // update sets hasBeenOpened to true if currentPage > 0;
        DBHelper::update(&comic.info, db);
    }

    db.commit();
}

void DBHelper::deleteComicsFromLabel(const QList<ComicDB> &comicsList, qulonglong labelId, QSqlDatabase &db)
{
    db.transaction();

    QLOG_DEBUG() << "deleteComicsFromLabel----------------------------------";

    QSqlQuery query(db);
    query.prepare("DELETE FROM comic_label WHERE comic_id = :comic_id AND label_id = :label_id");
    foreach (ComicDB comic, comicsList) {
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
    foreach (ComicDB comic, comicsList) {
        query.bindValue(":comic_id", comic.id);
        query.bindValue(":reading_list_id", readingListId);
        query.exec();
    }

    db.commit();
}

// updates
void DBHelper::update(ComicDB *comic, QSqlDatabase &db)
{
    Q_UNUSED(comic)
    Q_UNUSED(db)
    // do nothing
}

void DBHelper::update(qulonglong libraryId, ComicInfo &comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");
        DBHelper::update(&comicInfo, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void DBHelper::update(ComicInfo *comicInfo, QSqlDatabase &db)
{
    if (comicInfo == nullptr)
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
                            // new 7.0 fields
                            "hasBeenOpened = :hasBeenOpened,"

                            "currentPage = :currentPage,"
                            "bookmark1 = :bookmark1,"
                            "bookmark2 = :bookmark2,"
                            "bookmark3 = :bookmark3,"
                            "brightness = :brightness,"
                            "contrast = :contrast, "
                            "gamma = :gamma,"
                            "rating = :rating,"

                            // new 7.1 fields
                            "comicVineID = :comicVineID,"

                            // new 9.5 fields
                            "lastTimeOpened = :lastTimeOpened,"

                            "coverSizeRatio = :coverSizeRatio,"
                            "originalCoverSize = :originalCoverSize,"
                            //--

                            // new 9.8 fields
                            "manga = :manga"
                            //--
                            " WHERE id = :id ");

    updateComicInfo.bindValue(":title", comicInfo->title);

    updateComicInfo.bindValue(":coverPage", comicInfo->coverPage);
    updateComicInfo.bindValue(":numPages", comicInfo->numPages);

    updateComicInfo.bindValue(":number", comicInfo->number);
    updateComicInfo.bindValue(":isBis", comicInfo->isBis);
    updateComicInfo.bindValue(":count", comicInfo->count);

    updateComicInfo.bindValue(":volume", comicInfo->volume);
    updateComicInfo.bindValue(":storyArc", comicInfo->storyArc);
    updateComicInfo.bindValue(":arcNumber", comicInfo->arcNumber);
    updateComicInfo.bindValue(":arcCount", comicInfo->arcCount);

    updateComicInfo.bindValue(":genere", comicInfo->genere);

    updateComicInfo.bindValue(":writer", comicInfo->writer);
    updateComicInfo.bindValue(":penciller", comicInfo->penciller);
    updateComicInfo.bindValue(":inker", comicInfo->inker);
    updateComicInfo.bindValue(":colorist", comicInfo->colorist);
    updateComicInfo.bindValue(":letterer", comicInfo->letterer);
    updateComicInfo.bindValue(":coverArtist", comicInfo->coverArtist);

    updateComicInfo.bindValue(":date", comicInfo->date);
    updateComicInfo.bindValue(":publisher", comicInfo->publisher);
    updateComicInfo.bindValue(":format", comicInfo->format);
    updateComicInfo.bindValue(":color", comicInfo->color);
    updateComicInfo.bindValue(":ageRating", comicInfo->ageRating);
    updateComicInfo.bindValue(":manga", comicInfo->manga);

    updateComicInfo.bindValue(":synopsis", comicInfo->synopsis);
    updateComicInfo.bindValue(":characters", comicInfo->characters);
    updateComicInfo.bindValue(":notes", comicInfo->notes);

    bool read = comicInfo->read || comicInfo->currentPage == comicInfo->numPages.toInt(); // if current page is the las page, the comic is read(completed)
    comicInfo->read = read;
    updateComicInfo.bindValue(":read", read ? 1 : 0);
    updateComicInfo.bindValue(":id", comicInfo->id);
    updateComicInfo.bindValue(":edited", comicInfo->edited ? 1 : 0);

    updateComicInfo.bindValue(":hasBeenOpened", comicInfo->hasBeenOpened ? 1 : 0 || comicInfo->currentPage > 1);
    updateComicInfo.bindValue(":currentPage", comicInfo->currentPage);
    updateComicInfo.bindValue(":bookmark1", comicInfo->bookmark1);
    updateComicInfo.bindValue(":bookmark2", comicInfo->bookmark2);
    updateComicInfo.bindValue(":bookmark3", comicInfo->bookmark3);
    updateComicInfo.bindValue(":brightness", comicInfo->brightness);
    updateComicInfo.bindValue(":contrast", comicInfo->contrast);
    updateComicInfo.bindValue(":gamma", comicInfo->gamma);
    updateComicInfo.bindValue(":rating", comicInfo->rating);

    updateComicInfo.bindValue(":comicVineID", comicInfo->comicVineID);

    updateComicInfo.bindValue(":lastTimeOpened", comicInfo->lastTimeOpened);

    updateComicInfo.bindValue(":coverSizeRatio", comicInfo->coverSizeRatio);
    updateComicInfo.bindValue(":originalCoverSize", comicInfo->originalCoverSize);

    updateComicInfo.exec();
}

void DBHelper::updateRead(ComicInfo *comicInfo, QSqlDatabase &db)
{
    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info SET "
                            "read = :read"
                            " WHERE id = :id ");

    updateComicInfo.bindValue(":read", comicInfo->read ? 1 : 0);
    updateComicInfo.bindValue(":id", comicInfo->id);
    updateComicInfo.exec();
}

void DBHelper::update(const Folder &folder, QSqlDatabase &db)
{
    QSqlQuery updateFolderInfo(db);
    updateFolderInfo.prepare("UPDATE folder SET "
                             "finished = :finished, "
                             "completed = :completed, "
                             "manga = :manga "
                             "WHERE id = :id ");
    updateFolderInfo.bindValue(":finished", folder.isFinished() ? 1 : 0);
    updateFolderInfo.bindValue(":completed", folder.isCompleted() ? 1 : 0);
    updateFolderInfo.bindValue(":manga", folder.isManga() ? 1 : 0);
    updateFolderInfo.bindValue(":id", folder.id);
    updateFolderInfo.exec();
}

void DBHelper::updateChildrenInfo(qulonglong folderId, QSqlDatabase &db)
{
    QList<LibraryItem *> subfolders = DBHelper::getFoldersFromParent(folderId, db, false);
    QList<LibraryItem *> comics = DBHelper::getComicsFromParent(folderId, db, true);

    ComicDB *firstComic = NULL;
    if (comics.count() > 0)
        firstComic = static_cast<ComicDB *>(comics.first());

    QSqlQuery updateFolderInfo(db);
    updateFolderInfo.prepare("UPDATE folder SET "
                             "numChildren = :numChildren, "
                             "firstChildHash = :firstChildHash "
                             "WHERE id = :id ");
    updateFolderInfo.bindValue(":numChildren", subfolders.count() + comics.count());
    updateFolderInfo.bindValue(":firstChildHash", firstComic != NULL ? firstComic->info.hash : "");
    updateFolderInfo.bindValue(":id", folderId);
    updateFolderInfo.exec();
}

void DBHelper::updateChildrenInfo(QSqlDatabase &db)
{
    QSqlQuery selectQuery(db); // TODO check
    selectQuery.prepare("SELECT id FROM folder");
    selectQuery.exec();

    while (selectQuery.next()) {
        DBHelper::updateChildrenInfo(selectQuery.value(0).toULongLong(), db);
    }
}

void DBHelper::updateProgress(qulonglong libraryId, const ComicInfo &comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");

        ComicDB comic = DBHelper::loadComic(comicInfo.id, db);
        comic.info.currentPage = comicInfo.currentPage;
        comic.info.hasBeenOpened = comicInfo.currentPage > 0 || comic.info.hasBeenOpened;
        comic.info.read = comic.info.read || comic.info.currentPage == comic.info.numPages;

        DBHelper::updateReadingRemoteProgress(comic.info, db);

        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
}

void DBHelper::setComicAsReading(qulonglong libraryId, const ComicInfo &comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");

        ComicDB comic = DBHelper::loadComic(comicInfo.id, db);
        comic.info.hasBeenOpened = true;
        comic.info.read = comic.info.read || comic.info.currentPage == comic.info.numPages;

        DBHelper::updateReadingRemoteProgress(comic.info, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void DBHelper::updateReadingRemoteProgress(const ComicInfo &comicInfo, QSqlDatabase &db)
{
    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info SET "
                            "read = :read, "
                            "currentPage = :currentPage, "
                            "hasBeenOpened = :hasBeenOpened, "
                            "lastTimeOpened = :lastTimeOpened, "
                            "rating = :rating"
                            " WHERE id = :id ");

    updateComicInfo.bindValue(":read", comicInfo.read ? 1 : 0);
    updateComicInfo.bindValue(":currentPage", comicInfo.currentPage);
    updateComicInfo.bindValue(":hasBeenOpened", comicInfo.hasBeenOpened ? 1 : 0);
    updateComicInfo.bindValue(":lastTimeOpened", QDateTime::currentMSecsSinceEpoch() / 1000);
    updateComicInfo.bindValue(":id", comicInfo.id);
    updateComicInfo.bindValue(":rating", comicInfo.rating);
    updateComicInfo.exec();

    updateComicInfo.clear();
}

void DBHelper::updateFromRemoteClient(qulonglong libraryId, const ComicInfo &comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");

        ComicDB comic = DBHelper::loadComic(comicInfo.id, db);

        if (comic.info.hash == comicInfo.hash) {
            if (comicInfo.currentPage > 0) {
                comic.info.currentPage = comicInfo.currentPage;

                if (comic.info.currentPage == comic.info.numPages)
                    comic.info.read = true;

                comic.info.hasBeenOpened = true;

                if (comic.info.lastTimeOpened.toULongLong() < comicInfo.lastTimeOpened.toULongLong())
                    comic.info.lastTimeOpened = comicInfo.lastTimeOpened;
            }

            if (comicInfo.rating > 0)
                comic.info.rating = comicInfo.rating;

            DBHelper::updateReadingRemoteProgress(comic.info, db);
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void DBHelper::updateFromRemoteClientWithHash(const ComicInfo &comicInfo)
{
    YACReaderLibraries libraries = DBHelper::getLibraries();

    QStringList names = libraries.getNames();
    QString connectionName = "";

    foreach (QString name, names) {
        QString libraryPath = DBHelper::getLibraries().getPath(libraries.getId(name));

        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");
            ComicInfo info = loadComicInfo(comicInfo.hash, db);

            if (!info.existOnDb) {
                continue;
            }

            if (comicInfo.currentPage > 0) {
                info.currentPage = comicInfo.currentPage;

                if (info.currentPage == info.numPages)
                    info.read = true;

                info.hasBeenOpened = true;

                if (info.lastTimeOpened.toULongLong() < comicInfo.lastTimeOpened.toULongLong())
                    info.lastTimeOpened = comicInfo.lastTimeOpened;
            }

            if (comicInfo.rating > 0)
                info.rating = comicInfo.rating;

            DBHelper::update(&info, db);
            connectionName = db.connectionName();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }
}

QMap<qulonglong, QList<ComicDB>> DBHelper::updateFromRemoteClient(const QMap<qulonglong, QList<ComicInfo>> &comics)
{
    QMap<qulonglong, QList<ComicDB>> moreRecentComics;

    foreach (qulonglong libraryId, comics.keys()) {
        QList<ComicDB> libraryMoreRecentComics;

        QString libraryPath = DBHelper::getLibraries().getPath(libraryId);

        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");

            db.transaction();

            QSqlQuery updateComicInfo(db);
            updateComicInfo.prepare("UPDATE comic_info SET "
                                    "read = :read, "
                                    "currentPage = :currentPage, "
                                    "hasBeenOpened = :hasBeenOpened, "
                                    "lastTimeOpened = :lastTimeOpened, "
                                    "rating = :rating"
                                    " WHERE id = :id ");

            foreach (ComicInfo comicInfo, comics[libraryId]) {
                ComicDB comic = DBHelper::loadComic(comicInfo.id, db);

                if (comic.info.hash == comicInfo.hash) {
                    bool isMoreRecent = false;

                    // completion takes precedence over lastTimeOpened, if we just want to synchronize the lastest status we should use only lastTimeOpened
                    if ((comic.info.currentPage > 1 && comic.info.currentPage > comicInfo.currentPage) || comic.info.hasBeenOpened || (comic.info.read && !comicInfo.read)) {
                        isMoreRecent = true;
                    }

                    if (comic.info.lastTimeOpened.toULongLong() > 0 && comicInfo.lastTimeOpened.toULongLong() == 0) {
                        isMoreRecent = true;
                    }

                    comic.info.currentPage = qMax(comic.info.currentPage, comicInfo.currentPage);

                    if (comic.info.currentPage == comic.info.numPages)
                        comic.info.read = true;

                    comic.info.read = comic.info.read || comicInfo.read;

                    comic.info.hasBeenOpened = comic.info.hasBeenOpened || comicInfo.currentPage > 0;

                    if (comic.info.lastTimeOpened.toULongLong() < comicInfo.lastTimeOpened.toULongLong() && comicInfo.lastTimeOpened.toULongLong() > 0)
                        comic.info.lastTimeOpened = comicInfo.lastTimeOpened;

                    if (comicInfo.rating > 0)
                        comic.info.rating = comicInfo.rating;

                    updateComicInfo.bindValue(":read", comic.info.read ? 1 : 0);
                    updateComicInfo.bindValue(":currentPage", comic.info.currentPage);
                    updateComicInfo.bindValue(":hasBeenOpened", comic.info.hasBeenOpened ? 1 : 0);
                    updateComicInfo.bindValue(":lastTimeOpened", comic.info.lastTimeOpened);
                    updateComicInfo.bindValue(":id", comic.info.id);
                    updateComicInfo.bindValue(":rating", comic.info.rating);
                    updateComicInfo.exec();

                    if (isMoreRecent) {
                        libraryMoreRecentComics.append(comic);
                    }
                }
            }

            if (!libraryMoreRecentComics.isEmpty()) {
                moreRecentComics[libraryId] = libraryMoreRecentComics;
            }

            db.commit();
            connectionName = db.connectionName();
        }

        QSqlDatabase::removeDatabase(connectionName);
    }

    return moreRecentComics;
}

void DBHelper::updateFromRemoteClientWithHash(const QList<ComicInfo> &comics)
{
    YACReaderLibraries libraries = DBHelper::getLibraries();

    QStringList names = libraries.getNames();

    foreach (QString name, names) {
        QString libraryPath = DBHelper::getLibraries().getPath(libraries.getId(name));
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");

            db.transaction();

            QSqlQuery updateComicInfo(db);
            updateComicInfo.prepare("UPDATE comic_info SET "
                                    "read = :read, "
                                    "currentPage = :currentPage, "
                                    "hasBeenOpened = :hasBeenOpened, "
                                    "lastTimeOpened = :lastTimeOpened, "
                                    "rating = :rating"
                                    " WHERE id = :id ");

            foreach (ComicInfo comicInfo, comics) {
                ComicInfo info = loadComicInfo(comicInfo.hash, db);

                if (!info.existOnDb) {
                    continue;
                }

                if (comicInfo.currentPage > 0) {
                    info.currentPage = comicInfo.currentPage;

                    if (info.currentPage == info.numPages)
                        info.read = true;

                    info.hasBeenOpened = true;

                    if (info.lastTimeOpened.toULongLong() < comicInfo.lastTimeOpened.toULongLong())
                        info.lastTimeOpened = comicInfo.lastTimeOpened;
                }

                if (comicInfo.rating > 0) {
                    info.rating = comicInfo.rating;
                }

                updateComicInfo.bindValue(":read", info.read ? 1 : 0);
                updateComicInfo.bindValue(":currentPage", info.currentPage);
                updateComicInfo.bindValue(":hasBeenOpened", info.hasBeenOpened ? 1 : 0);
                updateComicInfo.bindValue(":lastTimeOpened", QDateTime::currentMSecsSinceEpoch() / 1000);
                updateComicInfo.bindValue(":id", info.id);
                updateComicInfo.bindValue(":rating", info.rating);
                updateComicInfo.exec();
            }

            db.commit();
            connectionName = db.connectionName();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }
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
    foreach (qulonglong id, ids) {
        updateOrdering.bindValue(":ordering", order++);
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
                           "WHERE comic_id = :comic_id AND default_reading_list_id = 1");
    db.transaction();
    int order = 0;
    foreach (qulonglong id, comicIds) {
        updateOrdering.bindValue(":ordering", order++);
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
    foreach (qulonglong id, comicIds) {
        updateOrdering.bindValue(":ordering", order++);
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
    foreach (qulonglong id, comicIds) {
        updateOrdering.bindValue(":ordering", order++);
        updateOrdering.bindValue(":comic_id", id);
        updateOrdering.bindValue(":reading_list_id", readingListId);
        updateOrdering.exec();
        QLOG_TRACE() << updateOrdering.lastError().databaseText() << "-" << updateOrdering.lastError().driverText();
    }

    db.commit();
}

// inserts
qulonglong DBHelper::insert(Folder *folder, QSqlDatabase &db)
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

qulonglong DBHelper::insert(ComicDB *comic, QSqlDatabase &db, bool insertAllInfo)
{
    if (!comic->info.existOnDb) {
        QSqlQuery comicInfoInsert(db);
        comicInfoInsert.prepare("INSERT INTO comic_info (hash,numPages,coverSizeRatio,originalCoverSize) "
                                "VALUES (:hash,:numPages,:coverSizeRatio,:originalCoverSize)");
        comicInfoInsert.bindValue(":hash", comic->info.hash);
        comicInfoInsert.bindValue(":numPages", comic->info.numPages);
        comicInfoInsert.bindValue(":coverSizeRatio", comic->info.coverSizeRatio);
        comicInfoInsert.bindValue(":originalCoverSize", comic->info.originalCoverSize);
        comicInfoInsert.exec();
        comic->info.id = comicInfoInsert.lastInsertId().toULongLong();
        comic->_hasCover = false;

        if (insertAllInfo) {
            DBHelper::update(&(comic->info), db); // TODO use insert to insert all the info values, the common binding need to be extracted and shared between update and insert
        }
    } else
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
    QSqlQuery getNumComicsInFavoritesQuery("SELECT count(*) FROM comic_default_reading_list WHERE default_reading_list_id = 1;", db);
    getNumComicsInFavoritesQuery.next();

    int numComics = getNumComicsInFavoritesQuery.value(0).toInt();

    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic_default_reading_list (default_reading_list_id, comic_id, ordering) "
                  "VALUES (1, :comic_id, :ordering)");

    foreach (ComicDB comic, comicsList) {
        query.bindValue(":comic_id", comic.id);
        query.bindValue(":ordering", numComics++);
        query.exec();
    }

    QLOG_TRACE() << query.lastError();

    db.commit();
}

void DBHelper::insertComicsInLabel(const QList<ComicDB> &comicsList, qulonglong labelId, QSqlDatabase &db)
{
    QSqlQuery getNumComicsInFavoritesQuery(QString("SELECT count(*) FROM comic_label WHERE label_id = %1;").arg(labelId), db);
    getNumComicsInFavoritesQuery.next();

    int numComics = getNumComicsInFavoritesQuery.value(0).toInt();

    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic_label (label_id, comic_id, ordering) "
                  "VALUES (:label_id, :comic_id, :ordering)");

    foreach (ComicDB comic, comicsList) {
        query.bindValue(":label_id", labelId);
        query.bindValue(":comic_id", comic.id);
        query.bindValue(":ordering", numComics++);
        query.exec();
    }

    QLOG_TRACE() << query.lastError();

    db.commit();
}

void DBHelper::insertComicsInReadingList(const QList<ComicDB> &comicsList, qulonglong readingListId, QSqlDatabase &db)
{
    QSqlQuery getNumComicsInFavoritesQuery("SELECT count(*) FROM comic_reading_list;", db);
    getNumComicsInFavoritesQuery.next();

    int numComics = getNumComicsInFavoritesQuery.value(0).toInt();

    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic_reading_list (reading_list_id, comic_id, ordering) "
                  "VALUES (:reading_list_id, :comic_id, :ordering)");

    foreach (ComicDB comic, comicsList) {
        query.bindValue(":reading_list_id", readingListId);
        query.bindValue(":comic_id", comic.id);
        query.bindValue(":ordering", numComics++);
        query.exec();
    }

    db.commit();
}
// queries
QList<LibraryItem *> DBHelper::getFoldersFromParent(qulonglong parentId, QSqlDatabase &db, bool sort)
{
    QList<LibraryItem *> list;

    QSqlQuery selectQuery(db); // TODO check
    selectQuery.prepare("SELECT * FROM folder WHERE parentId = :parentId and id <> 1");
    selectQuery.bindValue(":parentId", parentId);
    selectQuery.exec();

    QSqlRecord record = selectQuery.record();

    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int id = record.indexOf("id");
    int numChildren = record.indexOf("numChildren");
    int firstChildHash = record.indexOf("firstChildHash");
    int customImage = record.indexOf("customImage");

    Folder *currentItem;
    while (selectQuery.next()) {
        // TODO sort by sort indicator and name
        currentItem = new Folder(selectQuery.value(id).toULongLong(), parentId, selectQuery.value(name).toString(), selectQuery.value(path).toString());

        if (!selectQuery.value(numChildren).isNull() && selectQuery.value(numChildren).isValid())
            currentItem->setNumChildren(selectQuery.value(numChildren).toInt());
        currentItem->setFirstChildHash(selectQuery.value(firstChildHash).toString());
        currentItem->setCustomImage(selectQuery.value(customImage).toString());

        int lessThan = 0;

        if (list.isEmpty() || !sort)
            list.append(currentItem);
        else {
            auto last = static_cast<Folder *>(list.back());
            QString nameLast = last->name;
            QString nameCurrent = currentItem->name;
            QList<LibraryItem *>::iterator i;
            i = list.end();
            i--;
            while ((0 > (lessThan = naturalCompare(nameCurrent, nameLast, Qt::CaseInsensitive))) && i != list.begin()) {
                i--;
                nameLast = (*i)->name;
            }
            if (lessThan >= 0) // si se ha encontrado un elemento menor que current, se inserta justo después
                list.insert(++i, currentItem);
            else
                list.insert(i, currentItem);
        }
    }

    return list;
}

QList<ComicDB> DBHelper::getSortedComicsFromParent(qulonglong parentId, QSqlDatabase &db)
{
    QList<ComicDB> list;

    QSqlQuery selectQuery(db);

    selectQuery.setForwardOnly(true);
    selectQuery.prepare("select * from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
    selectQuery.bindValue(":parentId", parentId);
    selectQuery.exec();

    QSqlRecord record = selectQuery.record();

    int id = record.indexOf("id");
    // int parentIdIndex = record.indexOf("parentId");
    int fileName = record.indexOf("fileName");
    int path = record.indexOf("path");

    ComicDB currentItem;
    while (selectQuery.next()) {
        currentItem.id = selectQuery.value(id).toULongLong();
        currentItem.parentId = parentId; // selectQuery.value(parentId).toULongLong();
        currentItem.name = selectQuery.value(fileName).toString();
        currentItem.path = selectQuery.value(path).toString();

        currentItem.info = getComicInfoFromQuery(selectQuery, "comicInfoId");

        list.append(currentItem);
    }

    std::sort(list.begin(), list.end(), [](const ComicDB &c1, const ComicDB &c2) {
        if (c1.info.number.isNull() && c2.info.number.isNull()) {
            return naturalSortLessThanCI(c1.name, c2.name);
        } else {
            if (c1.info.number.isNull() == false && c2.info.number.isNull() == false) {
                return c1.info.number.toInt() < c2.info.number.toInt();
            } else {
                return c2.info.number.isNull();
            }
        }
    });

    // selectQuery.finish();
    return list;
}
QList<LibraryItem *> DBHelper::getComicsFromParent(qulonglong parentId, QSqlDatabase &db, bool sort)
{
    QList<LibraryItem *> list;

    QSqlQuery selectQuery(db);
    selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
    selectQuery.bindValue(":parentId", parentId);
    selectQuery.exec();

    QSqlRecord record = selectQuery.record();

    int id = record.indexOf("id");

    ComicDB *currentItem;
    while (selectQuery.next()) {
        currentItem = new ComicDB();
        currentItem->id = selectQuery.value(id).toULongLong();
        currentItem->parentId = selectQuery.value(1).toULongLong();
        currentItem->name = selectQuery.value(2).toString();
        currentItem->path = selectQuery.value(3).toString();
        currentItem->info = DBHelper::loadComicInfo(selectQuery.value(4).toString(), db);

        list.append(currentItem);
    }

    if (sort) {
        std::sort(list.begin(), list.end(), [](const LibraryItem *c1, const LibraryItem *c2) {
            return c1->name.localeAwareCompare(c2->name) < 0;
        });
    }

    return list;
}

QList<Label> DBHelper::getLabels(qulonglong libraryId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<Label> labels;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryPath + "/.yacreaderlibrary");

        QSqlQuery selectQuery("SELECT * FROM label ORDER BY ordering,name", db); // TODO add some kind of
        QSqlRecord record = selectQuery.record();

        int name = record.indexOf("name");
        int id = record.indexOf("id");
        int ordering = record.indexOf("ordering");

        while (selectQuery.next()) {
            Label item(selectQuery.value(name).toString(),
                       selectQuery.value(id).toLongLong(),
                       static_cast<YACReader::LabelColors>(selectQuery.value(ordering).toInt()));

            if (labels.isEmpty()) {
                labels << item;
            } else {
                int i = 0;

                while (i < labels.count() && (labels.at(i).getColorID() < item.getColorID()))
                    i++;

                if (i < labels.count()) {
                    if (labels.at(i).getColorID() == item.getColorID()) // sort by name
                    {
                        while (i < labels.count() && labels.at(i).getColorID() == item.getColorID() && naturalSortLessThanCI(labels.at(i).getName(), item.getName()))
                            i++;
                    }
                }
                if (i >= labels.count()) {
                    labels << item;
                } else {
                    labels.insert(i, item);
                }
            }
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    return labels;
}

void DBHelper::updateFolderTreeManga(qulonglong id, QSqlDatabase &db, bool manga)
{
    QSqlQuery updateFolderQuery(db);
    updateFolderQuery.prepare("UPDATE folder "
                              "SET manga = :manga "
                              "WHERE id = :id");
    updateFolderQuery.bindValue(":manga", manga ? 1 : 0);
    updateFolderQuery.bindValue(":id", id);
    updateFolderQuery.exec();

    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info "
                            "SET manga = :manga "
                            "WHERE id IN (SELECT ci.id FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) WHERE c.parentId = :parentId)");
    updateComicInfo.bindValue(":manga", manga ? 1 : 0);
    updateComicInfo.bindValue(":parentId", id);
    updateComicInfo.exec();

    QSqlQuery getSubFoldersQuery(db);
    getSubFoldersQuery.prepare("SELECT id FROM folder WHERE parentId = :parentId AND id <> 1"); // do not select the root folder
    getSubFoldersQuery.bindValue(":parentId", id);
    getSubFoldersQuery.exec();

    int childFolderIdPos = getSubFoldersQuery.record().indexOf("id");

    while (getSubFoldersQuery.next()) {
        updateFolderTreeManga(getSubFoldersQuery.value(childFolderIdPos).toULongLong(), db, manga);
    }
}

// loads
Folder DBHelper::loadFolder(qulonglong id, QSqlDatabase &db)
{
    Folder folder;

    QSqlQuery query(db);
    query.prepare("SELECT * FROM folder WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();
    folder.id = id;
    folder.parentId = 0;

    QSqlRecord record = query.record();

    int parentId = record.indexOf("parentId");
    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int finished = record.indexOf("finished");
    int completed = record.indexOf("completed");
    int manga = record.indexOf("manga");
    int numChildren = record.indexOf("numChildren");
    int firstChildHash = record.indexOf("firstChildHash");
    int customImage = record.indexOf("customImage");

    if (query.next()) {
        folder.parentId = query.value(parentId).toULongLong();
        folder.name = query.value(name).toString();
        folder.path = query.value(path).toString();
        folder.knownId = true;

        // new 7.1
        folder.setFinished(query.value(finished).toBool());
        folder.setCompleted(query.value(completed).toBool());

        // new 9.5
        if (!query.value(numChildren).isNull() && query.value(numChildren).isValid())
            folder.setNumChildren(query.value(numChildren).toInt());
        folder.setFirstChildHash(query.value(firstChildHash).toString());
        folder.setCustomImage(query.value(customImage).toString());

        // new 9.8
        folder.setManga(query.value(manga).toBool());
    }

    return folder;
}

Folder DBHelper::loadFolder(const QString &folderName, qulonglong parentId, QSqlDatabase &db)
{
    Folder folder;

    QSqlQuery query(db);
    query.prepare("SELECT * FROM folder WHERE parentId = :parentId AND name = :folderName");
    query.bindValue(":parentId", parentId);
    query.bindValue(":folderName", folderName);
    query.exec();

    QSqlRecord record = query.record();

    int id = record.indexOf("id");
    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int finished = record.indexOf("finished");
    int completed = record.indexOf("completed");
    int manga = record.indexOf("manga");
    int numChildren = record.indexOf("numChildren");
    int firstChildHash = record.indexOf("firstChildHash");
    int customImage = record.indexOf("customImage");

    folder.parentId = parentId;
    if (query.next()) {
        folder.id = query.value(id).toULongLong();
        folder.name = query.value(name).toString();
        folder.path = query.value(path).toString();
        folder.knownId = true;

        // new 7.1
        folder.setFinished(query.value(finished).toBool());
        folder.setCompleted(query.value(completed).toBool());

        // new 9.5
        if (!query.value(numChildren).isNull() && query.value(numChildren).isValid())
            folder.setNumChildren(query.value(numChildren).toInt());
        folder.setFirstChildHash(query.value(firstChildHash).toString());
        folder.setCustomImage(query.value(customImage).toString());

        // new 9.8
        folder.setManga(query.value(manga).toBool());
    }

    return folder;
}

ComicDB DBHelper::loadComic(qulonglong id, QSqlDatabase &db)
{
    ComicDB comic;

    QSqlQuery selectQuery(db);
    selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.id = :id");
    selectQuery.bindValue(":id", id);
    selectQuery.exec();

    QSqlRecord record = selectQuery.record();

    int parentId = record.indexOf("parentId");
    int name = record.indexOf("fileName");
    int path = record.indexOf("path");
    int hash = record.indexOf("hash");

    comic.id = id;
    if (selectQuery.next()) {
        comic.parentId = selectQuery.value(parentId).toULongLong();
        comic.name = selectQuery.value(name).toString();
        comic.path = selectQuery.value(path).toString();
        comic.info = DBHelper::loadComicInfo(selectQuery.value(hash).toString(), db);
    }

    return comic;
}

ComicDB DBHelper::loadComic(QString cname, QString cpath, QString chash, QSqlDatabase &database)
{
    ComicDB comic;

    // comic.parentId = cparentId;
    comic.name = cname;
    comic.path = cpath;

    comic.info = DBHelper::loadComicInfo(chash, database);

    if (!comic.info.existOnDb) {
        comic.info.hash = chash;
        comic.info.coverPage = 1;
        comic._hasCover = false;
    } else
        comic._hasCover = true;

    return comic;
}

ComicInfo DBHelper::loadComicInfo(QString hash, QSqlDatabase &db)
{
    ComicInfo comicInfo;

    QSqlQuery findComicInfo(db);
    findComicInfo.prepare("SELECT * FROM comic_info WHERE hash = :hash");
    findComicInfo.bindValue(":hash", hash);
    findComicInfo.exec();

    if (findComicInfo.next()) {
        comicInfo = getComicInfoFromQuery(findComicInfo);
    } else
        comicInfo.existOnDb = false;

    return comicInfo;
}

ComicInfo DBHelper::getComicInfoFromQuery(QSqlQuery &query, const QString &idKey)
{
    QSqlRecord record = query.record();

    int hash = record.indexOf("hash");
    int id = record.indexOf(idKey);
    int read = record.indexOf("read");
    int edited = record.indexOf("edited");

    // new 7.0 fields
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
    int manga = record.indexOf("manga");

    int synopsis = record.indexOf("synopsis");
    int characters = record.indexOf("characters");
    int notes = record.indexOf("notes");

    int comicVineID = record.indexOf("comicVineID");

    int lastTimeOpened = record.indexOf("lastTimeOpened");

    int coverSizeRatio = record.indexOf("coverSizeRatio");
    int originalCoverSize = record.indexOf("originalCoverSize");

    ComicInfo comicInfo;

    comicInfo.hash = query.value(hash).toString();
    comicInfo.id = query.value(id).toULongLong();
    comicInfo.read = query.value(read).toBool();
    comicInfo.edited = query.value(edited).toBool();

    // new 7.0 fields
    comicInfo.hasBeenOpened = query.value(hasBeenOpened).toBool();
    comicInfo.currentPage = query.value(currentPage).toInt();
    comicInfo.bookmark1 = query.value(bookmark1).toInt();
    comicInfo.bookmark2 = query.value(bookmark2).toInt();
    comicInfo.bookmark3 = query.value(bookmark3).toInt();
    comicInfo.brightness = query.value(brightness).toInt();
    comicInfo.contrast = query.value(contrast).toInt();
    comicInfo.gamma = query.value(gamma).toInt();
    comicInfo.rating = query.value(rating).toInt();
    //--
    comicInfo.title = query.value(title);
    comicInfo.numPages = query.value(numPages);

    comicInfo.coverPage = query.value(coverPage);

    comicInfo.number = query.value(number);
    comicInfo.isBis = query.value(isBis);
    comicInfo.count = query.value(count);

    comicInfo.volume = query.value(volume);
    comicInfo.storyArc = query.value(storyArc);
    comicInfo.arcNumber = query.value(arcNumber);
    comicInfo.arcCount = query.value(arcCount);

    comicInfo.genere = query.value(genere);

    comicInfo.writer = query.value(writer);
    comicInfo.penciller = query.value(penciller);
    comicInfo.inker = query.value(inker);
    comicInfo.colorist = query.value(colorist);
    comicInfo.letterer = query.value(letterer);
    comicInfo.coverArtist = query.value(coverArtist);

    comicInfo.date = query.value(date);
    comicInfo.publisher = query.value(publisher);
    comicInfo.format = query.value(format);
    comicInfo.color = query.value(color);
    comicInfo.ageRating = query.value(ageRating);

    comicInfo.synopsis = query.value(synopsis);
    comicInfo.characters = query.value(characters);
    comicInfo.notes = query.value(notes);

    comicInfo.comicVineID = query.value(comicVineID);

    // new 9.5 fields
    comicInfo.lastTimeOpened = query.value(lastTimeOpened);

    comicInfo.coverSizeRatio = query.value(coverSizeRatio);
    comicInfo.originalCoverSize = query.value(originalCoverSize);
    //--

    // new 9.8 fields
    comicInfo.manga = query.value(manga);
    //--

    comicInfo.existOnDb = true;

    return comicInfo;
}

QList<QString> DBHelper::loadSubfoldersNames(qulonglong folderId, QSqlDatabase &db)
{
    QList<QString> result;
    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT name FROM folder WHERE parentId = :parentId AND id <> 1"); // do not select the root folder
    selectQuery.bindValue(":parentId", folderId);
    selectQuery.exec();

    int name = selectQuery.record().indexOf("name");

    while (selectQuery.next()) {
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

    if (selectQuery.next()) {
        return true;
    }

    return false;
}
