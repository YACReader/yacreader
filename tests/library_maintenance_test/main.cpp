#include "comic_info_repairer.h"
#include "console_ui_library_creator.h"
#include "data_base_management.h"
#include "libraries_update_coordinator.h"
#include "library_creator.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTemporaryDir>
#include <QTest>

#ifdef Q_OS_WIN
// sddl.h requires Windows types to be defined first.
// clang-format off
#include <windows.h>
#include <sddl.h>
// clang-format on
#endif

#include <future>
#include <memory>

using namespace YACReader;

class LibraryMaintenanceTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void backupAndRestoreKeepListData();
    void repairLeavesDeletedComicsAlone();
    void missingLockFolderCanBeRetried();
    void activeMaintenanceLocks_data();
    void activeMaintenanceLocks();
    void maintenanceLocksAreReleased();
    void unwritableLockFolderIsNotContention();
    void creatorReportsMissingMetadata();
    void serverCreationOnlyRegistersSuccess();
    void unknownMaintenanceOwner_data();
    void unknownMaintenanceOwner();
    void deadLocalOwnerIsRecoveredAutomatically();
    void confirmedRecovery_data();
    void confirmedRecovery();
    void recoveryRejectsChangedOwner();
    void recoveryDoesNotDisplaceLiveProcess();
    void undeletableStaleLockNamesFileAccess();
    void concurrentRecoveriesAreExclusive();
    void serverRecoveryIsExplicitAndNoninteractive();
    void backgroundUpdatesRecordLockProblems();
    void rapidMaintenanceBackupsRemainDistinct();
    void killedProcessLocksAreRecovered();
    void lockDirectoriesAreNotContention_data();
    void lockDirectoriesAreNotContention();
    void repairSettlesInterruptedRestore();
    void salvageChecksLocksEvenForValidDatabase();
    void salvageRebuildsWithoutLosingRows();
    void directRecreationHonorsOwnership();
    void creationRetryPreservesRecoveredDatabase_data();
    void creationRetryPreservesRecoveredDatabase();

private:
    bool exec(const QString &sql);
    QList<QVariantList> rows(const QString &sql);
    QList<QVariantList> listData();

    std::unique_ptr<QTemporaryDir> root;
    QString libraryPath;
    QString dataPath;
};

void LibraryMaintenanceTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    libraryPath = QDir::cleanPath(root->filePath("library"));
    dataPath = LibraryPaths::libraryDataPath(libraryPath);
    QVERIFY(QDir().mkpath(LibraryPaths::libraryCoversPathFromLibraryDataPath(dataPath)));

    QString name;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(libraryPath));
        name = db.connectionName();
        db.close();
    }
    QSqlDatabase::removeDatabase(name);

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (2, 1, 'Series', '/Series')"));
    for (int i = 1; i <= 4; i++) {
        QVERIFY(exec(QString("INSERT INTO comic_info (id, hash, title, numPages, coverSizeRatio) VALUES (%1, 'hash-%1', 'Title %1', 20, 0.66)").arg(i)));
        QVERIFY(exec(QString("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (%1, 2, %1, 'c%1.cbz', '/Series/c%1.cbz')").arg(i)));
    }
    QVERIFY(exec("INSERT INTO placeholder (id, title, coverKey) VALUES (1, 'Expected issue', 'cover-1')"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'List')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id, placeholder_id) VALUES "
                 "(1, 0, 2, NULL), (1, 1, 4, NULL), (1, 2, NULL, 1), (1, 3, 4, NULL), (1, 4, 3, NULL)"));
    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Tag', 'red', 0)"));
    QVERIFY(exec("INSERT INTO label_entry (label_id, ordering, comic_info_id, placeholder_id) VALUES (1, 0, 4, NULL), (1, 1, NULL, 1), (1, 2, 3, NULL)"));

    // comic_info 4 loses its file but stays, because the lists refer to it
    QVERIFY(exec("DELETE FROM comic WHERE id = 4"));
}

void LibraryMaintenanceTest::cleanup()
{
    root.reset();
}

bool LibraryMaintenanceTest::exec(const QString &sql)
{
    QString name;
    bool ok = false;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        name = db.connectionName();
        QSqlQuery query(db);
        ok = query.exec(sql);
        if (!ok)
            qWarning() << sql;
    }
    QSqlDatabase::removeDatabase(name);
    return ok;
}

QList<QVariantList> LibraryMaintenanceTest::rows(const QString &sql)
{
    QList<QVariantList> result;
    QString name;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        name = db.connectionName();
        QSqlQuery query(db);
        if (query.exec(sql)) {
            const auto columns = query.record().count();
            while (query.next()) {
                QVariantList row;
                for (int i = 0; i < columns; i++)
                    row.append(query.value(i));
                result.append(row);
            }
        } else {
            qWarning() << sql;
        }
    }
    QSqlDatabase::removeDatabase(name);
    return result;
}

QList<QVariantList> LibraryMaintenanceTest::listData()
{
    return rows("SELECT id, reading_list_id, ordering, comic_info_id, placeholder_id, last_known_name, last_known_path FROM reading_list_entry ORDER BY id") +
            rows("SELECT id, label_id, ordering, comic_info_id, placeholder_id, last_known_name, last_known_path FROM label_entry ORDER BY id") +
            rows("SELECT id, title, coverKey FROM placeholder ORDER BY id") +
            rows("SELECT id, hash, title FROM comic_info ORDER BY id") +
            rows("SELECT name FROM sqlite_master WHERE type = 'trigger' ORDER BY name");
}

