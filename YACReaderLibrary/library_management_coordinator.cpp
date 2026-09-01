#include "library_management_coordinator.h"

#include "add_library_dialog.h"
#include "create_library_dialog.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "export_library_dialog.h"
#include "folder_model.h"
#include "import_library_dialog.h"
#include "library_creator.h"
#include "package_manager.h"
#include "xml_info_library_scanner.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpacerItem>
#include <QSqlError>
#include <QUrl>
#include <QWidget>
#include <QsLog.h>

#include <utility>

using namespace YACReader;

LibraryManagementCoordinator::LibraryManagementCoordinator(QSettings *settings,
                                                           YACReaderLibraries &libraries,
                                                           QWidget *dialogParent,
                                                           CreateLibraryDialog *createLibraryDialog,
                                                           AddLibraryDialog *addLibraryDialog,
                                                           ExportLibraryDialog *exportLibraryDialog,
                                                           ImportLibraryDialog *importLibraryDialog,
                                                           FolderModel *foldersModel,
                                                           CurrentLibraryNameProvider currentLibraryNameProvider,
                                                           CurrentFolderProvider currentFolderProvider,
                                                           QString libraryInfoDialogTitle)
    : QObject(dialogParent), libraries(libraries), dialogParent(dialogParent), createLibraryDialog(createLibraryDialog), addLibraryDialog(addLibraryDialog), exportLibraryDialog(exportLibraryDialog), importLibraryDialog(importLibraryDialog), foldersModel(foldersModel), currentLibraryNameProvider(std::move(currentLibraryNameProvider)), currentFolderProvider(std::move(currentFolderProvider)), libraryInfoDialogTitle(std::move(libraryInfoDialogTitle)), libraryCreator(new LibraryCreator(settings)), packageManager(new PackageManager()), xmlInfoLibraryScanner(new XMLInfoLibraryScanner())
{
    libraryCreator->setParent(this);
    packageManager->setParent(this);
    xmlInfoLibraryScanner->setParent(this);

    connect(this, &LibraryManagementCoordinator::upgradeFailed, this, [this](const QString &libraryDataPath) { QMessageBox::critical(this->dialogParent,
                                                                                                                                     QCoreApplication::translate("LibraryWindow", "Upgrade failed"),
                                                                                                                                     QCoreApplication::translate("LibraryWindow", "There were errors during library upgrade in: ") + libraryDataPath + "/library.ydb"); }, Qt::QueuedConnection);

    connect(libraryCreator, &QThread::finished, this, &LibraryManagementCoordinator::operationFinished);
    connect(libraryCreator, &LibraryCreator::updated, this, &LibraryManagementCoordinator::currentLibraryReloadRequested);
    connect(libraryCreator, &LibraryCreator::created, this, &LibraryManagementCoordinator::finishAddingLibrary);
    connect(libraryCreator, &LibraryCreator::updatedCurrentFolder, this, &LibraryManagementCoordinator::folderUpdateFinished);
    connect(libraryCreator, &LibraryCreator::comicAdded, this, &LibraryManagementCoordinator::comicAdded);
    connect(libraryCreator, &LibraryCreator::failedCreatingDB, this, &LibraryManagementCoordinator::creationFailed);
    connect(libraryCreator, &LibraryCreator::failedOpeningDB, this, &LibraryManagementCoordinator::handleCreatorOpeningFailure);

    connect(this, &LibraryManagementCoordinator::libraryReloadRequested, this, &LibraryManagementCoordinator::loadLibrary);
    connect(this, &LibraryManagementCoordinator::libraryRecreationRequested, createLibraryDialog, &CreateLibraryDialog::setDataAndStart);
    connect(this, &LibraryManagementCoordinator::openingError, this, [this](const QString &error) {
        QMessageBox::critical(this->dialogParent, tr("Error opening the library"), error);
    });
    connect(this, &LibraryManagementCoordinator::creationFailed, this, [this](const QString &error) {
        QMessageBox::critical(this->dialogParent, tr("Error creating the library"), error);
    });
    connect(this, &LibraryManagementCoordinator::updateFailed, this, [this](const QString &error) {
        QMessageBox::critical(this->dialogParent, tr("Error updating the library"), error);
    });

    connect(createLibraryDialog, &CreateLibraryDialog::createLibrary, this, &LibraryManagementCoordinator::createLibrary);
    connect(createLibraryDialog, &CreateLibraryDialog::libraryExists, this, &LibraryManagementCoordinator::showLibraryAlreadyExists);
    connect(createLibraryDialog, &CreateLibraryDialog::cancelCreate, this, &LibraryManagementCoordinator::stop);
    connect(addLibraryDialog, &AddLibraryDialog::addLibrary, this, &LibraryManagementCoordinator::addExistingLibrary);

    connect(exportLibraryDialog, &ExportLibraryDialog::exportPath, this, &LibraryManagementCoordinator::exportCurrentLibrary);
    connect(exportLibraryDialog, &QDialog::rejected, packageManager, &PackageManager::cancel);
    connect(packageManager, &PackageManager::exported, exportLibraryDialog, &ExportLibraryDialog::close);
    connect(importLibraryDialog, &ImportLibraryDialog::unpackCLC, this, &LibraryManagementCoordinator::importLibraryPackage);
    connect(importLibraryDialog, &QDialog::rejected, packageManager, &PackageManager::cancel);
    connect(importLibraryDialog, &QDialog::rejected, this, [this] { deleteCurrentLibrary(true); });
    connect(importLibraryDialog, &ImportLibraryDialog::libraryExists, this, &LibraryManagementCoordinator::showLibraryAlreadyExists);
    connect(packageManager, &PackageManager::imported, importLibraryDialog, &QWidget::hide);
    connect(packageManager, &PackageManager::imported, this, &LibraryManagementCoordinator::finishAddingLibrary);
    connect(packageManager, &PackageManager::failed, this, &LibraryManagementCoordinator::packageFailed);

    connect(xmlInfoLibraryScanner, &QThread::finished, this, &LibraryManagementCoordinator::xmlScanFinished);
    connect(xmlInfoLibraryScanner, &XMLInfoLibraryScanner::comicScanned, this, &LibraryManagementCoordinator::xmlComicScanned);
}

