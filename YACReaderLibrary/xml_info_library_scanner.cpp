#include "xml_info_library_scanner.h"

#include "comic_db.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "initial_comic_info_extractor.h"
#include "xml_info_parser.h"
#include "yacreader_global.h"
#include "folder_item.h"

#include "QsLog.h"

using namespace YACReader;

XMLInfoLibraryScanner::XMLInfoLibraryScanner()
    : QThread()
{
}

void XMLInfoLibraryScanner::scanLibrary(const QString &source, const QString &target)
{
    this->source = source;
    this->target = target;

    this->stopRunning = false;

    partialUpdate = false;

    start();
}

void XMLInfoLibraryScanner::scanFolder(const QString &source, const QString &target, const QString &folder, const QModelIndex &dest)
{
    this->source = source;
    this->target = target;

    this->stopRunning = false;

    partialUpdate = true;
    folderDestinationModelIndex = dest;

    start();
}

void XMLInfoLibraryScanner::run()
{
#if !defined use_unarr && !defined use_libarchive
// check for 7z lib
#if defined Q_OS_UNIX && !defined Q_OS_MAC
    QLibrary *sevenzLib = new QLibrary(QString(LIBDIR) + "/p7zip/7z.so");
#else
    QLibrary *sevenzLib = new QLibrary(QCoreApplication::applicationDirPath() + "/utils/7z");
#endif

    if (!sevenzLib->load()) {
        QLOG_ERROR() << "Loading 7z.dll : " + sevenzLib->errorString() << Qt::endl;
        QCoreApplication::exit(YACReader::SevenZNotFound);
        exit();
    }
    sevenzLib->deleteLater();
#endif

    QString databaseConnection;

    {
        auto database = DataBaseManagement::loadDatabase(this->target);
        databaseConnection = database.connectionName();

        database.transaction();

        if (!partialUpdate) {
            QSqlQuery comicsInfo("SELECT * FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id)", database);
            comicsInfo.exec();

            updateFromSQLQuery(database, comicsInfo);
        } else {
            if (folderDestinationModelIndex.isValid()) {
                YACReader::iterate(folderDestinationModelIndex, folderDestinationModelIndex.model(), [&](const QModelIndex &idx) {
                    if (stopRunning) {
                        return false;
                    }

                    auto item = static_cast<FolderItem *>(idx.internalPointer());

                    QSqlQuery comicsInfo(database);
                    comicsInfo.prepare("SELECT * FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) WHERE c.parentId = :parentId");
                    comicsInfo.bindValue(":parentId", item->id);
                    comicsInfo.exec();

                    updateFromSQLQuery(database, comicsInfo);

                    return true;
                });
            }
        }

        database.commit();
        database.close();
    }

    QSqlDatabase::removeDatabase(databaseConnection);
}

void XMLInfoLibraryScanner::stop()
{
    stopRunning = true;
}

void XMLInfoLibraryScanner::updateFromSQLQuery(QSqlDatabase &db, QSqlQuery &query)
{
    QSqlRecord record = query.record();

    int id = record.indexOf("id");
    // int parentIdIndex = record.indexOf("parentId");
    int fileNameIndex = record.indexOf("fileName");
    int pathIndex = record.indexOf("path");

    while (query.next()) {
        if (this->stopRunning) {
            break;
        }
        auto fileName = query.value(fileNameIndex).toString();
        auto path = query.value(pathIndex).toString();

        emit comicScanned(path, fileName);

        auto info = DBHelper::getComicInfoFromQuery(query, "comicInfoId");

        InitialComicInfoExtractor ie(QDir::cleanPath(this->source + path), "None", info.coverPage.toInt(), true);

        ie.extract();

        if (parseXMLIntoInfo(ie.getXMLInfoRawData(), info)) {
            DBHelper::update(&info, db);
        }
    }
}
