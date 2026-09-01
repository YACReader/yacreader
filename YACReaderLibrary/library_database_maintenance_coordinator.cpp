#include "library_database_maintenance_coordinator.h"

#include "data_base_management.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QThread>
#include <QWidget>

#include <memory>
#include <utility>

using namespace YACReader;

LibraryDatabaseMaintenanceCoordinator::LibraryDatabaseMaintenanceCoordinator(YACReaderLibraries &libraries, QWidget *dialogParent, CurrentLibraryNameProvider currentLibraryNameProvider)
    : QObject(dialogParent), libraries(libraries), dialogParent(dialogParent), currentLibraryNameProvider(std::move(currentLibraryNameProvider))
{
}

void LibraryDatabaseMaintenanceCoordinator::backupCurrentLibrary(const QString &dialogTitle)
{
    backupLibrary(libraries.getPath(currentLibraryNameProvider()), dialogTitle);
}

void LibraryDatabaseMaintenanceCoordinator::restoreCurrentLibrary(const QString &dialogTitle)
{
    const auto libraryName = currentLibraryNameProvider();
    restoreLibrary(libraryName, libraries.getPath(libraryName), dialogTitle);
}

void LibraryDatabaseMaintenanceCoordinator::backupLibrary(const QString &libraryPath, const QString &dialogTitle)
{
    if (libraryPath.isEmpty())
        return;

    auto version = DataBaseManagement::checkValidDB(LibraryPaths::libraryDatabasePath(libraryPath));
    if (version.isEmpty())
        version = "unknown";
    const auto suggestedName = QString("library-%1-db-%2-manual.ydb")
                                       .arg(QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss"), version);
    const auto destination = QFileDialog::getSaveFileName(dialogParent,
                                                          dialogTitle,
                                                          QDir::home().filePath(suggestedName),
                                                          QCoreApplication::translate("LibraryWindow", "YACReader library database (*.ydb)"));
    if (destination.isEmpty())
        return;

    struct BackupResult {
        bool success { false };
        QString error;
    };

    auto result = std::make_shared<BackupResult>();
    auto worker = QThread::create([libraryPath, destination, result] {
        result->success = DataBaseManagement::backupLibrary(libraryPath, DatabaseBackupReason::Manual, &result->error, destination);
    });

    emit backupAvailabilityChanged(false);
    connect(worker, &QThread::finished, this, [this, destination, dialogTitle, result] {
        emit backupAvailabilityChanged(true);
        if (result->success) {
            QMessageBox::information(dialogParent,
                                     dialogTitle,
                                     QCoreApplication::translate("LibraryWindow", "The library database backup was created at:\n%1").arg(destination));
        } else {
            QMessageBox::critical(dialogParent,
                                  dialogTitle,
                                  QCoreApplication::translate("LibraryWindow", "Unable to create the library database backup:\n%1").arg(result->error));
        }
    });
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}

void LibraryDatabaseMaintenanceCoordinator::restoreLibrary(const QString &libraryName, const QString &libraryPath, const QString &dialogTitle)
{
    if (libraryPath.isEmpty())
        return;

    const auto backupPath = QFileDialog::getOpenFileName(dialogParent,
                                                         dialogTitle,
                                                         QDir(LibraryPaths::libraryDataPath(libraryPath)).filePath("backups"),
                                                         QCoreApplication::translate("LibraryWindow", "YACReader library database (*.ydb)"));
    if (backupPath.isEmpty())
        return;

    const auto answer = QMessageBox::warning(dialogParent,
                                             dialogTitle,
                                             QCoreApplication::translate("LibraryWindow", "Close YACReaderLibraryServer and any other YACReader application using this library before restoring. Continue?"),
                                             QMessageBox::Yes | QMessageBox::Cancel,
                                             QMessageBox::Cancel);
    if (answer == QMessageBox::Yes)
        startLibraryRestore(libraryName, libraryPath, backupPath, dialogTitle);
}

void LibraryDatabaseMaintenanceCoordinator::startLibraryRestore(const QString &libraryName, const QString &libraryPath, const QString &backupPath, const QString &dialogTitle, bool allowInvalidCurrent, bool removeStaleLock)
{
    auto result = std::make_shared<DatabaseRestoreResult>();
    auto progress = new QProgressDialog(QCoreApplication::translate("LibraryWindow", "Restoring library database..."), QString(), 0, 0, dialogParent);
    progress->setCancelButton(nullptr);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);

    emit maintenanceStarted();

    auto worker = QThread::create([libraryPath, backupPath, allowInvalidCurrent, removeStaleLock, result] {
        *result = DataBaseManagement::restoreLibrary(libraryPath, backupPath, allowInvalidCurrent, removeStaleLock);
    });
    connect(worker, &QThread::finished, this, [this, libraryName, libraryPath, backupPath, dialogTitle, allowInvalidCurrent, result, progress] {
        progress->deleteLater();

        if (result->status == DatabaseRestoreStatus::InvalidCurrentDatabase && !allowInvalidCurrent) {
            const auto answer = QMessageBox::warning(dialogParent,
                                                     dialogTitle,
                                                     QCoreApplication::translate("LibraryWindow", "The current library database is invalid. Restore the selected backup anyway?"),
                                                     QMessageBox::Yes | QMessageBox::Cancel,
                                                     QMessageBox::Cancel);
            if (answer == QMessageBox::Yes) {
                startLibraryRestore(libraryName, libraryPath, backupPath, dialogTitle, true);
                return;
            }
            emit invalidDatabaseRestoreCancelled();
            return;
        } else if (result->status == DatabaseRestoreStatus::LockFailed && !result->lockHolderIsRunningLocally) {
            const auto answer = QMessageBox::warning(dialogParent,
                                                     dialogTitle,
                                                     QCoreApplication::translate("LibraryWindow", "The library maintenance lock may be stale. Remove it and retry?"),
                                                     QMessageBox::Yes | QMessageBox::Cancel,
                                                     QMessageBox::Cancel);
            if (answer == QMessageBox::Yes) {
                startLibraryRestore(libraryName, libraryPath, backupPath, dialogTitle, allowInvalidCurrent, true);
                return;
            }
            emit libraryReloadRequested(libraryName);
            return;
        }

        if (!result->success()) {
            auto error = result->error;
            if (result->status == DatabaseRestoreStatus::RollbackFailed)
                error += QCoreApplication::translate("LibraryWindow", "\n\nRestart YACReaderLibrary before attempting recovery again.");
            QMessageBox::critical(dialogParent, dialogTitle, error);
            if (result->status != DatabaseRestoreStatus::RollbackFailed)
                emit libraryReloadRequested(libraryName);
            else
                emit databaseUnavailableAfterRestore();
            return;
        }

        emit libraryReloadRequested(libraryName);
        const auto answer = QMessageBox::question(dialogParent,
                                                  dialogTitle,
                                                  QCoreApplication::translate("LibraryWindow", "The library database was restored successfully. Update the library now?"),
                                                  QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::Yes);
        if (answer == QMessageBox::Yes)
            emit libraryUpdateRequested();
    });
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}

