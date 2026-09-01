#include "folder_management_coordinator.h"

#include "comics_remover.h"
#include "cover_utils.h"
#include "folder_model.h"
#include "yacreader_global.h"
#include "yacreader_global_gui.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QThread>
#include <QUrl>
#include <QWidget>

#include <utility>

namespace {
bool containsInvalidFolderNameCharacters(const QString &folderName)
{
    static const QRegularExpression invalidCharacters(QStringLiteral("[\\/\\\\:*?\"<>|]"));
    return folderName.contains(invalidCharacters);
}
}

FolderManagementCoordinator::FolderManagementCoordinator(FolderModel *foldersModel,
                                                         QWidget *dialogParent,
                                                         CurrentFolderProvider currentFolderProvider,
                                                         SelectedFolderProvider selectedFolderProvider,
                                                         LibraryPathProvider libraryPathProvider)
    : QObject(dialogParent), foldersModel(foldersModel), dialogParent(dialogParent), currentFolderProvider(std::move(currentFolderProvider)), selectedFolderProvider(std::move(selectedFolderProvider)), libraryPathProvider(std::move(libraryPathProvider))
{
}

QModelIndex FolderManagementCoordinator::createFolder(const QModelIndex &parent, const QString &parentPath, const QString &folderName)
{
    if (folderName.isEmpty() || containsInvalidFolderNameCharacters(folderName))
        return { };

    QDir parentDirectory(parentPath);
    const QDir newFolder(parentDirectory.filePath(folderName));
    if (!parentDirectory.mkdir(folderName) && !newFolder.exists())
        return { };

    return foldersModel->addFolderAtParent(folderName, parent);
}

void FolderManagementCoordinator::addFolderToCurrentFolder()
{
    emit folderCreationStarted();

    const auto parent = selectedFolderProvider();
    bool accepted = false;
    const auto folderName = QInputDialog::getText(dialogParent,
                                                  tr("Add new folder"),
                                                  tr("Folder name:"),
                                                  QLineEdit::Normal,
                                                  QString(),
                                                  &accepted);
    if (!accepted)
        return;

    const auto parentPath = QDir::cleanPath(libraryPathProvider() + foldersModel->getFolderPath(parent));
    const auto folder = createFolder(parent, parentPath, folderName);
    if (folder.isValid())
        emit folderNavigationRequested(folder);
}

void FolderManagementCoordinator::openCurrentFolder()
{
    const auto libraryPath = libraryPathProvider();
    const auto folder = currentFolderProvider();
    const auto path = folder.isValid()
            ? QDir::cleanPath(libraryPath + foldersModel->getFolderPath(folder))
            : QDir::cleanPath(libraryPath);
    QDesktopServices::openUrl(QUrl("file:///" + path, QUrl::TolerantMode));
}

void FolderManagementCoordinator::openFolder(qulonglong folderId, const QString &libraryPath)
{
    const auto folder = folderIndex(folderId, libraryPath);
    if (!folder.isValid())
        return;

    QDesktopServices::openUrl(QUrl("file:///" + QDir::cleanPath(libraryPath + foldersModel->getFolderPath(folder)), QUrl::TolerantMode));
}

FolderManagementCoordinator::RenameResult FolderManagementCoordinator::renameFolder(const QModelIndex &folder, const QString &libraryPath, const QString &newName)
{
    const auto oldName = folder.data(FolderModel::FolderNameRole).toString();
    if (newName.isEmpty() || newName == "." || newName == ".." || containsInvalidFolderNameCharacters(newName))
        return { RenameError::InvalidName };

    const auto oldPath = QDir::cleanPath(libraryPath + foldersModel->getFolderPath(folder));
    const QFileInfo oldFolder(oldPath);
    QDir parentDirectory(oldFolder.absolutePath());
    const auto newPath = QDir::cleanPath(parentDirectory.filePath(newName));

    if (QFileInfo::exists(newPath) && QString::compare(oldPath, newPath, Qt::CaseInsensitive) != 0)
        return { RenameError::TargetAlreadyExists };

    if (!parentDirectory.rename(oldName, newName))
        return { RenameError::FileSystemRenameFailed, oldPath };

    QString databaseError;
    if (foldersModel->renameFolder(folder, newName, &databaseError))
        return { };

    if (!parentDirectory.rename(newName, oldName))
        return { RenameError::DatabaseUpdateAndRollbackFailed, oldPath, databaseError };

    return { RenameError::DatabaseUpdateFailed, oldPath, databaseError };
}

void FolderManagementCoordinator::renameFolder(qulonglong folderId, const QString &libraryPath)
{
    renameFolder(folderIndex(folderId, libraryPath), libraryPath);
}

void FolderManagementCoordinator::renameCurrentFolder()
{
    const auto libraryPath = libraryPathProvider();
    const auto folder = selectedFolderProvider();
    if (!folder.isValid()) {
        QMessageBox::information(dialogParent,
                                 QCoreApplication::translate("LibraryWindow", "No folder selected"),
                                 QCoreApplication::translate("LibraryWindow", "Please, select a folder first"));
        return;
    }

    renameFolder(folder.data(FolderModel::IdRole).toULongLong(), libraryPath);
}

