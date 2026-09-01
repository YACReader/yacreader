#ifndef FOLDER_MANAGEMENT_COORDINATOR_H
#define FOLDER_MANAGEMENT_COORDINATOR_H

#include "yacreader_global.h"

#include <QModelIndex>
#include <QObject>
#include <QString>

#include <functional>

class FolderModel;
class QWidget;

class FolderManagementCoordinator : public QObject
{
    Q_OBJECT

public:
    using CurrentFolderProvider = std::function<QModelIndex()>;
    using SelectedFolderProvider = std::function<QModelIndex()>;
    using LibraryPathProvider = std::function<QString()>;

    explicit FolderManagementCoordinator(FolderModel *foldersModel,
                                         QWidget *dialogParent,
                                         CurrentFolderProvider currentFolderProvider,
                                         SelectedFolderProvider selectedFolderProvider,
                                         LibraryPathProvider libraryPathProvider);

    void renameFolder(qulonglong folderId, const QString &libraryPath);
    void setFolderCompleted(qulonglong folderId, const QString &libraryPath, bool completed);
    void setFolderRead(qulonglong folderId, const QString &libraryPath, bool read);
    void setFolderType(qulonglong folderId, const QString &libraryPath, YACReader::FileType type);
    void openFolder(qulonglong folderId, const QString &libraryPath);
    void selectAndSetCustomCover(qulonglong folderId, const QString &libraryPath);
    void setCustomCover(qulonglong folderId, const QString &imagePath);
    void resetCustomCover(qulonglong folderId, const QString &libraryPath);

public slots:
    void addFolderToCurrentFolder();
    void openCurrentFolder();
    void renameCurrentFolder();
    void deleteCurrentFolder();
    void setCurrentFolderCompleted(bool completed);
    void setCurrentFolderRead(bool read);
    void setCurrentFolderType(YACReader::FileType type);
    void selectAndSetCurrentFolderCover();
    void resetCurrentFolderCover();

signals:
    void folderCreationStarted();
    void folderNavigationRequested(const QModelIndex &folder);
    void folderRenamed();
    void folderAboutToBeDeleted(const QModelIndex &parentFolder);
    void folderDeletionFinished();

private:
    QModelIndex createFolder(const QModelIndex &parent, const QString &parentPath, const QString &folderName);

    enum class RenameError {
        None,
        InvalidName,
        TargetAlreadyExists,
        FileSystemRenameFailed,
        DatabaseUpdateFailed,
        DatabaseUpdateAndRollbackFailed
    };

    struct RenameResult {
        RenameError error { RenameError::None };
        QString folderPath;
        QString databaseError;
    };

    void renameFolder(const QModelIndex &folder, const QString &libraryPath);
    RenameResult renameFolder(const QModelIndex &folder, const QString &libraryPath, const QString &newName);
    void deleteFolder(const QModelIndex &folder, const QString &folderPath);
    void showFolderDeletionError();
    QModelIndex folderIndex(qulonglong folderId, const QString &libraryPath) const;
    void setCustomCover(qulonglong folderId, const QString &libraryPath, const QString &imagePath);

    FolderModel *foldersModel;
    QWidget *dialogParent;
    CurrentFolderProvider currentFolderProvider;
    SelectedFolderProvider selectedFolderProvider;
    LibraryPathProvider libraryPathProvider;
};

#endif // FOLDER_MANAGEMENT_COORDINATOR_H
