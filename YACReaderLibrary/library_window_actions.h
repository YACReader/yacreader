#ifndef LIBRARY_WINDOW_ACTIONS_H
#define LIBRARY_WINDOW_ACTIONS_H

#include "qaction.h"

#include <QAction>
#include <QSettings>

class LibraryWindow;
class YACReaderHistoryController;
class YACReaderNavigationController;
class EditShortcutsDialog;
class HelpAboutDialog;
class YACReaderContentViewsManager;
class YACReaderFoldersView;
class YACReaderOptionsDialog;
class ServerConfigDialog;
class RecentVisibilityCoordinator;
class ComicManagementCoordinator;
class ReadingListManagementCoordinator;
class FolderManagementCoordinator;
class OrganizeFilesCoordinator;
class LibraryManagementCoordinator;
class LibraryDatabaseMaintenanceCoordinator;
class LibraryRepairCoordinator;
class RenameLibraryDialog;
struct Theme;

class LibraryWindowActions
{
public:
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
    QAction *backupLibraryAction;
    QAction *restoreLibraryAction;
    QAction *repairLibraryAction;
    QAction *removeLibraryAction;
    QAction *helpAboutAction;
    QAction *renameLibraryAction;

    QAction *openLibraryFolderAction;
    QAction *showLibraryInfo;

#ifndef Q_OS_MACOS
    QAction *toggleFullScreenAction;
#endif
    QAction *optionsAction;
    QAction *serverConfigAction;
    QAction *toggleComicsViewAction;

    // tree actions
    QAction *addFolderAction;
    QAction *renameFolderAction;
    QAction *deleteFolderAction;
    //--
    QAction *setRootIndexAction;
    QAction *expandAllNodesAction;
    QAction *colapseAllNodesAction;

    QAction *openContainingFolderAction;
    QAction *renameFilesAction;
    QAction *organizeFilesAction;
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
    //--
    QAction *setFolderCoverAction;
    QAction *deleteCustomFolderCoverAction;

    QAction *openContainingFolderComicAction;
    QAction *renameComicsFilesAction;
    QAction *organizeComicsFilesAction;
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

    LibraryWindowActions();
    void createActions(LibraryWindow *window, QSettings *settings);
    void createConnections(YACReaderHistoryController *historyController,
                           YACReaderNavigationController *navigationController,
                           LibraryWindow *window,
                           HelpAboutDialog *had,
                           YACReaderContentViewsManager *contentViewsManager,
                           EditShortcutsDialog *editShortcutsDialog,
                           YACReaderFoldersView *foldersView,
                           YACReaderOptionsDialog *optionsDialog,
                           ServerConfigDialog *serverConfigDialog,
                           RecentVisibilityCoordinator *recentVisibilityCoordinator,
                           ComicManagementCoordinator *comicManagementCoordinator,
                           ReadingListManagementCoordinator *readingListManagementCoordinator,
                           FolderManagementCoordinator *folderManagementCoordinator,
                           OrganizeFilesCoordinator *organizeFilesCoordinator,
                           LibraryManagementCoordinator *libraryManagementCoordinator,
                           LibraryDatabaseMaintenanceCoordinator *libraryDatabaseMaintenanceCoordinator,
                           LibraryRepairCoordinator *libraryRepairCoordinator,
                           RenameLibraryDialog *renameLibraryDialog);

    void setComicActionsDisabled(bool disabled);
    void setComicSelectionActionsEnabled(bool enabled);
    void disableLibrariesActions(bool disabled);
    void disableNoUpdatedLibrariesActions(bool disabled);
    void disableFoldersActions(bool disabled);
    void disableAllActions();
    void setUpShortcutsManagement(EditShortcutsDialog *editShortcutsDialog);
    void updateTheme(const Theme &theme);
};

#endif // LIBRARY_WINDOW_ACTIONS_H