void FolderManagementCoordinator::renameFolder(const QModelIndex &folder, const QString &libraryPath)
{
    if (!folder.isValid()) {
        QMessageBox::information(dialogParent,
                                 QCoreApplication::translate("LibraryWindow", "No folder selected"),
                                 QCoreApplication::translate("LibraryWindow", "Please, select a folder first"));
        return;
    }

    const auto oldName = folder.data(FolderModel::FolderNameRole).toString();
    bool accepted = false;
    const auto newName = QInputDialog::getText(dialogParent,
                                               QCoreApplication::translate("LibraryWindow", "Rename folder"),
                                               QCoreApplication::translate("LibraryWindow", "Folder name:"),
                                               QLineEdit::Normal,
                                               oldName,
                                               &accepted);
    if (!accepted || newName == oldName)
        return;

    const auto result = renameFolder(folder, libraryPath, newName);
    switch (result.error) {
    case RenameError::None:
        emit folderRenamed();
        return;
    case RenameError::InvalidName:
        QMessageBox::warning(dialogParent,
                             QCoreApplication::translate("LibraryWindow", "Invalid folder name"),
                             QCoreApplication::translate("LibraryWindow", "The folder name is empty or contains characters that are not supported."));
        return;
    case RenameError::TargetAlreadyExists:
        QMessageBox::warning(dialogParent,
                             QCoreApplication::translate("LibraryWindow", "Unable to rename folder"),
                             QCoreApplication::translate("LibraryWindow", "A file or folder named '%1' already exists.").arg(newName));
        return;
    case RenameError::FileSystemRenameFailed:
        QMessageBox::critical(dialogParent,
                              QCoreApplication::translate("LibraryWindow", "Unable to rename folder"),
                              QCoreApplication::translate("LibraryWindow", "The folder could not be renamed on disk. Please check the folder name and write permissions.\n\nFolder: %1").arg(result.folderPath));
        return;
    case RenameError::DatabaseUpdateFailed:
    case RenameError::DatabaseUpdateAndRollbackFailed: {
        auto message = result.error == RenameError::DatabaseUpdateFailed
                ? QCoreApplication::translate("LibraryWindow", "The library database could not be updated. The folder rename on disk was reverted.")
                : QCoreApplication::translate("LibraryWindow", "The library database could not be updated, and the folder rename on disk could not be reverted. The library now needs to be updated manually.");
        if (!result.databaseError.isEmpty())
            message += "\n\n" + result.databaseError;
        QMessageBox::critical(dialogParent, QCoreApplication::translate("LibraryWindow", "Unable to rename folder"), message);
        return;
    }
    }
}

void FolderManagementCoordinator::deleteCurrentFolder()
{
    const auto folder = selectedFolderProvider();
    if (!folder.isValid()) {
        QMessageBox::information(dialogParent,
                                 QCoreApplication::translate("LibraryWindow", "No folder selected"),
                                 QCoreApplication::translate("LibraryWindow", "Please, select a folder first"));
        return;
    }

    const auto libraryPath = QDir::cleanPath(libraryPathProvider());
    const auto relativePath = foldersModel->getFolderPath(folder);
    const auto folderPath = QDir::cleanPath(libraryPath + relativePath);
    if (libraryPath == folderPath || relativePath.isEmpty() || relativePath == "/") {
        QMessageBox::critical(dialogParent,
                              QCoreApplication::translate("LibraryWindow", "Error in path"),
                              QCoreApplication::translate("LibraryWindow", "There was an error accessing the folder's path"));
        return;
    }

    const auto result = QMessageBox::question(
            dialogParent,
            QCoreApplication::translate("LibraryWindow", "Delete folder"),
            QCoreApplication::translate("LibraryWindow", "The selected folder and all its contents will be deleted from your disk. Are you sure?") + "\n\nFolder : " + folderPath,
            QMessageBox::Yes,
            QMessageBox::No);
    if (result != QMessageBox::Yes)
        return;

    emit folderAboutToBeDeleted(folder.parent());
    deleteFolder(folder, folderPath);
}

