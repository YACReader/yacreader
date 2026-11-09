#include "xml_info_library_scanner.h"

#include "QsLog.h"
#include "comic_db.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "initial_comic_info_extractor.h"
#include "xml_info_parser.h"
#include "yacreader_global.h"

#include <QSqlRecord>

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
    cancelRequested = false;

    partialUpdate = false;

    start();
}

void XMLInfoLibraryScanner::scanFolder(const QString &source, const QString &target, const QString &folder, const QModelIndex & /* dest */)
{
    this->source = source;
    this->target = target;

    this->stopRunning = false;
    cancelRequested = false;

    partialUpdate = true;
    const auto relative = QDir(source).relativeFilePath(folder);
    folderPath = relative == "." ? QString("/") : "/" + relative;

    start();
}

void XMLInfoLibraryScanner::run()
{
    scanError.clear();
    lockFailure = { };
    completed = false;
    if (cancelRequested)
        return;
    LibraryMaintenanceLock maintenance(source);
    if (!maintenance.tryLock()) {
        lockFailure = maintenance.failure();
        scanError = maintenance.errorString();
        emit failed(scanError);
        return;
    }
    if (!DataBaseManagement::recoverInterruptedRestore(source, &scanError, true) || !QFile::exists(LibraryPaths::libraryDatabasePath(source))) {
        if (scanError.isEmpty())
            scanError = tr("The library database is missing. Restore a backup or recreate the library before rescanning XML information.");
        emit failed(scanError);
        return;
    }
    if (!DataBaseManagement::backupLibrary(source, DatabaseBackupReason::BeforeRepair, &scanError)) {
        emit failed(scanError);
        return;
    }
#if !defined use_unarr && !defined use_libarchive
    auto sevenzLib = YACReader::load7zLibrary();

    if (!sevenzLib->load()) {
        scanError = tr("Unable to load the archive backend: %1").arg(sevenzLib->errorString());
        emit failed(scanError);
        sevenzLib->deleteLater();
        return;
    }
    sevenzLib->deleteLater();
#endif

    QString databaseConnection;

    {
        auto database = DataBaseManagement::loadDatabase(this->target);
        databaseConnection = database.connectionName();

        if (!database.isOpen() || !database.transaction()) {
            scanError = tr("Unable to start an XML rescan transaction.");
        } else {
            QSqlQuery comicsInfo(database);
            auto sql = "SELECT " + ComicDB::selectList("c", "ci") + " FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id)";
            bool folderFound = true;
            if (partialUpdate) {
                QSqlQuery folder(database);
                folder.prepare("SELECT id FROM folder WHERE path = :path");
                folder.bindValue(":path", folderPath);
                folderFound = folder.exec() && folder.next();
                sql = "WITH RECURSIVE folders(id) AS (SELECT id FROM folder WHERE path = :path UNION SELECT f.id FROM folder f JOIN folders p ON f.parentId = p.id) " + sql + " WHERE c.parentId IN (SELECT id FROM folders)";
            }
            comicsInfo.prepare(sql);
            if (partialUpdate)
                comicsInfo.bindValue(":path", folderPath);
            if (!folderFound)
                scanError = tr("The selected folder is no longer in this library. Reload the library before rescanning.");
            else if (!comicsInfo.exec())
                scanError = tr("Unable to read comics for the XML rescan.");
            else if (updateFromSQLQuery(database, comicsInfo) && !cancelRequested) {
                completed = database.commit();
                if (!completed)
                    scanError = tr("Unable to commit XML metadata changes.");
            }
            if (!completed)
                database.rollback();
        }
        database.close();
    }

    QSqlDatabase::removeDatabase(databaseConnection);
    if (!scanError.isEmpty())
        emit failed(scanError);
}

void XMLInfoLibraryScanner::stop()
{
    stopRunning = true;
}

void XMLInfoLibraryScanner::cancel()
{
    cancelRequested = true;
    stopRunning = true;
}

bool XMLInfoLibraryScanner::updateFromSQLQuery(QSqlDatabase &db, QSqlQuery &query)
{
    const auto indexes = ComicDB::columnIndexes(query.record(), "c", "ci");
    while (query.next()) {
        if (this->stopRunning) {
            break;
        }
        const auto comic = ComicDB::fromQuery(query, indexes);
        const auto &path = comic.file.path;

        emit comicScanned(path, comic.file.fileName);

        auto info = comic.info;

        InitialComicInfoExtractor ie(QDir::cleanPath(this->source + path), "None", info.coverPage.toInt(), true);

        ie.extract();

        if (parseXMLIntoInfo(ie.getXMLInfoRawData(), info)) {
            if (!DBHelper::update(&info, db)) {
                scanError = tr("Unable to update XML metadata for %1.").arg(path);
                return false;
            }
        }
    }
    return true;
}
