#include "db_helper.h"

#include "QsLog.h"
#include "comic_db.h"
#include "data_base_management.h"
#include "descriptive_comic_metadata.h"
#include "folder.h"
#include "library_item_types.h"
#include "overloaded.h"
#include "qnaturalsorting.h"
#include "reading_list.h"
#include "table_projection.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QTextStream>

#include <algorithm>
#include <utility>

using namespace YACReader;

// server

YACReaderLibraries DBHelper::getLibraries()
{
    YACReaderLibraries libraries;
    libraries.load();
    return libraries;
}

QList<Folder> DBHelper::getFolderSubfoldersFromLibrary(qulonglong libraryId, qulonglong folderId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<Folder> list;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        list = DBHelper::getFoldersFromParent(folderId, db, false);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return list;
}

QList<ComicDB> DBHelper::getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId)
{
    return DBHelper::getFolderComicsFromLibrary(libraryId, folderId, false);
}

QList<ComicDB> DBHelper::getFolderComicsFromLibraryForReading(qulonglong libraryId, qulonglong folderId)
{
    auto list = DBHelper::getFolderComicsFromLibrary(libraryId, folderId, false);

    std::sort(list.begin(), list.end(), [](const ComicDB &c1, const ComicDB &c2) {
        return comicNumberLessThan(c1.info.number, c1.file.fileName, c2.info.number, c2.file.fileName);
    });

    return list;
}

QList<ComicDB> DBHelper::getFolderComicsFromLibrary(qulonglong libraryId, qulonglong folderId, bool sort)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    QList<ComicDB> list;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        list = DBHelper::getComicsFromParent(folderId, db, sort);

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return list;
}

std::optional<ComicDB> DBHelper::getComicInfo(qulonglong libraryId, qulonglong id)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    std::optional<ComicDB> comic;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        bool found;
        auto loaded = DBHelper::loadComic(id, db, found);
        if (found) {
            comic = loaded;
        }
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
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        comics = DBHelper::getSortedComicsFromParent(parentId, db);
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
    return comics;
}

