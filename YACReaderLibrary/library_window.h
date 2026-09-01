#ifndef __LIBRARYWINDOW_H
#define __LIBRARYWINDOW_H

#include "comic_db.h"
#include "comic_model.h"
#include "folder.h"
#include "libraries_update_coordinator.h"
#include "library_window_actions.h"
#include "themable.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"
#include "yacreader_navigation_controller.h"

#include <QMainWindow>
#include <QModelIndex>

#ifdef Y_MAC_UI
#include "yacreader_macosx_toolbar.h"
#endif

class QTreeView;
class QDirModel;
class QAction;
class QMenu;
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
class HelpAboutDialog;
class RenameLibraryDialog;
class PropertiesDialog;
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
class QShowEvent;
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
class NoSearchResultsWidget;
class EditShortcutsDialog;
class ReadingListModel;
class ReadingListModelProxy;
class YACReaderReadingListsView;
class YACReaderHistoryController;
class EmptyLabelWidget;
class EmptySpecialListWidget;
class EmptyReadingListWidget;
class RecentVisibilityCoordinator;
class OrganizeFilesCoordinator;
class QToolButton;
class ComicManagementCoordinator;
class ReadingListManagementCoordinator;
class FolderManagementCoordinator;
class LibraryDatabaseMaintenanceCoordinator;
class LibraryRepairCoordinator;
class LibraryManagementCoordinator;
class LibraryWindowMenus;
class LibrarySearchCoordinator;

namespace YACReader {
class TrayIconController;
}

#include "comic_db.h"

using namespace YACReader;

class LibraryWindow : public QMainWindow, protected Themable
{
    friend class YACReaderNavigationController;

    Q_OBJECT
public:
    YACReaderSideBar *sideBar;
    QSplitter *mainSplitter;

    CreateLibraryDialog *createLibraryDialog;
    ExportLibraryDialog *exportLibraryDialog;
    ImportLibraryDialog *importLibraryDialog;
    ExportComicsInfoDialog *exportComicsInfoDialog;
    ImportComicsInfoDialog *importComicsInfoDialog;
    AddLibraryDialog *addLibraryDialog;
    HelpAboutDialog *had;
    RenameLibraryDialog *renameLibraryDialog;
    PropertiesDialog *propertiesDialog;
    ComicVineDialog *comicVineDialog;
    EditShortcutsDialog *editShortcutsDialog;
    bool fullscreen;
    bool importedCovers; // if true, the library is read only (not updates,open comic or properties)
    bool fromMaximized;

    QSize slideSizeW;
    QSize slideSizeF;
    // search filter
#ifdef Y_MAC_UI
    YACReaderMacOSXSearchLineEdit *searchEdit;
#else
    YACReaderSearchLineEdit *searchEdit;
#endif

    YACReaderNavigationController *navigationController;
    YACReaderContentViewsManager *contentViewsManager;
    LibraryWindowMenus *menus;

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

    LibraryWindowActions actions;

#ifdef Y_MAC_UI
    YACReaderMacOSXToolbar *libraryToolBar;
#else
    YACReaderMainToolBar *libraryToolBar;
#endif
    QToolBar *treeActions;
    QToolBar *comicsToolBar;
    QToolBar *editInfoToolBar;
    QToolButton *organizeToolButton = nullptr;
    QToolButton *setTypeToolButton = nullptr;
    QList<QAction *> comicToolbarEntries;
    QAction *comicToolbarEndAnchor = nullptr;

    OptionsDialog *optionsDialog;
    ServerConfigDialog *serverConfigDialog;

    void createSettings();
    void setupUI();
    void createToolBars();
    void createConnections();
    void doLayout();
    void doDialogs();
    void setUpShortcutsManagement();
    void doModels();
    void setupCoordinators();
    bool hasLoadedLibraryModels() const;
    QMenu *createSearchMenu();
    void applySearchQuery(const QString &query);
    void setSearchInputEnabled(bool enabled);
    void clearSearchInput(bool notify);
    void focusSearchInput();
    void showSearchSyntax();

    QString currentPath();

    // settings
    QSettings *settings;

    // navigation backward and forward
    YACReaderHistoryController *historyController;

    // QTBUG-41883
    QSize _size;
    QPoint _pos;

protected:
    virtual void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void applyTheme(const Theme &theme) override;

public:
    LibraryWindow();
    QString searchText() const;

public slots:
    void checkEmptyFolder();
    void loadLibraries();
    void reloadCurrentLibrary();
    void setRootIndex();
    void toggleFullScreen();
    void toNormal();
    void toFullScreen();
    void reloadOptions();
    void showExportComicsInfo();
    void showImportComicsInfo();
    void showNoLibrariesWidget();
    void showRootWidget();
    void showImportingWidget();
    QModelIndexList getSelectedComics();
    void updateViewsOnClientSync();
    void updateViewsOnComicUpdateWithId(quint64 libraryId, quint64 comicId);
    void updateViewsOnComicUpdate(quint64 libraryId, const ComicDB &comic);
    void loadCoversFromCurrentModel();
    void reloadCurrentFolderComicsContent();
    void reloadAfterCopyMove(const QModelIndex &mi);
    QModelIndex getCurrentFolderIndex();
    void enableNeededActions();
    void setComicActionsDisabled(bool disabled);
    void setComicToolbarEntriesVisible(bool visible);
    void setToolbarTitle(const QModelIndex &modelIndex);
    void setCurrentLibraryAs(FileType fileType);

    void prepareToCloseApp();
    void closeApp();

    void afterLaunchTasks();

    bool eventFilter(QObject *object, QEvent *event) override;

private:
    bool startsHiddenInTray() const;

    void applyLoadedLibrary(const QString &libraryDataPath, bool readOnly);
    void showLibraryManagementOnly();
    void addLibraryToSelector(const QString &libraryName, const QString &libraryPath);
    void handleLibraryRemoved(const QString &libraryName, bool librariesEmpty);

    TrayIconController *trayIconController;

    LibrarySearchCoordinator *librarySearchCoordinator;
    RecentVisibilityCoordinator *recentVisibilityCoordinator;
    OrganizeFilesCoordinator *organizeFilesCoordinator;
    ComicManagementCoordinator *comicManagementCoordinator;
    ReadingListManagementCoordinator *readingListManagementCoordinator;
    FolderManagementCoordinator *folderManagementCoordinator;
    LibraryDatabaseMaintenanceCoordinator *libraryDatabaseMaintenanceCoordinator;
    LibraryRepairCoordinator *libraryRepairCoordinator;
    LibraryManagementCoordinator *libraryManagementCoordinator;
    bool pendingAfterLaunchTasks;
};

#endif
