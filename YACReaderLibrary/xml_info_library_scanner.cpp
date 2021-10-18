#include "xml_info_library_scanner.h"

#include "comic_db.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "initial_comic_info_extractor.h"
#include "xml_info_parser.h"
#include "yacreader_global.h"

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

    start();
}

void XMLInfoLibraryScanner::run()
{
#ifndef use_unarr
// check for 7z lib
#if defined Q_OS_UNIX && !defined Q_OS_MAC
    QLibrary *sevenzLib = new QLibrary(QString(LIBDIR) + "/p7zip/7z.so");
#else
    QLibrary *sevenzLib = new QLibrary(QCoreApplication::applicationDirPath() + "/utils/7z");
#endif

    if (!sevenzLib->load()) {
        QLOG_ERROR() << "Loading 7z.dll : " + sevenzLib->errorString() << endl;
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

        QSqlQuery comicsInfo("SELECT * FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id)", database);
        comicsInfo.exec();

        QSqlRecord record = comicsInfo.record();

        int id = record.indexOf("id");
        // int parentIdIndex = record.indexOf("parentId");
        int fileNameIndex = record.indexOf("fileName");
        int pathIndex = record.indexOf("path");

        while (comicsInfo.next()) {
            if (this->stopRunning) {
                break;
            }
            /* currentItem.id = selectQuery.value(id).toULongLong();
            currentItem.parentId = parentId; //selectQuery.value(parentId).toULongLong();
            currentItem.name = selectQuery.value(fileName).toString(); */
            auto fileName = comicsInfo.value(fileNameIndex).toString();
            auto path = comicsInfo.value(pathIndex).toString();

            emit comicScanned(path, fileName);

            auto info = DBHelper::getComicInfoFromQuery(comicsInfo, "comicInfoId");

            InitialComicInfoExtractor ie(QDir::cleanPath(this->source + path), "None");

            ie.extract();

            if (parseXMLIntoInfo(ie.getXMLInfoRawData(), info)) {
                DBHelper::update(&info, database);
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
