#ifndef LIBRARY_MANAGEMENT_COORDINATOR_H
#define LIBRARY_MANAGEMENT_COORDINATOR_H

#include <QModelIndex>
#include <QObject>
#include <QString>

#include <functional>
#include <future>

class AddLibraryDialog;
class CreateLibraryDialog;
class ExportLibraryDialog;
class FolderModel;
class ImportLibraryDialog;
class LibraryCreator;
class PackageManager;
class QSettings;
class QWidget;
class YACReaderLibraries;

namespace YACReader {
class XMLInfoLibraryScanner;
}

class LibraryManagementCoordinator : public QObject
{
    Q_OBJECT

public:
    using CurrentLibraryNameProvider = std::function<QString()>;
    using CurrentFolderProvider = std::function<QModelIndex()>;

    LibraryManagementCoordinator(QSettings *settings,
                                 YACReaderLibraries &libraries,
                                 QWidget *dialogParent,
                                 CreateLibraryDialog *createLibraryDialog,
                                 AddLibraryDialog *addLibraryDialog,
                                 ExportLibraryDialog *exportLibraryDialog,
                                 ImportLibraryDialog *importLibraryDialog,
                                 FolderModel *foldersModel,
                                 CurrentLibraryNameProvider currentLibraryNameProvider,
                                 CurrentFolderProvider currentFolderProvider,
                                 QString libraryInfoDialogTitle);

    QList<QPair<QString, QString>> loadLibraries();

public slots:
    void loadLibrary(const QString &libraryName);
    void showCreateLibraryDialog();
    void showAddLibraryDialog();
    void showExportLibraryDialog();
    void showImportLibraryDialog();
    void createLibrary(const QString &source, const QString &destination, const QString &name);
    void updateCurrentLibrary();
    void updateCurrentFolder();
    void updateFolder(const QModelIndex &folderIndex);
    void rescanCurrentLibraryForXMLInfo();
    void rescanCurrentFolderForXMLInfo();
    void rescanFolderForXMLInfo(const QModelIndex &folderIndex);
    void exportCurrentLibrary(const QString &destinationPath);
    void importLibraryPackage(const QString &packagePath, const QString &destinationPath, const QString &libraryName);
    void addExistingLibrary(QString libraryPath, const QString &libraryName);

    void askToRemoveCurrentLibrary();
    void deleteCurrentLibrary(bool deleteMetadata);
    void renameCurrentLibrary(const QString &newName);
    void openCurrentLibraryFolder();
    void showCurrentLibraryInfo();

    void warnIfLibraryCountIsHigh();
    void showLibraryAlreadyExists(const QString &libraryName);
    void stop();

signals:
    void loadStarted();
    void noLibrariesRequested();
    void libraryReady(const QString &libraryDataPath, bool readOnly);
    void libraryManagementOnlyRequested();
    void databaseRecoveryRequested(const QString &libraryName);
    void upgradeStarted();
    void upgradeFailed(const QString &libraryDataPath);
    void libraryReloadRequested(const QString &libraryName);
    void libraryRecreationRequested(const QString &libraryName, const QString &libraryPath);
    void openingError(const QString &error);

    void creationStarted();
    void updateStarted();
    void operationUiResetRequested();
    void operationFinished();
    void currentLibraryReloadRequested();
    void libraryAdded(const QString &libraryName, const QString &libraryPath);
    void libraryRemoved(const QString &libraryName, bool librariesEmpty);
    void libraryRenamed(const QString &oldName, const QString &newName);
    void folderUpdateFinished(qulonglong folderId);
    void comicAdded(const QString &relativePath, const QString &coverPath);
    void creationFailed(const QString &error);
    void updateFailed(const QString &error);
    void xmlScanStarted();
    void xmlScanFinished();
    void xmlComicScanned(const QString &relativePath, const QString &coverPath);
    void packageFailed(const QString &error);

private:
    void loadLibraryAtPath(const QString &libraryName, const QString &libraryPath);
    void updateLibrary(const QString &libraryName, const QString &libraryPath);
    void startFolderUpdate(const QString &libraryName, const QString &libraryPath, const QString &folderPath, qulonglong folderId);
    void prepareImportedLibrary(const QString &libraryName, const QString &libraryPath);
    void finishAddingLibrary();
    void askToRemoveLibrary(const QString &libraryName);
    void deleteLibrary(const QString &libraryName, bool deleteMetadata);
    bool renameLibrary(const QString &currentName, const QString &newName);
    void startUpgrade(const QString &libraryName, const QString &libraryPath, const QString &libraryDataPath);
    void handleCreatorOpeningFailure(const QString &error);

    YACReaderLibraries &libraries;
    QWidget *dialogParent;
    CreateLibraryDialog *createLibraryDialog;
    AddLibraryDialog *addLibraryDialog;
    ExportLibraryDialog *exportLibraryDialog;
    ImportLibraryDialog *importLibraryDialog;
    FolderModel *foldersModel;
    CurrentLibraryNameProvider currentLibraryNameProvider;
    CurrentFolderProvider currentFolderProvider;
    QString libraryInfoDialogTitle;
    LibraryCreator *libraryCreator;
    PackageManager *packageManager;
    YACReader::XMLInfoLibraryScanner *xmlInfoLibraryScanner;
    QString pendingLibraryName;
    QString pendingLibraryPath;
    QString operationLibraryName;
    QString operationLibraryPath;
    std::future<void> upgradeFuture;
};

#endif