void LibraryDatabaseMaintenanceCoordinator::offerDatabaseRecovery(const QString &libraryName, const QString &restoreDialogTitle)
{
    const auto libraryPath = libraries.getPath(libraryName);
    QMessageBox messageBox(QMessageBox::Warning,
                           QCoreApplication::translate("LibraryWindow", "Library database damaged"),
                           QCoreApplication::translate("LibraryWindow", "The database of library '%1' is damaged, so normal updates, maintenance, and backups are unavailable. YACReader can attempt to repair the database. Some damaged data may not be recoverable. Existing backups will not be changed.").arg(libraryName),
                           QMessageBox::NoButton,
                           dialogParent);
    const auto repairButton = messageBox.addButton(QCoreApplication::translate("LibraryWindow", "Attempt repair"), QMessageBox::AcceptRole);
    const auto restoreButton = messageBox.addButton(QCoreApplication::translate("LibraryWindow", "Restore a backup..."), QMessageBox::ActionRole);
    messageBox.addButton(QMessageBox::Cancel);
    messageBox.setWindowModality(Qt::WindowModal);
    messageBox.exec();

    if (messageBox.clickedButton() == repairButton)
        startDatabaseSalvage(libraryName, libraryPath);
    else if (messageBox.clickedButton() == restoreButton)
        restoreLibrary(libraryName, libraryPath, restoreDialogTitle);
}

