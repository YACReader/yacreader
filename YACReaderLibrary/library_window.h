#ifndef __LIBRARYWINDOW_H
#define __LIBRARYWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QModelIndex>
#include <QFileInfo>

#include "yacreader_global.h"
#include "yacreader_global_gui.h"
#include "yacreader_libraries.h"
#include "libraries_update_coordinator.h"

#include "yacreader_navigation_controller.h"
#include "comic_query_result_processor.h"
#include "folder_query_result_processor.h"

#include "comic_model.h"
#include "comic_db.h"
#include "folder.h"

#include <future>
#include <memory>

#ifdef Y_MAC_UI
#include "yacreader_macosx_toolbar.h"
#endif

class QTreeView;
class QDirModel;
class QAction;
class QToolBar;
class QComboBox;
class QThread;
class QStackedWidget;
class YACReaderSearchLineEdit;
class CreateLibraryDialog;
class ExportLibraryDialog;
class ImportLibraryDialog;
class ExportComicsInfoDialog;
class ImportComicsInfoDialog;
class AddLibraryDialog;
class LibraryCreator;
class HelpAboutDialog;
class RenameLibraryDialog;
class PropertiesDialog;
class PackageManager;
class QCheckBox;
class QPushButton;
class ComicModel;
class QSplitter;
class FolderModel;
class FolderModelProxy;
class QItemSelectionModel;
class QString;
class QLabel;
class NoLibrariesWidget;
class OptionsDialog;
class ServerConfigDialog;
class QCloseEvent;
class ImportWidget;
class QSettings;
class LibraryItem;
class YACReaderTableView;
class YACReaderSideBar;
class YACReaderLibraryListWidget;
class YACReaderFoldersView;
class YACReaderMainToolBar;
class ComicVineDialog;
class ComicsView;
class ClassicComicsView;
class GridComicsView;
class ComicsViewTransition;
class EmptyFolderWidget;
class NoSearchResultsWidget;
class EditShortcutsDialog;
class ComicFilesManager;
class QProgressDialog;
class ReadingListModel;
class ReadingListModelProxy;
class YACReaderReadingListsView;
class YACReaderHistoryController;
class EmptyLabelWidget;
class EmptySpecialListWidget;
class EmptyReadingListWidget;
class RecentVisibilityCoordinator;

namespace YACReader {
class TrayIconController;
class XMLInfoLibraryScanner;
}

#include "comic_db.h"

using namespace YACReader;

class LibraryWindow : public QMainWindow
{
    friend class YACReaderNavigationController;

    Q_OBJECT
public:
    YACReaderSideBar *sideBar;

    CreateLibraryDialog *createLibraryDialog;
    ExportLibraryDialog *exportLibraryDialog;
    ImportLibraryDialog *importLibraryDialog;
    ExportComicsInfoDialog *exportComicsInfoDialog;
    ImportComicsInfoDialog *importComicsInfoDialog;
    AddLibraryDialog *addLibraryDialog;
    LibraryCreator *libraryCreator;
    XMLInfoLibraryScanner *xmlInfoLibraryScanner;
    HelpAboutDialog *had;
    RenameLibraryDialog *renameLibraryDialog;
    PropertiesDialog *propertiesDialog;
    ComicVineDialog *comicVineDialog;
    EditShortcutsDialog *editShortcutsDialog;
    // YACReaderSocialDialog * socialDialog;
    bool fullscreen;
    bool importedCovers; // if true, the library is read only (not updates,open comic or properties)
    bool fromMaximized;

    PackageManager *packageManager;

    QSize slideSizeW;
    QSize slideSizeF;
    // search filter
#ifdef Y_MAC_UI
    YACReaderMacOSXSearchLineEdit *searchEdit;
#else
    YACReaderSearchLineEdit *searchEdit;
#endif

    QString previousFilter;
    QCheckBox *includeComicsCheckBox;
    //-------------

    YACReaderNavigationController *navigationController;
    YACReaderContentViewsManager *contentViewsManager;

    YACReaderFoldersView *foldersView;
    YACReaderReadingListsView *listsView;
    YACReaderLibraryListWidget *selectedLibrary;
    FolderModel *foldersModel;
    FolderModelProxy *foldersModelProxy;
    ComicModel *comicsModel;
    ReadingListModel *listsModel;
    ReadingListModelProxy *listsModelProxy;

    YACReaderLibraries libraries;
    LibrariesUpdateCoordinator *librariesUpdateCoordinator;

    QStackedWidget *mainWidget;
    NoLibrariesWidget *noLibrariesWidget;
    ImportWidget *importWidget;

    bool fetching;

    int i;

    QAction *backAction;
    QAction *forwardAction;

    QAction *openComicAction;
    QAction *createLibraryAction;
    QAction *openLibraryAction;

    QAction *exportComicsInfoAction;
    QAction *importComicsInfoAction;

    QAction *exportLibraryAction;
    QAction *importLibraryAction;