void LibraryManagementCoordinator::loadLibrary(const QString &libraryName)
{
    if (libraries.isEmpty()) {
        emit noLibrariesRequested();
        return;
    }

    loadLibraryAtPath(libraryName, libraries.getPath(libraryName));
}

void LibraryManagementCoordinator::loadLibraryAtPath(const QString &libraryName, const QString &libraryPath)
{
    emit loadStarted();

    QString recoveryError;
    if (!DataBaseManagement::recoverInterruptedRestore(libraryPath, &recoveryError)) {
        QMessageBox::critical(dialogParent, QCoreApplication::translate("LibraryWindow", "Restore recovery failed"), recoveryError);
        return;
    }

    const auto libraryDataPath = LibraryPaths::libraryDataPath(libraryPath);
    const auto customFolderCoversPath = LibraryPaths::libraryCustomFoldersCoverPath(libraryPath);
    const auto databasePath = LibraryPaths::libraryDatabasePath(libraryPath);
    QDir directory;
    QString databaseVersion;

    if (directory.exists(libraryDataPath) && directory.exists(databasePath) && !(databaseVersion = DataBaseManagement::checkValidDB(databasePath)).isEmpty()) {
        directory.mkdir(customFolderCoversPath);

        const auto versionComparison = DataBaseManagement::compareVersions(databaseVersion, DB_VERSION);
        if (versionComparison < 0) {
            if (!DataBaseManagement::isLibraryDatabaseValid(libraryPath)) {
                emit libraryManagementOnlyRequested();
                emit databaseRecoveryRequested(libraryName);
                return;
            }

            const auto answer = QMessageBox::question(dialogParent,
                                                      QCoreApplication::translate("LibraryWindow", "Update needed"),
                                                      QCoreApplication::translate("LibraryWindow", "This library was created with a previous version of YACReaderLibrary. It needs to be updated. Update now?"),
                                                      QMessageBox::Yes,
                                                      QMessageBox::No);
            if (answer == QMessageBox::Yes) {
                startUpgrade(libraryName, libraryPath, libraryDataPath);
                return;
            }

            emit libraryManagementOnlyRequested();
            return;
        }

        if (versionComparison == 0) {
            QDir rootDirectory(libraryPath);
            rootDirectory.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
            emit libraryReady(libraryDataPath, rootDirectory.count() <= 1);
            return;
        }

        const auto answer = QMessageBox::question(dialogParent,
                                                  QCoreApplication::translate("LibraryWindow", "Download new version"),
                                                  QCoreApplication::translate("LibraryWindow", "This library was created with a newer version of YACReaderLibrary. Download the new version now?"),
                                                  QMessageBox::Yes,
                                                  QMessageBox::No);
        if (answer == QMessageBox::Yes)
            QDesktopServices::openUrl(QUrl("http://www.yacreader.com"));
        emit libraryManagementOnlyRequested();
        return;
    }

    emit libraryManagementOnlyRequested();

    if (!directory.exists(libraryDataPath)) {
        const auto libraryDescription = libraryName + " -> " + libraryPath;
        if (QMessageBox::question(dialogParent,
                                  QCoreApplication::translate("LibraryWindow", "Library not available"),
                                  QCoreApplication::translate("LibraryWindow", "Library '%1' is no longer available. Do you want to remove it?").arg(libraryDescription),
                                  QMessageBox::Yes,
                                  QMessageBox::No) == QMessageBox::Yes) {
            deleteLibrary(libraryName, true);
        }
        return;
    }

    if (directory.exists(databasePath)) {
        const auto database = DataBaseManagement::loadDatabase(libraryDataPath);
        emit openingError(database.lastError().databaseText() + "-" + database.lastError().driverText());
        return;
    }

    if (QMessageBox::question(dialogParent,
                              QCoreApplication::translate("LibraryWindow", "Old library"),
                              QCoreApplication::translate("LibraryWindow", "Library '%1' has been created with an older version of YACReaderLibrary. It must be created again. Do you want to create the library now?").arg(libraryName),
                              QMessageBox::Yes,
                              QMessageBox::No) == QMessageBox::Yes) {
        emit libraryRecreationRequested(libraryName, libraryPath);
    }
}