Folder DBHelper::getFolder(qulonglong libraryId, qulonglong id)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);

    Folder folder;
    QString connectionName = "";

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        QSqlQuery selectQuery(db); // TODO check
        selectQuery.prepare("SELECT * FROM folder WHERE id = :id");
        selectQuery.bindValue(":id", id);
        selectQuery.exec();

        auto record = selectQuery.record();

        int name = record.indexOf("name");
        int path = record.indexOf("path");
        int finished = record.indexOf("finished");
        int completed = record.indexOf("completed");
        int id = record.indexOf("id");
        int parentId = record.indexOf("parentId");
        int numChildren = record.indexOf("numChildren");
        int firstChildHash = record.indexOf("firstChildHash");
        int customImage = record.indexOf("customImage");
        int type = record.indexOf("type");
        int added = record.indexOf("added");
        int updated = record.indexOf("updated");

        if (selectQuery.next()) {
            folder = Folder(selectQuery.value(id).toULongLong(), selectQuery.value(parentId).toULongLong(), selectQuery.value(name).toString(), selectQuery.value(path).toString());

            folder.finished = selectQuery.value(finished).toBool();
            folder.completed = selectQuery.value(completed).toBool();
            if (!selectQuery.value(numChildren).isNull() && selectQuery.value(numChildren).isValid()) {
                folder.numChildren = selectQuery.value(numChildren).toInt();
            }
            folder.firstChildHash = selectQuery.value(firstChildHash).toString();
            folder.customImage = selectQuery.value(customImage).toString();
            folder.type = selectQuery.value(type).value<YACReader::FileType>();
            folder.added = selectQuery.value(added).toLongLong();
            folder.updated = selectQuery.value(updated).toLongLong();
        }
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
    return folder;
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
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT c.id,c.fileName,ci.title,ci.currentPage,ci.numPages,ci.hash,ci.read,ci.coverSizeRatio,ci.number "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "INNER JOIN comic_label cl ON (c.id == cl.comic_id) "
                            "WHERE cl.label_id = :parentLabelId "
                            "ORDER BY cl.ordering");
        selectQuery.bindValue(":parentLabelId", labelId);
        selectQuery.exec();

        auto record = selectQuery.record();
        int id = record.indexOf("id");
        int fileName = record.indexOf("fileName");
        int title = record.indexOf("title");
        int currentPage = record.indexOf("currentPage");
        int numPages = record.indexOf("numPages");
        int hash = record.indexOf("hash");
        int read = record.indexOf("read");
        int coverSizeRatio = record.indexOf("coverSizeRatio");
        int number = record.indexOf("number");

        while (selectQuery.next()) {
            ComicDB comic;

            comic.file.id = ComicId(selectQuery.value(id).toULongLong());
            comic.file.folderId = FolderId(labelId);
            comic.file.fileName = selectQuery.value(fileName).toString();
            comic.info.title = selectQuery.value(title).toString();
            comic.info.currentPage = selectQuery.value(currentPage).toInt();
            comic.info.numPages = selectQuery.value(numPages).toInt();
            comic.info.hash = selectQuery.value(hash).toString();
            comic.info.read = selectQuery.value(read).toBool();
            comic.info.coverSizeRatio = selectQuery.value(coverSizeRatio).toFloat();
            comic.info.number = selectQuery.value(number);

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
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT c.id,c.fileName,ci.title,ci.currentPage,ci.numPages,ci.hash,ci.read,ci.coverSizeRatio,ci.number "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "INNER JOIN comic_default_reading_list cdrl ON (c.id == cdrl.comic_id) "
                            "WHERE cdrl.default_reading_list_id = :parentDefaultListId "
                            "ORDER BY cdrl.ordering");
        selectQuery.bindValue(":parentDefaultListId", FAV_ID);
        selectQuery.exec();

        auto record = selectQuery.record();
        int id = record.indexOf("id");
        int fileName = record.indexOf("fileName");
        int title = record.indexOf("title");
        int currentPage = record.indexOf("currentPage");
        int numPages = record.indexOf("numPages");
        int hash = record.indexOf("hash");
        int read = record.indexOf("read");
        int coverSizeRatio = record.indexOf("coverSizeRatio");
        int number = record.indexOf("number");

        while (selectQuery.next()) {
            ComicDB comic;

            comic.file.id = ComicId(selectQuery.value(id).toULongLong());
            comic.file.folderId = FolderId(FAV_ID);
            comic.file.fileName = selectQuery.value(fileName).toString();
            comic.info.title = selectQuery.value(title).toString();
            comic.info.currentPage = selectQuery.value(currentPage).toInt();
            comic.info.numPages = selectQuery.value(numPages).toInt();
            comic.info.hash = selectQuery.value(hash).toString();
            comic.info.read = selectQuery.value(read).toBool();
            comic.info.coverSizeRatio = selectQuery.value(coverSizeRatio).toFloat();
            comic.info.number = selectQuery.value(number);

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
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT c.id,c.parentId,c.fileName,ci.title,ci.currentPage,ci.numPages,ci.hash,ci.read,ci.coverSizeRatio,ci.number "
                            "FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) "
                            "WHERE ci.hasBeenOpened = 1 AND ci.read = 0 "
                            "ORDER BY ci.lastTimeOpened DESC");
        selectQuery.exec();

        auto record = selectQuery.record();
        int id = record.indexOf("id");
        int parentId = record.indexOf("parentId");
        int fileName = record.indexOf("fileName");
        int title = record.indexOf("title");
        int currentPage = record.indexOf("currentPage");
        int numPages = record.indexOf("numPages");
        int hash = record.indexOf("hash");
        int read = record.indexOf("read");
        int coverSizeRatio = record.indexOf("coverSizeRatio");
        int number = record.indexOf("number");

        while (selectQuery.next()) {
            ComicDB comic;

            // TODO: use QVariant when possible to keep nulls
            comic.file.id = ComicId(selectQuery.value(id).toULongLong());
            comic.file.folderId = FolderId(selectQuery.value(parentId).toULongLong());
            comic.file.fileName = selectQuery.value(fileName).toString();
            comic.info.title = selectQuery.value(title).toString();
            comic.info.currentPage = selectQuery.value(currentPage).toInt();
            comic.info.numPages = selectQuery.value(numPages).toInt();
            comic.info.hash = selectQuery.value(hash).toString();
            comic.info.read = selectQuery.value(read).toBool();
            comic.info.coverSizeRatio = selectQuery.value(coverSizeRatio).toFloat();
            comic.info.number = selectQuery.value(number);

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
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        QSqlQuery selectQuery("SELECT " + TableProjection::selectList("rl", ReadingList::columns()) +
                                      " FROM reading_list rl WHERE rl.parentId IS NULL ORDER BY rl.name DESC",
                              db);

        selectQuery.exec();

        while (selectQuery.next()) {
            const auto item = ReadingList::fromRecord(selectQuery.record(), "rl");

            if (list.isEmpty()) {
                list.append(item);
            } else {
                int i = 0;
                while (i < list.length() && naturalSortLessThanCI(list.at(i).name, item.name))
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
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
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

        const auto &subfolderIds = ids;
        for (const auto id : subfolderIds) {
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
            int title = record.indexOf("title");
            int currentPage = record.indexOf("currentPage");
            int numPages = record.indexOf("numPages");
            int hash = record.indexOf("hash");
            int read = record.indexOf("read");
            int coverSizeRatio = record.indexOf("coverSizeRatio");
            int number = record.indexOf("number");

            while (selectQuery.next()) {
                ComicDB comic;

                if (getFullComicInfoFields) {
                    comic.file.id = ComicId(selectQuery.value(idComicIndex).toULongLong());
                    comic.file.folderId = FolderId(selectQuery.value(parentIdIndex).toULongLong());
                    comic.file.fileName = selectQuery.value(fileName).toString();
                    comic.file.path = selectQuery.value(path).toString();

                    comic.info = getComicInfoFromQuery(selectQuery, "comicInfoId");
                } else {
                    comic.file.id = ComicId(selectQuery.value(idComicIndex).toULongLong());
                    comic.file.folderId = FolderId(selectQuery.value(parentIdIndex).toULongLong());
                    comic.file.fileName = selectQuery.value(fileName).toString();
                    comic.file.path = selectQuery.value(path).toString();

                    comic.info.title = selectQuery.value(title).toString();
                    comic.info.currentPage = selectQuery.value(currentPage).toInt();
                    comic.info.numPages = selectQuery.value(numPages).toInt();
                    comic.info.hash = selectQuery.value(hash).toString();
                    comic.info.read = selectQuery.value(read).toBool();
                    comic.info.coverSizeRatio = selectQuery.value(coverSizeRatio).toFloat();
                    comic.info.number = selectQuery.value(number);
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
void DBHelper::removeFromDB(DiskItem &item, QSqlDatabase &db)
{
    std::visit(overloaded {
                       [&db](Folder &folder) { DBHelper::removeFromDB(&folder, db); },
                       [&db](ComicDB &comic) { DBHelper::removeFromDB(&comic, db); },
               },
               item);
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
    query.bindValue(":id", comic->file.id.value());
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
    for (const auto &comic : comicsList) {
        query.bindValue(":comic_id", comic.file.id.value());
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
    for (const auto &comic : comicsList) {
        query.bindValue(":comic_id", comic.file.id.value());
        query.bindValue(":label_id", labelId);
        query.exec();

        QLOG_DEBUG() << "cid = " << comic.file.id.value() << "lid = " << labelId;
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
    for (const auto &comic : comicsList) {
        query.bindValue(":comic_id", comic.file.id.value());
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
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
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
    updateComicInfo.prepare("UPDATE comic_info SET " + DescriptiveComicMetadata::assignments().join(",") + "," +
                            "coverPage = :coverPage,"
                            "numPages = :numPages,"

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

                            // new 9.5 fields
                            "lastTimeOpened = :lastTimeOpened,"

                            "coverSizeRatio = :coverSizeRatio,"
                            "originalCoverSize = :originalCoverSize,"
                            //--

                            // new 9.8 fields
                            // "manga = :manga," removed in 9.13
                            //--

                            // new 9.13 fields
                            "added = :added,"
                            "type = :type,"

                            // new 9.16 fields
                            "imageFiltersJson = :imageFiltersJson,"
                            "lastTimeImageFiltersSet = :lastTimeImageFiltersSet,"
                            "lastTimeCoverSet = :lastTimeCoverSet,"
                            "usesExternalCover = :usesExternalCover,"
                            "lastTimeMetadataSet = :lastTimeMetadataSet"

                            //--
                            " WHERE id = :id");

    comicInfo->bindTo(updateComicInfo);

    updateComicInfo.bindValue(":coverPage", comicInfo->coverPage);
    updateComicInfo.bindValue(":numPages", comicInfo->numPages);

    bool read = comicInfo->read || comicInfo->currentPage == comicInfo->numPages.toInt(); // if current page is the las page, the comic is read(completed)
    comicInfo->read = read;
    updateComicInfo.bindValue(":read", read ? 1 : 0);
    updateComicInfo.bindValue(":id", comicInfo->id.value());
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

    updateComicInfo.bindValue(":lastTimeOpened", comicInfo->lastTimeOpened);

    updateComicInfo.bindValue(":coverSizeRatio", comicInfo->coverSizeRatio);
    updateComicInfo.bindValue(":originalCoverSize", comicInfo->originalCoverSize);

    updateComicInfo.bindValue(":added", comicInfo->added);
    auto intType = static_cast<int>(comicInfo->type.value<YACReader::FileType>());
    updateComicInfo.bindValue(":type", intType);

    updateComicInfo.bindValue(":imageFiltersJson", comicInfo->imageFiltersJson);
    updateComicInfo.bindValue(":lastTimeImageFiltersSet", comicInfo->lastTimeImageFiltersSet);
    updateComicInfo.bindValue(":lastTimeCoverSet", comicInfo->lastTimeCoverSet);
    updateComicInfo.bindValue(":usesExternalCover", comicInfo->usesExternalCover);
    updateComicInfo.bindValue(":lastTimeMetadataSet", comicInfo->lastTimeMetadataSet);

    updateComicInfo.exec();

    QLOG_INFO() << updateComicInfo.lastError().databaseText();
    QLOG_INFO() << updateComicInfo.lastError().text();
    QLOG_INFO() << updateComicInfo.lastQuery();
}

void DBHelper::updateAdded(ComicInfo *comicInfo, QSqlDatabase &db)
{
    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info SET "
                            "added = :added"
                            " WHERE id = :id ");

    updateComicInfo.bindValue(":added", comicInfo->added);
    updateComicInfo.bindValue(":id", comicInfo->id.value());
    updateComicInfo.exec();
}

void DBHelper::update(const Folder &folder, QSqlDatabase &db)
{
    QSqlQuery updateFolderInfo(db);

    updateFolderInfo.prepare("UPDATE folder SET "
                             "parentId = :parentId, "
                             "name = :name, "
                             "path = :path, "
                             "finished = :finished, "
                             "completed = :completed, "
                             "numChildren = :numChildren, "
                             "firstChildHash = :firstChildHash, "
                             "customImage = :customImage, "
                             "type = :type, "
                             "added = :added, "
                             "updated = :updated "
                             "WHERE id = :id");

    updateFolderInfo.bindValue(":parentId", folder.parentId);
    updateFolderInfo.bindValue(":name", folder.name);
    updateFolderInfo.bindValue(":path", folder.path);
    updateFolderInfo.bindValue(":finished", folder.finished ? 1 : 0);
    updateFolderInfo.bindValue(":completed", folder.completed ? 1 : 0);
    updateFolderInfo.bindValue(":numChildren", folder.numChildren);
    updateFolderInfo.bindValue(":firstChildHash", folder.firstChildHash);
    updateFolderInfo.bindValue(":customImage", folder.customImage);
    updateFolderInfo.bindValue(":type", static_cast<int>(folder.type));
    updateFolderInfo.bindValue(":added", folder.added);
    updateFolderInfo.bindValue(":updated", folder.updated);
    updateFolderInfo.bindValue(":id", folder.id);

    updateFolderInfo.exec();
}

void DBHelper::propagateFolderUpdatesToParent(const Folder &folder, QSqlDatabase &db)
{
    auto currentParentId = folder.parentId;
    auto currentId = folder.id;
    while (currentParentId != 1 && currentParentId != 0) { // currentParentId is 0 if `folder` is the root folder
        auto f = loadFolder(currentParentId, db);
        currentParentId = f.parentId;
        currentId = f.id;
    }

    if (currentId != folder.id) {
        updateChildrenInfo(currentId, db);
    }
}

Folder DBHelper::updateChildrenInfo(qulonglong folderId, QSqlDatabase &db)
{
    auto folder = loadFolder(folderId, db);
    const auto subfolders = DBHelper::getFoldersFromParent(folderId, db, true);
    const auto comics = DBHelper::getComicsFromParent(folderId, db, true);

    QList<Folder> updatedSubfolders;
    for (const auto &sf : subfolders) {
        updatedSubfolders.append(updateChildrenInfo(sf.id, db));
    }

    QString coverHash = "";

    if (!comics.isEmpty()) {
        coverHash = comics.first().info.hash;
    } else {
        for (const auto &f : updatedSubfolders) {
            auto firstChildHash = f.firstChildHash;
            if (!firstChildHash.isEmpty()) {
                coverHash = firstChildHash;
                break;
            }
        }
    }

    if (folder.numChildren == subfolders.count() + comics.count() && folder.firstChildHash == coverHash) {
        return folder;
    }

    folder.numChildren = subfolders.count() + comics.count();
    folder.firstChildHash = coverHash;

    QSqlQuery updateFolderInfo(db);
    updateFolderInfo.prepare("UPDATE folder SET "
                             "numChildren = :numChildren, "
                             "firstChildHash = :firstChildHash "
                             "WHERE id = :id ");
    updateFolderInfo.bindValue(":numChildren", folder.numChildren);
    updateFolderInfo.bindValue(":firstChildHash", folder.firstChildHash);
    updateFolderInfo.bindValue(":id", folderId);
    updateFolderInfo.exec();

    return folder;
}

void DBHelper::updateChildrenInfo(QSqlDatabase &db)
{
    // measure time
    auto start = std::chrono::high_resolution_clock::now();

    QSqlQuery selectQuery(db); // TODO check
    selectQuery.prepare("SELECT id FROM folder f WHERE f.parentId = 1 AND f.id <> 1");
    selectQuery.exec();

    while (selectQuery.next()) {
        DBHelper::updateChildrenInfo(selectQuery.value(0).toULongLong(), db);
    }

    auto end = std::chrono::high_resolution_clock::now();

    QString time = QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    QString message = "updateChildrenInfo took " + time + "ms";
    QLOG_INFO() << message;
}

void DBHelper::updateProgress(qulonglong libraryId, ComicId comicId, const ComicInfo &comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        bool found;
        ComicDB comic = DBHelper::loadComic(comicId.value(), db, found);
        comic.info.currentPage = comicInfo.currentPage;
        comic.info.hasBeenOpened = comicInfo.currentPage > 0 || comic.info.hasBeenOpened;
        comic.info.read = comic.info.read || comic.info.currentPage == comic.info.numPages;

        DBHelper::updateReadingRemoteProgress(comic.info, db);

        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
}

void DBHelper::updateImageFilters(qulonglong libraryId, ComicId comicId, const ComicInfo &comicInfo)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        bool found;
        ComicDB comic = DBHelper::loadComic(comicId.value(), db, found);

        if (found && comic.info.lastTimeImageFiltersSet.toULongLong() < comicInfo.lastTimeImageFiltersSet.toULongLong()) {
            QSqlQuery updateComicInfo(db);
            updateComicInfo.prepare("UPDATE comic_info SET "
                                    "lastTimeImageFiltersSet = :lastTimeImageFiltersSet, "
                                    "imageFiltersJson = :imageFiltersJson "
                                    " WHERE id = :id ");

            updateComicInfo.bindValue(":lastTimeImageFiltersSet", comicInfo.lastTimeImageFiltersSet.toULongLong());
            updateComicInfo.bindValue(":imageFiltersJson", comicInfo.imageFiltersJson);
            updateComicInfo.bindValue(":id", comic.info.id.value());

            auto ret = updateComicInfo.exec();
            if (!ret) {
                QLOG_ERROR() << "Error updating image filters for comic ID:" << comicId.value();
                QLOG_ERROR() << updateComicInfo.lastError().databaseText();
                QLOG_ERROR() << updateComicInfo.lastError().text();
            }
        }

        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
}

void DBHelper::setComicAsReading(qulonglong libraryId, ComicId comicId)
{
    QString libraryPath = DBHelper::getLibraries().getPath(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        bool found;
        ComicDB comic = DBHelper::loadComic(comicId.value(), db, found);
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
    updateComicInfo.bindValue(":lastTimeOpened", QDateTime::currentSecsSinceEpoch());
    updateComicInfo.bindValue(":id", comicInfo.id.value());
    updateComicInfo.bindValue(":rating", comicInfo.rating);
    updateComicInfo.exec();

    updateComicInfo.clear();
}

QMap<qulonglong, QList<ComicDB>> DBHelper::updateFromRemoteClient(const QMap<qulonglong, QList<ComicProgressUpdate>> &comics, bool clientSendsHasBeenOpened, bool clientSendsImageFilters)
{
    QMap<qulonglong, QList<ComicDB>> moreRecentComics;

    const auto libraryIds = comics.keys();
    for (const auto libraryId : libraryIds) {
        QList<ComicDB> libraryMoreRecentComics;

        QString libraryPath = DBHelper::getLibraries().getPath(libraryId);

        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

            if (!db.isValid()) {
                QLOG_ERROR() << "updateFromRemoteClient: could not open database for library" << libraryPath;
                continue;
            }

            db.transaction();

            QSqlQuery updateComicInfo(db);
            updateComicInfo.prepare("UPDATE comic_info SET "
                                    "read = :read, "
                                    "currentPage = :currentPage, "
                                    "hasBeenOpened = :hasBeenOpened, "
                                    "lastTimeOpened = :lastTimeOpened, "
                                    "rating = :rating, "
                                    "imageFiltersJson = :imageFiltersJson, "
                                    "lastTimeImageFiltersSet = :lastTimeImageFiltersSet "
                                    " WHERE id = :id ");

            const auto &updates = comics.constFind(libraryId).value();
            for (const auto &update : updates) {
                const auto &comicInfo = update.info;
                bool found;
                ComicDB comic = DBHelper::loadComic(update.comicId.value(), db, found);

                if (comic.info.hash == comicInfo.hash) {
                    bool isMoreRecent = false;

                    // completion takes precedence over lastTimeOpened, if we just want to synchronize the lastest status we should use only lastTimeOpened
                    if ((comic.info.currentPage > 1 && comic.info.currentPage > comicInfo.currentPage) || (comic.info.read && !comicInfo.read)) {
                        isMoreRecent = true;
                    }

                    if (comic.info.hasBeenOpened && comic.info.currentPage > comicInfo.currentPage) {
                        isMoreRecent = true;
                    }

                    if (comic.info.lastTimeOpened.toULongLong() > 0 && comicInfo.lastTimeOpened.toULongLong() == 0) {
                        isMoreRecent = true;
                    }

                    if (clientSendsImageFilters) {
                        if (comic.info.lastTimeImageFiltersSet.toULongLong() > comicInfo.lastTimeImageFiltersSet.toULongLong()) {
                            isMoreRecent = true;
                        }
                    }

                    comic.info.currentPage = qMax(comic.info.currentPage, comicInfo.currentPage);

                    if (comic.info.currentPage == comic.info.numPages)
                        comic.info.read = true;

                    comic.info.read = comic.info.read || comicInfo.read;

                    if (clientSendsHasBeenOpened) {
                        comic.info.hasBeenOpened = comic.info.hasBeenOpened || comicInfo.hasBeenOpened; // android
                    } else {
                        comic.info.hasBeenOpened = comic.info.hasBeenOpened || comicInfo.currentPage > 0; // ios (legacy)
                    }

                    if (comic.info.lastTimeOpened.toULongLong() < comicInfo.lastTimeOpened.toULongLong() && comicInfo.lastTimeOpened.toULongLong() > 0)
                        comic.info.lastTimeOpened = comicInfo.lastTimeOpened;

                    if (comicInfo.rating > 0)
                        comic.info.rating = comicInfo.rating;

                    if (clientSendsImageFilters) {
                        if (comic.info.lastTimeImageFiltersSet.toULongLong() < comicInfo.lastTimeImageFiltersSet.toULongLong()) {
                            comic.info.imageFiltersJson = comicInfo.imageFiltersJson;
                            comic.info.lastTimeImageFiltersSet = comicInfo.lastTimeImageFiltersSet;
                        }
                    }

                    updateComicInfo.bindValue(":read", comic.info.read ? 1 : 0);
                    updateComicInfo.bindValue(":currentPage", comic.info.currentPage);
                    updateComicInfo.bindValue(":hasBeenOpened", comic.info.hasBeenOpened ? 1 : 0);
                    updateComicInfo.bindValue(":lastTimeOpened", comic.info.lastTimeOpened);
                    updateComicInfo.bindValue(":id", comic.info.id.value());
                    updateComicInfo.bindValue(":rating", comic.info.rating);
                    updateComicInfo.bindValue(":imageFiltersJson", comic.info.imageFiltersJson);
                    updateComicInfo.bindValue(":lastTimeImageFiltersSet", comic.info.lastTimeImageFiltersSet);
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

QList<QUuid> DBHelper::updateFromRemoteClientWithHash(const QList<ComicInfo> &comics)
{
    QList<QUuid> updatedLibraries;
    const YACReaderLibraries libraries = DBHelper::getLibraries();
    const QStringList names = libraries.getNames();

    for (const auto &name : names) {
        bool libraryUpdated = false;
        QString libraryPath = DBHelper::getLibraries().getPath(libraries.getId(name));
        QString connectionName = "";
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

            if (!db.isValid()) {
                QLOG_ERROR() << "updateFromRemoteClientWithHash: could not open database for library" << libraryPath;
                continue;
            }

            db.transaction();

            QSqlQuery updateComicInfo(db);
            updateComicInfo.prepare("UPDATE comic_info SET "
                                    "read = :read, "
                                    "currentPage = :currentPage, "
                                    "hasBeenOpened = :hasBeenOpened, "
                                    "lastTimeOpened = :lastTimeOpened, "
                                    "rating = :rating"
                                    " WHERE id = :id ");

            for (const auto &comicInfo : comics) {
                const auto storedInfo = loadComicInfo(comicInfo.hash, db);

                if (!storedInfo) {
                    continue;
                }

                ComicInfo info = *storedInfo;

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
                updateComicInfo.bindValue(":lastTimeOpened", QDateTime::currentSecsSinceEpoch());
                updateComicInfo.bindValue(":id", info.id.value());
                updateComicInfo.bindValue(":rating", info.rating);
                updateComicInfo.exec();
                libraryUpdated = true;
            }

            db.commit();
            connectionName = db.connectionName();
        }
        QSqlDatabase::removeDatabase(connectionName);

        if (libraryUpdated) {
            updatedLibraries.append(libraries.getUuid(name));
        }
    }

    return updatedLibraries;
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
    const auto &readingListIds = ids;
    for (const auto id : readingListIds) {
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
    const auto &favoriteComicIds = comicIds;
    for (const auto id : favoriteComicIds) {
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
    const auto &labelComicIds = comicIds;
    for (const auto id : labelComicIds) {
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
    const auto &readingListComicIds = comicIds;
    for (const auto id : readingListComicIds) {
        updateOrdering.bindValue(":ordering", order++);
        updateOrdering.bindValue(":comic_id", id);
        updateOrdering.bindValue(":reading_list_id", readingListId);
        updateOrdering.exec();
        QLOG_TRACE() << updateOrdering.lastError().databaseText() << "-" << updateOrdering.lastError().driverText();
    }

    db.commit();
}

void DBHelper::updateComicsInfo(QList<ComicDB> &comics, const QString &databasePath)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        db.open();
        db.transaction();
        const auto &comicsRef = comics;
        for (auto comic : comicsRef) {
            DBHelper::update(&(comic.info), db);
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

bool DBHelper::renameFolder(qulonglong id, const QString &name, const QString &oldPath, const QString &newPath, QSqlDatabase &db, QString *error)
{
    if (error != nullptr)
        error->clear();

    const auto execute = [error](QSqlQuery &query) {
        if (query.exec())
            return true;

        if (error != nullptr)
            *error = query.lastError().text();
        return false;
    };

    QSqlQuery renameFolder(db);
    renameFolder.prepare("UPDATE folder SET name = :name, path = :newPath WHERE id = :id AND path = :oldPath");
    renameFolder.bindValue(":name", name);
    renameFolder.bindValue(":newPath", newPath);
    renameFolder.bindValue(":id", id);
    renameFolder.bindValue(":oldPath", oldPath);
    if (!execute(renameFolder) || renameFolder.numRowsAffected() != 1) {
        if (error != nullptr && error->isEmpty())
            *error = QCoreApplication::translate("DBHelper", "The folder entry could not be found in the library database.");
        return false;
    }

    QSqlQuery updateSubfolders(db);
    updateSubfolders.prepare("UPDATE folder SET path = :newPath || substr(path, length(:oldPath) + 1) "
                             "WHERE substr(path, 1, length(:oldPath) + 1) = :oldPath || '/'");
    updateSubfolders.bindValue(":newPath", newPath);
    updateSubfolders.bindValue(":oldPath", oldPath);
    if (!execute(updateSubfolders))
        return false;

    QSqlQuery updateComics(db);
    updateComics.prepare("UPDATE comic SET path = :newPath || substr(path, length(:oldPath) + 1) "
                         "WHERE substr(path, 1, length(:oldPath) + 1) = :oldPath || '/'");
    updateComics.bindValue(":newPath", newPath);
    updateComics.bindValue(":oldPath", oldPath);
    return execute(updateComics);
}

bool DBHelper::moveComic(qulonglong comicId, qulonglong newParentId, const QString &newFileName, const QString &newRelativePath, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("UPDATE comic SET parentId = :parentId, fileName = :fileName, path = :path WHERE id = :id");
    query.bindValue(":parentId", newParentId);
    query.bindValue(":fileName", newFileName);
    query.bindValue(":path", newRelativePath);
    query.bindValue(":id", comicId);

    return query.exec() && query.numRowsAffected() == 1;
}

qulonglong DBHelper::ensureFolderPath(const QString &relativePath, QSqlDatabase &db, QList<qulonglong> *createdFolderIds)
{
    const auto segments = relativePath.split('/', Qt::SkipEmptyParts);

    qulonglong parentId = 1;
    auto inheritedType = DBHelper::loadFolder(parentId, db).type;
    QString currentPath;

    for (const auto &segment : segments) {
        currentPath += '/' + segment;

        const auto existing = DBHelper::loadFolder(segment, parentId, db);
        if (existing.knownId) {
            parentId = existing.id;
            inheritedType = existing.type;
            continue;
        }

        Folder folder(segment, currentPath);
        folder.setFather(parentId);
        folder.type = inheritedType;

        parentId = DBHelper::insert(&folder, db);
        if (createdFolderIds != nullptr)
            createdFolderIds->append(parentId);
    }

    return parentId;
}

void DBHelper::syncFolderAddedFromContents(const QList<qulonglong> &folderIds, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("UPDATE folder SET added = COALESCE("
                  "(SELECT MIN(ci.added) FROM comic c INNER JOIN comic_info ci ON c.comicInfoId = ci.id WHERE c.parentId = folder.id), added) "
                  "WHERE id = :id");

    for (const auto id : folderIds) {
        query.bindValue(":id", id);
        if (!query.exec()) {
            QLOG_ERROR() << "syncFolderAddedFromContents: update failed for folder" << id << query.lastError().text();
        }
    }
}

void DBHelper::removeEmptyFolderPaths(const QStringList &relativePaths, QSqlDatabase &db, QList<QVariantMap> *removedRows)
{
    QSqlQuery select(db);
    select.prepare("SELECT * FROM folder WHERE path = :path AND id <> 1"
                   " AND NOT EXISTS (SELECT 1 FROM comic WHERE comic.parentId = folder.id)"
                   " AND NOT EXISTS (SELECT 1 FROM folder AS child WHERE child.parentId = folder.id)");

    QSqlQuery remove(db);
    remove.prepare("DELETE FROM folder WHERE id = :id");

    for (const auto &path : relativePaths) {
        select.bindValue(":path", path);
        if (!select.exec()) {
            QLOG_ERROR() << "removeEmptyFolderPaths: select failed for" << path << select.lastError().text();
            continue;
        }

        if (!select.next())
            continue;

        const auto record = select.record();

        QVariantMap row;
        for (int i = 0; i < record.count(); ++i)
            row.insert(record.fieldName(i), record.value(i));

        remove.bindValue(":id", row.value(QStringLiteral("id")));
        if (!remove.exec()) {
            QLOG_ERROR() << "removeEmptyFolderPaths: delete failed for" << path << remove.lastError().text();
            continue;
        }

        if (removedRows != nullptr)
            removedRows->append(row);
    }
}

void DBHelper::removeEmptyFolderRows(const QList<qulonglong> &folderIds, QSqlDatabase &db)
{
    QSqlQuery remove(db);
    remove.prepare("DELETE FROM folder WHERE id = :id AND id <> 1"
                   " AND NOT EXISTS (SELECT 1 FROM comic WHERE comic.parentId = folder.id)"
                   " AND NOT EXISTS (SELECT 1 FROM folder AS child WHERE child.parentId = folder.id)");

    for (const auto id : folderIds) {
        remove.bindValue(":id", id);
        if (!remove.exec()) {
            QLOG_ERROR() << "removeEmptyFolderRows: delete failed for folder" << id << remove.lastError().text();
        }
    }
}

bool DBHelper::restoreFolderRows(const QList<QVariantMap> &rows, QSqlDatabase &db)
{
    // A child cannot be inserted before its parent, because parentId is a foreign
    // key into the same table.
    auto ordered = rows;
    std::sort(ordered.begin(), ordered.end(), [](const QVariantMap &a, const QVariantMap &b) {
        return a.value(QStringLiteral("path")).toString().count(QLatin1Char('/')) < b.value(QStringLiteral("path")).toString().count(QLatin1Char('/'));
    });

    bool success = true;

    for (const auto &row : std::as_const(ordered)) {
        if (row.value(QStringLiteral("id")).toULongLong() == 0)
            continue;

        QStringList columns;
        QStringList placeholders;
        for (auto it = row.constBegin(); it != row.constEnd(); ++it) {
            columns << it.key();
            placeholders << QLatin1Char(':') + it.key();
        }

        QSqlQuery insert(db);
        insert.prepare(QStringLiteral("INSERT OR IGNORE INTO folder (%1) VALUES (%2)")
                               .arg(columns.join(QStringLiteral(", ")), placeholders.join(QStringLiteral(", "))));

        for (auto it = row.constBegin(); it != row.constEnd(); ++it)
            insert.bindValue(QLatin1Char(':') + it.key(), it.value());

        if (!insert.exec()) {
            QLOG_ERROR() << "restoreFolderRows: insert failed for"
                         << row.value(QStringLiteral("path")).toString() << insert.lastError().text();
            success = false;
        }
    }

    return success;
}

// inserts
qulonglong DBHelper::insert(Folder *folder, QSqlDatabase &db)
{
    auto added = QDateTime::currentSecsSinceEpoch();
    folder->added = added;

    QSqlQuery query(db);
    query.prepare("INSERT INTO folder (parentId, name, path, added, type) "
                  "VALUES (:parentId, :name, :path, :added, :type)");
    query.bindValue(":parentId", folder->parentId);
    query.bindValue(":name", folder->name);
    query.bindValue(":path", folder->path);
    query.bindValue(":added", added);
    auto intType = static_cast<int>(folder->type);
    query.bindValue(":type", intType);
    query.exec();

    return query.lastInsertId().toULongLong();
}

qulonglong DBHelper::insert(ComicDB *comic, QSqlDatabase &db, bool insertAllInfo)
{
    auto added = QDateTime::currentSecsSinceEpoch();

    if (comic->info.id == ComicInfoId()) {
        QSqlQuery comicInfoInsert(db);

        comicInfoInsert.prepare("INSERT INTO comic_info (hash,numPages,coverSizeRatio,originalCoverSize,added,type) "
                                "VALUES (:hash,:numPages,:coverSizeRatio,:originalCoverSize,:added,:type)");
        comicInfoInsert.bindValue(":hash", comic->info.hash);
        comicInfoInsert.bindValue(":numPages", comic->info.numPages);
        comicInfoInsert.bindValue(":coverSizeRatio", comic->info.coverSizeRatio);
        comicInfoInsert.bindValue(":originalCoverSize", comic->info.originalCoverSize);
        comicInfoInsert.bindValue(":added", added);
        auto intType = static_cast<int>(comic->info.type.value<YACReader::FileType>());
        comicInfoInsert.bindValue(":type", intType);
        comicInfoInsert.exec();
        comic->info.id = ComicInfoId(comicInfoInsert.lastInsertId().toULongLong());
        comic->info.added = added;

        if (insertAllInfo) {
            DBHelper::update(&(comic->info), db); // TODO use insert to insert all the info values, the common binding need to be extracted and shared between update and insert
        }
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic (parentId, comicInfoId, fileName, path) "
                  "VALUES (:parentId,:comicInfoId,:name, :path)");
    query.bindValue(":parentId", comic->file.folderId.value());
    query.bindValue(":comicInfoId", comic->info.id.value());
    query.bindValue(":name", comic->file.fileName);
    query.bindValue(":path", comic->file.path);
    query.exec();

    // loop through parents and update their updated field
    // TODO: use stored procedures
    QSqlQuery updateFolder(db);
    updateFolder.prepare("UPDATE folder SET "
                         "updated = :updated "
                         "WHERE id = :id ");
    auto currentParentId = comic->file.folderId.value();
    while (currentParentId != 1 && currentParentId != 0) {
        updateFolder.bindValue(":updated", added);
        updateFolder.bindValue(":id", currentParentId);
        updateFolder.exec();

        auto f = loadFolder(currentParentId, db);
        currentParentId = f.parentId;
    }
    //----

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

    for (const auto &comic : comicsList) {
        query.bindValue(":comic_id", comic.file.id.value());
        query.bindValue(":ordering", numComics++);
        query.exec();
    }

    QLOG_TRACE() << query.lastError();

    db.commit();
}

void DBHelper::insertComicsInLabel(const QList<ComicDB> &comicsList, qulonglong labelId, QSqlDatabase &db)
{
    QSqlQuery getNumComics(QString("SELECT count(*) FROM comic_label WHERE label_id = %1;").arg(labelId), db);
    getNumComics.next();

    int numComics = getNumComics.value(0).toInt();

    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic_label (label_id, comic_id, ordering) "
                  "VALUES (:label_id, :comic_id, :ordering)");

    for (const auto &comic : comicsList) {
        query.bindValue(":label_id", labelId);
        query.bindValue(":comic_id", comic.file.id.value());
        query.bindValue(":ordering", numComics++);
        query.exec();
    }

    QLOG_TRACE() << query.lastError();

    db.commit();
}

void DBHelper::insertComicsInReadingList(const QList<ComicDB> &comicsList, qulonglong readingListId, QSqlDatabase &db)
{
    QSqlQuery getNumComics("SELECT count(*) FROM comic_reading_list;", db);
    getNumComics.next();

    int numComics = getNumComics.value(0).toInt();

    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO comic_reading_list (reading_list_id, comic_id, ordering) "
                  "VALUES (:reading_list_id, :comic_id, :ordering)");

    for (const auto &comic : comicsList) {
        query.bindValue(":reading_list_id", readingListId);
        query.bindValue(":comic_id", comic.file.id.value());
        query.bindValue(":ordering", numComics++);
        query.exec();
    }

    db.commit();
}

// queries
QList<Folder> DBHelper::getFoldersFromParent(qulonglong parentId, QSqlDatabase &db, bool sort)
{
    QList<Folder> list;

    QSqlQuery selectQuery(db); // TODO check
    selectQuery.prepare("SELECT * FROM folder WHERE parentId = :parentId and id <> 1");
    selectQuery.bindValue(":parentId", parentId);
    selectQuery.exec();

    QSqlRecord record = selectQuery.record();

    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int finished = record.indexOf("finished");
    int completed = record.indexOf("completed");
    int id = record.indexOf("id");
    int numChildren = record.indexOf("numChildren");
    int firstChildHash = record.indexOf("firstChildHash");
    int customImage = record.indexOf("customImage");
    int type = record.indexOf("type");
    int added = record.indexOf("added");
    int updated = record.indexOf("updated");

    while (selectQuery.next()) {
        // TODO sort by sort indicator and name
        Folder currentItem(selectQuery.value(id).toULongLong(), parentId, selectQuery.value(name).toString(), selectQuery.value(path).toString());

        currentItem.finished = selectQuery.value(finished).toBool();
        currentItem.completed = selectQuery.value(completed).toBool();
        if (!selectQuery.value(numChildren).isNull() && selectQuery.value(numChildren).isValid()) {
            currentItem.numChildren = selectQuery.value(numChildren).toInt();
        }
        currentItem.firstChildHash = selectQuery.value(firstChildHash).toString();
        currentItem.customImage = selectQuery.value(customImage).toString();
        currentItem.type = selectQuery.value(type).value<YACReader::FileType>();
        currentItem.added = selectQuery.value(added).toLongLong();
        currentItem.updated = selectQuery.value(updated).toLongLong();

        int lessThan = 0;

        if (list.isEmpty() || !sort)
            list.append(currentItem);
        else {
            QString nameLast = list.back().name;
            QString nameCurrent = currentItem.name;
            QList<Folder>::iterator i;
            i = list.end();
            i--;
            while ((0 > (lessThan = naturalCompare(nameCurrent, nameLast, Qt::CaseInsensitive))) && i != list.begin()) {
                i--;
                nameLast = i->name;
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
        currentItem.file.id = ComicId(selectQuery.value(id).toULongLong());
        currentItem.file.folderId = FolderId(parentId); // selectQuery.value(parentId).toULongLong();
        currentItem.file.fileName = selectQuery.value(fileName).toString();
        currentItem.file.path = selectQuery.value(path).toString();

        currentItem.info = getComicInfoFromQuery(selectQuery, "comicInfoId");

        list.append(currentItem);
    }

    std::sort(list.begin(), list.end(), [](const ComicDB &c1, const ComicDB &c2) {
        return comicNumberLessThan(c1.info.number, c1.file.fileName, c2.info.number, c2.file.fileName);
    });

    // selectQuery.finish();
    return list;
}
QList<ComicDB> DBHelper::getComicsFromParent(qulonglong parentId, QSqlDatabase &db, bool sort)
{
    QList<ComicDB> list;

    QSqlQuery selectQuery(db);
    selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
    selectQuery.bindValue(":parentId", parentId);
    selectQuery.exec();

    QSqlRecord record = selectQuery.record();

    int id = record.indexOf("id");

    while (selectQuery.next()) {
        ComicDB currentItem;
        currentItem.file.id = ComicId(selectQuery.value(id).toULongLong());
        currentItem.file.folderId = FolderId(selectQuery.value(1).toULongLong());
        currentItem.file.fileName = selectQuery.value(2).toString();
        currentItem.file.path = selectQuery.value(3).toString();
        if (auto info = DBHelper::loadComicInfo(selectQuery.value(4).toString(), db)) {
            currentItem.info = *info;
        }

        list.append(currentItem);
    }

    if (sort) {
        std::sort(list.begin(), list.end(), [](const ComicDB &c1, const ComicDB &c2) {
            return c1.file.fileName.localeAwareCompare(c2.file.fileName) < 0;
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
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));

        QSqlQuery selectQuery("SELECT " + TableProjection::selectList("l", Label::columns()) +
                                      " FROM label l ORDER BY l.ordering, l.name",
                              db); // TODO add some kind of

        while (selectQuery.next()) {
            const auto item = Label::fromRecord(selectQuery.record(), "l");

            if (labels.isEmpty()) {
                labels << item;
            } else {
                int i = 0;

                while (i < labels.count() && (labels.at(i).ordering < item.ordering))
                    i++;

                if (i < labels.count()) {
                    if (labels.at(i).ordering == item.ordering) // sort by name
                    {
                        while (i < labels.count() && labels.at(i).ordering == item.ordering && naturalSortLessThanCI(labels.at(i).name, item.name))
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

bool DBHelper::updateFolderTreeType(qulonglong id, QSqlDatabase &db, YACReader::FileType type)
{
    QSqlQuery updateFolderQuery(db);
    updateFolderQuery.prepare("UPDATE folder "
                              "SET type = :type "
                              "WHERE id = :id");
    updateFolderQuery.bindValue(":type", static_cast<int>(type));
    updateFolderQuery.bindValue(":id", id);
    bool success = updateFolderQuery.exec();

    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info "
                            "SET type = :type "
                            "WHERE id IN (SELECT ci.id FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) WHERE c.parentId = :parentId)");
    updateComicInfo.bindValue(":type", static_cast<int>(type));
    updateComicInfo.bindValue(":parentId", id);
    success = updateComicInfo.exec() && success;

    QSqlQuery getSubFoldersQuery(db);
    getSubFoldersQuery.prepare("SELECT id FROM folder WHERE parentId = :parentId AND id <> 1"); // do not select the root folder
    getSubFoldersQuery.bindValue(":parentId", id);
    success = getSubFoldersQuery.exec() && success;

    int childFolderIdPos = getSubFoldersQuery.record().indexOf("id");

    while (getSubFoldersQuery.next()) {
        success = updateFolderTreeType(getSubFoldersQuery.value(childFolderIdPos).toULongLong(), db, type) && success;
    }

    return success;
}

bool DBHelper::setComicRead(qulonglong comicId, bool read, QSqlDatabase &db)
{
    QSqlQuery updateComicInfo(db);
    if (read) {
        updateComicInfo.prepare("UPDATE comic_info SET "
                                "read = 1 "
                                "WHERE id = (SELECT comicInfoId FROM comic WHERE id = :id)");
    } else {
        updateComicInfo.prepare("UPDATE comic_info SET "
                                "read = 0, "
                                "currentPage = 1, "
                                "hasBeenOpened = 0, "
                                "lastTimeOpened = NULL "
                                "WHERE id = (SELECT comicInfoId FROM comic WHERE id = :id)");
    }
    updateComicInfo.bindValue(":id", comicId);

    if (!updateComicInfo.exec()) {
        QLOG_ERROR() << "setComicRead failed for comic" << comicId << ":" << updateComicInfo.lastError().text();
        return false;
    }

    return true;
}

bool DBHelper::setComicType(qulonglong comicId, YACReader::FileType type, QSqlDatabase &db)
{
    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info SET "
                            "type = :type "
                            "WHERE id = (SELECT comicInfoId FROM comic WHERE id = :id)");
    updateComicInfo.bindValue(":type", static_cast<int>(type));
    updateComicInfo.bindValue(":id", comicId);

    if (!updateComicInfo.exec()) {
        QLOG_ERROR() << "setComicType failed for comic" << comicId << ":" << updateComicInfo.lastError().text();
        return false;
    }

    return true;
}

bool DBHelper::setFolderFinished(qulonglong folderId, bool finished, QSqlDatabase &db)
{
    QSqlQuery updateFolder(db);
    updateFolder.prepare("UPDATE folder SET finished = :finished WHERE id = :id");
    updateFolder.bindValue(":finished", finished ? 1 : 0);
    updateFolder.bindValue(":id", folderId);

    if (!updateFolder.exec()) {
        QLOG_ERROR() << "setFolderFinished failed for folder" << folderId << ":" << updateFolder.lastError().text();
        return false;
    }

    return true;
}

bool DBHelper::setFolderCompleted(qulonglong folderId, bool completed, QSqlDatabase &db)
{
    QSqlQuery updateFolder(db);
    updateFolder.prepare("UPDATE folder SET completed = :completed WHERE id = :id");
    updateFolder.bindValue(":completed", completed ? 1 : 0);
    updateFolder.bindValue(":id", folderId);

    if (!updateFolder.exec()) {
        QLOG_ERROR() << "setFolderCompleted failed for folder" << folderId << ":" << updateFolder.lastError().text();
        return false;
    }

    return true;
}

void DBHelper::updateDBType(QSqlDatabase &db, YACReader::FileType type)
{
    QSqlQuery updateFolderQuery(db);
    updateFolderQuery.prepare("UPDATE folder "
                              "SET type = :type");
    updateFolderQuery.bindValue(":type", static_cast<int>(type));
    updateFolderQuery.exec();

    QSqlQuery updateComicInfo(db);
    updateComicInfo.prepare("UPDATE comic_info "
                            "SET type = :type");
    updateComicInfo.bindValue(":type", static_cast<int>(type));
    updateComicInfo.exec();
}

Folder DBHelper::loadFolder(qulonglong id, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("SELECT * FROM folder WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();

    Folder folder;

    folder.id = id;
    folder.parentId = 0;

    DBHelper::readFolderFromQuery(folder, query);

    return folder;
}

Folder DBHelper::loadFolder(const QString &folderName, qulonglong parentId, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("SELECT * FROM folder WHERE parentId = :parentId AND name = :folderName");
    query.bindValue(":parentId", parentId);
    query.bindValue(":folderName", folderName);
    query.exec();

    Folder folder;

    folder.parentId = parentId;

    DBHelper::readFolderFromQuery(folder, query);

    return folder;
}

void DBHelper::readFolderFromQuery(Folder &folder, QSqlQuery &query)
{
    QSqlRecord record = query.record();

    int id = record.indexOf("id");
    int parentId = record.indexOf("parentId");
    int name = record.indexOf("name");
    int path = record.indexOf("path");
    int finished = record.indexOf("finished");
    int completed = record.indexOf("completed");
    int numChildren = record.indexOf("numChildren");
    int firstChildHash = record.indexOf("firstChildHash");
    int customImage = record.indexOf("customImage");
    int type = record.indexOf("type");
    int added = record.indexOf("added");
    int updated = record.indexOf("updated");

    if (query.next()) {
        folder.id = query.value(id).toULongLong();
        folder.parentId = query.value(parentId).toULongLong();
        folder.name = query.value(name).toString();
        folder.path = query.value(path).toString();
        folder.knownId = true;

        // new 7.1
        folder.finished = query.value(finished).toBool();
        folder.completed = query.value(completed).toBool();

        // new 9.5
        if (!query.value(numChildren).isNull() && query.value(numChildren).isValid()) {
            folder.numChildren = query.value(numChildren).toInt();
        }
        folder.firstChildHash = query.value(firstChildHash).toString();
        folder.customImage = query.value(customImage).toString();

        // new 9.8
        // folder.manga = query.value(manga).toBool(); //removed in 9.13

        // new 9.13
        folder.type = query.value(type).value<YACReader::FileType>();
        folder.added = query.value(added).toLongLong();
        folder.updated = query.value(updated).toLongLong();
    }
}

ComicDB DBHelper::loadComic(qulonglong id, QSqlDatabase &db, bool &found)
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

    comic.file.id = ComicId(id);
    if (selectQuery.next()) {
        comic.file.folderId = FolderId(selectQuery.value(parentId).toULongLong());
        comic.file.fileName = selectQuery.value(name).toString();
        comic.file.path = selectQuery.value(path).toString();
        if (auto info = DBHelper::loadComicInfo(selectQuery.value(hash).toString(), db)) {
            comic.info = *info;
        }
        found = true;
    } else {
        found = false;
    }

    return comic;
}

ComicDB DBHelper::loadComic(QString cname, QString cpath, QString chash, QSqlDatabase &database)
{
    ComicDB comic;

    // comic.parentId = cparentId;
    comic.file.fileName = cname;
    comic.file.path = cpath;

    if (auto info = DBHelper::loadComicInfo(chash, database)) {
        comic.info = *info;
    } else {
        comic.info.hash = chash;
        comic.info.coverPage = 1;
    }

    return comic;
}

std::optional<ComicInfo> DBHelper::loadComicInfo(QString hash, QSqlDatabase &db)
{
    QSqlQuery findComicInfo(db);
    findComicInfo.prepare("SELECT * FROM comic_info WHERE hash = :hash");
    findComicInfo.bindValue(":hash", hash);
    findComicInfo.exec();

    if (findComicInfo.next()) {
        return getComicInfoFromQuery(findComicInfo);
    }

    return std::nullopt;
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

    int numPages = record.indexOf("numPages");

    int coverPage = record.indexOf("coverPage");

    int lastTimeOpened = record.indexOf("lastTimeOpened");

    int coverSizeRatio = record.indexOf("coverSizeRatio");
    int originalCoverSize = record.indexOf("originalCoverSize");

    int added = record.indexOf("added");
    int type = record.indexOf("type");

    int imageFiltersJson = record.indexOf("imageFiltersJson");
    int lastTimeImageFiltersSet = record.indexOf("lastTimeImageFiltersSet");
    int lastTimeCoverSet = record.indexOf("lastTimeCoverSet");
    int usesExternalCover = record.indexOf("usesExternalCover");
    int lastTimeMetadataSet = record.indexOf("lastTimeMetadataSet");

    ComicInfo comicInfo;
    comicInfo.loadFrom(record);

    comicInfo.hash = query.value(hash).toString();
    comicInfo.id = ComicInfoId(query.value(id).toULongLong());
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
    comicInfo.numPages = query.value(numPages);

    comicInfo.coverPage = query.value(coverPage);

    // new 9.5 fields
    comicInfo.lastTimeOpened = query.value(lastTimeOpened);

    comicInfo.coverSizeRatio = query.value(coverSizeRatio);
    comicInfo.originalCoverSize = query.value(originalCoverSize);
    //--

    // new 9.8 fields
    // comicInfo.manga = query.value(manga); // removed in 9.13
    //--

    // new 9.13 fields
    comicInfo.added = query.value(added);
    comicInfo.type = query.value(type);
    //--

    // new 9.16 fields
    comicInfo.imageFiltersJson = query.value(imageFiltersJson);
    comicInfo.lastTimeImageFiltersSet = query.value(lastTimeImageFiltersSet);
    comicInfo.lastTimeCoverSet = query.value(lastTimeCoverSet);
    comicInfo.usesExternalCover = query.value(usesExternalCover).toBool();
    comicInfo.lastTimeMetadataSet = query.value(lastTimeMetadataSet);
    //--

    return comicInfo;
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

QVariantMap DBHelper::getLibraryInfoData(QUuid id)
{
    const QString libraryPath = DBHelper::getLibraries().getPath(id);
    QVariantMap info {
        { QStringLiteral("path"), libraryPath },
    };

    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(libraryPath));
        connectionName = db.connectionName();

        // num folders
        auto foldersQuery = db.exec("SELECT COUNT(*) FROM folder WHERE id <> 1");
        foldersQuery.next();
        info.insert(QStringLiteral("folderCount"), foldersQuery.value(0));

        // num comics
        auto comicsQuery = db.exec("SELECT COUNT(*) FROM comic");
        comicsQuery.next();
        info.insert(QStringLiteral("comicCount"), comicsQuery.value(0));

        // num read comics
        auto readComicsQuery = db.exec("SELECT count(*) FROM comic c INNER JOIN comic_info ci ON c.comicInfoId = ci.id WHERE ci.read = 1");
        readComicsQuery.next();
        info.insert(QStringLiteral("readComicCount"), readComicsQuery.value(0));
    }
    QSqlDatabase::removeDatabase(connectionName);

    return info;
}

QString DBHelper::getLibraryInfo(QUuid id)
{
    const auto libraryInfo = getLibraryInfoData(id);
    QString info = "<b>Library path</b>:<br/>" + libraryInfo.value(QStringLiteral("path")).toString() + "<br/><br/>";
    info += "<b>Number of folders</b>:<br/>" + libraryInfo.value(QStringLiteral("folderCount")).toString() + "<br/><br/>";
    info += "<b>Number of comics</b>:<br/>" + libraryInfo.value(QStringLiteral("comicCount")).toString() + "<br/><br/>";
    info += "<b>Number of read comics</b>:<br/>" + libraryInfo.value(QStringLiteral("readComicCount")).toString() + "<br/><br/>";
    return info;
}