void LibraryDatabaseMaintenanceCoordinator::startDatabaseSalvage(const QString &libraryName, const QString &libraryPath, bool removeStaleLock)
{
    if (libraryPath.isEmpty())
        return;

    auto result = std::make_shared<DatabaseSalvageResult>();
    auto progress = new QProgressDialog(QCoreApplication::translate("LibraryWindow", "Repairing library database..."), QString(), 0, 0, dialogParent);
    progress->setCancelButton(nullptr);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);

    auto worker = QThread::create([libraryPath, removeStaleLock, result] {
        *result = DataBaseManagement::salvageLibrary(libraryPath, removeStaleLock);
    });
    connect(worker, &QThread::finished, this, [this, libraryName, libraryPath, result, progress] {
        progress->deleteLater();

        if (result->status == DatabaseSalvageStatus::LockFailed) {
            if (!result->lockHolderIsRunningLocally) {
                const auto answer = QMessageBox::warning(dialogParent,
                                                         QCoreApplication::translate("LibraryWindow", "Library database repair"),
                                                         QCoreApplication::translate("LibraryWindow", "The library maintenance lock may be stale. Remove it and retry?"),
                                                         QMessageBox::Yes | QMessageBox::Cancel,
                                                         QMessageBox::Cancel);
                if (answer == QMessageBox::Yes)
                    startDatabaseSalvage(libraryName, libraryPath, true);
            } else {
                QMessageBox::warning(dialogParent,
                                     QCoreApplication::translate("LibraryWindow", "Library database repair"),
                                     QCoreApplication::translate("LibraryWindow", "Another maintenance operation is currently using this library. Try again after it finishes."));
            }
            return;
        }

        if (result->success()) {
            emit libraryReloadRequested(libraryName);
            if (result->status == DatabaseSalvageStatus::AlreadyValid) {
                QMessageBox::information(dialogParent,
                                         QCoreApplication::translate("LibraryWindow", "Library database repair"),
                                         QCoreApplication::translate("LibraryWindow", "The library database is already valid."));
            } else if (result->status == DatabaseSalvageStatus::Reindexed) {
                QMessageBox::information(dialogParent,
                                         QCoreApplication::translate("LibraryWindow", "Library database repaired"),
                                         QCoreApplication::translate("LibraryWindow", "The library database was repaired by rebuilding its indexes. The damaged original was preserved at:\n%1").arg(result->preservedDatabasePath));
            } else {
                const auto answer = QMessageBox::question(dialogParent,
                                                          QCoreApplication::translate("LibraryWindow", "Library database rebuilt"),
                                                          QCoreApplication::translate("LibraryWindow", "The library database was rebuilt successfully. The damaged original was preserved at:\n%1\n\nUpdate the library now?").arg(result->preservedDatabasePath),
                                                          QMessageBox::Yes | QMessageBox::No,
                                                          QMessageBox::Yes);
                if (answer == QMessageBox::Yes)
                    emit libraryUpdateRequested();
            }
        } else {
            const auto recovery = result->preservedDatabasePath.isEmpty()
                    ? QString()
                    : QCoreApplication::translate("LibraryWindow", "\n\nThe damaged original was preserved at:\n%1").arg(result->preservedDatabasePath);
            QMessageBox::critical(dialogParent,
                                  QCoreApplication::translate("LibraryWindow", "Library database repair failed"),
                                  QCoreApplication::translate("LibraryWindow", "The library database could not be repaired:\n%1%2\n\nYou can restore a backup from the Library menu or recreate the library.").arg(result->error, recovery));
            emit databaseSalvageFailed();
        }
    });
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}