QList<QPair<QString, QString>> LibraryManagementCoordinator::loadLibraries()
{
    libraries.load();
    QList<QPair<QString, QString>> result;
    const auto libraryNames = libraries.getNames();
    result.reserve(libraryNames.size());
    for (const auto &libraryName : libraryNames)
        result.append({ libraryName, libraries.getPath(libraryName) });
    return result;
}

void LibraryManagementCoordinator::showCreateLibraryDialog()
{
    warnIfLibraryCountIsHigh();
    createLibraryDialog->open(libraries);
}

void LibraryManagementCoordinator::showAddLibraryDialog()
{
    warnIfLibraryCountIsHigh();
    addLibraryDialog->open();
}

void LibraryManagementCoordinator::showExportLibraryDialog()
{
    exportLibraryDialog->open();
}

void LibraryManagementCoordinator::showImportLibraryDialog()
{
    importLibraryDialog->open(libraries);
}

void LibraryManagementCoordinator::createLibrary(const QString &source, const QString &destination, const QString &name)
{
    QLOG_INFO() << QString("About to create a library from '%1' to '%2' with name '%3'").arg(source, destination, name);
    pendingLibraryName = name;
    pendingLibraryPath = source;
    operationLibraryName = name;
    operationLibraryPath = source;
    emit creationStarted();
    libraryCreator->createLibrary(source, destination);
    libraryCreator->start();
}

void LibraryManagementCoordinator::updateCurrentLibrary()
{
    const auto libraryName = currentLibraryNameProvider();
    updateLibrary(libraryName, libraries.getPath(libraryName));
}

void LibraryManagementCoordinator::updateCurrentFolder()
{
    updateFolder(currentFolderProvider());
}

void LibraryManagementCoordinator::updateFolder(const QModelIndex &folderIndex)
{
    const auto libraryName = currentLibraryNameProvider();
    const auto libraryPath = QDir::cleanPath(libraries.getPath(libraryName));
    emit updateStarted();
    startFolderUpdate(
            libraryName,
            libraryPath,
            QDir::cleanPath(libraryPath + foldersModel->getFolderPath(folderIndex)),
            folderIndex.data(FolderModel::IdRole).toULongLong());
}

void LibraryManagementCoordinator::updateLibrary(const QString &libraryName, const QString &libraryPath)
{
    operationLibraryName = libraryName;
    operationLibraryPath = libraryPath;
    emit updateStarted();
    libraryCreator->updateLibrary(libraryPath, LibraryPaths::libraryDataPath(libraryPath));
    libraryCreator->start();
}

