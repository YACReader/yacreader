#include "comic_info_repairer.h"

#include "QsLog.h"
#include "comic_db.h"
#include "cover_utils.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "initial_comic_info_extractor.h"
#include "xml_info_parser.h"
#include "yacreader_global.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QLibrary>
#include <QLockFile>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSysInfo>

#include <cmath>
#include <memory>

using namespace YACReader;

namespace {

// deterministic suffixes so leftovers from an interrupted repair can be swept on the next run
// the temporary cover keeps the .jpg suffix so QImage::save can infer the format
constexpr auto repairNewCoverSuffix = ".repair-new.jpg";
constexpr auto repairOldCoverSuffix = ".repair-old";

enum class RepairCase {
    InvalidPageCount,
    MissingCover,
    MissingCoverRatio,
    Healthy
};

// Everything a repair needs to know about a broken comic; the full comic_info
// row is only loaded lazily, one comic at a time, when a repair requires it.
struct RepairTask {
    RepairCase repairCase { RepairCase::Healthy };
    qulonglong id { 0 };
    QString hash;
    int coverPage { 1 };
    bool usesExternalCover { false };
    QStringList relativePaths;
};

struct ExtractedInfo {
    int numPages { 0 };
    QImage cover;
    QPair<int, int> originalCoverSize { 0, 0 };
    QByteArray xmlData;
    bool hasValidCover { false };
};

bool hasValidRatio(const QVariant &coverSizeRatio)
{
    bool converted = false;
    const auto ratio = coverSizeRatio.toDouble(&converted);
    return converted && std::isfinite(ratio) && ratio > 0;
}

RepairCase classify(const QVariant &numPages, const QVariant &coverSizeRatio, bool coverExists)
{
    if (numPages.isNull() || numPages.toInt() <= 0) {
        return RepairCase::InvalidPageCount;
    }

    if (!coverExists) {
        return RepairCase::MissingCover;
    }

    if (!hasValidRatio(coverSizeRatio)) {
        return RepairCase::MissingCoverRatio;
    }

    return RepairCase::Healthy;
}

// keeps the peak memory usage and the lifetime of the scan cursor bounded no
// matter how many comics need repair
constexpr int scanBatchSize = 1000;

struct ScanResult {
    QList<RepairTask> tasks;
    qulonglong lastScannedId { 0 };
    bool scanComplete { true };
};

// Streams over the comics with id > afterId classifying them on the fly and
// keeps only the broken ones, stopping after scanBatchSize tasks. The scan
// only stops at group boundaries, so a comic available at several paths is
// never split across batches.
ScanResult findRepairTasks(QSqlDatabase &db, const QString &target, qulonglong afterId, const std::atomic_bool &stopRequested, QString &error)
{
    ScanResult result;
    result.lastScannedId = afterId;

    QSqlQuery query(db);
    // without this the SQLite driver caches every visited row to support
    // backward navigation, defeating the batched scan's memory bound
    query.setForwardOnly(true);
    query.prepare("SELECT ci.id, ci.hash, ci.numPages, ci.coverSizeRatio, ci.coverPage, ci.usesExternalCover, c.path "
                  "FROM comic_info ci "
                  "INNER JOIN comic c ON c.comicInfoId = ci.id "
                  "WHERE ci.id > :afterId "
                  "ORDER BY ci.id, c.id");
    query.bindValue(":afterId", afterId);

    if (!query.exec()) {
        error = query.lastError().text();
        QLOG_ERROR() << "Unable to load comics for repair:" << error;
        return result;
    }

    qulonglong lastSeenId = afterId;
    bool lastNeedsRepair = false;
    while (query.next()) {
        if (stopRequested.load(std::memory_order_acquire)) {
            result.scanComplete = false;
            break;
        }

        const auto id = query.value(0).toULongLong();
        const auto relativePath = query.value(6).toString();

        // rows are grouped by comic_info id, extra rows of a group are extra paths of the same comic
        if (id == lastSeenId) {
            if (lastNeedsRepair) {
                result.tasks.last().relativePaths.append(relativePath);
            }
            continue;
        }

        if (result.tasks.size() >= scanBatchSize) {
            // batch full, the next batch will resume from the last fully scanned comic
            result.scanComplete = false;
            break;
        }
        lastSeenId = id;

        const auto hash = query.value(1).toString();
        const auto coverExists = QFile::exists(LibraryPaths::coverPathFromLibraryDataPath(target, hash));
        const auto repairCase = classify(query.value(2), query.value(3), coverExists);

        lastNeedsRepair = repairCase != RepairCase::Healthy;
        if (lastNeedsRepair) {
            RepairTask task;
            task.repairCase = repairCase;
            task.id = id;
            task.hash = hash;
            task.coverPage = query.value(4).toInt();
            task.usesExternalCover = query.value(5).toBool();
            task.relativePaths.append(relativePath);
            result.tasks.append(task);
        }
    }

    result.lastScannedId = lastSeenId;
    return result;
}

QString existingComicPath(const QString &source, const QStringList &relativePaths, QString &relativePath)
{
    for (const auto &path : relativePaths) {
        const auto absolutePath = QDir::cleanPath(source + path);
        const QFileInfo fileInfo(absolutePath);
        if (fileInfo.exists() && fileInfo.isFile()) {
            relativePath = path;
            return absolutePath;
        }
    }

    relativePath = relativePaths.value(0);
    return { };
}

ExtractedInfo extractInfo(const QString &comicPath, int coverPage, bool importXmlMetadata)
{
    if (coverPage <= 0) {
        coverPage = 1;
    }

    InitialComicInfoExtractor extractor(comicPath, QString(), coverPage, importXmlMetadata);
    extractor.extract();

    ExtractedInfo result;
    result.numPages = extractor.getNumPages();
    result.cover = extractor.getCoverImage();
    result.originalCoverSize = extractor.getOriginalCoverSize();
    result.xmlData = extractor.getXMLInfoRawData();
    result.hasValidCover = extractor.hasValidCover();

    if (result.numPages > 0 && !result.hasValidCover && coverPage != 1) {
        InitialComicInfoExtractor fallbackExtractor(comicPath, QString(), 1, importXmlMetadata);
        fallbackExtractor.extract();

        result.numPages = fallbackExtractor.getNumPages();
        result.cover = fallbackExtractor.getCoverImage();
        result.originalCoverSize = fallbackExtractor.getOriginalCoverSize();
        if (result.xmlData.isEmpty()) {
            result.xmlData = fallbackExtractor.getXMLInfoRawData();
        }
        result.hasValidCover = fallbackExtractor.hasValidCover();
    }

    return result;
}

QString coverSizeString(const QPair<int, int> &size)
{
    return QString("%1x%2").arg(size.first).arg(size.second);
}

double coverRatio(const QPair<int, int> &size)
{
    return static_cast<double>(size.first) / static_cast<double>(size.second);
}

void bindXmlMetadata(QSqlQuery &query, const ComicInfo &info)
{
    for (const auto &field : xmlMetadataFields()) {
        const auto placeholder = QString(":") + field.column;
        if (qstrcmp(field.column, "type") == 0) {
            // type holds a FileType enum, store it as int like DBHelper::update does
            query.bindValue(placeholder, static_cast<int>(info.type.value<FileType>()));
        } else {
            query.bindValue(placeholder, info.*(field.member));
        }
    }
}

void prepareInvalidPageCountUpdate(QSqlQuery &query,
                                   const ComicInfo &info,
                                   const ExtractedInfo &extracted,
                                   bool preserveCustomCover,
                                   const QImage &customCover,
                                   bool xmlParsed,
                                   const ComicInfo &updatedInfo)
{
    QStringList assignments { "numPages = :numPages" };

    if (preserveCustomCover) {
        if (!hasValidRatio(info.coverSizeRatio)) {
            assignments.append("coverSizeRatio = :coverSizeRatio");
        }
    } else {
        assignments.append("originalCoverSize = :originalCoverSize");
        assignments.append("coverSizeRatio = :coverSizeRatio");
        if (info.usesExternalCover.toBool()) {
            assignments.append("usesExternalCover = 0");
            assignments.append("lastTimeCoverSet = 0");
        }
    }

    if (xmlParsed) {
        for (const auto &field : xmlMetadataFields()) {
            assignments.append(QString("%1 = :%1").arg(field.column));
        }
    }

    query.prepare(QString("UPDATE comic_info SET %1 WHERE id = :id").arg(assignments.join(", ")));
    query.bindValue(":numPages", extracted.numPages);
    query.bindValue(":id", info.id);

    if (preserveCustomCover) {
        if (!hasValidRatio(info.coverSizeRatio)) {
            query.bindValue(":coverSizeRatio", static_cast<double>(customCover.width()) / customCover.height());
        }
    } else {
        query.bindValue(":originalCoverSize", coverSizeString(extracted.originalCoverSize));
        query.bindValue(":coverSizeRatio", coverRatio(extracted.originalCoverSize));
    }

    if (xmlParsed) {
        bindXmlMetadata(query, updatedInfo);
    }
}

void prepareMissingCoverUpdate(QSqlQuery &query, const RepairTask &task, const ExtractedInfo &extracted)
{
    QStringList assignments {
        "originalCoverSize = :originalCoverSize",
        "coverSizeRatio = :coverSizeRatio",
    };
    if (task.usesExternalCover) {
        assignments.append("usesExternalCover = 0");
        assignments.append("lastTimeCoverSet = 0");
    }

    query.prepare(QString("UPDATE comic_info SET %1 WHERE id = :id").arg(assignments.join(", ")));
    query.bindValue(":originalCoverSize", coverSizeString(extracted.originalCoverSize));
    query.bindValue(":coverSizeRatio", coverRatio(extracted.originalCoverSize));
    query.bindValue(":id", task.id);
}

bool executeUpdate(QSqlDatabase &db, QSqlQuery &query)
{
    if (!db.transaction()) {
        QLOG_ERROR() << "Unable to start comic info repair transaction:" << db.lastError().text();
        return false;
    }

    if (!query.exec() || query.numRowsAffected() != 1) {
        QLOG_ERROR() << "Unable to update repaired comic info:" << query.lastError().text();
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        QLOG_ERROR() << "Unable to commit repaired comic info:" << db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

bool restorePreviousCover(const QString &coverPath, const QString &backupPath)
{
    if (QFile::exists(coverPath) && !QFile::remove(coverPath)) {
        QLOG_ERROR() << "Unable to remove failed repaired cover:" << coverPath;
        return false;
    }

    if (!backupPath.isEmpty() && !QFile::rename(backupPath, coverPath)) {
        QLOG_ERROR() << "Unable to restore previous cover:" << backupPath;
        return false;
    }

    return true;
}

bool executeUpdateWithCover(QSqlDatabase &db, QSqlQuery &query, const QString &coverPath, const QImage &cover)
{
    const auto temporaryPath = coverPath + repairNewCoverSuffix;
    const auto backupPath = QFile::exists(coverPath) ? coverPath + repairOldCoverSuffix : QString();

    if (!QDir().mkpath(QFileInfo(coverPath).absolutePath()) || !saveCover(temporaryPath, cover)) {
        QFile::remove(temporaryPath);
        QLOG_ERROR() << "Unable to save temporary repaired cover:" << temporaryPath;
        return false;
    }

    if (!db.transaction()) {
        QFile::remove(temporaryPath);
        QLOG_ERROR() << "Unable to start comic info repair transaction:" << db.lastError().text();
        return false;
    }

    if (!query.exec() || query.numRowsAffected() != 1) {
        QFile::remove(temporaryPath);
        db.rollback();
        QLOG_ERROR() << "Unable to update repaired comic info:" << query.lastError().text();
        return false;
    }

    if (!backupPath.isEmpty() && !QFile::rename(coverPath, backupPath)) {
        QFile::remove(temporaryPath);
        db.rollback();
        QLOG_ERROR() << "Unable to back up existing cover:" << coverPath;
        return false;
    }

    if (!QFile::rename(temporaryPath, coverPath)) {
        if (!backupPath.isEmpty()) {
            QFile::rename(backupPath, coverPath);
        }
        db.rollback();
        QLOG_ERROR() << "Unable to install repaired cover:" << coverPath;
        return false;
    }

    if (!db.commit()) {
        restorePreviousCover(coverPath, backupPath);
        db.rollback();
        QLOG_ERROR() << "Unable to commit repaired comic info:" << db.lastError().text();
        return false;
    }

    if (!backupPath.isEmpty()) {
        QFile::remove(backupPath);
    }
    return true;
}

}

ComicInfoRepairer::ComicInfoRepairer(QSettings *settings, QObject *parent)
    : QThread(parent), settings(settings)
{
}

void ComicInfoRepairer::repairLibrary(const QString &source, const QString &target, bool removeStaleLock)
{
    if (isRunning()) {
        return;
    }

    this->source = QDir::cleanPath(source);
    this->target = QDir::cleanPath(target);
    this->removeStaleLock = removeStaleLock;
    importXmlMetadata = settings && settings->value(IMPORT_COMIC_INFO_XML_METADATA, false).toBool();
    stopRequested.store(false, std::memory_order_release);
    repairSummary = { };
    start();
}

ComicInfoRepairSummary ComicInfoRepairer::summary() const
{
    return repairSummary;
}

void ComicInfoRepairer::stop()
{
    stopRequested.store(true, std::memory_order_release);
}

void ComicInfoRepairer::run()
{
#if !defined use_unarr && !defined use_libarchive
    std::unique_ptr<QLibrary> sevenzLibrary;
    bool sevenzLibraryLoaded = false;
    auto ensure7zLibraryLoaded = [&]() {
        if (!sevenzLibrary) {
            sevenzLibrary.reset(load7zLibrary());
            sevenzLibrary->setLoadHints(QLibrary::PreventUnloadHint);
            sevenzLibraryLoaded = sevenzLibrary->load();
        }
        return sevenzLibraryLoaded;
    };
#endif

    LibraryMaintenanceLock maintenanceLock(source);
    if (!maintenanceLock.tryLock(removeStaleLock)) {
        repairSummary.lockedByAnotherProcess = true;
        repairSummary.lockHolderInfo = maintenanceLock.holderInfo();
        repairSummary.lockHolderIsRunningLocally = maintenanceLock.holderIsRunningLocally();
        repairSummary.error = maintenanceLock.errorString();
        return;
    }

    QString backupError;
    if (!DataBaseManagement::backupLibrary(source, DatabaseBackupReason::BeforeRepair, &backupError)) {
        repairSummary.error = QString("Unable to back up library database: %1").arg(backupError);
        QLOG_ERROR() << repairSummary.error;
        emit failed(repairSummary.error);
        return;
    }

    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(target);
        if (!db.isValid() || !db.isOpen()) {
            repairSummary.error = QString("Unable to open database at: %1").arg(target);
            emit failed(repairSummary.error);
            return;
        }
        connectionName = db.connectionName();

        // sweep leftovers from a previously interrupted repair
        {
            const auto coversPath = LibraryPaths::libraryCoversPathFromLibraryDataPath(target);

            // a repair interrupted between backing up the old cover and installing
            // the new one leaves the only copy of the cover in the backup: restore
            // it instead of deleting it
            QDirIterator backups(coversPath, { QString("*") + repairOldCoverSuffix }, QDir::Files);
            while (backups.hasNext()) {
                const auto backupPath = backups.next();
                const auto coverPath = backupPath.chopped(qstrlen(repairOldCoverSuffix));
                if (QFile::exists(coverPath)) {
                    QFile::remove(backupPath);
                } else {
                    QFile::rename(backupPath, coverPath);
                }
            }

            QDirIterator leftovers(coversPath, { QString("*") + repairNewCoverSuffix }, QDir::Files);
            while (leftovers.hasNext()) {
                QFile::remove(leftovers.next());
            }
        }

        qulonglong afterId = 0;
        bool moreToScan = true;
        while (moreToScan) {
            if (stopRequested.load(std::memory_order_acquire)) {
                repairSummary.canceled = true;
                break;
            }

            QString scanError;
            const auto scan = findRepairTasks(db, target, afterId, stopRequested, scanError);
            if (!scanError.isEmpty()) {
                repairSummary.error = scanError;
                emit failed(repairSummary.error);
                break;
            }
            moreToScan = !scan.scanComplete;
            afterId = scan.lastScannedId;

            for (const auto &task : scan.tasks) {
                if (stopRequested.load(std::memory_order_acquire)) {
                    repairSummary.canceled = true;
                    break;
                }

                const auto repairCase = task.repairCase;
                const auto coverPath = LibraryPaths::coverPathFromLibraryDataPath(target, task.hash);
                auto relativePath = task.relativePaths.value(0);

                const auto comicPath = existingComicPath(source, task.relativePaths, relativePath);
                if (comicPath.isEmpty()) {
                    repairSummary.missingFiles++;
                    repairSummary.missingFilePaths.append(relativePath);
                    emit comicProcessed(relativePath, coverPath);
                    continue;
                }

#if !defined use_unarr && !defined use_libarchive
                if (repairCase == RepairCase::InvalidPageCount || repairCase == RepairCase::MissingCover) {
#ifndef NO_PDF
                    const bool needs7zLibrary = QFileInfo(comicPath).suffix().compare("pdf", Qt::CaseInsensitive) != 0;
#else
                    const bool needs7zLibrary = true;
#endif
                    if (needs7zLibrary && !ensure7zLibraryLoaded()) {
                        repairSummary.error = sevenzLibrary->errorString();
                        emit failed(repairSummary.error);
                        break;
                    }
                }
#endif

                bool repaired = false;
                if (repairCase == RepairCase::InvalidPageCount) {
                    // this repair needs the full comic_info row: it is the base the parsed XML metadata is merged over
                    const auto info = DBHelper::loadComicInfo(task.hash, db);
                    QImage customCover;
                    const bool preserveCustomCover = info.existOnDb && info.usesExternalCover.toBool() && customCover.load(coverPath);
                    const auto extracted = extractInfo(comicPath, task.coverPage, importXmlMetadata);

                    if (info.existOnDb && extracted.numPages > 0 && (preserveCustomCover || (extracted.hasValidCover && extracted.originalCoverSize.second > 0))) {
                        auto updatedInfo = info;
                        const bool xmlParsed = importXmlMetadata && parseXMLIntoInfo(extracted.xmlData, updatedInfo);
                        QSqlQuery update(db);
                        prepareInvalidPageCountUpdate(update, info, extracted, preserveCustomCover, customCover, xmlParsed, updatedInfo);
                        if (preserveCustomCover) {
                            repaired = executeUpdate(db, update);
                        } else {
                            repaired = executeUpdateWithCover(db, update, coverPath, extracted.cover);
                        }
                    }
                } else if (repairCase == RepairCase::MissingCover) {
                    const auto extracted = extractInfo(comicPath, task.coverPage, false);
                    if (extracted.hasValidCover && extracted.originalCoverSize.second > 0) {
                        QSqlQuery update(db);
                        prepareMissingCoverUpdate(update, task, extracted);
                        repaired = executeUpdateWithCover(db, update, coverPath, extracted.cover);
                    }
                } else if (repairCase == RepairCase::MissingCoverRatio) {
                    QImage cover;
                    if (cover.load(coverPath) && cover.height() > 0) {
                        QSqlQuery update(db);
                        update.prepare("UPDATE comic_info SET coverSizeRatio = :coverSizeRatio WHERE id = :id");
                        update.bindValue(":coverSizeRatio", static_cast<double>(cover.width()) / cover.height());
                        update.bindValue(":id", task.id);
                        repaired = executeUpdate(db, update);
                    }
                }

                if (repaired) {
                    repairSummary.repaired++;
                } else {
                    repairSummary.failed++;
                    repairSummary.failedFilePaths.append(relativePath);
                }
                emit comicProcessed(relativePath, coverPath);
            }

            // propagate cancellation or a fatal error out of the batch loop
            if (repairSummary.canceled || !repairSummary.error.isEmpty()) {
                break;
            }
        }

        db.close();
    }

    if (!connectionName.isEmpty()) {
        QSqlDatabase::removeDatabase(connectionName);
    }
}