void LibraryMaintenanceTest::backupAndRestoreKeepListData()
{
    const auto before = listData();
    QCOMPARE(rows("SELECT last_known_name, last_known_path FROM reading_list_entry WHERE comic_info_id = 4"),
             QList<QVariantList>({ { "c4.cbz", "/Series/c4.cbz" }, { "c4.cbz", "/Series/c4.cbz" } }));

    const auto backupPath = root->filePath("backup.ydb");
    QString error;
    QVERIFY2(DataBaseManagement::backupLibrary(libraryPath, DatabaseBackupReason::Manual, &error, backupPath), qPrintable(error));

    QVERIFY(exec("DELETE FROM reading_list_entry"));
    QVERIFY(exec("DELETE FROM label_entry"));
    QVERIFY(exec("DELETE FROM placeholder"));
    QVERIFY(exec("DELETE FROM comic_info WHERE id = 4"));
    QVERIFY(listData() != before);

    const auto result = DataBaseManagement::restoreLibrary(libraryPath, backupPath);
    QVERIFY2(result.success(), qPrintable(result.error));
    QCOMPARE(listData(), before);

    // the trigger still records the last known file
    QVERIFY(exec("DELETE FROM comic WHERE id = 3"));
    QCOMPARE(rows("SELECT last_known_name FROM reading_list_entry WHERE comic_info_id = 3"), QList<QVariantList>({ { "c3.cbz" } }));
    QCOMPARE(rows("SELECT last_known_name FROM label_entry WHERE comic_info_id = 3"), QList<QVariantList>({ { "c3.cbz" } }));
}

void LibraryMaintenanceTest::repairLeavesDeletedComicsAlone()
{
    // a deleted comic that a repair would change if it had a file
    QVERIFY(exec("UPDATE comic_info SET numPages = NULL, coverSizeRatio = NULL WHERE id = 4"));
    const auto deletedInfo = rows("SELECT * FROM comic_info WHERE id = 4");

    // what a crash in a cover save leaves: only the copy of the previous cover
    const auto coverPath = LibraryPaths::coverPathFromLibraryDataPath(dataPath, "hash-4");
    QImage cover(60, 90, QImage::Format_RGB32);
    cover.fill(Qt::red);
    QVERIFY(cover.save(coverPath + ".old", "JPG"));

    YACReader::ComicInfoRepairer repairer(nullptr);
    repairer.repairLibrary(libraryPath, dataPath);
    QVERIFY(repairer.wait(30000));

    const auto summary = repairer.summary();
    QVERIFY2(summary.error.isEmpty(), qPrintable(summary.error));
    QCOMPARE(summary.repaired, 0);
    QCOMPARE(summary.failed, 0);
    // the comics with a file (that is not on disk); the deleted comic is not one of them
    QCOMPARE(summary.missingFiles, 3);
    QVERIFY(!summary.missingFilePaths.contains("/Series/c4.cbz"));

    QCOMPARE(rows("SELECT * FROM comic_info WHERE id = 4"), deletedInfo);
    QVERIFY(QFile::exists(coverPath));
    QVERIFY(!QFile::exists(coverPath + ".old"));
    QCOMPARE(rows("SELECT COUNT(*) FROM reading_list_entry WHERE comic_info_id = 4"), QList<QVariantList>({ { 2 } }));
}

void LibraryMaintenanceTest::missingLockFolderCanBeRetried()
{
    const auto missingLibrary = root->filePath("missing-library");
    const auto missingData = LibraryPaths::libraryDataPath(missingLibrary);
    LibraryMaintenanceLock lock(missingLibrary);
    QVERIFY(!lock.tryLock(true));
    QVERIFY(lock.error() != QLockFile::NoError);
    QVERIFY(lock.error() != QLockFile::LockFailedError);
    QVERIFY(lock.errorString().contains("folder is missing"));
    QVERIFY(lock.errorString().contains(missingData));
    QVERIFY(lock.holderInfo().isEmpty());

    QVERIFY(QDir().mkpath(missingData));
    QVERIFY2(lock.tryLock(), qPrintable(lock.errorString()));
    QCOMPARE(lock.error(), QLockFile::NoError);
    QVERIFY(lock.errorString().isEmpty());
}

void LibraryMaintenanceTest::activeMaintenanceLocks_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::newRow("maintenance") << QString("maintenance.lock");
    QTest::newRow("legacy-repair") << QString("repair.lock");
}