    QAction *rescanLibraryForXMLInfoAction;

    QAction *updateLibraryAction;
    QAction *removeLibraryAction;
    QAction *helpAboutAction;
    QAction *renameLibraryAction;
#ifndef Q_OS_MACOS
    QAction *toggleFullScreenAction;
#endif
    QAction *optionsAction;
    QAction *serverConfigAction;
    QAction *toggleComicsViewAction;
    // QAction * socialAction;

    // tree actions
    QAction *addFolderAction;
    QAction *deleteFolderAction;
    //--
    QAction *setRootIndexAction;
    QAction *expandAllNodesAction;
    QAction *colapseAllNodesAction;

    QAction *openContainingFolderAction;
    QAction *saveCoversToAction;
    //--
    QAction *setFolderAsNotCompletedAction;
    QAction *setFolderAsCompletedAction;
    //--
    QAction *setFolderAsReadAction;
    QAction *setFolderAsUnreadAction;
    //--
    QAction *setFolderAsMangaAction;
    QAction *setFolderAsNormalAction;
    QAction *setFolderAsWesternMangaAction;
    QAction *setFolderAsWebComicAction;
    QAction *setFolderAsYonkomaAction;

    QAction *openContainingFolderComicAction;
    QAction *setAsReadAction;
    QAction *setAsNonReadAction;

    QAction *setMangaAction;
    QAction *setNormalAction;
    QAction *setWesternMangaAction;
    QAction *setWebComicAction;
    QAction *setYonkomaAction;

    QAction *showHideMarksAction;
    QAction *getInfoAction; // comic vine
    QAction *resetComicRatingAction;

    QAction *toogleShowRecentIndicatorAction;

    // edit info actions
    QAction *selectAllComicsAction;
    QAction *editSelectedComicsAction;
    QAction *asignOrderAction;
    QAction *forceCoverExtractedAction;
    QAction *deleteComicsAction;
    QAction *deleteMetadataAction;

    QAction *focusSearchLineAction;
    QAction *focusComicsViewAction;

    QAction *showEditShortcutsAction;

    QAction *quitAction;

    QAction *updateFolderAction;
    QAction *updateCurrentFolderAction;
    QAction *rescanXMLFromCurrentFolderAction;

    // reading lists actions
    QAction *addReadingListAction;
    QAction *deleteReadingListAction;
    QAction *addLabelAction;
    QAction *renameListAction;
    //--
    QAction *addToMenuAction;
    QAction *addToFavoritesAction;

#ifdef Y_MAC_UI
    YACReaderMacOSXToolbar *libraryToolBar;
#else
    YACReaderMainToolBar *libraryToolBar;
#endif
    QToolBar *treeActions;
    QToolBar *comicsToolBar;
    QToolBar *editInfoToolBar;

    OptionsDialog *optionsDialog;
    ServerConfigDialog *serverConfigDialog;

    QString libraryPath;
    QString comicsPath;

    QString _lastAdded;
    QString _sourceLastAdded;

    quint64 _comicIdEdited;

    enum NavigationStatus {
        Normal, //
        Searching
    };

    NavigationStatus status;

    void createSettings();
    void setupOpenglSetting();
    void setupUI();
    void createActions();
    void createToolBars();
    void createMenus();
    void createConnections();
    void doLayout();
    void doDialogs();
    void setUpShortcutsManagement();
    void doModels();
    void setupCoordinators();

    // ACTIONS MANAGEMENT
    void disableComicsActions(bool disabled);
    void disableLibrariesActions(bool disabled);
    void disableNoUpdatedLibrariesActions(bool disabled);
    void disableFoldersActions(bool disabled);

    void disableAllActions();

    QString currentPath();
    QString currentFolderPath();

    // settings
    QSettings *settings;

    // navigation backward and forward
    YACReaderHistoryController *historyController;

    bool removeError;

