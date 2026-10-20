#ifndef COMIC_MANAGEMENT_COORDINATOR_H
#define COMIC_MANAGEMENT_COORDINATOR_H

#include "comic_model.h"
#include "yacreader_global.h"

#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QPair>
#include <QString>

#include <functional>

class ComicFilesManager;
class ComicDB;
class ComicVineDialog;
class FolderModel;
class FolderModelProxy;
class PropertiesDialog;
class QImage;
class QProgressDialog;
class QSettings;
class QWidget;

class ComicManagementCoordinator : public QObject
{
    Q_OBJECT

public:
    using SelectionProvider = std::function<QModelIndexList()>;
    using CurrentListProvider = std::function<QModelIndex()>;
    using CurrentFolderProvider = std::function<QModelIndex()>;
    using CurrentComicProvider = std::function<QModelIndex()>;
    using ComicOpeningAllowedProvider = std::function<bool()>;
    using LibraryIdProvider = std::function<qulonglong()>;
    using LibraryPathProvider = std::function<QString()>;

    explicit ComicManagementCoordinator(QWidget *window,
                                        QSettings *settings,
                                        ComicModel *comicsModel,
                                        FolderModel *foldersModel,
                                        FolderModelProxy *foldersModelProxy,
                                        PropertiesDialog *propertiesDialog,
                                        ComicVineDialog *comicVineDialog,
                                        SelectionProvider selectionProvider,
                                        CurrentListProvider currentListProvider,
                                        CurrentFolderProvider currentFolderProvider,
                                        CurrentComicProvider currentComicProvider,
                                        ComicOpeningAllowedProvider comicOpeningAllowedProvider,
                                        LibraryIdProvider libraryIdProvider,
                                        LibraryPathProvider libraryPathProvider);

    bool hasCustomCoverInSelection() const;
    // one missing entry or deleted comic, or several entries of a list
    bool canMatchSelectionToLibraryComic() const;

public slots:
    void copyAndImportComicsToCurrentFolder(const QList<QPair<QString, QString>> &comics);
    void moveAndImportComicsToCurrentFolder(const QList<QPair<QString, QString>> &comics);
    void copyAndImportComicsToFolder(const QList<QPair<QString, QString>> &comics, const QModelIndex &folder);
    void moveAndImportComicsToFolder(const QList<QPair<QString, QString>> &comics, const QModelIndex &folder);
    void addSelectedComicsToFavorites();
    void addSelectedComicsToLabel(qulonglong labelId);
    void openCurrentComic();
    void openComic(const ComicDB &comic, ComicModel::Mode mode);
    void openContainingFolderOfCurrentComic();
    void showComicVineScraper();
    void showProperties();
    void setSelectedComicsRead();
    void setSelectedComicsUnread();
    void setSelectedComicsType(YACReader::FileType type);
    void resetSelectedComicRatings();
    void assignNumbers();
    void deleteMetadataFromSelectedComics();
    void deleteSelectedComics();
    void saveSelectedCoversTo();
    void setCustomCover(const ComicRowId &row, const QString &imagePath);
    void resetSelectedCustomCovers();
    void matchSelectedToLibraryComic();

    void setComicUnread(qulonglong libraryId, const ComicDB &comic);

signals:
    void importRequested(qulonglong destinationFolderId);
    void currentComicViewUpdateRequested();
    void currentSourceRefreshStarted();
    void currentSourceRefreshAccepted();
    void currentSourceRefreshCancelled();
    void comicNumbersAssigned(const ComicRowId &firstRow);
    void comicDeletionFinished();
    void rootContinueReadingReloadRequested();

private:
    struct SourceContext {
        QString libraryPath;
        int mode;
        qulonglong sourceId;
    };

    // entryId: the reading list entry of the opened row, 0 if none
    void startReader(const ComicDB &comic, ComicModel::Mode mode, qulonglong entryId);
    bool changePlaceholderCover(const Placeholder &placeholder, const QImage &cover);
    void matchSelectedEntriesToLibraryComics(const QModelIndexList &selection);
    QProgressDialog *newProgressDialog(const QString &label, int maximum);
    void copyAndImportComics(const QList<QPair<QString, QString>> &comics,
                             const QModelIndex &destinationFolder,
                             const QString &libraryPath);
    void moveAndImportComics(const QList<QPair<QString, QString>> &comics,
                             const QModelIndex &destinationFolder,
                             const QString &libraryPath);
    void processComicFiles(ComicFilesManager *comicFilesManager, QProgressDialog *progressDialog);
    // the selected rows that support the action
    QList<ComicRowId> selectedRowIds(ComicAction action) const;
    SourceContext currentSource() const;
    // empty when the source changed or a row is no longer in the model
    QModelIndexList indexesForRows(const QList<ComicRowId> &rowIds, const SourceContext &source) const;
    bool isCurrentSource(const SourceContext &source) const;
    void deleteComicsFromDisk(const QList<ComicRowId> &rowIds, const SourceContext &source);
    void deleteComicsFromList(const QList<ComicRowId> &rowIds, const SourceContext &source, int listType, qulonglong listId);
    void finishComicDeletion();

    QWidget *window;
    QSettings *settings;
    ComicModel *comicsModel;
    FolderModel *foldersModel;
    FolderModelProxy *foldersModelProxy;
    PropertiesDialog *propertiesDialog;
    ComicVineDialog *comicVineDialog;
    SelectionProvider selectionProvider;
    CurrentListProvider currentListProvider;
    CurrentFolderProvider currentFolderProvider;
    CurrentComicProvider currentComicProvider;
    ComicOpeningAllowedProvider comicOpeningAllowedProvider;
    LibraryIdProvider libraryIdProvider;
    LibraryPathProvider libraryPathProvider;
    bool comicDeletionFailed { false };
};

#endif // COMIC_MANAGEMENT_COORDINATOR_H
