#include "library_repair_coordinator.h"

#include "comic_info_repairer.h"
#include "data_base_management.h"
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
        if (summary.lockHolderIsRunningLocally) {
            QMessageBox::information(dialogParent,
                                     dialogTitle,
                                     QCoreApplication::translate("LibraryWindow", "A repair of this library is already running (%1). Wait for it to finish.").arg(summary.lockHolderInfo));
            return;
        }

        auto text = summary.lockHolderInfo.isEmpty()
                ? QCoreApplication::translate("LibraryWindow", "The library is locked by a repair that did not finish.")
                : QCoreApplication::translate("LibraryWindow", "The library is locked by a repair started by %1.").arg(summary.lockHolderInfo);
        text += "\n\n";
        text += QCoreApplication::translate("LibraryWindow", "If you are sure that no other repair is running, the lock can be removed. Remove the lock and continue?");

        const auto answer = QMessageBox::question(dialogParent,
                                                  dialogTitle,
                                                  text,
                                                  QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::No);
        if (answer == QMessageBox::Yes)
            startRepair(true);
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
    if (!libraryPath.isEmpty() && QFile::exists(LibraryPaths::libraryDatabasePath(libraryPath)) && !DataBaseManagement::isLibraryDatabaseValid(libraryPath)) {
        emit databaseRecoveryRequested(libraryName);
        return;
    }
    QMessageBox::critical(dialogParent, dialogTitle, error);
}