void LibraryManagementCoordinator::startFolderUpdate(const QString &libraryName, const QString &libraryPath, const QString &folderPath, qulonglong folderId)
{
    operationLibraryName = libraryName;
    operationLibraryPath = libraryPath;
    libraryCreator->updateFolder(libraryPath, LibraryPaths::libraryDataPath(libraryPath), folderPath, folderId);
    libraryCreator->start();
}

void LibraryManagementCoordinator::rescanCurrentLibraryForXMLInfo()
{
    const auto libraryPath = libraries.getPath(currentLibraryNameProvider());
    emit xmlScanStarted();
    xmlInfoLibraryScanner->scanLibrary(libraryPath, LibraryPaths::libraryDataPath(libraryPath));
}

void LibraryManagementCoordinator::rescanCurrentFolderForXMLInfo()
{
    rescanFolderForXMLInfo(currentFolderProvider());
}

void LibraryManagementCoordinator::rescanFolderForXMLInfo(const QModelIndex &folderIndex)
{
    if (!folderIndex.isValid())
        return;

    const auto libraryPath = libraries.getPath(currentLibraryNameProvider());
    emit xmlScanStarted();
    xmlInfoLibraryScanner->scanFolder(
            libraryPath,
            LibraryPaths::libraryDataPath(libraryPath),
            QDir::cleanPath(libraryPath + foldersModel->getFolderPath(folderIndex)),
            folderIndex);
}

void LibraryManagementCoordinator::exportCurrentLibrary(const QString &destinationPath)
{
    const auto libraryName = currentLibraryNameProvider();
    packageManager->createPackage(LibraryPaths::libraryDataPath(libraries.getPath(libraryName)), destinationPath + "/" + libraryName);
}

void LibraryManagementCoordinator::importLibraryPackage(const QString &packagePath, const QString &destinationPath, const QString &libraryName)
{
    const auto libraryPath = destinationPath + "/" + libraryName;
    packageManager->extractPackage(packagePath, libraryPath);
    prepareImportedLibrary(libraryName, libraryPath);
}

void LibraryManagementCoordinator::addExistingLibrary(QString libraryPath, const QString &libraryName)
{
    if (libraries.contains(libraryName)) {
        showLibraryAlreadyExists(libraryName);
        return;
    }

    libraryPath.remove("/.yacreaderlibrary");
    if (!QDir(LibraryPaths::libraryDataPath(libraryPath)).exists()) {
        QMessageBox::warning(dialogParent,
                             QCoreApplication::translate("LibraryWindow", "Library not found"),
                             QCoreApplication::translate("LibraryWindow", "The selected folder doesn't contain any library."));
        return;
    }

    prepareImportedLibrary(libraryName, libraryPath);
    finishAddingLibrary();
}

void LibraryManagementCoordinator::prepareImportedLibrary(const QString &libraryName, const QString &libraryPath)
{
    pendingLibraryName = libraryName;
    pendingLibraryPath = libraryPath;
}

void LibraryManagementCoordinator::finishAddingLibrary()
{
    if (pendingLibraryName.isEmpty() || pendingLibraryPath.isEmpty())
        return;

    libraries.addLibrary(pendingLibraryName, pendingLibraryPath);
    libraries.save();
    emit libraryAdded(pendingLibraryName, pendingLibraryPath);
    pendingLibraryName.clear();
    pendingLibraryPath.clear();
}

void LibraryManagementCoordinator::askToRemoveCurrentLibrary()
{
    askToRemoveLibrary(currentLibraryNameProvider());
}

void LibraryManagementCoordinator::deleteCurrentLibrary(bool deleteMetadata)
{
    deleteLibrary(currentLibraryNameProvider(), deleteMetadata);
}

void LibraryManagementCoordinator::renameCurrentLibrary(const QString &newName)
{
    const auto currentName = currentLibraryNameProvider();
    if (!renameLibrary(currentName, newName))
        return;

    emit libraryRenamed(currentName, newName);
}

void LibraryManagementCoordinator::openCurrentLibraryFolder()
{
    const auto path = libraries.getPath(currentLibraryNameProvider());
    if (!path.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::cleanPath(path)));
}