void LibraryMaintenanceTest::activeMaintenanceLocks()
{
    QFETCH(QString, fileName);
    const auto backupPath = root->filePath("lock-result-backup.ydb");
    QVERIFY(DataBaseManagement::backupLibrary(libraryPath, DatabaseBackupReason::Manual, nullptr, backupPath));
    const auto lockPath = QDir(dataPath).filePath(fileName);
    QLockFile owner(lockPath);
    owner.setStaleLockTime(0);
    QVERIFY(owner.tryLock());
    LibraryMaintenanceLock contender(libraryPath);
    QVERIFY(!contender.tryLock(true));
    QCOMPARE(contender.error(), QLockFile::LockFailedError);
    QVERIFY(contender.holderIsRunning());
    QVERIFY(contender.errorString().contains(lockPath));
    QVERIFY(contender.errorString().contains("Another maintenance operation"));
    QVERIFY(!contender.holderInfo().isEmpty());
    QVERIFY(QFile::exists(lockPath));
    if (fileName == "repair.lock")
        QVERIFY(!QFile::exists(QDir(dataPath).filePath("maintenance.lock")));

    const auto restoreResult = DataBaseManagement::restoreLibrary(libraryPath, backupPath);
    QCOMPARE(restoreResult.status, DatabaseRestoreStatus::LockFailed);
    ComicInfoRepairer repairer(nullptr);
    QSignalSpy failed(&repairer, &ComicInfoRepairer::failed);
    repairer.repairLibrary(libraryPath, dataPath);
    QVERIFY(repairer.wait(30000));
    QVERIFY(repairer.summary().lockedByAnotherProcess);
    QCOMPARE(failed.count(), 0);

    QFile damagedDatabase(LibraryPaths::libraryDatabasePath(libraryPath));
    QVERIFY(damagedDatabase.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(damagedDatabase.write("invalid database") > 0);
    damagedDatabase.close();
    const auto salvageResult = DataBaseManagement::salvageLibrary(libraryPath);
    QCOMPARE(salvageResult.status, DatabaseSalvageStatus::LockFailed);

    owner.unlock();
    QVERIFY2(contender.tryLock(), qPrintable(contender.errorString()));
    QCOMPARE(contender.error(), QLockFile::NoError);
    QVERIFY(contender.errorString().isEmpty());
    QVERIFY(contender.holderInfo().isEmpty());
}

void LibraryMaintenanceTest::maintenanceLocksAreReleased()
{
    {
        LibraryMaintenanceLock lock(libraryPath);
        QVERIFY(lock.tryLock());
    }
    QVERIFY(!QFile::exists(QDir(dataPath).filePath("maintenance.lock")));
    QVERIFY(!QFile::exists(QDir(dataPath).filePath("repair.lock")));
    LibraryMaintenanceLock next(libraryPath);
    QVERIFY(next.tryLock());
}

void LibraryMaintenanceTest::unwritableLockFolderIsNotContention()
{
#ifdef Q_OS_WIN
    const auto backupPath = root->filePath("permission-result-backup.ydb");
    QVERIFY(DataBaseManagement::backupLibrary(libraryPath, DatabaseBackupReason::Manual, nullptr, backupPath));
    QFile damagedDatabase(LibraryPaths::libraryDatabasePath(libraryPath));
    QVERIFY(damagedDatabase.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(damagedDatabase.write("invalid database") > 0);
    damagedDatabase.close();

    const auto nativePath = QDir::toNativeSeparators(dataPath);
    const auto path = reinterpret_cast<LPCWSTR>(nativePath.utf16());
    DWORD size = 0;
    GetFileSecurityW(path, DACL_SECURITY_INFORMATION, nullptr, 0, &size);
    QVERIFY(size > 0);
    QByteArray originalSecurity(size, '\0');
    QVERIFY(GetFileSecurityW(path, DACL_SECURITY_INFORMATION, originalSecurity.data(), size, &size));
    PSECURITY_DESCRIPTOR deniedSecurity = nullptr;
    // Deny creating files in this temporary folder, even for an elevated test
    // user. Keep other access so Qt can inspect the directory and missing lock.
    QVERIFY(ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:P(D;;FW;;;WD)(A;;FA;;;WD)", SDDL_REVISION_1, &deniedSecurity, nullptr));
    const auto freeSecurity = qScopeGuard([&] { LocalFree(deniedSecurity); });
    QVERIFY(SetFileSecurityW(path, DACL_SECURITY_INFORMATION, deniedSecurity));
    const auto restoreSecurity = qScopeGuard([&] {
        QVERIFY(SetFileSecurityW(path, DACL_SECURITY_INFORMATION, originalSecurity.data()));
    });
    LibraryMaintenanceLock lock(libraryPath);
    QVERIFY(!lock.tryLock(true));
    QCOMPARE(lock.error(), QLockFile::PermissionError);
    QVERIFY(lock.errorString().contains("write permissions"));
    QVERIFY(!lock.errorString().contains("Another maintenance operation"));
    QVERIFY(lock.holderInfo().isEmpty());
    QVERIFY(!QFile::exists(QDir(dataPath).filePath("maintenance.lock")));

    const auto restoreResult = DataBaseManagement::restoreLibrary(libraryPath, backupPath, false, true);
    QCOMPARE(restoreResult.status, DatabaseRestoreStatus::Failed);
    QVERIFY(restoreResult.error.contains("write permissions"));
    const auto salvageResult = DataBaseManagement::salvageLibrary(libraryPath, true);
    QCOMPARE(salvageResult.status, DatabaseSalvageStatus::Failed);
    QVERIFY(salvageResult.error.contains("write permissions"));
    ComicInfoRepairer repairer(nullptr);
    QSignalSpy failed(&repairer, &ComicInfoRepairer::failed);
    repairer.repairLibrary(libraryPath, dataPath, true);
    QVERIFY(repairer.wait(30000));
    QVERIFY(!repairer.summary().lockedByAnotherProcess);
    QVERIFY(repairer.summary().error.contains("write permissions"));
    QCOMPARE(failed.count(), 1);
#else
    QSKIP("This fixture uses Windows directory ACLs.");
#endif
}

void LibraryMaintenanceTest::creatorReportsMissingMetadata()
{
    const auto emptyLibrary = root->filePath("empty-library");
    const auto emptyData = LibraryPaths::libraryDataPath(emptyLibrary);
    QVERIFY(QDir().mkpath(emptyLibrary));
    QSettings settings(root->filePath("settings.ini"), QSettings::IniFormat);
    LibraryCreator updater(&settings);
    QSignalSpy openingFailure(&updater, &LibraryCreator::failedOpeningDB);
    updater.updateLibrary(emptyLibrary, emptyData);
    updater.start();
    QVERIFY(updater.wait(30000));
    QCOMPARE(openingFailure.count(), 1);
    QVERIFY(openingFailure.first().first().toString().contains("data folder is missing"));
    QVERIFY(!QDir(emptyData).exists());

    QVERIFY(QDir().mkpath(emptyData));
    openingFailure.clear();
    updater.start();
    QVERIFY(updater.wait(30000));
    QCOMPARE(openingFailure.count(), 1);
    QVERIFY(openingFailure.first().first().toString().contains("database is missing"));
    QVERIFY(!QFile::exists(LibraryPaths::libraryDatabasePath(emptyLibrary)));

    LibraryCreator folderUpdater(&settings);
    QSignalSpy folderFailure(&folderUpdater, &LibraryCreator::failedOpeningDB);
    folderUpdater.updateFolder(emptyLibrary, emptyData, emptyLibrary, 1);
    QVERIFY(!QFile::exists(LibraryPaths::libraryDatabasePath(emptyLibrary)));
    folderUpdater.start();
    QVERIFY(folderUpdater.wait(30000));
    QCOMPARE(folderFailure.count(), 1);
    QVERIFY(folderFailure.first().first().toString().contains("database is missing"));
    QVERIFY(!QFile::exists(LibraryPaths::libraryDatabasePath(emptyLibrary)));

    const auto blockedData = root->filePath("blocked-data");
    QFile blocker(blockedData);
    QVERIFY(blocker.open(QIODevice::WriteOnly));
    blocker.close();
    LibraryCreator creator(&settings);
    QSignalSpy creationFailure(&creator, &LibraryCreator::failedCreatingDB);
    creator.createLibrary(emptyLibrary, blockedData);
    creator.start();
    QVERIFY(creator.wait(30000));
    QCOMPARE(creationFailure.count(), 1);
    QVERIFY(creationFailure.first().first().toString().contains("Unable to create the library data folder"));
    QVERIFY(QFileInfo(blockedData).isFile());
}

void LibraryMaintenanceTest::serverCreationOnlyRegistersSuccess()
{
    const auto oldName = QCoreApplication::applicationName();
    const auto oldOrganization = QCoreApplication::organizationName();
    const auto oldTestMode = QStandardPaths::isTestModeEnabled();
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("YACReaderTests");
    QCoreApplication::setApplicationName(QUuid::createUuid().toString(QUuid::WithoutBraces));
    const auto registryPath = getSettingsPath() + "/" + QCoreApplication::applicationName() + ".ini";
    const auto restoreSettings = qScopeGuard([&] {
        QFile::remove(registryPath);
        QCoreApplication::setApplicationName(oldName);
        QCoreApplication::setOrganizationName(oldOrganization);
        QStandardPaths::setTestModeEnabled(oldTestMode);
    });
    QVERIFY(QDir().mkpath(getSettingsPath()));
    QSettings settings(root->filePath("server-settings.ini"), QSettings::IniFormat);
    ConsoleUILibraryCreator console(&settings);
    const auto retryLibrary = root->filePath("retry-library");
    QVERIFY(QDir().mkpath(LibraryPaths::libraryDataPath(retryLibrary)));
    {
        LibraryMaintenanceLock owner(retryLibrary);
        QVERIFY(owner.tryLock());
        QVERIFY(!console.createLibrary("created", retryLibrary));
    }
    YACReaderLibraries registry;
    registry.load();
    QVERIFY(!registry.contains("created"));

    QVERIFY(console.createLibrary("created", retryLibrary));
    registry.load();
    QVERIFY(registry.contains("created"));
    QVERIFY(!console.createLibrary("created", retryLibrary));
}

void LibraryMaintenanceTest::unknownMaintenanceOwner_data()
{
    QTest::addColumn<QByteArray>("contents");
    const auto pid = QByteArray::number(QCoreApplication::applicationPid());
    auto host = QSysInfo::machineHostName().toUtf8();
#ifdef Q_OS_WIN
    host = qgetenv("COMPUTERNAME");
#endif
    QTest::newRow("remote") << pid + "\nother-app\nother-host\nother-machine\n";
    QTest::newRow("changed-machine") << pid + "\nother-app\n" + host + "\nother-machine\n";
    QTest::newRow("malformed") << QByteArray("not a lock record");
}

void LibraryMaintenanceTest::unknownMaintenanceOwner()
{
    QFETCH(QByteArray, contents);
    QFile file(QDir(dataPath).filePath("maintenance.lock"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write(contents), contents.size());
    // Even an old record is not safe to expire just because of its age.
    QVERIFY(file.setFileTime(QDateTime::currentDateTimeUtc().addYears(-1), QFileDevice::FileModificationTime));
    file.close();
    LibraryMaintenanceLock lock(libraryPath);
    QVERIFY(!lock.tryLock());
    QCOMPARE(lock.error(), QLockFile::LockFailedError);
    QVERIFY(!lock.holderIsRunning());
    QVERIFY(file.exists());
}

void LibraryMaintenanceTest::deadLocalOwnerIsRecoveredAutomatically()
{
    auto host = QSysInfo::machineHostName().toUtf8();
#ifdef Q_OS_WIN
    host = qgetenv("COMPUTERNAME");
#endif
    QFile file(QDir(dataPath).filePath("maintenance.lock"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write("2147483647\nold-app\n" + host + "\n" + QSysInfo::machineUniqueId() + "\n") > 0);
    file.close();
    LibraryMaintenanceLock lock(libraryPath);
    QVERIFY2(lock.tryLock(), qPrintable(lock.errorString()));
}

void LibraryMaintenanceTest::confirmedRecovery_data()
{
    activeMaintenanceLocks_data();
}

void LibraryMaintenanceTest::confirmedRecovery()
{
    QFETCH(QString, fileName);
    QFile file(QDir(dataPath).filePath(fileName));
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write("123\nold-app\nprevious-computer\nprevious-machine\n") > 0);
    file.close();
    LibraryMaintenanceLock lock(libraryPath);
    QVERIFY(!lock.tryLock());
    QVERIFY2(lock.tryLock(true), qPrintable(lock.errorString()));
    QCOMPARE(lock.error(), QLockFile::NoError);
    QVERIFY(lock.errorString().isEmpty());
    LibraryMaintenanceLock contender(libraryPath);
    QVERIFY(!contender.tryLock(true));
}

void LibraryMaintenanceTest::recoveryRejectsChangedOwner()
{
    QFile file(QDir(dataPath).filePath("maintenance.lock"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write("123\nold-app\nprevious-computer\nprevious-machine\n") > 0);
    file.close();
    LibraryMaintenanceLock lock(libraryPath);
    QVERIFY(!lock.tryLock());
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    const QByteArray successor("456\nnew-app\nanother-computer\nanother-machine\n");
    QCOMPARE(file.write(successor), successor.size());
    file.close();
    QVERIFY(!lock.tryLock(true));
    QVERIFY(lock.errorString().contains("owner changed"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), successor);
}

void LibraryMaintenanceTest::recoveryDoesNotDisplaceLiveProcess()
{
    const auto readyPath = root->filePath("holder-ready");
    QProcess holder;
    const auto cleanup = qScopeGuard([&] {
        holder.kill();
        holder.waitForFinished(10000);
    });
    holder.start(QCoreApplication::applicationFilePath(), { "--hold-maintenance-lock", libraryPath, readyPath });
    QVERIFY(holder.waitForStarted(10000));
    QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(readyPath), 10000);
    LibraryMaintenanceLock lock(libraryPath);
    QVERIFY(!lock.tryLock());
    QVERIFY(lock.holderIsRunning());
    QVERIFY(!lock.tryLock(true));
    QVERIFY(lock.errorString().contains("Another maintenance operation"));
    QVERIFY(QFile::exists(QDir(dataPath).filePath("maintenance.lock")));
}

void LibraryMaintenanceTest::undeletableStaleLockNamesFileAccess()
{
#ifdef Q_OS_WIN
    auto host = qgetenv("COMPUTERNAME");
    const auto lockPath = QDir(dataPath).filePath("maintenance.lock");
    QFile file(lockPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write("2147483647\nold-app\n" + host + "\n" + QSysInfo::machineUniqueId() + "\n") > 0);
    file.close();
    // QLockFile cannot remove a read-only file, so its dead owner's record stays
    QVERIFY(QFile::setPermissions(lockPath, QFileDevice::ReadOwner | QFileDevice::ReadUser));
    const auto restoreWrite = qScopeGuard([&] {
        QFile::setPermissions(lockPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser);
    });
    LibraryMaintenanceLock lock(libraryPath);
    QVERIFY(!lock.tryLock());
    QCOMPARE(lock.error(), QLockFile::LockFailedError);
    QVERIFY(!lock.holderIsRunning());
    QVERIFY(!lock.tryLock(true));
    QVERIFY(lock.errorString().contains("read-only"));
    QVERIFY(QFile::exists(lockPath));

    QVERIFY(QFile::setPermissions(lockPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser));
    QVERIFY2(lock.tryLock(), qPrintable(lock.errorString()));
#else
    QSKIP("A read-only file in a writable folder can be removed on this platform.");
#endif
}

void LibraryMaintenanceTest::concurrentRecoveriesAreExclusive()
{
    QFile file(QDir(dataPath).filePath("maintenance.lock"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write("123\nold-app\nprevious-computer\nprevious-machine\n") > 0);
    file.close();
    LibraryMaintenanceLock first(libraryPath);
    LibraryMaintenanceLock second(libraryPath);
    QVERIFY(!first.tryLock());
    QVERIFY(!second.tryLock());
    auto one = std::async(std::launch::async, [&] { return first.tryLock(true); });
    auto two = std::async(std::launch::async, [&] { return second.tryLock(true); });
    QVERIFY(one.get() != two.get());
}

void LibraryMaintenanceTest::serverRecoveryIsExplicitAndNoninteractive()
{
    QFile file(QDir(dataPath).filePath("maintenance.lock"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    const QByteArray contents("123\nold-app\nprevious-computer\nprevious-machine\n");
    QCOMPARE(file.write(contents), contents.size());
    file.close();
    QSettings settings(root->filePath("server-settings.ini"), QSettings::IniFormat);
    ConsoleUILibraryCreator console(&settings);
    // These must return without trying to read stdin or removing the record.
    QVERIFY(!console.updateLibrary(libraryPath));
    QCOMPARE(console.repairLibrary(libraryPath), 1);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), contents);
    file.close();
    QVERIFY(console.updateLibrary(libraryPath, true));
    QVERIFY(!file.exists());
    LibraryMaintenanceLock active(libraryPath);
    QVERIFY(active.tryLock());
    QVERIFY(!console.updateLibrary(libraryPath, true));
    QVERIFY(file.exists());
}

void LibraryMaintenanceTest::backgroundUpdatesRecordLockProblems()
{
    const auto oldName = QCoreApplication::applicationName();
    const auto oldOrganization = QCoreApplication::organizationName();
    const auto oldTestMode = QStandardPaths::isTestModeEnabled();
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("YACReaderTests");
    QCoreApplication::setApplicationName(QUuid::createUuid().toString(QUuid::WithoutBraces));
    const auto restoreSettings = qScopeGuard([&] {
        QCoreApplication::setApplicationName(oldName);
        QCoreApplication::setOrganizationName(oldOrganization);
        QStandardPaths::setTestModeEnabled(oldTestMode);
    });
    const auto otherPath = root->filePath("other-library");
    const auto otherData = LibraryPaths::libraryDataPath(otherPath);
    QVERIFY(QDir().mkpath(otherData));
    QString connection;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(otherPath));
        QVERIFY(db.isOpen());
        connection = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connection);
    YACReaderLibraries registry;
    registry.addLibrary("Blocked", libraryPath);
    registry.addLibrary("Available", otherPath);
    QSettings settings(root->filePath("background-settings.ini"), QSettings::IniFormat);
    LibrariesUpdateCoordinator coordinator(&settings, registry, [] { return true; });
    QSignalSpy finished(&coordinator, &LibrariesUpdateCoordinator::updateEnded);
    {
        LibraryMaintenanceLock owner(libraryPath);
        QVERIFY(owner.tryLock());
        QCOMPARE(coordinator.requestLibrariesUpdate(), LibrariesUpdateCoordinator::UpdateRequestResult::Started);
        QTRY_COMPARE_WITH_TIMEOUT(finished.count(), 1, 30000);
        QTRY_VERIFY(!coordinator.isRunning());
    }
    const auto problems = coordinator.lockProblems();
    QCOMPARE(problems.size(), 1);
    const auto problem = problems.first().toObject();
    QCOMPARE(problem.value("library_id").toInt(), registry.getId("Blocked"));
    QCOMPARE(problem.value("operation").toString(), QString("update"));
    QCOMPARE(problem.value("state").toString(), QString("busy"));
    QVERIFY(!problem.contains("recovery_token"));

    QCOMPARE(coordinator.requestSingleLibraryUpdate(registry.getId("Blocked")), LibrariesUpdateCoordinator::UpdateRequestResult::Started);
    QTRY_COMPARE_WITH_TIMEOUT(finished.count(), 2, 30000);
    QTRY_VERIFY(!coordinator.isRunning());
    QVERIFY(coordinator.lockProblems().isEmpty());
}

void LibraryMaintenanceTest::rapidMaintenanceBackupsRemainDistinct()
{
    LibraryMaintenanceLock maintenance(libraryPath);
    QVERIFY(maintenance.tryLock());
    QString error;
    QString firstBackup;
    QByteArray firstContents;
    for (int i = 0; i < 3; ++i) {
        QVERIFY2(DataBaseManagement::backupLibrary(libraryPath, DatabaseBackupReason::BeforeRepair, &error), qPrintable(error));
        const auto backups = DataBaseManagement::libraryBackups(libraryPath);
        QCOMPARE(backups.size(), i + 1);
        if (i == 0) {
            firstBackup = backups.first().absoluteFilePath();
            QFile file(firstBackup);
            QVERIFY(file.open(QIODevice::ReadOnly));
            firstContents = file.readAll();
        }
    }
    QFile original(firstBackup);
    QVERIFY(original.open(QIODevice::ReadOnly));
    QCOMPARE(original.readAll(), firstContents);
}

void LibraryMaintenanceTest::killedProcessLocksAreRecovered()
{
    const auto readyPath = root->filePath("holder-ready");
    QProcess holder;
    const auto cleanup = qScopeGuard([&] {
        if (holder.state() != QProcess::NotRunning) {
            holder.kill();
            holder.waitForFinished(10000);
        }
    });
    holder.start(QCoreApplication::applicationFilePath(), { "--hold-maintenance-lock", libraryPath, readyPath });
    QVERIFY(holder.waitForStarted(10000));
    QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(readyPath), 10000);
    holder.kill();
    QVERIFY(holder.waitForFinished(10000));
    QVERIFY(QFile::exists(QDir(dataPath).filePath("maintenance.lock")));
    QVERIFY(QFile::exists(QDir(dataPath).filePath("repair.lock")));
    LibraryMaintenanceLock recovered(libraryPath);
    QVERIFY2(recovered.tryLock(), qPrintable(recovered.errorString()));
    LibraryMaintenanceLock contender(libraryPath);
    QVERIFY(!contender.tryLock());
}

void LibraryMaintenanceTest::lockDirectoriesAreNotContention_data()
{
    activeMaintenanceLocks_data();
}

void LibraryMaintenanceTest::lockDirectoriesAreNotContention()
{
    QFETCH(QString, fileName);
    const auto lockPath = QDir(dataPath).filePath(fileName);
    QVERIFY(QDir().mkdir(lockPath));
    LibraryMaintenanceLock lock(libraryPath);
    QVERIFY(!lock.tryLock(true));
    QCOMPARE(lock.error(), QLockFile::UnknownError);
    QVERIFY(lock.errorString().contains("folder is occupying"));
    QVERIFY(QDir(lockPath).exists());
    QVERIFY(QDir().rmdir(lockPath));
    QVERIFY2(lock.tryLock(), qPrintable(lock.errorString()));
}

void LibraryMaintenanceTest::repairSettlesInterruptedRestore()
{
    const auto databasePath = LibraryPaths::libraryDatabasePath(libraryPath);
    const auto backup = root->filePath("restore-original.ydb");
    QString error;
    QVERIFY2(DataBaseManagement::backupLibrary(libraryPath, DatabaseBackupReason::Manual, &error, backup), qPrintable(error));
    const auto interruptRestore = [&] {
        if (!QFile::copy(backup, databasePath + ".rollback"))
            return false;
        QFile marker(databasePath + ".restore");
        return marker.open(QIODevice::WriteOnly) && marker.write("original=1\n") > 0;
    };
    QVERIFY(exec("UPDATE comic_info SET title = 'Interrupted' WHERE id = 1"));
    QVERIFY(interruptRestore());
    ComicInfoRepairer repairer(nullptr);
    repairer.repairLibrary(libraryPath, dataPath);
    QVERIFY(repairer.wait(30000));
    QVERIFY2(repairer.summary().error.isEmpty(), qPrintable(repairer.summary().error));
    QCOMPARE(rows("SELECT title FROM comic_info WHERE id = 1"), QList<QVariantList>({ { "Title 1" } }));
    QVERIFY(!QFile::exists(databasePath + ".restore"));
    QVERIFY(exec("UPDATE comic_info SET title = 'Interrupted again' WHERE id = 1"));
    QVERIFY(interruptRestore());
    const auto result = DataBaseManagement::salvageLibrary(libraryPath);
    QCOMPARE(result.status, DatabaseSalvageStatus::AlreadyValid);
    QCOMPARE(rows("SELECT title FROM comic_info WHERE id = 1"), QList<QVariantList>({ { "Title 1" } }));
    QVERIFY(!QFile::exists(databasePath + ".restore"));
    QVERIFY(exec("UPDATE comic_info SET title = 'Interrupted upgrade' WHERE id = 1"));
    QVERIFY(exec("UPDATE db_info SET version = '0.0.0'"));
    QVERIFY(interruptRestore());
    QVERIFY(DataBaseManagement::updateToCurrentVersion(libraryPath));
    QCOMPARE(rows("SELECT title FROM comic_info WHERE id = 1"), QList<QVariantList>({ { "Title 1" } }));
    QVERIFY(!QFile::exists(databasePath + ".restore"));
}

void LibraryMaintenanceTest::salvageChecksLocksEvenForValidDatabase()
{
    {
        LibraryMaintenanceLock owner(libraryPath);
        QVERIFY(owner.tryLock());
        QCOMPARE(DataBaseManagement::salvageLibrary(libraryPath).status, DatabaseSalvageStatus::LockFailed);
    }
    QFile file(QDir(dataPath).filePath("maintenance.lock"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write("123\nold-app\nold-host\nold-machine\n") > 0);
    file.close();
    QCOMPARE(DataBaseManagement::salvageLibrary(libraryPath).status, DatabaseSalvageStatus::LockFailed);
    QCOMPARE(DataBaseManagement::salvageLibrary(libraryPath, true).status, DatabaseSalvageStatus::AlreadyValid);
    QVERIFY(!file.exists());
}

void LibraryMaintenanceTest::salvageRebuildsWithoutLosingRows()
{
    const auto before = listData();
    QFile file(LibraryPaths::libraryDatabasePath(libraryPath));
    QVERIFY(file.open(QIODevice::ReadWrite));
    // Corrupt only the freelist pointer in a temporary fixture. Table pages
    // remain readable, but quick_check fails until VACUUM rebuilds the file.
    QVERIFY(file.seek(32));
    QCOMPARE(file.write(QByteArray::fromHex("7fffffff00000001")), qint64(8));
    file.close();
    const auto result = DataBaseManagement::salvageLibrary(libraryPath);
    QVERIFY2(result.success(), qPrintable(result.error));
    QCOMPARE(result.status, DatabaseSalvageStatus::Rebuilt);
    QCOMPARE(listData(), before);
    QVERIFY(QFile::exists(result.preservedDatabasePath));
    LibraryMaintenanceLock next(libraryPath);
    QVERIFY(next.tryLock());
}

void LibraryMaintenanceTest::directRecreationHonorsOwnership()
{
    QString error;
    const auto database = LibraryPaths::libraryDatabasePath(libraryPath);
    const auto backup = QDir(dataPath).filePath("backups/preserve.ydb");
    QVERIFY(QDir().mkpath(QFileInfo(backup).absolutePath()));
    QVERIFY(DataBaseManagement::backupLibrary(libraryPath, DatabaseBackupReason::Manual, &error, backup));
    {
        LibraryMaintenanceLock owner(libraryPath);
        QVERIFY(owner.tryLock());
        QVERIFY(!DataBaseManagement::prepareForRecreation(libraryPath, &error));
        QVERIFY(!DataBaseManagement::updateToCurrentVersion(libraryPath));
        QVERIFY(QFile::exists(database));
    }
    QVERIFY2(DataBaseManagement::prepareForRecreation(libraryPath, &error), qPrintable(error));
    QVERIFY(!QFile::exists(database));
    QVERIFY(QFile::exists(backup));
    LibraryMaintenanceLock next(libraryPath);
    QVERIFY(next.tryLock());
}

void LibraryMaintenanceTest::creationRetryPreservesRecoveredDatabase_data()
{
    QTest::addColumn<QString>("entryPoint");
    QTest::newRow("desktop-retry") << QString("retry");
    QTest::newRow("automatic-restore-recovery") << QString("direct");
    QTest::newRow("cli-explicit-recovery") << QString("cli");
}

void LibraryMaintenanceTest::creationRetryPreservesRecoveredDatabase()
{
    QFETCH(QString, entryPoint);
    const auto databasePath = LibraryPaths::libraryDatabasePath(libraryPath);
    QVERIFY(exec("UPDATE comic_info SET currentPage = 7, bookmark1 = 5 WHERE id = 2"));
    const auto expectedLists = rows("SELECT id, name FROM reading_list ORDER BY id");
    const auto expectedEntries = rows("SELECT reading_list_id, ordering, comic_info_id, placeholder_id FROM reading_list_entry ORDER BY id");
    const auto expectedProgress = rows("SELECT currentPage, bookmark1 FROM comic_info WHERE id = 2");
    QVERIFY(!expectedLists.isEmpty());
    QVERIFY(QFile::rename(databasePath, databasePath + ".rollback"));
    QFile marker(databasePath + ".restore");
    QVERIFY(marker.open(QIODevice::WriteOnly));
    QVERIFY(marker.write("original=1\n") > 0);
    marker.close();
    if (entryPoint != "direct") {
        QFile stale(QDir(dataPath).filePath("maintenance.lock"));
        QVERIFY(stale.open(QIODevice::WriteOnly));
        QVERIFY(stale.write("123\nold-app\nold-computer\nold-machine\n") > 0);
    }
    QSettings settings(root->filePath("retry.ini"), QSettings::IniFormat);
    if (entryPoint == "cli") {
        const auto oldName = QCoreApplication::applicationName();
        const auto oldOrganization = QCoreApplication::organizationName();
        const auto oldTestMode = QStandardPaths::isTestModeEnabled();
        QStandardPaths::setTestModeEnabled(true);
        QCoreApplication::setOrganizationName("YACReaderTests");
        QCoreApplication::setApplicationName(QUuid::createUuid().toString(QUuid::WithoutBraces));
        const auto registryPath = getSettingsPath() + "/" + QCoreApplication::applicationName() + ".ini";
        const auto restoreSettings = qScopeGuard([&] {
            QFile::remove(registryPath);
            QCoreApplication::setApplicationName(oldName);
            QCoreApplication::setOrganizationName(oldOrganization);
            QStandardPaths::setTestModeEnabled(oldTestMode);
        });
        QVERIFY(QDir().mkpath(getSettingsPath()));
        ConsoleUILibraryCreator console(&settings);
        QVERIFY(console.createLibrary("recovered", libraryPath, true));
        YACReaderLibraries registry;
        registry.load();
        QVERIFY(registry.contains("recovered"));
    } else {
        LibraryCreator creator(&settings);
        QSignalSpy failures(&creator, &LibraryCreator::failedCreatingDB);
        QSignalSpy created(&creator, &LibraryCreator::created);
        creator.createLibrary(libraryPath, dataPath);
        if (entryPoint == "retry") {
            creator.start();
            QVERIFY(creator.wait(30000));
            QCOMPARE(failures.count(), 1);
            QCOMPARE(creator.maintenanceFailure().error, QLockFile::LockFailedError);
            {
                LibraryMaintenanceLock recovery(libraryPath);
                QVERIFY(recovery.tryLock(true));
                QString error;
                QVERIFY2(DataBaseManagement::recoverInterruptedRestore(libraryPath, &error, true), qPrintable(error));
            }
            QCOMPARE(rows("SELECT id, name FROM reading_list ORDER BY id"), expectedLists);
        }
        // The desktop coordinator retries the same worker without re-preparing it.
        creator.start();
        QVERIFY(creator.wait(30000));
        QCOMPARE(created.count(), 1);
    }
    QCOMPARE(rows("SELECT id, name FROM reading_list ORDER BY id"), expectedLists);
    QCOMPARE(rows("SELECT reading_list_id, ordering, comic_info_id, placeholder_id FROM reading_list_entry ORDER BY id"), expectedEntries);
    QCOMPARE(rows("SELECT currentPage, bookmark1 FROM comic_info WHERE id = 2"), expectedProgress);
    QVERIFY(!QFile::exists(databasePath + ".restore"));
    QVERIFY(!QFile::exists(databasePath + ".rollback"));
}
int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    const auto arguments = application.arguments();
    if (arguments.size() == 4 && arguments[1] == "--hold-maintenance-lock") {
        LibraryMaintenanceLock lock(arguments[2]);
        if (!lock.tryLock())
            return 2;
        QFile ready(arguments[3]);
        if (!ready.open(QIODevice::WriteOnly) || ready.write("ready") != 5)
            return 3;
        ready.close();
        return application.exec();
    }
    LibraryMaintenanceTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "main.moc"