    // QTBUG-41883
    QSize _size;
    QPoint _pos;

protected:
    virtual void closeEvent(QCloseEvent *event) override;

public:
    LibraryWindow();

signals:
    void libraryUpgraded(const QString &libraryName);
    void errorUpgradingLibrary(const QString &path);
public slots:
    void loadLibrary(const QString &path);
    void selectSubfolder(const QModelIndex &mi, int child);
    void checkEmptyFolder();
    void openComic();
    void openComic(const ComicDB &comic, const ComicModel::Mode mode);
    void createLibrary();
    void create(QString source, QString dest, QString name);
    void showAddLibrary();
    void openLibrary(QString path, QString name);
    void loadLibraries();
    void saveLibraries();
    void reloadCurrentLibrary();
    void openLastCreated();
    void updateLibrary();
    // void deleteLibrary();
    void openContainingFolder();
    void setFolderAsNotCompleted();
    void setFolderAsCompleted();
    void setFolderAsRead();
    void setFolderAsUnread();
    void setFolderType(FileType type);
    void openContainingFolderComic();
    void deleteCurrentLibrary();
    void removeLibrary();
    void renameLibrary();
    void rescanLibraryForXMLInfo();
    void rescanCurrentFolderForXMLInfo();
    void rescanFolderForXMLInfo(QModelIndex modelIndex);
    void rename(QString newName);
    void cancelCreating();
    void stopLibraryCreator();
    void stopXMLScanning();
    void setRootIndex();
    void toggleFullScreen();
    void toNormal();
    void toFullScreen();
    void setSearchFilter(QString filter);
    void setComicSearchFilterData(QList<ComicItem *> *, const QString &);
    void setFolderSearchFilterData(QMap<unsigned long long int, FolderItem *> *filteredItems, FolderItem *root);
    void clearSearchFilter();
    void showProperties();
    void exportLibrary(QString destPath);
    void importLibrary(QString clc, QString destPath, QString name);
    void reloadOptions();
    void setCurrentComicsStatusReaded(YACReaderComicReadStatus readStatus);
    void setCurrentComicReaded();
    void setCurrentComicUnreaded();
    void setSelectedComicsType(FileType type);
    void showExportComicsInfo();
    void showImportComicsInfo();
    void asignNumbers();
    void showNoLibrariesWidget();
    void showRootWidget();
    void showImportingWidget();
    void manageCreatingError(const QString &error);
    void manageUpdatingError(const QString &error);
    void manageOpeningLibraryError(const QString &error);
    QModelIndexList getSelectedComics();
    void deleteMetadataFromSelectedComics();
    void deleteComics();
    void deleteComicsFromDisk();
    void deleteComicsFromList();
    // void showSocial();
    void showFoldersContextMenu(const QPoint &point);
    void showGridFoldersContextMenu(QPoint point, Folder folder);
    void showContinueReadingContextMenu(QPoint point, ComicDB comic);
    void libraryAlreadyExists(const QString &name);
    void importLibraryPackage();
    void updateViewsOnClientSync();
    void updateViewsOnComicUpdateWithId(quint64 libraryId, quint64 comicId);
    void updateViewsOnComicUpdate(quint64 libraryId, const ComicDB &comic);
    void showComicVineScraper();
    void setRemoveError();
    void checkRemoveError();
    void resetComicRating();
    void checkSearchNumResults(int numResults);
    void loadCoversFromCurrentModel();
    void copyAndImportComicsToCurrentFolder(const QList<QPair<QString, QString>> &comics);
    void moveAndImportComicsToCurrentFolder(const QList<QPair<QString, QString>> &comics);
    void copyAndImportComicsToFolder(const QList<QPair<QString, QString>> &comics, const QModelIndex &miFolder);
    void moveAndImportComicsToFolder(const QList<QPair<QString, QString>> &comics, const QModelIndex &miFolder);
    void processComicFiles(ComicFilesManager *comicFilesManager, QProgressDialog *progressDialog);
    void updateCopyMoveFolderDestination(const QModelIndex &mi); // imports new comics from the current folder
    void updateCurrentFolder();
    void updateFolder(const QModelIndex &miFolder);
    QProgressDialog *newProgressDialog(const QString &label, int maxValue);
    void reloadCurrentFolderComicsContent();
    void reloadAfterCopyMove(const QModelIndex &mi);
    QModelIndex getCurrentFolderIndex();
    void enableNeededActions();
    void addFolderToCurrentIndex();
    void deleteSelectedFolder();
    void errorDeletingFolder();
    void addNewReadingList();
    void deleteSelectedReadingList();
    void showAddNewLabelDialog();
    void showRenameCurrentList();
    void addSelectedComicsToFavorites();
    void showComicsViewContextMenu(const QPoint &point);
    void showComicsItemContextMenu(const QPoint &point);
    void showComicsContextMenu(const QPoint &point, bool showFullScreenAction);
    void setupAddToSubmenu(QMenu &menu);
    void onAddComicsToLabel();
    void setToolbarTitle(const QModelIndex &modelIndex);
    void saveSelectedCoversTo();
    void checkMaxNumLibraries();
    void showErrorUpgradingLibrary(const QString &path);

    void prepareToCloseApp();
    void closeApp();

    void afterLaunchTasks();

    bool eventFilter(QObject *object, QEvent *event) override;

private:
    //! @brief Exits search mode if it is active.
    //! @return true If the search mode was active when this function was called.
    bool exitSearchMode();

    // fullscreen mode in Windows for preventing this bug: QTBUG-41309 https://bugreports.qt.io/browse/QTBUG-41309
    Qt::WindowFlags previousWindowFlags;
    QPoint previousPos;
    QSize previousSize;
    std::future<void> upgradeLibraryFuture;

    TrayIconController *trayIconController;
    ComicQueryResultProcessor comicQueryResultProcessor;
    std::unique_ptr<FolderQueryResultProcessor> folderQueryResultProcessor;

    RecentVisibilityCoordinator *recentVisibilityCoordinator;
};

#endif
