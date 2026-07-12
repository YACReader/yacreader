#include "data_base_management.h"

#include "QsLog.h"
#include "db_helper.h"
#include "initial_comic_info_extractor.h"

#include <QImageReader>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QtCore>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <cstdio>
#endif

using namespace YACReader;

namespace {
const auto backupTimestampFormat = QStringLiteral("yyyyMMdd-HHmmss");

QString backupReasonName(DatabaseBackupReason reason)
{
    switch (reason) {
    case DatabaseBackupReason::AutoUpdate:
        return "auto-update";
    case DatabaseBackupReason::BeforeUpgrade:
        return "before-upgrade";
    case DatabaseBackupReason::BeforeRepair:
        return "before-repair";
    case DatabaseBackupReason::BeforeRestore:
        return "before-restore";
    case DatabaseBackupReason::Manual:
        return "manual";
    }
    return { };
}

bool validateDatabase(const QString &path, QString *error, QString *versionOut = nullptr)
{
    QString connectionName;
    bool valid = false;
    {
        auto db = DataBaseManagement::loadDatabaseFromFile(path);
        connectionName = db.connectionName();
        if (!db.isOpen()) {
            if (error)
                *error = QString("Unable to open database: %1").arg(path);
        } else {
            QSqlQuery versionQuery(db);
            QSqlQuery check(db);
            valid = versionQuery.exec("SELECT version FROM db_info") && versionQuery.next() && check.exec("PRAGMA quick_check") && check.next() && check.value(0).toString() == "ok";
            if (valid && versionOut)
                *versionOut = versionQuery.value(0).toString();
            if (!valid && error)
                *error = QString("Database validation failed: %1").arg(path);
        }
    }
    if (!connectionName.isEmpty())
        QSqlDatabase::removeDatabase(connectionName);
    return valid;
}

bool vacuumDatabaseInto(const QString &sourcePath, const QString &destinationPath, QString *error)
{
    QString connectionName;
    bool created = false;
    {
        auto db = DataBaseManagement::loadDatabaseFromFile(sourcePath);
        connectionName = db.connectionName();
        if (!db.isOpen()) {
            if (error)
                *error = QString("Unable to open database: %1").arg(sourcePath);
        } else {
            QString escapedPath = QDir::toNativeSeparators(destinationPath);
            escapedPath.replace('\'', "''");
            QSqlQuery vacuum(db);
            created = vacuum.exec(QString("VACUUM main INTO '%1'").arg(escapedPath));
            if (!created && error)
                *error = vacuum.lastError().text();
        }
    }
    if (!connectionName.isEmpty())
        QSqlDatabase::removeDatabase(connectionName);
    return created;
}

QFileInfoList backupsForReason(const QDir &directory, const QString &reason)
{
    return directory.entryInfoList({ QString("library-*-*-%1.ydb").arg(reason) }, QDir::Files, QDir::Name | QDir::Reversed);
}

QDateTime backupDateTime(const QFileInfo &backup)
{
    constexpr qsizetype prefixLength = 8; // "library-"
    constexpr qsizetype timestampLength = 15;
    return QDateTime::fromString(backup.completeBaseName().mid(prefixLength, timestampLength), backupTimestampFormat);
}

void removeOldBackups(const QDir &directory, DatabaseBackupReason reason, const QString &newBackup, const QString &protectedBackup = { })
{
    const auto name = backupReasonName(reason);
    const auto backups = backupsForReason(directory, name);
    QSet<QString> keep;
    keep.insert(QFileInfo(newBackup).absoluteFilePath());
    if (!protectedBackup.isEmpty())
        keep.insert(QFileInfo(protectedBackup).absoluteFilePath());

    if (reason == DatabaseBackupReason::AutoUpdate) {
        const auto now = QDateTime::currentDateTime();
        QSet<QString> olderMonths;
        for (const auto &backup : backups) {
            const auto created = backupDateTime(backup);
            if (!created.isValid())
                continue;
            const auto age = created.daysTo(now);
            if (age < 14) {
                keep.insert(backup.absoluteFilePath());
            } else if (age < 14 + 366) {
                const auto month = created.toString("yyyy-MM");
                if (!olderMonths.contains(month)) {
                    olderMonths.insert(month);
                    keep.insert(backup.absoluteFilePath());
                }
            }
        }
    } else {
        int limit = reason == DatabaseBackupReason::Manual ? 10 : 3;
        for (int i = 0; i < qMin(limit, backups.size()); ++i)
            keep.insert(backups.at(i).absoluteFilePath());
    }

    for (const auto &backup : backups) {
        if (!keep.contains(backup.absoluteFilePath())) {
            QLOG_INFO() << "Removing old database backup" << backup.absoluteFilePath();
            QFile::remove(backup.absoluteFilePath());
        }
    }
}

const QStringList sqliteSidecarSuffixes { "-journal", "-wal", "-shm" };

QString restoreMarkerPath(const QString &databasePath)
{
    return databasePath + ".restore";
}

QString stagedDatabasePath(const QString &databasePath)
{
    return databasePath + ".staged";
}

QString rollbackDatabasePath(const QString &databasePath)
{
    return databasePath + ".rollback";
}

QString salvageDatabasePath(const QString &databasePath)
{
    return databasePath + ".salvage";
}

QString damagedDatabasePath(const QString &libraryPath)
{
    const auto recoveryDirectory = QDir(LibraryPaths::libraryDataPath(libraryPath)).filePath("recovery");
    return QDir(recoveryDirectory).filePath(QString("library-%1-damaged.ydb").arg(QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss-zzz")));
}

bool removeDatabaseUnit(const QString &databasePath)
{
    bool success = !QFile::exists(databasePath) || QFile::remove(databasePath);
    for (const auto &suffix : sqliteSidecarSuffixes) {
        const auto path = databasePath + suffix;
        success = (!QFile::exists(path) || QFile::remove(path)) && success;
    }
    return success;
}

bool moveDatabaseUnit(const QString &source, const QString &destination, QString *error)
{
    QStringList movedSuffixes;
    const QStringList suffixes = { QString(), "-journal", "-wal", "-shm" };
    for (const auto &suffix : suffixes) {
        const auto sourcePath = source + suffix;
        if (!QFile::exists(sourcePath))
            continue;

        const auto destinationPath = destination + suffix;
        if (QFile::exists(destinationPath) || !QFile::rename(sourcePath, destinationPath)) {
            for (auto it = movedSuffixes.crbegin(); it != movedSuffixes.crend(); ++it)
                QFile::rename(destination + *it, source + *it);
            if (error)
                *error = QString("Unable to move %1 to %2").arg(sourcePath, destinationPath);
            return false;
        }
        movedSuffixes.append(suffix);
    }
    return true;
}

bool copyDatabaseUnit(const QString &source, const QString &destination, QString *error)
{
    if (!QDir().mkpath(QFileInfo(destination).absolutePath())) {
        if (error)
            *error = QString("Unable to create recovery folder: %1").arg(QFileInfo(destination).absolutePath());
        return false;
    }

    const QStringList suffixes = { QString(), "-journal", "-wal", "-shm" };
    for (const auto &suffix : suffixes) {
        const auto sourcePath = source + suffix;
        if (!QFile::exists(sourcePath))
            continue;
        if (!QFile::copy(sourcePath, destination + suffix)) {
            removeDatabaseUnit(destination);
            if (error)
                *error = QString("Unable to preserve damaged database: %1").arg(sourcePath);
            return false;
        }
    }
    return true;
}

bool writeRestoreMarker(const QString &path, bool hadOriginal, const QString &stage, QString *error)
{
    QSaveFile marker(path);
    if (!marker.open(QIODevice::WriteOnly)) {
        if (error)
            *error = marker.errorString();
        return false;
    }
    marker.write(hadOriginal ? "original=1\n" : "original=0\n");
    marker.write("stage=" + stage.toUtf8() + "\n");
    if (!marker.commit()) {
        if (error)
            *error = marker.errorString();
        return false;
    }
    return true;
}

bool markerHadOriginal(const QString &path)
{
    QFile marker(path);
    return marker.open(QIODevice::ReadOnly) && marker.readAll().contains("original=1\n");
}

bool replaceFileAtomically(const QString &source, const QString &destination, QString *error)
{
#ifdef Q_OS_WIN
    const bool replaced = MoveFileExW(reinterpret_cast<LPCWSTR>(source.utf16()), reinterpret_cast<LPCWSTR>(destination.utf16()), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
#else
    const auto sourceName = QFile::encodeName(source);
    const auto destinationName = QFile::encodeName(destination);
    const bool replaced = std::rename(sourceName.constData(), destinationName.constData()) == 0;
#endif
    if (!replaced && error)
        *error = QString("Unable to replace destination file: %1").arg(destination);
    return replaced;
}
}

LibraryMaintenanceLock::LibraryMaintenanceLock(const QString &libraryPath)
    : maintenanceLock(std::make_unique<QLockFile>(QDir(LibraryPaths::libraryDataPath(libraryPath)).filePath("maintenance.lock"))), legacyRepairLock(std::make_unique<QLockFile>(QDir(LibraryPaths::libraryDataPath(libraryPath)).filePath("repair.lock")))
{
    maintenanceLock->setStaleLockTime(0);
    legacyRepairLock->setStaleLockTime(0);
}

LibraryMaintenanceLock::~LibraryMaintenanceLock() = default;

bool LibraryMaintenanceLock::tryLockFile(QLockFile &lock, bool removeStaleLock)
{
    if (lock.tryLock())
        return true;
    if (removeStaleLock && lock.removeStaleLockFile() && lock.tryLock())
        return true;
    captureLockInfo(lock);
    failedLockPath = lock.fileName();
    return false;
}

bool LibraryMaintenanceLock::tryLock(bool removeStaleLock)
{
    failedLockPath.clear();
    currentHolderInfo.clear();
    currentHolderIsRunningLocally = false;
    if (!tryLockFile(*maintenanceLock, removeStaleLock))
        return false;
    if (!tryLockFile(*legacyRepairLock, removeStaleLock)) {
        maintenanceLock->unlock();
        return false;
    }
    return true;
}

void LibraryMaintenanceLock::captureLockInfo(QLockFile &lock)
{
    qint64 pid = 0;
    QString hostname;
    QString appname;
    if (lock.getLockInfo(&pid, &hostname, &appname)) {
        currentHolderInfo = QString("%1 (PID %2) on %3").arg(appname).arg(pid).arg(hostname.isEmpty() ? QString("unknown host") : hostname);
        currentHolderIsRunningLocally = !hostname.isEmpty() && hostname == QSysInfo::machineHostName();
    }
}

QString LibraryMaintenanceLock::errorString() const
{
    return QString("Another maintenance operation is using this library (%1)%2")
            .arg(failedLockPath, currentHolderInfo.isEmpty() ? QString() : QString(": %1").arg(currentHolderInfo));
}

QString LibraryMaintenanceLock::holderInfo() const
{
    return currentHolderInfo;
}

bool LibraryMaintenanceLock::holderIsRunningLocally() const
{
    return currentHolderIsRunningLocally;
}

static QString fields = "title,"

                        "coverPage,"
                        "numPages,"

                        "number,"
                        "isBis,"
                        "count,"

                        "volume,"
                        "storyArc,"
                        "arcNumber,"
                        "arcCount,"

                        "genere,"

                        "writer,"
                        "penciller,"
                        "inker,"
                        "colorist,"
                        "letterer,"
                        "coverArtist,"

                        "date,"
                        "publisher,"
                        "format,"
                        "color,"
                        "ageRating,"

                        "synopsis,"
                        "characters,"
                        "notes,"

                        "hash,"
                        "edited,"
                        "read,"

                        "comicVineID,"

                        "hasBeenOpened,"
                        "rating,"
                        "currentPage,"
                        "bookmark1,"
                        "bookmark2,"
                        "bookmark3,"
                        "brightness,"
                        "contrast,"
                        "gamma,"
                        // new 7.1 fields
                        "comicVineID,"
                        // new 9.5 fields
                        "lastTimeOpened,"
                        //"coverSizeRatio," cover may have changed since the info was exported...
                        //"originalCoverSize," // h/w
                        // new 9.8 fields
                        // "manga," //removed in 9.13
                        // new 9.13 fields
                        "added,"
                        "type,"
                        "editor,"
                        "imprint,"
                        "teams,"
                        "locations,"
                        "series,"
                        "alternateSeries,"
                        "alternateNumber,"
                        "alternateCount,"
                        "languageISO,"
                        "seriesGroup,"
                        "mainCharacterOrTeam,"
                        "review,"
                        "tags,"
                        // new 9.16 fields
                        "imageFiltersJson,"
                        "lastTimeImageFiltersSet,"
                        "lastTimeCoverSet,"
                        "usesExternalCover,"
                        "lastTimeMetadataSet";

DataBaseManagement::DataBaseManagement()
    : QObject(), dataBasesList()
{
}

QSqlDatabase DataBaseManagement::createDatabase(QString name, QString path)
{
    return createDatabase(QDir::cleanPath(path) + "/" + name + ".ydb");
}

QSqlDatabase DataBaseManagement::createDatabase(QString dest)
{
    QString threadId = QString::number((long long)QThread::currentThreadId(), 16);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", dest + threadId);
    db.setDatabaseName(dest);
    if (!db.open())
        qDebug() << db.lastError();
    else {
        qDebug() << db.tables();
    }

    {
        QSqlQuery pragma("PRAGMA foreign_keys = ON", db);
        // pragma.finish();
        DataBaseManagement::createTables(db);

        QSqlQuery insertRootQuery(db);
        insertRootQuery.prepare("INSERT INTO folder (parentId, name, path, added) "
                                "VALUES (1, 'root', '/', :added)");
        insertRootQuery.bindValue(":added", QDateTime::currentSecsSinceEpoch());
        insertRootQuery.exec();
    }

    return db;
}

QSqlDatabase DataBaseManagement::loadDatabase(QString libraryDataPath)
{
    const QString dbPath = QDir::cleanPath(libraryDataPath + "/library.ydb");
    if (!QFile::exists(dbPath)) {
        return QSqlDatabase();
    }

    QString threadId = QString::number((quintptr)QThread::currentThreadId(), 16);
    QString connectionName = dbPath + threadId;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=2000");
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        const QString error = db.lastError().text();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        QLOG_ERROR() << "loadDatabase: failed to open" << dbPath << error;
        return QSqlDatabase();
    }
    QSqlQuery pragmaFK(db);
    if (!pragmaFK.exec("PRAGMA foreign_keys = ON")) {
        QLOG_ERROR() << "loadDatabase: failed to enable foreign keys for" << dbPath << pragmaFK.lastError().text();
    }

    return db;
}

QSqlDatabase DataBaseManagement::loadDatabaseFromFile(QString filePath)
{
    const QString dbPath = QDir::cleanPath(filePath);
    if (!QFile::exists(dbPath)) {
        return QSqlDatabase();
    }

    QString threadId = QString::number((quintptr)QThread::currentThreadId(), 16);
    QString connectionName = dbPath + threadId;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=2000");
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        // se devuelve una base de datos vacía e inválida

        const QString error = db.lastError().text();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        QLOG_ERROR() << "loadDatabaseFromFile: failed to open" << dbPath << error;
        return QSqlDatabase();
    }
    QSqlQuery pragmaFK(db);
    if (!pragmaFK.exec("PRAGMA foreign_keys = ON")) {
        QLOG_ERROR() << "loadDatabaseFromFile: failed to enable foreign keys for" << dbPath << pragmaFK.lastError().text();
    }

    return db;
}

bool DataBaseManagement::createTables(QSqlDatabase &database)
{
    bool success = true;

    {
        // COMIC INFO (representa la información de un cómic, cada cómic tendrá un idéntificador único formado por un hash sha1'de los primeros 512kb' + su tamaño en bytes)
        success = success && DataBaseManagement::createComicInfoTable(database, "comic_info");

        // FOLDER (representa una carpeta en disco)
        QSqlQuery queryFolder(database);
        queryFolder.prepare("CREATE TABLE folder ("
                            "id INTEGER PRIMARY KEY,"
                            "parentId INTEGER NOT NULL,"
                            "name TEXT NOT NULL,"
                            "path TEXT NOT NULL,"
                            // new 7.1 fields
                            "finished BOOLEAN DEFAULT 0," // reading
                            "completed BOOLEAN DEFAULT 1," // collecting
                            // new 9.5 fields
                            "numChildren INTEGER,"
                            "firstChildHash TEXT,"
                            "customImage TEXT,"
                            // new 9.8 fields
                            "manga BOOLEAN DEFAULT 0," // deprecated 9.13
                            // new 9.13 fields
                            "type INTEGER DEFAULT 0," // 0 = comic, 1 = manga, 2 = manga left to right, 3 = webcomic, 4 = 4koma
                            "added INTEGER,"
                            "updated INTEGER," // updated when the folder gets new content
                            "FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE)");
        success = success && queryFolder.exec();

        // COMIC (representa un cómic en disco, contiene el nombre de fichero)
        QSqlQuery queryComic(database);
        queryComic.prepare("CREATE TABLE comic (id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, comicInfoId INTEGER NOT NULL,  fileName TEXT NOT NULL, path TEXT, FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE, FOREIGN KEY(comicInfoId) REFERENCES comic_info(id))");
        success = success && queryComic.exec();
        // queryComic.finish();
        // DB INFO
        QSqlQuery queryDBInfo(database);
        queryDBInfo.prepare("CREATE TABLE db_info (version TEXT NOT NULL)");
        success = success && queryDBInfo.exec();
        // queryDBInfo.finish();

        QSqlQuery query("INSERT INTO db_info (version) "
                        "VALUES ('" DB_VERSION "')",
                        database);
        // query.finish();

        // 8.0> tables
        success = success && DataBaseManagement::createV8Tables(database);
    }

    return success;
}

bool DataBaseManagement::createComicInfoTable(QSqlDatabase &database, QString tableName)
{
    QSqlQuery queryComicInfo(database);
    queryComicInfo.prepare("CREATE TABLE " + tableName + " ("
                                                         "id INTEGER PRIMARY KEY,"
                                                         "title TEXT,"

                                                         "coverPage INTEGER DEFAULT 1,"
                                                         "numPages INTEGER,"

                                                         "number TEXT," // changed to text from INTEGER (9.13)
                                                         "isBis BOOLEAN,"
                                                         "count INTEGER,"

                                                         "volume TEXT,"
                                                         "storyArc TEXT,"
                                                         "arcNumber TEXT," // changed to text from INTEGER (9.13)
                                                         "arcCount INTEGER,"

                                                         "genere TEXT,"

                                                         "writer TEXT,"
                                                         "penciller TEXT,"
                                                         "inker TEXT,"
                                                         "colorist TEXT,"
                                                         "letterer TEXT,"
                                                         "coverArtist TEXT,"

                                                         "date TEXT," // publication date dd/mm/yyyy --> se mostrará en 3 campos diferentes
                                                         "publisher TEXT,"
                                                         "format TEXT,"
                                                         "color BOOLEAN,"
                                                         "ageRating TEXT,"

                                                         "synopsis TEXT,"
                                                         "characters TEXT,"
                                                         "notes TEXT,"

                                                         "hash TEXT UNIQUE NOT NULL,"
                                                         "edited BOOLEAN DEFAULT 0,"
                                                         "read BOOLEAN DEFAULT 0,"
                                                         // new 7.0 fields

                                                         "hasBeenOpened BOOLEAN DEFAULT 0,"
                                                         "rating REAL DEFAULT 0," // changed to REAL from INTEGER (9.13)
                                                         "currentPage INTEGER DEFAULT 1, "
                                                         "bookmark1 INTEGER DEFAULT -1, "
                                                         "bookmark2 INTEGER DEFAULT -1, "
                                                         "bookmark3 INTEGER DEFAULT -1, "
                                                         "brightness INTEGER DEFAULT -1, "
                                                         "contrast INTEGER DEFAULT -1, "
                                                         "gamma INTEGER DEFAULT -1, "
                                                         // new 7.1 fields
                                                         "comicVineID TEXT,"
                                                         // new 9.5 fields
                                                         "lastTimeOpened INTEGER,"
                                                         "coverSizeRatio REAL,"
                                                         "originalCoverSize STRING," // h/w
                                                         // new 9.8 fields
                                                         "manga BOOLEAN DEFAULT 0," // deprecated 9.13
                                                         // new 9.13 fields
                                                         "added INTEGER,"
                                                         "type INTEGER DEFAULT 0," // 0 = comic, 1 = manga, 2 = manga left to right, 3 = webcomic, 4 = 4koma
                                                         "editor TEXT,"
                                                         "imprint TEXT,"
                                                         "teams TEXT,"
                                                         "locations TEXT,"
                                                         "series TEXT,"
                                                         "alternateSeries TEXT,"
                                                         "alternateNumber TEXT,"
                                                         "alternateCount INTEGER,"
                                                         "languageISO TEXT,"
                                                         "seriesGroup TEXT,"
                                                         "mainCharacterOrTeam TEXT,"
                                                         "review TEXT,"
                                                         "tags TEXT,"
                                                         // new 9.16 fields
                                                         "imageFiltersJson TEXT,"
                                                         "lastTimeImageFiltersSet INTEGER DEFAULT 0,"
                                                         "lastTimeCoverSet INTEGER DEFAULT 0,"
                                                         "usesExternalCover BOOLEAN DEFAULT 0,"
                                                         "lastTimeMetadataSet INTEGER DEFAULT 0"
                                                         ")");

    return queryComicInfo.exec();
}

bool DataBaseManagement::createComicInfoTable9_14(QSqlDatabase &database, QString tableName)
{
    QSqlQuery queryComicInfo(database);
    queryComicInfo.prepare("CREATE TABLE " + tableName + " ("
                                                         "id INTEGER PRIMARY KEY,"
                                                         "title TEXT,"

                                                         "coverPage INTEGER DEFAULT 1,"
                                                         "numPages INTEGER,"

                                                         "number TEXT," // changed to text from INTEGER (9.13)
                                                         "isBis BOOLEAN,"
                                                         "count INTEGER,"

                                                         "volume TEXT,"
                                                         "storyArc TEXT,"
                                                         "arcNumber TEXT," // changed to text from INTEGER (9.13)
                                                         "arcCount INTEGER,"

                                                         "genere TEXT,"

                                                         "writer TEXT,"
                                                         "penciller TEXT,"
                                                         "inker TEXT,"
                                                         "colorist TEXT,"
                                                         "letterer TEXT,"
                                                         "coverArtist TEXT,"

                                                         "date TEXT," // publication date dd/mm/yyyy --> se mostrará en 3 campos diferentes
                                                         "publisher TEXT,"
                                                         "format TEXT,"
                                                         "color BOOLEAN,"
                                                         "ageRating TEXT,"

                                                         "synopsis TEXT,"
                                                         "characters TEXT,"
                                                         "notes TEXT,"

                                                         "hash TEXT UNIQUE NOT NULL,"
                                                         "edited BOOLEAN DEFAULT 0,"
                                                         "read BOOLEAN DEFAULT 0,"
                                                         // new 7.0 fields

                                                         "hasBeenOpened BOOLEAN DEFAULT 0,"
                                                         "rating REAL DEFAULT 0," // changed to REAL from INTEGER (9.13)
                                                         "currentPage INTEGER DEFAULT 1, "
                                                         "bookmark1 INTEGER DEFAULT -1, "
                                                         "bookmark2 INTEGER DEFAULT -1, "
                                                         "bookmark3 INTEGER DEFAULT -1, "
                                                         "brightness INTEGER DEFAULT -1, "
                                                         "contrast INTEGER DEFAULT -1, "
                                                         "gamma INTEGER DEFAULT -1, "
                                                         // new 7.1 fields
                                                         "comicVineID TEXT,"
                                                         // new 9.5 fields
                                                         "lastTimeOpened INTEGER,"
                                                         "coverSizeRatio REAL,"
                                                         "originalCoverSize STRING," // h/w
                                                         // new 9.8 fields
                                                         "manga BOOLEAN DEFAULT 0," // deprecated 9.13
                                                         // new 9.13 fields
                                                         "added INTEGER,"
                                                         "type INTEGER DEFAULT 0," // 0 = comic, 1 = manga, 2 = manga left to right, 3 = webcomic, 4 = 4koma
                                                         "editor TEXT,"
                                                         "imprint TEXT,"
                                                         "teams TEXT,"
                                                         "locations TEXT,"
                                                         "series TEXT,"
                                                         "alternateSeries TEXT,"
                                                         "alternateNumber TEXT,"
                                                         "alternateCount INTEGER,"
                                                         "languageISO TEXT,"
                                                         "seriesGroup TEXT,"
                                                         "mainCharacterOrTeam TEXT,"
                                                         "review TEXT,"
                                                         "tags TEXT"
                                                         ")");

    return queryComicInfo.exec();
}

bool DataBaseManagement::createV8Tables(QSqlDatabase &database)
{
    bool success = true;
    {
        // 8.0> tables
        // LABEL
        QSqlQuery queryLabel(database);
        success = success && queryLabel.exec("CREATE TABLE label (id INTEGER PRIMARY KEY, "
                                             "name TEXT NOT NULL, "
                                             "color TEXT NOT NULL, "
                                             "ordering INTEGER NOT NULL); "); // order depends on the color

        QSqlQuery queryIndexLabel(database);
        success = success && queryIndexLabel.exec("CREATE INDEX label_ordering_index ON label (ordering)");

        // COMIC LABEL
        QSqlQuery queryComicLabel(database);
        success = success && queryComicLabel.exec("CREATE TABLE comic_label ("
                                                  "comic_id INTEGER, "
                                                  "label_id INTEGER, "
                                                  "ordering INTEGER, " // TODO order????
                                                  "FOREIGN KEY(label_id) REFERENCES label(id) ON DELETE CASCADE, "
                                                  "FOREIGN KEY(comic_id) REFERENCES comic(id) ON DELETE CASCADE, "
                                                  "PRIMARY KEY(label_id, comic_id))");

        QSqlQuery queryIndexComicLabel(database);
        success = success && queryIndexComicLabel.exec("CREATE INDEX comic_label_ordering_index ON label (ordering)");

        // READING LIST
        QSqlQuery queryReadingList(database);
        success = success && queryReadingList.exec("CREATE TABLE reading_list ("
                                                   "id INTEGER PRIMARY KEY, "
                                                   "parentId INTEGER, "
                                                   "ordering INTEGER DEFAULT 0, " // only use it if the parentId is NULL
                                                   "name TEXT NOT NULL, "
                                                   "finished BOOLEAN DEFAULT 0, "
                                                   "completed BOOLEAN DEFAULT 1, "
                                                   "manga BOOLEAN DEFAULT 0, " // TODO never used, replace with `type`
                                                   "FOREIGN KEY(parentId) REFERENCES reading_list(id) ON DELETE CASCADE)");

        QSqlQuery queryIndexReadingList(database);
        success = success && queryIndexReadingList.exec("CREATE INDEX reading_list_ordering_index ON label (ordering)");

        // COMIC READING LIST
        QSqlQuery queryComicReadingList(database);
        success = success && queryComicReadingList.exec("CREATE TABLE comic_reading_list ("
                                                        "reading_list_id INTEGER, "
                                                        "comic_id INTEGER, "
                                                        "ordering INTEGER, "
                                                        "FOREIGN KEY(reading_list_id) REFERENCES reading_list(id) ON DELETE CASCADE, "
                                                        "FOREIGN KEY(comic_id) REFERENCES comic(id) ON DELETE CASCADE, "
                                                        "PRIMARY KEY(reading_list_id, comic_id))");

        QSqlQuery queryIndexComicReadingList(database);
        success = success && queryIndexComicReadingList.exec("CREATE INDEX comic_reading_list_ordering_index ON label (ordering)");

        // DEFAULT READING LISTS
        QSqlQuery queryDefaultReadingList(database);
        success = success && queryDefaultReadingList.exec("CREATE TABLE default_reading_list ("
                                                          "id INTEGER PRIMARY KEY, "
                                                          "name TEXT NOT NULL"
                                                          // TODO icon????
                                                          ")");

        // COMIC DEFAULT READING LISTS
        QSqlQuery queryComicDefaultReadingList(database);
        success = success && queryComicDefaultReadingList.exec("CREATE TABLE comic_default_reading_list ("
                                                               "comic_id INTEGER, "
                                                               "default_reading_list_id INTEGER, "
                                                               "ordering INTEGER, " // order????
                                                               "FOREIGN KEY(default_reading_list_id) REFERENCES default_reading_list(id) ON DELETE CASCADE, "
                                                               "FOREIGN KEY(comic_id) REFERENCES comic(id) ON DELETE CASCADE,"
                                                               "PRIMARY KEY(default_reading_list_id, comic_id))");

        QSqlQuery queryIndexComicDefaultReadingList(database);
        success = success && queryIndexComicDefaultReadingList.exec("CREATE INDEX comic_default_reading_list_ordering_index ON label (ordering)");

        // INSERT DEFAULT READING LISTS
        QSqlQuery queryInsertDefaultReadingList(database);
        // if(!queryInsertDefaultReadingList.prepare())

        // 1 Favorites
        // queryInsertDefaultReadingList.bindValue(":name", "Favorites");
        success = success && queryInsertDefaultReadingList.exec("INSERT INTO default_reading_list (name) VALUES (\"Favorites\")");

        // Reading doesn't need its onw list
    }
    return success;
}

void DataBaseManagement::exportComicsInfo(QString source, QString dest)
{
    QString connectionName = "";
    {
        QSqlDatabase destDB = loadDatabaseFromFile(dest);

        QSqlQuery attach(destDB);
        attach.prepare("ATTACH DATABASE '" + QDir().toNativeSeparators(dest) + "' AS dest;");
        attach.exec();

        QSqlQuery attach2(destDB);
        attach2.prepare("ATTACH DATABASE '" + QDir().toNativeSeparators(source) + "' AS source;");
        attach2.exec();

        QSqlQuery queryDBInfo(destDB);
        queryDBInfo.prepare("CREATE TABLE dest.db_info (version TEXT NOT NULL)");
        queryDBInfo.exec();

        QSqlQuery query("INSERT INTO dest.db_info (version) "
                        "VALUES ('" DB_VERSION "')",
                        destDB);
        query.exec();

        QSqlQuery exportData(destDB);
        exportData.prepare("CREATE TABLE dest.comic_info AS SELECT " + fields +
                           " FROM source.comic_info WHERE source.comic_info.edited = 1 OR source.comic_info.comicVineID IS NOT NULL");
        exportData.exec();
        connectionName = destDB.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
}

// TODO_METADATA: validate imported info
bool DataBaseManagement::importComicsInfo(QString source, QString dest)
{
    QString error;
    QString driver;
    QStringList hashes;

    bool b = false;

    QString sourceDBconnection = "";
    QString destDBconnection = "";

    {
        QSqlDatabase sourceDB = loadDatabaseFromFile(source);
        QSqlDatabase destDB = loadDatabaseFromFile(dest);

        QSqlQuery pragma("PRAGMA synchronous=OFF", destDB);

        QSqlQuery newInfo(sourceDB);
        newInfo.prepare("SELECT * FROM comic_info");
        newInfo.exec();
        destDB.transaction();
        int cp;
        while (newInfo.next()) // cada tupla deberá ser insertada o actualizada
        {
            QSqlQuery update(destDB);
            update.prepare("UPDATE comic_info SET "
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

                           //"coverSizeRatio = :coverSizeRatio,"
                           //"originalCoverSize = :originalCoverSize,"
                           //--

                           // new 9.8 fields
                           // "manga = :manga," //removed in 9.13

                           // new 9.13 fields
                           "added = :added,"
                           "type = :type," // 0 = comic, 1 = manga, 2 = manga left to right, 3 = webcomic,
                           "editor = :editor,"
                           "imprint = :imprint,"
                           "teams = :teams,"
                           "locations = :locations,"
                           "series = :series,"
                           "alternateSeries = :alternateSeries,"
                           "alternateNumber = :alternateNumber,"
                           "alternateCount = :alternateCount,"
                           "languageISO = :languageISO,"
                           "seriesGroup = :seriesGroup,"
                           "mainCharacterOrTeam = :mainCharacterOrTeam,"
                           "review = :review,"
                           "tags = :tags"

                           //--
                           " WHERE hash = :hash ");

            QSqlQuery insert(destDB);
            insert.prepare("INSERT INTO comic_info "
                           "(title,"
                           "coverPage,"
                           "numPages,"
                           "number,"
                           "isBis,"
                           "count,"
                           "volume,"
                           "storyArc,"
                           "arcNumber,"
                           "arcCount,"
                           "genere,"
                           "writer,"
                           "penciller,"
                           "inker,"
                           "colorist,"
                           "letterer,"
                           "coverArtist,"
                           "date,"
                           "publisher,"
                           "format,"
                           "color,"
                           "ageRating,"
                           "synopsis,"
                           "characters,"
                           "notes,"
                           "read,"
                           "edited,"
                           "comicVineID,"
                           "lastTimeOpened,"
                           "coverSizeRatio,"
                           "added,"
                           "type,"
                           "editor,"
                           "imprint,"
                           "teams,"
                           "locations,"
                           "series,"
                           "alternateSeries,"
                           "alternateNumber,"
                           "alternateCount,"
                           "languageISO,"
                           "seriesGroup,"
                           "mainCharacterOrTeam,"
                           "review,"
                           "hash)"

                           "VALUES (:title,"
                           ":coverPage,"
                           ":numPages,"
                           ":number,"
                           ":isBis,"
                           ":count,"

                           ":volume,"
                           ":storyArc,"
                           ":arcNumber,"
                           ":arcCount,"

                           ":genere,"

                           ":writer,"
                           ":penciller,"
                           ":inker,"
                           ":colorist,"
                           ":letterer,"
                           ":coverArtist,"

                           ":date,"
                           ":publisher,"
                           ":format,"
                           ":color,"
                           ":ageRating,"

                           ":synopsis,"
                           ":characters,"
                           ":notes,"

                           ":read,"
                           ":edited,"
                           ":comicVineID,"

                           ":lastTimeOpened,"

                           ":coverSizeRatio,"
                           ":originalCoverSize,"

                           ":added,"
                           ":type,"
                           ":editor,"
                           ":imprint,"
                           ":teams,"
                           ":locations,"
                           ":series,"
                           ":alternateSeries,"
                           ":alternateNumber,"
                           ":alternateCount,"
                           ":languageISO,"
                           ":seriesGroup,"
                           ":mainCharacterOrTeam,"
                           ":review,"
                           ":tags,"

                           ":hash )");

            QSqlRecord record = newInfo.record();
            cp = record.value("coverPage").toInt();
            if (cp > 1) {
                QSqlQuery checkCoverPage(destDB);
                checkCoverPage.prepare("SELECT coverPage FROM comic_info where hash = :hash");
                checkCoverPage.bindValue(":hash", record.value("hash").toString());
                checkCoverPage.exec();
                bool extract = false;
                if (checkCoverPage.next()) {
                    extract = checkCoverPage.record().value("coverPage").toInt() != cp;
                }
                if (extract)
                    hashes.append(record.value("hash").toString());
            }

            bindValuesFromRecord(record, update);

            update.bindValue(":edited", 1);

            update.exec();

            if (update.numRowsAffected() == 0) {

                bindValuesFromRecord(record, insert);
                insert.bindValue(":edited", 1);
                insert.bindValue(":read", 0);

                insert.exec();
            }
        }

        destDB.commit();
        for (const auto &hash : hashes) {
            QSqlQuery getComic(destDB);
            getComic.prepare("SELECT c.path,ci.coverPage FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) where ci.hash = :hash");
            getComic.bindValue(":hash", hash);
            getComic.exec();
            if (getComic.next()) {
                QString basePath = QString(dest).remove("/.yacreaderlibrary/library.ydb");
                QString path = basePath + getComic.record().value("path").toString();
                int coverPage = getComic.record().value("coverPage").toInt();
                auto coverPath = LibraryPaths::coverPath(basePath, hash);
                InitialComicInfoExtractor ie(path, coverPath, coverPage);
                ie.extract();
            }
        }
        sourceDBconnection = sourceDB.connectionName();
        destDBconnection = destDB.connectionName();
    }

    QSqlDatabase::removeDatabase(sourceDBconnection);
    QSqlDatabase::removeDatabase(destDBconnection);

    return b;
}

// TODO: update fields
// TODO fix these bindings
void DataBaseManagement::bindValuesFromRecord(const QSqlRecord &record, QSqlQuery &query)
{
    bindValue("title", record, query);

    bindValue("coverPage", record, query);
    bindValue("numPages", record, query);

    bindValue("number", record, query);
    bindValue("isBis", record, query);
    bindValue("count", record, query);

    bindValue("volume", record, query);
    bindValue("storyArc", record, query);
    bindValue("arcNumber", record, query);
    bindValue("arcCount", record, query);

    bindValue("genere", record, query);

    bindValue("writer", record, query);
    bindValue("penciller", record, query);
    bindValue("inker", record, query);
    bindValue("colorist", record, query);
    bindValue("letterer", record, query);
    bindValue("coverArtist", record, query);

    bindValue("date", record, query);
    bindValue("publisher", record, query);
    bindValue("format", record, query);
    bindValue("color", record, query);
    bindValue("ageRating", record, query);

    bindValue("synopsis", record, query);
    bindValue("characters", record, query);
    bindValue("notes", record, query);

    bindValue("read", record, query);
    bindValue("edited", record, query);

    bindValue("hasBeenOpened", record, query);
    bindValue("currentPage", record, query);
    bindValue("publisher", record, query);
    bindValue("bookmark1", record, query);
    bindValue("bookmark2", record, query);
    bindValue("bookmark3", record, query);
    bindValue("brightness", record, query);
    bindValue("contrast", record, query);
    bindValue("gamma", record, query);
    bindValue("rating", record, query);

    bindValue("comicVineID", record, query);

    bindValue("lastTimeOpened", record, query);

    bindValue("coverSizeRatio", record, query);
    bindValue("originalCoverSize", record, query);

    bindValue("added", record, query);
    bindValue("type", record, query);
    bindValue("editor", record, query);
    bindValue("imprint", record, query);
    bindValue("teams", record, query);
    bindValue("locations", record, query);
    bindValue("series", record, query);
    bindValue("alternateSeries", record, query);
    bindValue("alternateNumber", record, query);
    bindValue("alternateCount", record, query);
    bindValue("languageISO", record, query);
    bindValue("seriesGroup", record, query);
    bindValue("mainCharacterOrTeam", record, query);
    bindValue("review", record, query);
    bindValue("tags", record, query);

    bindValue("hash", record, query);
}

bool DataBaseManagement::addColumns(const QString &tableName, const QStringList &columnDefs, const QSqlDatabase &db)
{
    QString sql = "ALTER TABLE %1 ADD COLUMN %2";
    bool returnValue = true;

    for (const auto &columnDef : columnDefs) {
        QSqlQuery alterTable(db);
        alterTable.prepare(sql.arg(tableName).arg(columnDef));
        // alterTableComicInfo.bindValue(":column_def",columnDef);
        bool exec = alterTable.exec();
        returnValue = returnValue && exec;
        if (!exec) {
            QLOG_ERROR() << alterTable.lastError().text();
        }
        // returnValue = returnValue && (alterTable.numRowsAffected() > 0);
    }

    return returnValue;
}

bool DataBaseManagement::addConstraint(const QString &tableName, const QString &constraint, const QSqlDatabase &db)
{
    QString sql = "ALTER TABLE %1 ADD %2";
    bool returnValue = true;

    QSqlQuery alterTable(db);
    alterTable.prepare(sql.arg(tableName).arg(constraint));
    alterTable.exec();
    returnValue = returnValue && (alterTable.numRowsAffected() > 0);

    return returnValue;
}

void DataBaseManagement::bindValue(const QString &name, const QSqlRecord &record, QSqlQuery &query)
{
    if (!record.value(name).isNull()) {
        query.bindValue(":" + name, record.value(name));
    }
}

QString DataBaseManagement::checkValidDB(const QString &fullPath)
{
    QString versionString = "";
    QString connectionName = "";
    {
        QSqlDatabase db = loadDatabaseFromFile(fullPath);

        if (db.isValid() && db.isOpen()) {
            QSqlQuery version(db);
            version.prepare("SELECT * FROM db_info");
            version.exec();

            if (version.next())
                versionString = version.record().value("version").toString();
        }
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);

    return versionString;
}

int DataBaseManagement::compareVersions(const QString &v1, const QString v2)
{
    QStringList v1l = v1.split('.');
    QStringList v2l = v2.split('.');
    QList<int> v1il;
    QList<int> v2il;

    for (const auto &s : v1l)
        v1il.append(s.toInt());

    for (const auto &s : v2l)
        v2il.append(s.toInt());

    for (int i = 0; i < qMin(v1il.length(), v2il.length()); i++) {
        if (v1il[i] < v2il[i])
            return -1;
        if (v1il[i] > v2il[i])
            return 1;
    }

    if (v1il.length() < v2il.length())
        return -1;
    if (v1il.length() == v2il.length())
        return 0;
    if (v1il.length() > v2il.length())
        return 1;

    return 0;
}

bool DataBaseManagement::updateToCurrentVersion(const QString &libraryPath, bool maintenanceLockHeld)
{
    std::unique_ptr<LibraryMaintenanceLock> lock;
    if (!maintenanceLockHeld) {
        lock = std::make_unique<LibraryMaintenanceLock>(libraryPath);
        if (!lock->tryLock()) {
            QLOG_ERROR() << "Database upgrade blocked:" << lock->errorString();
            return false;
        }
    }

    QString libraryDatabasePath = LibraryPaths::libraryDatabasePath(libraryPath);
    QString backupError;
    if (!backupLibrary(libraryPath, DatabaseBackupReason::BeforeUpgrade, &backupError)) {
        QLOG_ERROR() << "Database upgrade blocked:" << backupError;
        return false;
    }

    const auto oldVersion = DataBaseManagement::checkValidDB(libraryDatabasePath);
    const bool pre7 = compareVersions(oldVersion, "7.0.0") < 0;
    const bool pre7_1 = compareVersions(oldVersion, "7.0.3") < 0;
    const bool pre8 = compareVersions(oldVersion, "8.0.0") < 0;
    const bool pre9_5 = compareVersions(oldVersion, "9.5.0") < 0;
    const bool pre9_8 = compareVersions(oldVersion, "9.8.0") < 0;
    const bool pre9_13 = compareVersions(oldVersion, "9.13.0") < 0;
    const bool pre9_14 = compareVersions(oldVersion, "9.14.0") < 0;
    const bool pre9_16 = compareVersions(oldVersion, "9.16.0") < 0;

    QString connectionName = "";
    bool returnValue = true;

    {
        QSqlDatabase db = loadDatabaseFromFile(libraryDatabasePath);
        if (db.isValid() && db.isOpen()) {

            QSqlQuery pragmaFKOFF("PRAGMA foreign_keys = OFF", db);

            if (!db.transaction()) {
                QLOG_ERROR() << "Failed to start transaction for database update";
                returnValue = false;
            } else {
                if (pre7) {
                    // new 7.0 fields
                    QStringList columnDefs;
                    columnDefs << "hasBeenOpened BOOLEAN DEFAULT 0"
                               << "rating INTEGER DEFAULT 0"
                               << "currentPage INTEGER DEFAULT 1"
                               << "bookmark1 INTEGER DEFAULT -1"
                               << "bookmark2 INTEGER DEFAULT -1"
                               << "bookmark3 INTEGER DEFAULT -1"
                               << "brightness INTEGER DEFAULT -1"
                               << "contrast INTEGER DEFAULT -1"
                               << "gamma INTEGER DEFAULT -1";

                    bool successAddingColumns = addColumns("comic_info", columnDefs, db);
                    returnValue = returnValue && successAddingColumns;
                }

                if (pre7_1) {
                    {
                        QStringList columnDefs;
                        columnDefs << "finished BOOLEAN DEFAULT 0"
                                   << "completed BOOLEAN DEFAULT 1";
                        bool successAddingColumns = addColumns("folder", columnDefs, db);
                        returnValue = returnValue && successAddingColumns;
                    }

                    { // comic_info
                        QStringList columnDefs;
                        columnDefs << "comicVineID TEXT DEFAULT NULL";
                        bool successAddingColumns = addColumns("comic_info", columnDefs, db);
                        returnValue = returnValue && successAddingColumns;
                    }
                }

                if (pre8) {
                    bool successCreatingNewTables = createV8Tables(db);
                    returnValue = returnValue && successCreatingNewTables;
                }

                if (pre9_5) {
                    { // folder
                        QStringList columnDefs;
                        // a full library update is needed after updating the table
                        columnDefs << "numChildren INTEGER";
                        columnDefs << "firstChildHash TEXT";
                        columnDefs << "customImage TEXT";
                        bool successAddingColumns = addColumns("folder", columnDefs, db);
                        returnValue = returnValue && successAddingColumns;
                    }

                    { // comic_info
                        QStringList columnDefs;
                        columnDefs << "lastTimeOpened INTEGER";
                        columnDefs << "coverSizeRatio REAL";
                        columnDefs << "originalCoverSize TEXT";
                        bool successAddingColumns = addColumns("comic_info", columnDefs, db);
                        returnValue = returnValue && successAddingColumns;

                        QSqlQuery queryIndexLastTimeOpened(db);
                        bool successCreatingIndex = queryIndexLastTimeOpened.exec("CREATE INDEX last_time_opened_index ON comic_info (lastTimeOpened)");
                        returnValue = returnValue && successCreatingIndex;
                    }

                    // update folders info
                    {
                        DBHelper::updateChildrenInfo(db);
                    }

                    {
                        QSqlQuery selectQuery(db);
                        selectQuery.prepare("SELECT id, hash FROM comic_info");
                        selectQuery.exec();

                        db.transaction();

                        QSqlQuery updateCoverInfo(db);
                        updateCoverInfo.prepare("UPDATE comic_info SET coverSizeRatio = :coverSizeRatio WHERE id = :id");

                        QImageReader thumbnail;
                        while (selectQuery.next()) {
                            auto coverPath = LibraryPaths::coverPath(libraryPath, selectQuery.value(1).toString());
                            thumbnail.setFileName(coverPath);

                            float coverSizeRatio = static_cast<float>(thumbnail.size().width()) / thumbnail.size().height();
                            updateCoverInfo.bindValue(":coverSizeRatio", coverSizeRatio);
                            updateCoverInfo.bindValue(":id", selectQuery.value(0));

                            updateCoverInfo.exec();
                        }

                        db.commit();
                    }
                }

                if (pre9_8) {
                    { // comic_info
                        QStringList columnDefs;
                        columnDefs << "manga BOOLEAN DEFAULT 0";
                        bool successAddingColumns = addColumns("comic_info", columnDefs, db);
                        returnValue = returnValue && successAddingColumns;
                    }
                    { // folder
                        QStringList columnDefs;
                        columnDefs << "manga BOOLEAN DEFAULT 0";
                        bool successAddingColumns = addColumns("folder", columnDefs, db);
                        returnValue = returnValue && successAddingColumns;
                    }
                }

                if (pre9_13) {
                    { // comic_info
                        QStringList columnDefs;
                        columnDefs << "added INTEGER";
                        columnDefs << "type INTEGER DEFAULT 0"; // 0 = comic, 1 = manga, 2 = manga left to right, 3 = webcomic,
                        columnDefs << "editor TEXT";
                        columnDefs << "imprint TEXT";
                        columnDefs << "teams TEXT";
                        columnDefs << "locations TEXT";
                        columnDefs << "series TEXT";
                        columnDefs << "alternateSeries TEXT";
                        columnDefs << "alternateNumber TEXT";
                        columnDefs << "alternateCount INTEGER";
                        columnDefs << "languageISO TEXT";
                        columnDefs << "seriesGroup TEXT";
                        columnDefs << "mainCharacterOrTeam TEXT";
                        columnDefs << "review TEXT";
                        columnDefs << "tags TEXT";
                        bool successAddingColumns = addColumns("comic_info", columnDefs, db);
                        returnValue = returnValue && successAddingColumns;

                        QSqlQuery updateTypeQueryToManga(db);
                        updateTypeQueryToManga.prepare("UPDATE comic_info SET type = manga");
                        bool successMigratingManga = updateTypeQueryToManga.exec();
                        returnValue = returnValue && successMigratingManga;

                        QSqlQuery updateNumberQueryToBis(db);
                        updateNumberQueryToBis.prepare("UPDATE comic_info SET number = number + 0.5 WHERE isBis = 1");
                        bool successMigratingBis = updateNumberQueryToBis.exec();
                        returnValue = returnValue && successMigratingBis;
                    }
                    { // folder
                        QStringList columnDefs;
                        columnDefs << "added INTEGER";
                        columnDefs << "updated INTEGER";
                        columnDefs << "type INTEGER DEFAULT 0";

                        bool successAddingColumns = addColumns("folder", columnDefs, db);
                        returnValue = returnValue && successAddingColumns;

                        QSqlQuery updateTypeQueryToManga(db);
                        updateTypeQueryToManga.prepare("UPDATE folder SET type = manga");
                        bool successMigratingManga = updateTypeQueryToManga.exec();
                        returnValue = returnValue && successMigratingManga;
                    }
                }

                // ensure that INTEGER types migrated to TEXT are actually changed in the table definition to avoid internal type castings, this happened in 9.13 but a migration wasn't shipped with that version.
                if (pre9_14) {
                    {
                        bool pre9_14_successfulMigration = true;

                        pre9_14_successfulMigration = pre9_14_successfulMigration && pragmaFKOFF.exec();

                        pre9_14_successfulMigration = pre9_14_successfulMigration && createComicInfoTable9_14(db, "comic_info_migration");

                        QSqlQuery copyComicInfoToComicInfoMigration(db);
                        copyComicInfoToComicInfoMigration.prepare("INSERT INTO comic_info_migration SELECT * FROM comic_info");

                        pre9_14_successfulMigration = pre9_14_successfulMigration && copyComicInfoToComicInfoMigration.exec();

                        QSqlQuery dropComicInfo(db);
                        dropComicInfo.prepare("DROP TABLE comic_info");
                        pre9_14_successfulMigration = pre9_14_successfulMigration && dropComicInfo.exec();

                        QLOG_ERROR() << "Migration failed1:" << dropComicInfo.lastError().text();

                        QSqlQuery renameComicInfoMigrationToComicInfo(db);
                        renameComicInfoMigrationToComicInfo.prepare("ALTER TABLE comic_info_migration RENAME TO comic_info");
                        pre9_14_successfulMigration = pre9_14_successfulMigration && renameComicInfoMigrationToComicInfo.exec();

                        QLOG_ERROR() << "Migration failed2:" << renameComicInfoMigrationToComicInfo.lastError().text();

                        returnValue = returnValue && pre9_14_successfulMigration;
                    }
                }

                if (pre9_16) {
                    { // comic_info
                        QStringList columnDefs;
                        columnDefs << "imageFiltersJson TEXT";
                        columnDefs << "lastTimeImageFiltersSet INTEGER DEFAULT 0";

                        columnDefs << "lastTimeCoverSet INTEGER DEFAULT 0";
                        columnDefs << "usesExternalCover BOOLEAN DEFAULT 0";

                        columnDefs << "lastTimeMetadataSet INTEGER DEFAULT 0";

                        bool successAddingColumns = addColumns("comic_info", columnDefs, db);
                        returnValue = returnValue && successAddingColumns;
                    }
                }

                if (returnValue) {
                    QSqlQuery updateVersion(db);
                    updateVersion.prepare("UPDATE db_info SET "
                                          "version = :version");
                    updateVersion.bindValue(":version", DB_VERSION);
                    updateVersion.exec();

                    returnValue = updateVersion.numRowsAffected() > 0;
                }
            }

            if (returnValue) {
                if (!db.commit()) {
                    QLOG_ERROR() << "Failed to commit transaction for database update";
                    returnValue = false;
                }
            } else {
                db.rollback();
            }

            QSqlQuery pragmaFKON("PRAGMA foreign_keys = ON", db);
        }

        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
    return returnValue;
}

bool DataBaseManagement::backupLibrary(const QString &libraryPath, DatabaseBackupReason reason, QString *error, const QString &destinationPath, const QString &protectedBackup)
{
    const auto databasePath = LibraryPaths::libraryDatabasePath(libraryPath);
    const auto dataPath = LibraryPaths::libraryDataPath(libraryPath);
    QDir backupsDirectory(QDir(dataPath).filePath("backups"));
    if (destinationPath.isEmpty() && !backupsDirectory.exists() && !QDir().mkpath(backupsDirectory.path())) {
        if (error)
            *error = QString("Unable to create backup directory: %1").arg(backupsDirectory.path());
        return false;
    }

    if (destinationPath.isEmpty() && (reason == DatabaseBackupReason::AutoUpdate || reason == DatabaseBackupReason::BeforeRepair)) {
        const auto automatic = backupsForReason(backupsDirectory, "auto-update");
        const auto maximumAge = reason == DatabaseBackupReason::AutoUpdate ? 24 * 60 * 60 : 60 * 60;
        for (const auto &backup : automatic) {
            const auto created = backupDateTime(backup);
            if (!created.isValid())
                continue;
            if (created.secsTo(QDateTime::currentDateTime()) >= maximumAge)
                break;
            QLOG_INFO() << "Skipping database backup: a recent automatic backup exists" << backup.absoluteFilePath();
            return true;
        }
    }

    QString sourceVersion;
    if (!validateDatabase(databasePath, error, &sourceVersion)) {
        QLOG_ERROR() << "Database backup blocked: source database is invalid" << databasePath;
        return false;
    }

    const auto reasonName = backupReasonName(reason);
    const auto finalPath = destinationPath.isEmpty()
            ? backupsDirectory.filePath(QString("library-%1-db-%2-%3.ydb").arg(QDateTime::currentDateTime().toString(backupTimestampFormat), sourceVersion, reasonName))
            : QDir::cleanPath(destinationPath);
    if (QFileInfo(finalPath).absoluteFilePath() == QFileInfo(databasePath).absoluteFilePath()) {
        if (error)
            *error = "The backup destination cannot be the live library database";
        return false;
    }

    QString backupPath = finalPath;
    if (!destinationPath.isEmpty()) {
        QTemporaryFile temporary(finalPath + ".XXXXXX.tmp");
        temporary.setAutoRemove(false);
        if (!temporary.open()) {
            if (error)
                *error = temporary.errorString();
            return false;
        }
        backupPath = temporary.fileName();
        temporary.close();
        temporary.remove();
    }
    if (QFile::exists(backupPath)) {
        if (error)
            *error = QString("Backup file already exists: %1").arg(backupPath);
        return false;
    }
    if (!vacuumDatabaseInto(databasePath, backupPath, error)) {
        QFile::remove(backupPath);
        QLOG_ERROR() << "Database backup failed" << backupPath << (error ? *error : QString());
        return false;
    }
    if (!validateDatabase(backupPath, error)) {
        QFile::remove(backupPath);
        QLOG_ERROR() << "Database backup failed" << backupPath << (error ? *error : QString());
        return false;
    }

    if (!destinationPath.isEmpty() && !replaceFileAtomically(backupPath, finalPath, error)) {
        QFile::remove(backupPath);
        QLOG_ERROR() << "Database backup replacement failed" << finalPath << (error ? *error : QString());
        return false;
    }

    QLOG_INFO() << "Database backup created" << finalPath;
    if (destinationPath.isEmpty())
        removeOldBackups(backupsDirectory, reason, finalPath, protectedBackup);
    return true;
}

DatabaseRestoreResult DataBaseManagement::restoreLibrary(const QString &libraryPath, const QString &backupPath, bool allowInvalidCurrent, bool removeStaleLock)
{
    DatabaseRestoreResult result;
    QString selectedVersion;
    if (!validateDatabase(backupPath, &result.error, &selectedVersion)) {
        result.status = DatabaseRestoreStatus::InvalidBackup;
        return result;
    }
    if (compareVersions(selectedVersion, DB_VERSION) > 0) {
        result.status = DatabaseRestoreStatus::NewerBackup;
        result.error = QString("The backup database version %1 is newer than supported version %2").arg(selectedVersion, DB_VERSION);
        return result;
    }

    if (!QDir(libraryPath).exists()) {
        result.status = DatabaseRestoreStatus::Failed;
        result.error = QString("Library folder does not exist: %1").arg(libraryPath);
        return result;
    }
    if (!QDir().mkpath(LibraryPaths::libraryDataPath(libraryPath))) {
        result.status = DatabaseRestoreStatus::Failed;
        result.error = QString("Unable to create library metadata folder: %1").arg(LibraryPaths::libraryDataPath(libraryPath));
        return result;
    }

    LibraryMaintenanceLock lock(libraryPath);
    if (!lock.tryLock(removeStaleLock)) {
        result.status = DatabaseRestoreStatus::LockFailed;
        result.error = lock.errorString();
        result.lockHolderInfo = lock.holderInfo();
        result.lockHolderIsRunningLocally = lock.holderIsRunningLocally();
        return result;
    }

    if (!recoverInterruptedRestore(libraryPath, &result.error, true)) {
        result.status = DatabaseRestoreStatus::Failed;
        return result;
    }

    const auto databasePath = LibraryPaths::libraryDatabasePath(libraryPath);
    const auto stagedPath = stagedDatabasePath(databasePath);
    const auto rollbackPath = rollbackDatabasePath(databasePath);
    const auto markerPath = restoreMarkerPath(databasePath);
    const bool hadOriginal = QFile::exists(databasePath);

    QString currentVersion;
    const bool currentValid = hadOriginal && validateDatabase(databasePath, nullptr, &currentVersion);
    if (hadOriginal && !currentValid && !allowInvalidCurrent) {
        result.status = DatabaseRestoreStatus::InvalidCurrentDatabase;
        result.error = "The current library database is invalid; confirmation is required before replacing it";
        return result;
    }

    if (currentValid && !backupLibrary(libraryPath, DatabaseBackupReason::BeforeRestore, &result.error, { }, backupPath)) {
        result.status = DatabaseRestoreStatus::Failed;
        return result;
    }

    removeDatabaseUnit(stagedPath);
    removeDatabaseUnit(rollbackPath);
    QFile::remove(markerPath);
    if (!QFile::copy(backupPath, stagedPath)) {
        result.status = DatabaseRestoreStatus::Failed;
        result.error = QString("Unable to stage backup: %1").arg(backupPath);
        return result;
    }

    QString stagedVersion;
    if (!validateDatabase(stagedPath, &result.error, &stagedVersion)) {
        removeDatabaseUnit(stagedPath);
        result.status = DatabaseRestoreStatus::InvalidBackup;
        return result;
    }
    if (compareVersions(stagedVersion, DB_VERSION) > 0) {
        removeDatabaseUnit(stagedPath);
        result.status = DatabaseRestoreStatus::NewerBackup;
        result.error = QString("The staged database version %1 is newer than supported version %2").arg(stagedVersion, DB_VERSION);
        return result;
    }

    if (!writeRestoreMarker(markerPath, hadOriginal, "prepared", &result.error)) {
        removeDatabaseUnit(stagedPath);
        result.status = DatabaseRestoreStatus::Failed;
        return result;
    }

    auto rollBack = [&]() {
        if (!removeDatabaseUnit(databasePath))
            return false;
        if (hadOriginal && !moveDatabaseUnit(rollbackPath, databasePath, &result.error))
            return false;
        if (!removeDatabaseUnit(stagedPath) || !removeDatabaseUnit(rollbackPath)) {
            result.error = "The original database was restored, but restore cleanup failed";
            return false;
        }
        if (!QFile::remove(markerPath)) {
            result.error = "The original database was restored, but the restore marker could not be removed";
            return false;
        }
        return true;
    };

    if (hadOriginal && !moveDatabaseUnit(databasePath, rollbackPath, &result.error)) {
        removeDatabaseUnit(stagedPath);
        QFile::remove(markerPath);
        result.status = DatabaseRestoreStatus::Failed;
        return result;
    }
    if (!writeRestoreMarker(markerPath, hadOriginal, "original-moved", &result.error)) {
        result.status = rollBack() ? DatabaseRestoreStatus::Failed : DatabaseRestoreStatus::RollbackFailed;
        return result;
    }
    if (!moveDatabaseUnit(stagedPath, databasePath, &result.error)) {
        result.status = rollBack() ? DatabaseRestoreStatus::Failed : DatabaseRestoreStatus::RollbackFailed;
        return result;
    }
    if (!writeRestoreMarker(markerPath, hadOriginal, "installed", &result.error)) {
        result.status = rollBack() ? DatabaseRestoreStatus::Failed : DatabaseRestoreStatus::RollbackFailed;
        return result;
    }

    if (compareVersions(stagedVersion, DB_VERSION) < 0) {
        if (!updateToCurrentVersion(libraryPath, true)) {
            result.error = "Unable to upgrade the restored database";
            result.status = rollBack() ? DatabaseRestoreStatus::Failed : DatabaseRestoreStatus::RollbackFailed;
            return result;
        }
        result.upgraded = true;
        result.restoredVersion = DB_VERSION;
    } else {
        result.restoredVersion = stagedVersion;
    }

    QFile::remove(markerPath);
    removeDatabaseUnit(rollbackPath);
    removeDatabaseUnit(stagedPath);
    result.status = DatabaseRestoreStatus::Success;
    return result;
}

bool DataBaseManagement::recoverInterruptedRestore(const QString &libraryPath, QString *error, bool maintenanceLockHeld)
{
    const auto databasePath = LibraryPaths::libraryDatabasePath(libraryPath);
    const auto stagedPath = stagedDatabasePath(databasePath);
    const auto rollbackPath = rollbackDatabasePath(databasePath);
    const auto markerPath = restoreMarkerPath(databasePath);
    if (!QFile::exists(markerPath) && !QFile::exists(stagedPath) && !QFile::exists(rollbackPath))
        return true;

    std::unique_ptr<LibraryMaintenanceLock> lock;
    if (!maintenanceLockHeld) {
        lock = std::make_unique<LibraryMaintenanceLock>(libraryPath);
        if (!lock->tryLock()) {
            if (error)
                *error = lock->errorString();
            return false;
        }
    }

    if (!QFile::exists(markerPath)) {
        removeDatabaseUnit(stagedPath);
        if (QFile::exists(databasePath))
            removeDatabaseUnit(rollbackPath);
        else if (QFile::exists(rollbackPath) && !moveDatabaseUnit(rollbackPath, databasePath, error))
            return false;
        return true;
    }

    const bool hadOriginal = markerHadOriginal(markerPath);
    if (QFile::exists(rollbackPath)) {
        if (!removeDatabaseUnit(databasePath) || !moveDatabaseUnit(rollbackPath, databasePath, error))
            return false;
    } else if (!hadOriginal) {
        if (!QFile::exists(databasePath)) {
            removeDatabaseUnit(stagedPath);
            QFile::remove(markerPath);
            if (error)
                *error = "Interrupted restore did not install a database; retry the restore";
            return false;
        }
        QString ignoredVersion;
        if (!validateDatabase(databasePath, error, &ignoredVersion)) {
            removeDatabaseUnit(stagedPath);
            removeDatabaseUnit(rollbackPath);
            QFile::remove(markerPath);
            if (error)
                error->clear();
            return true;
        }
    } else if (hadOriginal && !QFile::exists(databasePath)) {
        removeDatabaseUnit(stagedPath);
        QFile::remove(markerPath);
        if (error)
            error->clear();
        return true;
    }

    removeDatabaseUnit(stagedPath);
    removeDatabaseUnit(rollbackPath);
    QFile::remove(markerPath);
    return true;
}

bool DataBaseManagement::prepareForRecreation(const QString &libraryPath, QString *error, bool maintenanceLockHeld)
{
    if (!recoverInterruptedRestore(libraryPath, error, maintenanceLockHeld))
        return false;

    const auto databasePath = LibraryPaths::libraryDatabasePath(libraryPath);
    bool success = removeDatabaseUnit(databasePath);
    success = removeDatabaseUnit(stagedDatabasePath(databasePath)) && success;
    success = removeDatabaseUnit(rollbackDatabasePath(databasePath)) && success;
    success = removeDatabaseUnit(salvageDatabasePath(databasePath)) && success;
    success = (!QFile::exists(restoreMarkerPath(databasePath)) || QFile::remove(restoreMarkerPath(databasePath))) && success;

    QDir covers(LibraryPaths::libraryCoversFolderPath(libraryPath));
    success = (!covers.exists() || covers.removeRecursively()) && success;
    const auto idPath = LibraryPaths::idPath(libraryPath);
    success = (!QFile::exists(idPath) || QFile::remove(idPath)) && success;
    if (!success && error)
        *error = QString("Unable to prepare library metadata for recreation: %1").arg(LibraryPaths::libraryDataPath(libraryPath));
    return success;
}

QFileInfoList DataBaseManagement::libraryBackups(const QString &libraryPath)
{
    return QDir(QDir(LibraryPaths::libraryDataPath(libraryPath)).filePath("backups"))
            .entryInfoList({ "library-*.ydb" }, QDir::Files, QDir::Name | QDir::Reversed);
}

bool DataBaseManagement::isLibraryDatabaseValid(const QString &libraryPath)
{
    return validateDatabase(LibraryPaths::libraryDatabasePath(libraryPath), nullptr);
}

DatabaseSalvageResult DataBaseManagement::salvageLibrary(const QString &libraryPath, bool removeStaleLock)
{
    DatabaseSalvageResult result;
    const auto databasePath = LibraryPaths::libraryDatabasePath(libraryPath);
    if (!QFile::exists(databasePath)) {
        result.error = QString("Library database not found: %1").arg(databasePath);
        return result;
    }
    if (validateDatabase(databasePath, nullptr)) {
        result.status = DatabaseSalvageStatus::AlreadyValid;
        return result;
    }

    const auto salvagePath = salvageDatabasePath(databasePath);
    {
        LibraryMaintenanceLock lock(libraryPath);
        if (!lock.tryLock(removeStaleLock)) {
            result.status = DatabaseSalvageStatus::LockFailed;
            result.error = lock.errorString();
            result.lockHolderInfo = lock.holderInfo();
            result.lockHolderIsRunningLocally = lock.holderIsRunningLocally();
            return result;
        }

        result.preservedDatabasePath = damagedDatabasePath(libraryPath);
        if (!copyDatabaseUnit(databasePath, result.preservedDatabasePath, &result.error)) {
            result.preservedDatabasePath.clear();
            return result;
        }

        // index corruption can be repaired in place without losing anything
        QString connectionName;
        {
            auto db = loadDatabaseFromFile(databasePath);
            connectionName = db.connectionName();
            if (db.isOpen()) {
                QSqlQuery reindex(db);
                if (!reindex.exec("REINDEX"))
                    QLOG_INFO() << "REINDEX did not complete during salvage:" << reindex.lastError().text();
            }
        }
        if (!connectionName.isEmpty())
            QSqlDatabase::removeDatabase(connectionName);

        if (validateDatabase(databasePath, nullptr)) {
            QLOG_INFO() << "Library database salvaged with REINDEX" << databasePath;
            result.status = DatabaseSalvageStatus::Reindexed;
            return result;
        }

        // rebuild the reachable logical content into a fresh file; this fails if
        // table pages themselves are unreadable, in which case only restoring a
        // backup or recreating the library can help
        QFile::remove(salvagePath);
        QString rebuildError;
        if (!vacuumDatabaseInto(databasePath, salvagePath, &rebuildError) || !validateDatabase(salvagePath, &rebuildError)) {
            QFile::remove(salvagePath);
            result.error = QString("The library database could not be repaired: %1").arg(rebuildError);
            QLOG_ERROR() << "Library database salvage failed" << databasePath << rebuildError;
            return result;
        }
    }

    // the rebuilt database is installed through the restore machinery to get its
    // staging, rollback, and crash-recovery guarantees; restoreLibrary acquires
    // the maintenance lock itself, so it is released above
    const auto restoreResult = restoreLibrary(libraryPath, salvagePath, true);
    QFile::remove(salvagePath);
    if (!restoreResult.success()) {
        result.error = restoreResult.error;
        return result;
    }
    QLOG_INFO() << "Library database salvaged by rebuilding it" << databasePath;
    result.status = DatabaseSalvageStatus::Rebuilt;
    return result;
}

DatabaseAccess DataBaseManagement::getDatabaseAccess(const QString &libraryPath)
{
    DatabaseAccess access = { false, false, false, false };

    auto libraryDataPath = LibraryPaths::libraryDataPath(libraryPath);
    auto libraryDatabasePath = LibraryPaths::libraryDatabasePath(libraryPath);

    QFile libraryDatabase(libraryDatabasePath);
    if (!libraryDatabase.exists()) {
        return access;
    }

    access.libraryExists = true;

    QDir libraryData(libraryDataPath);
    QFile testFile(libraryData.filePath("test"));
    if (testFile.open(QIODevice::WriteOnly)) {
        access.canWriteToFolder = true;
        testFile.close();
        testFile.remove();
    }

    QString connectionName = "test";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabaseFromFile(libraryDatabasePath);

        QSqlQuery versionQuery(db);
        bool read = versionQuery.exec("SELECT version FROM db_info");

        read = read && versionQuery.next();
        read = read && !versionQuery.record().value(0).toString().isEmpty();

        access.canRead = read;

        QSqlQuery writeQuery(db);
        bool write = db.transaction();
        write = write && writeQuery.exec("CREATE TABLE test_write (id INTEGER);");
        write = write && db.rollback();

        access.canWrite = write;
    }
    QSqlDatabase::removeDatabase(connectionName);

    return access;
}

// COMICS_INFO_EXPORTER
ComicsInfoExporter::ComicsInfoExporter()
    : QThread()
{
}

void ComicsInfoExporter::exportComicsInfo(QSqlDatabase &source, QSqlDatabase &dest)
{
    Q_UNUSED(source)
    Q_UNUSED(dest)
    // TODO check this method
}

void ComicsInfoExporter::run()
{
}

// COMICS_INFO_IMPORTER
ComicsInfoImporter::ComicsInfoImporter()
    : QThread()
{
}

void ComicsInfoImporter::importComicsInfo(QSqlDatabase &source, QSqlDatabase &dest)
{
    Q_UNUSED(source)
    Q_UNUSED(dest)
    // TODO check this method
}

void ComicsInfoImporter::run()
{
}
