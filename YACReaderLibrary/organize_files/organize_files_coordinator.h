#ifndef ORGANIZE_FILES_COORDINATOR_H
#define ORGANIZE_FILES_COORDINATOR_H

#include "comic_db.h"
#include "organize_files_worker.h"

#include <QModelIndex>
#include <QObject>
#include <QVariantMap>

#include <functional>

class ComicModel;
class FolderModel;
class QSettings;
class QWidget;

class OrganizeFilesCoordinator : public QObject
{
    Q_OBJECT
public:
    struct LibraryContext {
        qulonglong id;
        QString rootPath;
    };

    using SelectionProvider = std::function<QModelIndexList()>;
    using CurrentFolderProvider = std::function<QModelIndex()>;
    using CurrentLibraryProvider = std::function<LibraryContext()>;

    explicit OrganizeFilesCoordinator(QSettings *settings,
                                      QWidget *window,
                                      ComicModel *comicsModel,
                                      FolderModel *foldersModel,
                                      SelectionProvider selectionProvider,
                                      CurrentFolderProvider currentFolderProvider,
                                      CurrentLibraryProvider currentLibraryProvider);

public slots:
    void renameCurrentFolder();
    void organizeCurrentFolder();
    void renameFolder(const QModelIndex &folderIndex);
    void organizeFolder(const QModelIndex &folderIndex);
    void renameSelectedComics();
    void organizeSelectedComics();

signals:
    void libraryContentChanged();

private:
    void runOnFolder(OrganizeFiles::Mode mode, const QModelIndex &folderIndex);
    void runOnSelectedComics(OrganizeFiles::Mode mode);
    void organizeComics(OrganizeFiles::Mode mode, const QList<ComicDB> &comics, const QString &libraryRoot, const QString &folderPath);

    bool applyToDatabase(const QList<OrganizeFiles::FileMove> &moves,
                         const QStringList &removedDirectories,
                         const QString &libraryRoot,
                         const QString &journalPath,
                         const QList<QVariantMap> &foldersToRestore,
                         const QList<qulonglong> &createdFolderIdsToRemove,
                         QString *error);
    // Runs on a worker thread; must not touch the GUI.
    bool undo(const QString &journalPath,
              const QString &libraryRoot,
              QList<OrganizeFiles::FileFailure> *failures,
              QString *error,
              const std::function<void(int done, int total, const QString &currentFile)> &fileProgress,
              const std::function<void()> &databasePhase);

    QSettings *settings;
    QWidget *window;
    ComicModel *comicsModel;
    FolderModel *foldersModel;
    SelectionProvider selectionProvider;
    CurrentFolderProvider currentFolderProvider;
    CurrentLibraryProvider currentLibraryProvider;
};

#endif // ORGANIZE_FILES_COORDINATOR_H
