#ifndef __DATA_BASE_MANAGEMENT_H
#define __DATA_BASE_MANAGEMENT_H

#include <QSqlDatabase>
#include <QtCore>

#include <memory>

class ComicsInfoExporter : public QThread
{
    Q_OBJECT
public:
    ComicsInfoExporter();
    void exportComicsInfo(QSqlDatabase &source, QSqlDatabase &dest);

private:
    void run() override;
};

class ComicsInfoImporter : public QThread
{
    Q_OBJECT
public:
    ComicsInfoImporter();
    void importComicsInfo(QSqlDatabase &source, QSqlDatabase &dest);

private:
    void run() override;
};

struct DatabaseAccess {
    bool libraryExists;
    bool canRead; // db read
    bool canWrite; // db write
    bool canWriteToFolder; // disk write

    operator QString() const
    {
        if (libraryExists && canRead && canWrite && canWriteToFolder) {
            return "OK";
        } else if (!libraryExists) {
            return "WARNING! Library does not exist on disk";
        } else {
            return QString("WARNING! DB read access: %1, DB write access: %2, can write to disk: %3")
                    .arg(canRead ? "YES" : "NO")
                    .arg(canWrite ? "YES" : "NO")
                    .arg(canWriteToFolder ? "YES" : "NO");
        }
    }
};

enum class DatabaseBackupReason {
    AutoUpdate,
    BeforeUpgrade,
    BeforeRepair,
    BeforeRestore,
    Manual
};

struct LibraryMaintenanceLockFailure {
    QLockFile::LockError error { QLockFile::NoError };
    bool ownerIsRunning { false };
    QString owner;
    QString recoveryToken;
};

class LibraryMaintenanceLock
{
public:
    explicit LibraryMaintenanceLock(const QString &libraryPath);
    ~LibraryMaintenanceLock();

    // true is an explicit recovery request, after confirming no other machine
    // is using the library. Reuse this object to detect ownership changes.
    bool tryLock(bool removeStaleLock = false);
    QLockFile::LockError error() const;
    QString errorString() const;
    QString holderInfo() const;
    bool holderIsRunning() const;
    QString recoveryToken() const;
    LibraryMaintenanceLockFailure failure() const;

private:
    bool tryLockFile(QLockFile &lock, bool removeStaleLock);
    QByteArray captureLockInfo(QLockFile &lock);

    std::unique_ptr<QLockFile> maintenanceLock;
    std::unique_ptr<QLockFile> legacyRepairLock;
    QString failedLockPath;
    QLockFile::LockError lastError { QLockFile::NoError };
    QString currentHolderInfo;
    bool currentHolderIsRunning { false };
    QHash<QString, QByteArray> observedLocks;
    QHash<QString, QByteArray> expectedLocks;
    QString recoveryError;
};

enum class DatabaseRestoreStatus {
    Success,
    InvalidBackup,
    NewerBackup,
    InvalidCurrentDatabase,
    LockFailed,
    Failed,
    RollbackFailed
};

struct DatabaseRestoreResult {
    DatabaseRestoreStatus status { DatabaseRestoreStatus::Failed };
    QString error;
    QString restoredVersion;
    QString lockHolderInfo;
    bool lockHolderIsRunningLocally { false };
    bool upgraded { false };

    bool success() const { return status == DatabaseRestoreStatus::Success; }
};

enum class DatabaseSalvageStatus {
    AlreadyValid,
    Reindexed,
    Rebuilt,
    LockFailed,
    Failed
};

struct DatabaseSalvageResult {
    DatabaseSalvageStatus status { DatabaseSalvageStatus::Failed };
    QString error;
    QString preservedDatabasePath;
    QString lockHolderInfo;
    bool lockHolderIsRunningLocally { false };

    bool success() const
    {
        return status == DatabaseSalvageStatus::AlreadyValid || status == DatabaseSalvageStatus::Reindexed || status == DatabaseSalvageStatus::Rebuilt;
    }
};

class DataBaseManagement : public QObject
{
    Q_OBJECT
private:
    QList<QString> dataBasesList;
    static void bindValue(const QString &name, const QSqlRecord &record, QSqlQuery &query);
    static void bindValuesFromRecord(const QSqlRecord &record, QSqlQuery &query);

    static bool addColumns(const QString &tableName, const QStringList &columnDefs, const QSqlDatabase &db);
    static bool addConstraint(const QString &tableName, const QString &constraint, const QSqlDatabase &db);
    static DatabaseRestoreResult restoreLibraryImpl(const QString &libraryPath, const QString &backupPath, bool allowInvalidCurrent, bool removeStaleLock, bool maintenanceLockHeld);

public:
    DataBaseManagement();
    // crea una base de datos y todas sus tablas
    static QSqlDatabase createDatabase(QString name, QString path);
    static QSqlDatabase createDatabase(QString dest);
    // carga una base de datos desde la ruta path
    static QSqlDatabase loadDatabase(QString libraryDataPath);
    static QSqlDatabase loadDatabaseFromFile(QString path);
    static bool createTables(QSqlDatabase &database);
    static bool createComicInfoTable(QSqlDatabase &database, QString tableName);
    static bool createComicInfoTable9_14(QSqlDatabase &database, QString tableName);
    static bool createV8Tables(QSqlDatabase &database);
    static bool createV10_5Tables(QSqlDatabase &database);
    static bool createPlaceholderTable(QSqlDatabase &database);
    static bool createReadingListEntryTable(QSqlDatabase &database);
    static bool createLabelEntryTable(QSqlDatabase &database);
    static bool createComicDeleteTrigger(QSqlDatabase &database);
    static bool createSmartListTable(QSqlDatabase &database);
    // the folder tables and the folder_id columns of reading_list and smart_list
    static bool createListFolderTables(QSqlDatabase &database);
    static bool createFolderPlaceholderTable(QSqlDatabase &database);

    static void exportComicsInfo(QString source, QString dest);
    static bool importComicsInfo(QString source, QString dest);

    static QString checkValidDB(const QString &fullPath); // retorna "" si la DB es inválida ó la versión si es válida.
    static int compareVersions(const QString &v1, const QString v2); // retorna <0 si v1 < v2, 0 si v1 = v2 y >0 si v1 > v2
    static bool updateToCurrentVersion(const QString &libraryPath, bool maintenanceLockHeld = false);
    static bool backupLibrary(const QString &libraryPath, DatabaseBackupReason reason, QString *error = nullptr, const QString &destinationPath = { }, const QString &protectedBackup = { });
    static DatabaseRestoreResult restoreLibrary(const QString &libraryPath, const QString &backupPath, bool allowInvalidCurrent = false, bool removeStaleLock = false);
    static bool recoverInterruptedRestore(const QString &libraryPath, QString *error = nullptr, bool maintenanceLockHeld = false, QLockFile::LockError *lockError = nullptr);
    static bool prepareForRecreation(const QString &libraryPath, QString *error = nullptr, bool maintenanceLockHeld = false);
    static QFileInfoList libraryBackups(const QString &libraryPath);
    static bool isLibraryDatabaseValid(const QString &libraryPath);
    static DatabaseSalvageResult salvageLibrary(const QString &libraryPath, bool removeStaleLock = false);

    static DatabaseAccess getDatabaseAccess(const QString &libraryPath);
};

#endif
