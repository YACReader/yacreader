#include "library_repair_coordinator.h"

#include "comic_info_repairer.h"
#include "data_base_management.h"
#include "library_maintenance_dialog.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <QCoreApplication>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QWidget>

#include <utility>

using namespace YACReader;

LibraryRepairCoordinator::LibraryRepairCoordinator(QSettings *settings, YACReaderLibraries &libraries, QWidget *dialogParent, CurrentLibraryNameProvider currentLibraryNameProvider)
    : QObject(dialogParent), libraries(libraries), dialogParent(dialogParent), currentLibraryNameProvider(std::move(currentLibraryNameProvider)), repairer(new ComicInfoRepairer(settings, this))
{
    connect(repairer, &QThread::finished, this, &LibraryRepairCoordinator::handleFinished);
    connect(repairer, &ComicInfoRepairer::comicProcessed, this, &LibraryRepairCoordinator::comicProcessed);
    connect(repairer, &ComicInfoRepairer::failed, this, &LibraryRepairCoordinator::handleFailure);
}

void LibraryRepairCoordinator::repairCurrentLibrary(const QString &dialogTitle)
{
    if (repairer->isRunning())
        return;

    libraryName = currentLibraryNameProvider();
    libraryPath = libraries.getPath(libraryName);
    this->dialogTitle = dialogTitle;
    recoveryAttempted = false;
    startRepair(false);
}

void LibraryRepairCoordinator::startRepair(bool removeStaleLock)
{
    if (libraryPath.isEmpty())
        return;

    emit repairStarted();
    repairer->repairLibrary(libraryPath, LibraryPaths::libraryDataPath(libraryPath), removeStaleLock);
}

void LibraryRepairCoordinator::stop()
{
    repairer->stop();
    repairer->wait();
}

void LibraryRepairCoordinator::handleFinished()
{
    const auto summary = repairer->summary();
    emit repairFinished();

    if (summary.lockedByAnotherProcess) {
        if (!recoveryAttempted) {
            recoveryAttempted = true;
            if (recoverLibraryMaintenance(dialogParent, libraryPath, dialogTitle))
                startRepair(false);
        } else {
            QMessageBox::critical(dialogParent, dialogTitle, summary.error);
        }
        return;
    }

    if (summary.canceled || !summary.error.isEmpty())
        return;

    QMessageBox messageBox(QMessageBox::Information,
                           dialogTitle,
                           QCoreApplication::translate("LibraryWindow", "Repaired: %1\nFailed: %2\nMissing files: %3").arg(summary.repaired).arg(summary.failed).arg(summary.missingFiles),
                           QMessageBox::Ok,
                           dialogParent);
    if (!summary.failedFilePaths.isEmpty())
        messageBox.setDetailedText(summary.failedFilePaths.join('\n'));
    messageBox.exec();
}

void LibraryRepairCoordinator::handleFailure(const QString &error)
{
    if (repairer->summary().maintenanceError != QLockFile::NoError) {
        QMessageBox::critical(dialogParent, dialogTitle, error);
        return;
    }
    if (!libraryPath.isEmpty() && QFile::exists(LibraryPaths::libraryDatabasePath(libraryPath)) && !DataBaseManagement::isLibraryDatabaseValid(libraryPath)) {
        emit databaseRecoveryRequested(libraryName);
        return;
    }
    QMessageBox::critical(dialogParent, dialogTitle, error);
}
