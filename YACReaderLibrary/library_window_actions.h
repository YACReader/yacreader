#ifndef LIBRARY_WINDOW_ACTIONS_H
#define LIBRARY_WINDOW_ACTIONS_H

#include <QAction>
#include <QSettings>

#include "qaction.h"

class LibraryWindow;
class YACReaderHistoryController;
class EditShortcutsDialog;
class HelpAboutDialog;
class ExportLibraryDialog;
class YACReaderContentViewsManager;
class YACReaderFoldersView;
class YACReaderOptionsDialog;
class ServerConfigDialog;
class RecentVisibilityCoordinator;

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
    QAction *removeLibraryAction;
    QAction *helpAboutAction;
    QAction *renameLibraryAction;

    QAction *showLibraryInfo;

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

    LibraryWindowActions();
    void createActions(LibraryWindow *window, QSettings *settings);
    void createConnections(YACReaderHistoryController *historyController,
                           LibraryWindow *window,
                           HelpAboutDialog *had,
                           ExportLibraryDialog *exportLibraryDialog,
                           YACReaderContentViewsManager *contentViewsManager,
                           EditShortcutsDialog *editShortcutsDialog,
                           YACReaderFoldersView *foldersView,
                           YACReaderOptionsDialog *optionsDialog,
                           ServerConfigDialog *serverConfigDialog,
                           RecentVisibilityCoordinator *recentVisibilityCoordinator);

    void disableComicsActions(bool disabled);
    void disableLibrariesActions(bool disabled);
    void disableNoUpdatedLibrariesActions(bool disabled);
    void disableFoldersActions(bool disabled);
    void disableAllActions();
    void setUpShortcutsManagement(EditShortcutsDialog *editShortcutsDialog);
};

#endif // LIBRARY_WINDOW_ACTIONS_H