void LibraryManagementCoordinator::showCurrentLibraryInfo()
{
    const auto id = libraries.getUuid(currentLibraryNameProvider());
    const auto info = DBHelper::getLibraryInfo(id);

    QMessageBox messageBox(dialogParent);
    messageBox.setWindowTitle(libraryInfoDialogTitle);
    messageBox.setText(info);
    auto horizontalSpacer = new QSpacerItem(420, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto layout = qobject_cast<QGridLayout *>(messageBox.layout());
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    messageBox.setStandardButtons(QMessageBox::Close);
    messageBox.setDefaultButton(QMessageBox::Close);
    messageBox.exec();
}

void LibraryManagementCoordinator::askToRemoveLibrary(const QString &libraryName)
{
    QMessageBox messageBox(QMessageBox::Question,
                           QCoreApplication::translate("LibraryWindow", "Are you sure?"),
                           QCoreApplication::translate("LibraryWindow", "Do you want remove ") + libraryName + QCoreApplication::translate("LibraryWindow", " library?"),
                           QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No,
                           dialogParent);
    messageBox.button(QMessageBox::YesToAll)->setText(QCoreApplication::translate("LibraryWindow", "Remove and delete metadata and backups"));
    messageBox.setWindowModality(Qt::WindowModal);

    const auto answer = messageBox.exec();
    if (answer == QMessageBox::Yes)
        deleteLibrary(libraryName, false);
    else if (answer == QMessageBox::YesToAll)
        deleteLibrary(libraryName, true);
}

void LibraryManagementCoordinator::deleteLibrary(const QString &libraryName, bool deleteMetadata)
{
    const auto libraryPath = libraries.getPath(libraryName);
    libraries.remove(libraryName);

    if (deleteMetadata)
        QDir(LibraryPaths::libraryDataPath(libraryPath)).removeRecursively();

    libraries.save();
    emit libraryRemoved(libraryName, libraries.isEmpty());
}

bool LibraryManagementCoordinator::renameLibrary(const QString &currentName, const QString &newName)
{
    if (newName == currentName)
        return true;
    if (libraries.contains(newName)) {
        showLibraryAlreadyExists(newName);
        return false;
    }

    libraries.rename(currentName, newName);
    libraries.save();
    return true;
}

void LibraryManagementCoordinator::warnIfLibraryCountIsHigh()
{
    if (libraries.getNames().size() < MAX_LIBRARIES_WARNING_NUM)
        return;

    QMessageBox::warning(dialogParent,
                         QCoreApplication::translate("LibraryWindow", "You are adding too many libraries."),
                         QCoreApplication::translate("LibraryWindow", "You are adding too many libraries.\n\nYou probably only need one library in your top level comics folder, you can browse any subfolders using the folders section in the left sidebar.\n\nYACReaderLibrary will not stop you from creating more libraries but you should keep the number of libraries low."));
}

void LibraryManagementCoordinator::showLibraryAlreadyExists(const QString &libraryName)
{
    QMessageBox::information(dialogParent,
                             QCoreApplication::translate("LibraryWindow", "Library name already exists"),
                             QCoreApplication::translate("LibraryWindow", "There is another library with the name '%1'.").arg(libraryName));
}

void LibraryManagementCoordinator::stop()
{
    libraryCreator->stop();
    libraryCreator->wait();
    xmlInfoLibraryScanner->stop();
    xmlInfoLibraryScanner->wait();
}

void LibraryManagementCoordinator::startUpgrade(const QString &libraryName, const QString &libraryPath, const QString &libraryDataPath)
{
    emit upgradeStarted();
    upgradeFuture = std::async(std::launch::async, [this, libraryName, libraryPath, libraryDataPath] {
        if (!DataBaseManagement::updateToCurrentVersion(libraryPath))
            emit upgradeFailed(libraryDataPath);
        emit libraryReloadRequested(libraryName);
    });
}

void LibraryManagementCoordinator::handleCreatorOpeningFailure(const QString &error)
{
    emit operationUiResetRequested();
    if (!operationLibraryPath.isEmpty() && QFile::exists(LibraryPaths::libraryDatabasePath(operationLibraryPath)) && !DataBaseManagement::isLibraryDatabaseValid(operationLibraryPath)) {
        emit databaseRecoveryRequested(operationLibraryName);
        return;
    }
    emit updateFailed(error);
}
