#ifndef __LIBRARY_CREATOR_H
#define __LIBRARY_CREATOR_H

#include "comic_db.h"
#include "data_base_management.h"
#include "folder.h"
#include "folder_placeholder_match.h"
#include "library_comic_match.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QRegExp>
#include <QSqlDatabase>
#include <QString>
#include <QThread>
#include <QtCore>

#include <atomic>
#include <optional>

enum class ReplaceComicResult {
    NotInserted, // the new file can't be read: keep the old comic
    SameComicInfo, // same content, only the date changed
    NewComicInfo, // new content: first time this hash is seen
    KnownComicInfo, // the content already has a comic_info in the library
};

// newComicInfoId is empty when no comic was inserted for the new file.
ReplaceComicResult classifyReplaceComic(ComicInfoId oldComicInfoId,
                                        std::optional<ComicInfoId> newComicInfoId,
                                        bool newComicInfoExisted);

class LibraryCreator : public QThread
{
    Q_OBJECT
public:
    LibraryCreator(QSettings *settings);
    void createLibrary(const QString &source, const QString &target);
    void updateLibrary(const QString &source, const QString &target);
    void updateFolder(const QString &source, const QString &target, const QString &folder, qulonglong folderId);
    void stop(); // used to stop the process and keep the changes
    void cancel(); // cancels this run and changes in the DB are rolled back
    LibraryMaintenanceLockFailure maintenanceFailure() const { return lastMaintenanceFailure; }
    bool wasCanceled() const { return canceled; }

private:
    void processLibrary(const QString &source, const QString &target);
    bool prepareFolderUpdate();
    enum Mode { CREATOR,
                UPDATER };
    // atributos "globales" durante el proceso de creación y actualización
    enum Mode _mode;
    QString _source;
    QString _target;
    QString _sourceFolder; // used for partial updates
    QStringList _nameFilter;
    QString _databaseConnection;
    struct ScanFolder {
        Folder folder;
        bool inDatabase;
    };
    QList<ScanFolder> _currentPathFolders; // lista de folders en el orden en el que están siendo explorados, el último es el folder actual
    // recursive method
    void create(QDir currentDirectory);
    void update(QDir currentDirectory);
    void run() override;
    qulonglong insertFolders(); // devuelve el id del último folder añadido (último en la ruta)
    bool checkCover(const QString &hash);
    // true when a comic row was inserted; a new file of an existing folder can be one of its missing comics
    bool insertComic(const QString &relativePath, const QFileInfo &fileInfo, bool matchMissingComics = false);
    void replaceComic(const QString &relativePath, const QFileInfo &fileInfo, ComicDB *comic);
    bool copyReplacedComicInfo(const ComicInfo &oldInfo, ComicInfo newInfo, const QFileInfo &fileInfo);
    // qulonglong insertFolder(qulonglong parentId,const Folder & folder);
    // qulonglong insertComic(const Comic & comic);
    std::atomic_bool stopRunning { false };
    std::atomic_bool canceled { false };
    // LibraryCreator está en modo creación si creation == true;
    bool creation;
    bool partialUpdate;
    LibraryMaintenanceLockFailure lastMaintenanceFailure;
    qulonglong _folderDestinationId;
    QSettings *settings;
    bool checkModifiedDatesOnUpdate;
    QStringList cleanup(QSqlDatabase &db, const QString &target); // returns the hashes of the covers to remove after the commit
    ComicMatchFiles missingComicMatchFiles;
    MissingComicMatcher missingComicMatcher;

signals:
    void coverExtracted(QString);
    void folderUpdated(QString);
    void comicAdded(QString, QString);
    void updated();
    void created();
    void failedCreatingDB(QString);
    void failedOpeningDB(QString);
    void updatedCurrentFolder(qulonglong folderId);
};

#endif