void FolderManagementCoordinator::deleteFolder(const QModelIndex &folder, const QString &folderPath)
{
    QModelIndexList folders { folder };
    QList<QString> paths { folderPath };

    auto remover = new FoldersRemover(folders, paths);
    auto thread = new QThread(this);
    remover->moveToThread(thread);

    connect(thread, &QThread::started, remover, &FoldersRemover::process);
    connect(remover, &FoldersRemover::remove, foldersModel, &FolderModel::deleteFolder);
    connect(remover, &FoldersRemover::removeError, this, &FolderManagementCoordinator::showFolderDeletionError);
    connect(remover, &FoldersRemover::finished, this, &FolderManagementCoordinator::folderDeletionFinished);
    connect(remover, &FoldersRemover::finished, remover, &QObject::deleteLater);
    connect(remover, &FoldersRemover::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
}

void FolderManagementCoordinator::showFolderDeletionError()
{
    QMessageBox::critical(dialogParent,
                          QCoreApplication::translate("LibraryWindow", "Unable to delete"),
                          QCoreApplication::translate("LibraryWindow", "There was an issue trying to delete the selected folders. Please, check for write permissions and be sure that no applications are using these folders or any of the contained files."));
}

void FolderManagementCoordinator::setFolderCompleted(qulonglong folderId, const QString &libraryPath, bool completed)
{
    const auto index = folderIndex(folderId, libraryPath);
    if (index.isValid())
        foldersModel->updateFolderCompletedStatus({ index }, completed);
}

void FolderManagementCoordinator::setFolderRead(qulonglong folderId, const QString &libraryPath, bool read)
{
    const auto index = folderIndex(folderId, libraryPath);
    if (index.isValid())
        foldersModel->updateFolderFinishedStatus({ index }, read);
}

void FolderManagementCoordinator::setFolderType(qulonglong folderId, const QString &libraryPath, YACReader::FileType type)
{
    const auto index = folderIndex(folderId, libraryPath);
    if (index.isValid())
        foldersModel->updateFolderType({ index }, type);
}

void FolderManagementCoordinator::setCurrentFolderCompleted(bool completed)
{
    const auto index = currentFolderProvider();
    if (index.isValid())
        setFolderCompleted(index.data(FolderModel::IdRole).toULongLong(), libraryPathProvider(), completed);
}

void FolderManagementCoordinator::setCurrentFolderRead(bool read)
{
    const auto index = currentFolderProvider();
    if (index.isValid())
        setFolderRead(index.data(FolderModel::IdRole).toULongLong(), libraryPathProvider(), read);
}

void FolderManagementCoordinator::setCurrentFolderType(YACReader::FileType type)
{
    const auto index = currentFolderProvider();
    if (index.isValid())
        setFolderType(index.data(FolderModel::IdRole).toULongLong(), libraryPathProvider(), type);
}

void FolderManagementCoordinator::selectAndSetCurrentFolderCover()
{
    const auto index = currentFolderProvider();
    if (index.isValid())
        selectAndSetCustomCover(index.data(FolderModel::IdRole).toULongLong(), libraryPathProvider());
}

void FolderManagementCoordinator::resetCurrentFolderCover()
{
    const auto index = currentFolderProvider();
    if (index.isValid())
        resetCustomCover(index.data(FolderModel::IdRole).toULongLong(), libraryPathProvider());
}

void FolderManagementCoordinator::selectAndSetCustomCover(qulonglong folderId, const QString &libraryPath)
{
    if (!folderIndex(folderId, libraryPath).isValid())
        return;

    const auto sourceImagePath = YACReader::imageFileLoader(dialogParent);
    if (sourceImagePath.isEmpty())
        return;

    setCustomCover(folderId, libraryPath, sourceImagePath);
}

void FolderManagementCoordinator::setCustomCover(qulonglong folderId, const QString &imagePath)
{
    setCustomCover(folderId, libraryPathProvider(), imagePath);
}

void FolderManagementCoordinator::setCustomCover(qulonglong folderId, const QString &libraryPath, const QString &imagePath)
{
    if (imagePath.isEmpty())
        return;

    const auto index = folderIndex(folderId, libraryPath);
    if (!index.isValid())
        return;

    const QImage cover(imagePath);
    if (cover.isNull()) {
        QMessageBox::warning(dialogParent,
                             QCoreApplication::translate("LibraryWindow", "Invalid image"),
                             QCoreApplication::translate("LibraryWindow", "The selected file is not a valid image."));
        return;
    }

    auto folderCoverPath = YACReader::LibraryPaths::customFolderCoverPath(libraryPath, QString::number(folderId));
    if (!YACReader::saveCover(folderCoverPath, cover)) {
        QMessageBox::warning(dialogParent,
                             QCoreApplication::translate("LibraryWindow", "Error saving cover"),
                             QCoreApplication::translate("LibraryWindow", "There was an error saving the cover image."));
        return;
    }

    const auto coversPath = YACReader::LibraryPaths::libraryCoversFolderPath(libraryPath);
    foldersModel->setCustomFolderCover(index, folderCoverPath.remove(coversPath));
}

void FolderManagementCoordinator::resetCustomCover(qulonglong folderId, const QString &libraryPath)
{
    const auto index = folderIndex(folderId, libraryPath);
    if (!index.isValid())
        return;

    const auto folderCoverPath = YACReader::LibraryPaths::customFolderCoverPath(libraryPath, QString::number(folderId));
    if (QFile::exists(folderCoverPath))
        QFile::remove(folderCoverPath);

    foldersModel->resetFolderCover(index);
}

QModelIndex FolderManagementCoordinator::folderIndex(qulonglong folderId, const QString &libraryPath) const
{
    if (QDir::cleanPath(foldersModel->getDatabase()) != QDir::cleanPath(YACReader::LibraryPaths::libraryDataPath(libraryPath)))
        return { };

    return foldersModel->getIndexFromFolderId(folderId);
}
