#include "library_window_actions.h"

#include "edit_shortcuts_dialog.h"
#include "library_window.h"
#include "shortcuts_manager.h"
#include "yacreader_history_controller.h"
#include "recent_visibility_coordinator.h"

#include "help_about_dialog.h"
#include "export_library_dialog.h"
#include "yacreader_content_views_manager.h"
#include "server_config_dialog.h"
#include "yacreader_folders_view.h"
#include "yacreader_options_dialog.h"

#include <QtCore>
#include <QtWidgets>

LibraryWindowActions::LibraryWindowActions()
{
}

void LibraryWindowActions::createActions(LibraryWindow *window, QSettings *settings)
{
    auto tr = [](const char *text) { return QObject::tr(text); };

    backAction = new QAction(window);
    QIcon icoBackButton;
    icoBackButton.addFile(addExtensionToIconPath(":/images/main_toolbar/back"), QSize(), QIcon::Normal);
    // icoBackButton.addPixmap(QPixmap(":/images/main_toolbar/back_disabled.png"), QIcon::Disabled);
    backAction->setData(BACK_ACTION_YL);
    backAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(BACK_ACTION_YL));
    backAction->setIcon(icoBackButton);
    backAction->setDisabled(true);

    forwardAction = new QAction(window);
    QIcon icoFordwardButton;
    icoFordwardButton.addFile(addExtensionToIconPath(":/images/main_toolbar/forward"), QSize(), QIcon::Normal);
    // icoFordwardButton.addPixmap(QPixmap(":/images/main_toolbar/forward_disabled.png"), QIcon::Disabled);
    forwardAction->setData(FORWARD_ACTION_YL);
    forwardAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(FORWARD_ACTION_YL));
    forwardAction->setIcon(icoFordwardButton);
    forwardAction->setDisabled(true);

    createLibraryAction = new QAction(window);
    createLibraryAction->setToolTip(tr("Create a new library"));
    createLibraryAction->setData(CREATE_LIBRARY_ACTION_YL);
    createLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(CREATE_LIBRARY_ACTION_YL));
    createLibraryAction->setIcon(QIcon(addExtensionToIconPath(":/images/sidebar/newLibraryIcon")));

    openLibraryAction = new QAction(window);
    openLibraryAction->setToolTip(tr("Open an existing library"));
    openLibraryAction->setData(OPEN_LIBRARY_ACTION_YL);
    openLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_LIBRARY_ACTION_YL));
    openLibraryAction->setIcon(QIcon(addExtensionToIconPath(":/images/sidebar/openLibraryIcon")));

    exportComicsInfoAction = new QAction(tr("Export comics info"), window);
    exportComicsInfoAction->setToolTip(tr("Export comics info"));
    exportComicsInfoAction->setData(EXPORT_COMICS_INFO_ACTION_YL);
    exportComicsInfoAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(EXPORT_COMICS_INFO_ACTION_YL));
    exportComicsInfoAction->setIcon(QIcon(":/images/menus_icons/exportComicsInfoIcon.svg"));

    importComicsInfoAction = new QAction(tr("Import comics info"), window);
    importComicsInfoAction->setToolTip(tr("Import comics info"));
    importComicsInfoAction->setData(IMPORT_COMICS_INFO_ACTION_YL);
    importComicsInfoAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(IMPORT_COMICS_INFO_ACTION_YL));
    importComicsInfoAction->setIcon(QIcon(":/images/menus_icons/importComicsInfoIcon.svg"));

    exportLibraryAction = new QAction(tr("Pack covers"), window);
    exportLibraryAction->setToolTip(tr("Pack the covers of the selected library"));
    exportLibraryAction->setData(EXPORT_LIBRARY_ACTION_YL);
    exportLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(EXPORT_LIBRARY_ACTION_YL));
    exportLibraryAction->setIcon(QIcon(":/images/menus_icons/exportLibraryIcon.svg"));

    importLibraryAction = new QAction(tr("Unpack covers"), window);
    importLibraryAction->setToolTip(tr("Unpack a catalog"));
    importLibraryAction->setData(IMPORT_LIBRARY_ACTION_YL);
    importLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(IMPORT_LIBRARY_ACTION_YL));
    importLibraryAction->setIcon(QIcon(":/images/menus_icons/importLibraryIcon.svg"));

    updateLibraryAction = new QAction(tr("Update library"), window);
    updateLibraryAction->setToolTip(tr("Update current library"));
    updateLibraryAction->setData(UPDATE_LIBRARY_ACTION_YL);
    updateLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(UPDATE_LIBRARY_ACTION_YL));
    updateLibraryAction->setIcon(QIcon(":/images/menus_icons/updateLibraryIcon.svg"));

    renameLibraryAction = new QAction(tr("Rename library"), window);
    renameLibraryAction->setToolTip(tr("Rename current library"));
    renameLibraryAction->setData(RENAME_LIBRARY_ACTION_YL);
    renameLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(RENAME_LIBRARY_ACTION_YL));
    renameLibraryAction->setIcon(QIcon(":/images/menus_icons/editIcon.svg"));

    removeLibraryAction = new QAction(tr("Remove library"), window);
    removeLibraryAction->setToolTip(tr("Remove current library from your collection"));
    removeLibraryAction->setData(REMOVE_LIBRARY_ACTION_YL);
    removeLibraryAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(REMOVE_LIBRARY_ACTION_YL));
    removeLibraryAction->setIcon(QIcon(":/images/menus_icons/removeLibraryIcon.svg"));

    rescanLibraryForXMLInfoAction = new QAction(tr("Rescan library for XML info"), window);
    rescanLibraryForXMLInfoAction->setToolTip(tr("Tries to find XML info embedded in comic files. You only need to do this if the library was created with 9.8.2 or earlier versions or if you are using third party software to embed XML info in the files."));
    rescanLibraryForXMLInfoAction->setData(RESCAN_LIBRARY_XML_INFO_ACTION_YL);
    rescanLibraryForXMLInfoAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(RESCAN_LIBRARY_XML_INFO_ACTION_YL));

    showLibraryInfo = new QAction(tr("Show library info"), window);
    showLibraryInfo->setToolTip(tr("Show information about the current library"));
    showLibraryInfo->setData(SHOW_LIBRARY_INFO_ACTION_YL);
    showLibraryInfo->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_LIBRARY_INFO_ACTION_YL));

    openComicAction = new QAction(tr("Open current comic"), window);
    openComicAction->setToolTip(tr("Open current comic on YACReader"));
    openComicAction->setData(OPEN_COMIC_ACTION_YL);
    openComicAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_COMIC_ACTION_YL));
    openComicAction->setIcon(QIcon(":/images/comics_view_toolbar/openInYACReader.svg"));

    saveCoversToAction = new QAction(tr("Save selected covers to..."), window);
    saveCoversToAction->setToolTip(tr("Save covers of the selected comics as JPG files"));
    saveCoversToAction->setData(SAVE_COVERS_TO_ACTION_YL);
    saveCoversToAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SAVE_COVERS_TO_ACTION_YL));

    setAsReadAction = new QAction(tr("Set as read"), window);
    setAsReadAction->setToolTip(tr("Set comic as read"));
    setAsReadAction->setData(SET_AS_READ_ACTION_YL);
    setAsReadAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_AS_READ_ACTION_YL));
    setAsReadAction->setIcon(QIcon(":/images/comics_view_toolbar/setReadButton.svg"));

    setAsNonReadAction = new QAction(tr("Set as unread"), window);
    setAsNonReadAction->setToolTip(tr("Set comic as unread"));
    setAsNonReadAction->setData(SET_AS_NON_READ_ACTION_YL);
    setAsNonReadAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_AS_NON_READ_ACTION_YL));
    setAsNonReadAction->setIcon(QIcon(":/images/comics_view_toolbar/setUnread.svg"));

    setMangaAction = new QAction(tr("manga"), window);
    setMangaAction->setToolTip(tr("Set issue as manga"));
    setMangaAction->setData(SET_AS_MANGA_ACTION_YL);
    setMangaAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_AS_MANGA_ACTION_YL));
    setMangaAction->setIcon(QIcon(":/images/comics_view_toolbar/setManga.svg"));

    setNormalAction = new QAction(tr("comic"), window);
    setNormalAction->setToolTip(tr("Set issue as normal"));
    setNormalAction->setData(SET_AS_NORMAL_ACTION_YL);
    setNormalAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_AS_NORMAL_ACTION_YL));
    setNormalAction->setIcon(QIcon(":/images/comics_view_toolbar/setNormal.svg"));

    setWesternMangaAction = new QAction(tr("western manga"), window);
    setWesternMangaAction->setToolTip(tr("Set issue as western manga"));
    setWesternMangaAction->setData(SET_AS_WESTERN_MANGA_ACTION_YL);
    setWesternMangaAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_AS_WESTERN_MANGA_ACTION_YL));
    // setWesternMangaAction->setIcon(QIcon(":/images/comics_view_toolbar/setWesternManga.svg"));

    setWebComicAction = new QAction(tr("web comic"), window);
    setWebComicAction->setToolTip(tr("Set issue as web comic"));
    setWebComicAction->setData(SET_AS_WEB_COMIC_ACTION_YL);
    setWebComicAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_AS_WEB_COMIC_ACTION_YL));
    // setWebComicAction->setIcon(QIcon(":/images/comics_view_toolbar/setWebComic.svg"));

    setYonkomaAction = new QAction(tr("yonkoma"), window);
    setYonkomaAction->setToolTip(tr("Set issue as yonkoma"));
    setYonkomaAction->setData(SET_AS_YONKOMA_ACTION_YL);
    setYonkomaAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_AS_YONKOMA_ACTION_YL));

    showHideMarksAction = new QAction(tr("Show/Hide marks"), window);
    showHideMarksAction->setToolTip(tr("Show or hide read marks"));
    showHideMarksAction->setData(SHOW_HIDE_MARKS_ACTION_YL);
    showHideMarksAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_HIDE_MARKS_ACTION_YL));
    showHideMarksAction->setCheckable(true);
    showHideMarksAction->setIcon(QIcon(":/images/comics_view_toolbar/showMarks.svg"));
    showHideMarksAction->setChecked(true);

    toogleShowRecentIndicatorAction = new QAction(tr("Show/Hide recent indicator"), window);
    toogleShowRecentIndicatorAction->setToolTip(tr("Show or hide recent indicator"));
    toogleShowRecentIndicatorAction->setData(SHOW_HIDE_RECENT_INDICATOR_ACTION_YL);
    toogleShowRecentIndicatorAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_HIDE_RECENT_INDICATOR_ACTION_YL));
    toogleShowRecentIndicatorAction->setCheckable(true);
    toogleShowRecentIndicatorAction->setIcon(QIcon(":/images/comics_view_toolbar/showRecentIndicator.svg"));
    toogleShowRecentIndicatorAction->setChecked(settings->value(DISPLAY_RECENTLY_INDICATOR, true).toBool());

#ifndef Q_OS_MACOS
    toggleFullScreenAction = new QAction(tr("Fullscreen mode on/off"), window);
    toggleFullScreenAction->setToolTip(tr("Fullscreen mode on/off"));
    toggleFullScreenAction->setData(TOGGLE_FULL_SCREEN_ACTION_YL);
    toggleFullScreenAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_FULL_SCREEN_ACTION_YL));
    QIcon icoFullscreenButton;
    icoFullscreenButton.addFile(addExtensionToIconPath(":/images/main_toolbar/fullscreen"), QSize(), QIcon::Normal);
    toggleFullScreenAction->setIcon(icoFullscreenButton);
#endif
    helpAboutAction = new QAction(window);
    helpAboutAction->setToolTip(tr("Help, About YACReader"));
    helpAboutAction->setData(HELP_ABOUT_ACTION_YL);
    helpAboutAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(HELP_ABOUT_ACTION_YL));
    QIcon icoHelpButton;
    icoHelpButton.addFile(addExtensionToIconPath(":/images/main_toolbar/help"), QSize(), QIcon::Normal);
    helpAboutAction->setIcon(icoHelpButton);

    addFolderAction = new QAction(tr("Add new folder"), window);
    addFolderAction->setData(ADD_FOLDER_ACTION_YL);
    addFolderAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ADD_FOLDER_ACTION_YL));
    addFolderAction->setToolTip(tr("Add new folder to the current library"));
    addFolderAction->setIcon(QIcon(addExtensionToIconPath(":/images/sidebar/addNew_sidebar")));

    deleteFolderAction = new QAction(tr("Delete folder"), window);
    deleteFolderAction->setData(REMOVE_FOLDER_ACTION_YL);
    deleteFolderAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(REMOVE_FOLDER_ACTION_YL));
    deleteFolderAction->setToolTip(tr("Delete current folder from disk"));
    deleteFolderAction->setIcon(QIcon(addExtensionToIconPath(":/images/sidebar/delete_sidebar")));

    setRootIndexAction = new QAction(window);
    setRootIndexAction->setData(SET_ROOT_INDEX_ACTION_YL);
    setRootIndexAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_ROOT_INDEX_ACTION_YL));
    setRootIndexAction->setToolTip(tr("Select root node"));
    setRootIndexAction->setIcon(QIcon(addExtensionToIconPath(":/images/sidebar/setRoot")));

    expandAllNodesAction = new QAction(window);
    expandAllNodesAction->setToolTip(tr("Expand all nodes"));
    expandAllNodesAction->setData(EXPAND_ALL_NODES_ACTION_YL);
    expandAllNodesAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(EXPAND_ALL_NODES_ACTION_YL));
    expandAllNodesAction->setIcon(QIcon(addExtensionToIconPath(":/images/sidebar/expand")));

    colapseAllNodesAction = new QAction(window);
    colapseAllNodesAction->setToolTip(tr("Collapse all nodes"));
    colapseAllNodesAction->setData(COLAPSE_ALL_NODES_ACTION_YL);
    colapseAllNodesAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(COLAPSE_ALL_NODES_ACTION_YL));
    colapseAllNodesAction->setIcon(QIcon(addExtensionToIconPath(":/images/sidebar/colapse")));

    optionsAction = new QAction(window);
    optionsAction->setToolTip(tr("Show options dialog"));
    optionsAction->setData(OPTIONS_ACTION_YL);
    optionsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPTIONS_ACTION_YL));
    QIcon icoSettingsButton;
    icoSettingsButton.addFile(addExtensionToIconPath(":/images/main_toolbar/settings"), QSize(), QIcon::Normal);
    optionsAction->setIcon(icoSettingsButton);

    serverConfigAction = new QAction(window);
    serverConfigAction->setToolTip(tr("Show comics server options dialog"));
    serverConfigAction->setData(SERVER_CONFIG_ACTION_YL);
    serverConfigAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SERVER_CONFIG_ACTION_YL));
    QIcon icoServerButton;
    icoServerButton.addFile(addExtensionToIconPath(":/images/main_toolbar/server"), QSize(), QIcon::Normal);
    serverConfigAction->setIcon(icoServerButton);

    toggleComicsViewAction = new QAction(tr("Change between comics views"), window);
    toggleComicsViewAction->setToolTip(tr("Change between comics views"));
    QIcon icoViewsButton;

    if (!settings->contains(COMICS_VIEW_STATUS) || settings->value(COMICS_VIEW_STATUS) == Flow)
        icoViewsButton.addFile(addExtensionToIconPath(":/images/main_toolbar/grid"), QSize(), QIcon::Normal);
    else if (settings->value(COMICS_VIEW_STATUS) == Grid)
        icoViewsButton.addFile(addExtensionToIconPath(":/images/main_toolbar/info"), QSize(), QIcon::Normal);
    else
        icoViewsButton.addFile(addExtensionToIconPath(":/images/main_toolbar/flow"), QSize(), QIcon::Normal);

    toggleComicsViewAction->setData(TOGGLE_COMICS_VIEW_ACTION_YL);
    toggleComicsViewAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(TOGGLE_COMICS_VIEW_ACTION_YL));
    toggleComicsViewAction->setIcon(icoViewsButton);
    // socialAction = new QAction(this);

    //----

    openContainingFolderAction = new QAction(window);
    openContainingFolderAction->setText(tr("Open folder..."));
    openContainingFolderAction->setData(OPEN_CONTAINING_FOLDER_ACTION_YL);
    openContainingFolderAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_CONTAINING_FOLDER_ACTION_YL));
    openContainingFolderAction->setIcon(QIcon(":/images/menus_icons/open_containing_folder.svg"));

    setFolderAsNotCompletedAction = new QAction(window);
    setFolderAsNotCompletedAction->setText(tr("Set as uncompleted"));
    setFolderAsNotCompletedAction->setData(SET_FOLDER_AS_NOT_COMPLETED_ACTION_YL);
    setFolderAsNotCompletedAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_NOT_COMPLETED_ACTION_YL));

    setFolderAsCompletedAction = new QAction(window);
    setFolderAsCompletedAction->setText(tr("Set as completed"));
    setFolderAsCompletedAction->setData(SET_FOLDER_AS_COMPLETED_ACTION_YL);
    setFolderAsCompletedAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_COMPLETED_ACTION_YL));

    setFolderAsReadAction = new QAction(window);
    setFolderAsReadAction->setText(tr("Set as read"));
    setFolderAsReadAction->setData(SET_FOLDER_AS_READ_ACTION_YL);
    setFolderAsReadAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_READ_ACTION_YL));

    setFolderAsUnreadAction = new QAction(window);
    setFolderAsUnreadAction->setText(tr("Set as unread"));
    setFolderAsUnreadAction->setData(SET_FOLDER_AS_UNREAD_ACTION_YL);
    setFolderAsUnreadAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_UNREAD_ACTION_YL));

    setFolderAsMangaAction = new QAction(window);
    setFolderAsMangaAction->setText(tr("manga"));
    setFolderAsMangaAction->setData(SET_FOLDER_AS_MANGA_ACTION_YL);
    setFolderAsMangaAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_MANGA_ACTION_YL));

    setFolderAsNormalAction = new QAction(window);
    setFolderAsNormalAction->setText(tr("comic"));
    setFolderAsNormalAction->setData(SET_FOLDER_AS_NORMAL_ACTION_YL);
    setFolderAsNormalAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_NORMAL_ACTION_YL));

    setFolderAsWesternMangaAction = new QAction(window);
    setFolderAsWesternMangaAction->setText(tr("western manga (left to right)"));
    setFolderAsWesternMangaAction->setData(SET_FOLDER_AS_WESTERN_MANGA_ACTION_YL);
    setFolderAsWesternMangaAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_WESTERN_MANGA_ACTION_YL));

    setFolderAsWebComicAction = new QAction(window);
    setFolderAsWebComicAction->setText(tr("web comic"));
    setFolderAsWebComicAction->setData(SET_FOLDER_AS_WEB_COMIC_ACTION_YL);
    setFolderAsWebComicAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_WEB_COMIC_ACTION_YL));

    setFolderAsYonkomaAction = new QAction(window);
    setFolderAsYonkomaAction->setText(tr("yonkoma"));
    setFolderAsYonkomaAction->setData(SET_FOLDER_AS_YONKOMA_ACTION_YL);
    setFolderAsYonkomaAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SET_FOLDER_AS_YONKOMA_ACTION_YL));

    //----

    openContainingFolderComicAction = new QAction(window);
    openContainingFolderComicAction->setText(tr("Open containing folder..."));
    openContainingFolderComicAction->setData(OPEN_CONTAINING_FOLDER_COMIC_ACTION_YL);
    openContainingFolderComicAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(OPEN_CONTAINING_FOLDER_COMIC_ACTION_YL));
    openContainingFolderComicAction->setIcon(QIcon(":/images/menus_icons/open_containing_folder.svg"));

    resetComicRatingAction = new QAction(window);
    resetComicRatingAction->setText(tr("Reset comic rating"));
    resetComicRatingAction->setData(RESET_COMIC_RATING_ACTION_YL);
    resetComicRatingAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(RESET_COMIC_RATING_ACTION_YL));

    // Edit comics actions------------------------------------------------------
    selectAllComicsAction = new QAction(window);
    selectAllComicsAction->setText(tr("Select all comics"));
    selectAllComicsAction->setData(SELECT_ALL_COMICS_ACTION_YL);
    selectAllComicsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SELECT_ALL_COMICS_ACTION_YL));
    selectAllComicsAction->setIcon(QIcon(":/images/comics_view_toolbar/selectAll.svg"));

    editSelectedComicsAction = new QAction(window);
    editSelectedComicsAction->setText(tr("Edit"));
    editSelectedComicsAction->setData(EDIT_SELECTED_COMICS_ACTION_YL);
    editSelectedComicsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(EDIT_SELECTED_COMICS_ACTION_YL));
    editSelectedComicsAction->setIcon(QIcon(":/images/comics_view_toolbar/editComic.svg"));

    asignOrderAction = new QAction(window);
    asignOrderAction->setText(tr("Assign current order to comics"));
    asignOrderAction->setData(ASIGN_ORDER_ACTION_YL);
    asignOrderAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ASIGN_ORDER_ACTION_YL));
    asignOrderAction->setIcon(QIcon(":/images/comics_view_toolbar/asignNumber.svg"));

    forceCoverExtractedAction = new QAction(window);
    forceCoverExtractedAction->setText(tr("Update cover"));
    forceCoverExtractedAction->setData(FORCE_COVER_EXTRACTED_ACTION_YL);
    forceCoverExtractedAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(FORCE_COVER_EXTRACTED_ACTION_YL));
    forceCoverExtractedAction->setIcon(QIcon(":/images/importCover.png"));

    deleteComicsAction = new QAction(window);
    deleteComicsAction->setText(tr("Delete selected comics"));
    deleteComicsAction->setData(DELETE_COMICS_ACTION_YL);
    deleteComicsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(DELETE_COMICS_ACTION_YL));
    deleteComicsAction->setIcon(QIcon(":/images/comics_view_toolbar/trash.svg"));

    deleteMetadataAction = new QAction(window);
    deleteMetadataAction->setText(tr("Delete metadata from selected comics"));
    deleteMetadataAction->setData(DELETE_METADATA_FROM_COMICS_ACTION_YL);
    deleteMetadataAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(DELETE_METADATA_FROM_COMICS_ACTION_YL));

    getInfoAction = new QAction(window);
    getInfoAction->setData(GET_INFO_ACTION_YL);
    getInfoAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(GET_INFO_ACTION_YL));
    getInfoAction->setText(tr("Download tags from Comic Vine"));
    getInfoAction->setIcon(QIcon(":/images/comics_view_toolbar/getInfo.svg"));
    //-------------------------------------------------------------------------

    focusSearchLineAction = new QAction(tr("Focus search line"), window);
    focusSearchLineAction->setData(FOCUS_SEARCH_LINE_ACTION_YL);
    focusSearchLineAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(FOCUS_SEARCH_LINE_ACTION_YL));
    focusSearchLineAction->setIcon(QIcon(":/images/iconSearch.png"));
    window->addAction(focusSearchLineAction);

    focusComicsViewAction = new QAction(tr("Focus comics view"), window);
    focusComicsViewAction->setData(FOCUS_COMICS_VIEW_ACTION_YL);
    focusComicsViewAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(FOCUS_COMICS_VIEW_ACTION_YL));
    window->addAction(focusComicsViewAction);

    showEditShortcutsAction = new QAction(tr("Edit shortcuts"), window);
    showEditShortcutsAction->setData(SHOW_EDIT_SHORTCUTS_ACTION_YL);
    showEditShortcutsAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SHOW_EDIT_SHORTCUTS_ACTION_YL));
    showEditShortcutsAction->setShortcutContext(Qt::ApplicationShortcut);
    window->addAction(showEditShortcutsAction);

    quitAction = new QAction(tr("&Quit"), window);
    quitAction->setIcon(QIcon(":/images/viewer_toolbar/close.svg"));
    quitAction->setData(QUIT_ACTION_YL);
    quitAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(QUIT_ACTION_YL));
    // TODO: is `quitAction->setMenuRole(QAction::QuitRole);` useful on macOS?
    window->addAction(quitAction);

    updateFolderAction = new QAction(tr("Update folder"), window);
    updateFolderAction->setIcon(QIcon(":/images/menus_icons/update_current_folder.svg"));

    updateCurrentFolderAction = new QAction(tr("Update current folder"), window);
    updateCurrentFolderAction->setData(UPDATE_CURRENT_FOLDER_ACTION_YL);
    updateCurrentFolderAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(UPDATE_CURRENT_FOLDER_ACTION_YL));
    updateCurrentFolderAction->setIcon(QIcon(":/images/menus_icons/update_current_folder.svg"));

    rescanXMLFromCurrentFolderAction = new QAction(tr("Scan legacy XML metadata"), window);
    rescanXMLFromCurrentFolderAction->setData(SCAN_XML_FROM_CURRENT_FOLDER_ACTION_YL);
    rescanXMLFromCurrentFolderAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(SCAN_XML_FROM_CURRENT_FOLDER_ACTION_YL));

    addReadingListAction = new QAction(tr("Add new reading list"), window);
    addReadingListAction->setData(ADD_READING_LIST_ACTION_YL);
    addReadingListAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ADD_READING_LIST_ACTION_YL));
    addReadingListAction->setToolTip(tr("Add a new reading list to the current library"));
    addReadingListAction->setIcon(QIcon(addExtensionToIconPath(":/images/sidebar/addNew_sidebar")));

    deleteReadingListAction = new QAction(tr("Remove reading list"), window);
    deleteReadingListAction->setData(REMOVE_READING_LIST_ACTION_YL);
    deleteReadingListAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(REMOVE_READING_LIST_ACTION_YL));
    deleteReadingListAction->setToolTip(tr("Remove current reading list from the library"));
    deleteReadingListAction->setIcon(QIcon(addExtensionToIconPath(":/images/sidebar/delete_sidebar")));

    addLabelAction = new QAction(tr("Add new label"), window);
    addLabelAction->setData(ADD_LABEL_ACTION_YL);
    addLabelAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ADD_LABEL_ACTION_YL));
    addLabelAction->setToolTip(tr("Add a new label to this library"));
    addLabelAction->setIcon(QIcon(addExtensionToIconPath(":/images/sidebar/addLabelIcon")));

    renameListAction = new QAction(tr("Rename selected list"), window);
    renameListAction->setData(RENAME_LIST_ACTION_YL);
    renameListAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(RENAME_LIST_ACTION_YL));
    renameListAction->setToolTip(tr("Rename any selected labels or lists"));
    renameListAction->setIcon(QIcon(addExtensionToIconPath(":/images/sidebar/renameListIcon")));

    //--
    addToMenuAction = new QAction(tr("Add to..."), window);

    addToFavoritesAction = new QAction(tr("Favorites"), window);
    addToFavoritesAction->setData(ADD_TO_FAVORITES_ACTION_YL);
    addToFavoritesAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(ADD_TO_FAVORITES_ACTION_YL));
    addToFavoritesAction->setToolTip(tr("Add selected comics to favorites list"));
    addToFavoritesAction->setIcon(QIcon(":/images/lists/default_1.svg"));

    // actions not asigned to any widget
    window->addAction(saveCoversToAction);
    window->addAction(openContainingFolderAction);
    window->addAction(updateCurrentFolderAction);
    window->addAction(resetComicRatingAction);
    window->addAction(setFolderAsCompletedAction);
    window->addAction(setFolderAsNotCompletedAction);
    window->addAction(setFolderAsReadAction);
    window->addAction(setFolderAsUnreadAction);
    window->addAction(setFolderAsMangaAction);
    window->addAction(setFolderAsNormalAction);
    window->addAction(setFolderAsWesternMangaAction);
    window->addAction(setFolderAsWebComicAction);
    window->addAction(setFolderAsYonkomaAction);
    window->addAction(deleteMetadataAction);
    window->addAction(rescanXMLFromCurrentFolderAction);
    window->addAction(openContainingFolderComicAction);
#ifndef Q_OS_MACOS
    window->addAction(toggleFullScreenAction);
#endif

    // disable actions
    disableAllActions();
}

void LibraryWindowActions::createConnections(
        YACReaderHistoryController *historyController,
        LibraryWindow *window,
        HelpAboutDialog *had,
        ExportLibraryDialog *exportLibraryDialog,
        YACReaderContentViewsManager *contentViewsManager,
        EditShortcutsDialog *editShortcutsDialog,
        YACReaderFoldersView *foldersView,
        YACReaderOptionsDialog *optionsDialog,
        ServerConfigDialog *serverConfigDialog,
        RecentVisibilityCoordinator *recentVisibilityCoordinator)
{
    // history navigation
    QObject::connect(backAction, &QAction::triggered, historyController, &YACReaderHistoryController::backward);
    QObject::connect(forwardAction, &QAction::triggered, historyController, &YACReaderHistoryController::forward);
    //--
    QObject::connect(historyController, &YACReaderHistoryController::enabledBackward, backAction, &QAction::setEnabled);
    QObject::connect(historyController, &YACReaderHistoryController::enabledForward, forwardAction, &QAction::setEnabled);
    // connect(foldersView, SIGNAL(clicked(QModelIndex)), historyController, SLOT(updateHistory(QModelIndex)));

    // actions
    QObject::connect(createLibraryAction, &QAction::triggered, window, &LibraryWindow::createLibrary);
    QObject::connect(exportLibraryAction, &QAction::triggered, exportLibraryDialog, &ExportLibraryDialog::open);
    QObject::connect(importLibraryAction, &QAction::triggered, window, &LibraryWindow::importLibraryPackage);

    QObject::connect(openLibraryAction, &QAction::triggered, window, &LibraryWindow::showAddLibrary);
    QObject::connect(setAsReadAction, &QAction::triggered, window, &LibraryWindow::setCurrentComicReaded);
    QObject::connect(setAsNonReadAction, &QAction::triggered, window, &LibraryWindow::setCurrentComicUnreaded);

    QObject::connect(setNormalAction, &QAction::triggered, window, [=]() {
        window->setSelectedComicsType(FileType::Comic);
    });
    QObject::connect(setMangaAction, &QAction::triggered, window, [=]() {
        window->setSelectedComicsType(FileType::Manga);
    });
    QObject::connect(setWesternMangaAction, &QAction::triggered, window, [=]() {
        window->setSelectedComicsType(FileType::WesternManga);
    });
    QObject::connect(setWebComicAction, &QAction::triggered, window, [=]() {
        window->setSelectedComicsType(FileType::WebComic);
    });
    QObject::connect(setYonkomaAction, &QAction::triggered, window, [=]() {
        window->setSelectedComicsType(FileType::Yonkoma);
    });

    // comicsInfoManagement
    QObject::connect(exportComicsInfoAction, &QAction::triggered, window, &LibraryWindow::showExportComicsInfo);
    QObject::connect(importComicsInfoAction, &QAction::triggered, window, &LibraryWindow::showImportComicsInfo);

    // ContextMenus
    QObject::connect(openContainingFolderComicAction, &QAction::triggered, window, &LibraryWindow::openContainingFolderComic);
    QObject::connect(setFolderAsNotCompletedAction, &QAction::triggered, window, &LibraryWindow::setFolderAsNotCompleted);
    QObject::connect(setFolderAsCompletedAction, &QAction::triggered, window, &LibraryWindow::setFolderAsCompleted);
    QObject::connect(setFolderAsReadAction, &QAction::triggered, window, &LibraryWindow::setFolderAsRead);
    QObject::connect(setFolderAsUnreadAction, &QAction::triggered, window, &LibraryWindow::setFolderAsUnread);
    QObject::connect(openContainingFolderAction, &QAction::triggered, window, &LibraryWindow::openContainingFolder);

    QObject::connect(setFolderAsMangaAction, &QAction::triggered, window, [=]() {
        window->setFolderType(FileType::Manga);
    });
    QObject::connect(setFolderAsNormalAction, &QAction::triggered, window, [=]() {
        window->setFolderType(FileType::Comic);
    });
    QObject::connect(setFolderAsWesternMangaAction, &QAction::triggered, window, [=]() {
        window->setFolderType(FileType::WesternManga);
    });
    QObject::connect(setFolderAsWebComicAction, &QAction::triggered, window, [=]() {
        window->setFolderType(FileType::WebComic);
    });
    QObject::connect(setFolderAsYonkomaAction, &QAction::triggered, window, [=]() {
        window->setFolderType(FileType::Yonkoma);
    });

    QObject::connect(resetComicRatingAction, &QAction::triggered, window, &LibraryWindow::resetComicRating);

    // Comicts edition
    QObject::connect(editSelectedComicsAction, &QAction::triggered, window, &LibraryWindow::showProperties);
    QObject::connect(asignOrderAction, &QAction::triggered, window, &LibraryWindow::asignNumbers);

    QObject::connect(deleteMetadataAction, &QAction::triggered, window, &LibraryWindow::deleteMetadataFromSelectedComics);

    QObject::connect(deleteComicsAction, &QAction::triggered, window, &LibraryWindow::deleteComics);

    QObject::connect(getInfoAction, &QAction::triggered, window, &LibraryWindow::showComicVineScraper);

    QObject::connect(focusComicsViewAction, &QAction::triggered, contentViewsManager, &YACReaderContentViewsManager::focusComicsViewViaShortcut);

    QObject::connect(showEditShortcutsAction, &QAction::triggered, editShortcutsDialog, &QWidget::show);

    QObject::connect(quitAction, &QAction::triggered, window, &LibraryWindow::closeApp);

    // update folders (partial updates)
    QObject::connect(updateCurrentFolderAction, &QAction::triggered, window, &LibraryWindow::updateCurrentFolder);
    QObject::connect(updateFolderAction, &QAction::triggered, window, &LibraryWindow::updateCurrentFolder);

    QObject::connect(rescanXMLFromCurrentFolderAction, &QAction::triggered, window, &LibraryWindow::rescanCurrentFolderForXMLInfo);

    // lists
    QObject::connect(addReadingListAction, &QAction::triggered, window, &LibraryWindow::addNewReadingList);
    QObject::connect(deleteReadingListAction, &QAction::triggered, window, &LibraryWindow::deleteSelectedReadingList);
    QObject::connect(addLabelAction, &QAction::triggered, window, &LibraryWindow::showAddNewLabelDialog);
    QObject::connect(renameListAction, &QAction::triggered, window, &LibraryWindow::showRenameCurrentList);

    QObject::connect(updateLibraryAction, &QAction::triggered, window, &LibraryWindow::updateLibrary);
    QObject::connect(renameLibraryAction, &QAction::triggered, window, &LibraryWindow::renameLibrary);
    // connect(deleteLibraryAction,SIGNAL(triggered()),window,SLOT(deleteLibrary()));
    QObject::connect(removeLibraryAction, &QAction::triggered, window, &LibraryWindow::removeLibrary);
    QObject::connect(rescanLibraryForXMLInfoAction, &QAction::triggered, window, &LibraryWindow::rescanLibraryForXMLInfo);
    QObject::connect(showLibraryInfo, &QAction::triggered, window, &LibraryWindow::showLibraryInfo);

    QObject::connect(openComicAction, &QAction::triggered, window, QOverload<>::of(&LibraryWindow::openComic));
    QObject::connect(helpAboutAction, &QAction::triggered, had, &QWidget::show);
    QObject::connect(addFolderAction, &QAction::triggered, window, &LibraryWindow::addFolderToCurrentIndex);
    QObject::connect(deleteFolderAction, &QAction::triggered, window, &LibraryWindow::deleteSelectedFolder);
    QObject::connect(setRootIndexAction, &QAction::triggered, window, &LibraryWindow::setRootIndex);
    QObject::connect(expandAllNodesAction, &QAction::triggered, foldersView, &QTreeView::expandAll);
    QObject::connect(colapseAllNodesAction, &QAction::triggered, foldersView, &QTreeView::collapseAll);
#ifndef Q_OS_MACOS
    QObject::connect(toggleFullScreenAction, &QAction::triggered, window, &LibraryWindow::toggleFullScreen);
#endif
    QObject::connect(toggleComicsViewAction, &QAction::triggered, contentViewsManager, &YACReaderContentViewsManager::toggleComicsView);
    QObject::connect(optionsAction, &QAction::triggered, optionsDialog, &QWidget::show);
#ifdef SERVER_RELEASE
    QObject::connect(serverConfigAction, &QAction::triggered, serverConfigDialog, &QWidget::show);
#endif

    QObject::connect(addToFavoritesAction, &QAction::triggered, window, &LibraryWindow::addSelectedComicsToFavorites);

    // save covers
    QObject::connect(saveCoversToAction, &QAction::triggered, window, &LibraryWindow::saveSelectedCoversTo);

    QObject::connect(toogleShowRecentIndicatorAction, &QAction::toggled, recentVisibilityCoordinator, &RecentVisibilityCoordinator::toggleVisibility);
}

void LibraryWindowActions::setUpShortcutsManagement(EditShortcutsDialog *editShortcutsDialog)
{

    QList<QAction *> allActions;
    QList<QAction *> tmpList;

    editShortcutsDialog->addActionsGroup("Comics", QIcon(":/images/shortcuts_group_comics.svg"),
                                         tmpList = QList<QAction *>()
                                                 << openComicAction
                                                 << saveCoversToAction
                                                 << setAsReadAction
                                                 << setAsNonReadAction
                                                 << setMangaAction
                                                 << setNormalAction
                                                 << openContainingFolderComicAction
                                                 << resetComicRatingAction
                                                 << selectAllComicsAction
                                                 << editSelectedComicsAction
                                                 << asignOrderAction
                                                 << deleteMetadataAction
                                                 << deleteComicsAction
                                                 << getInfoAction);

    allActions << tmpList;

    editShortcutsDialog->addActionsGroup("Folders", QIcon(":/images/shortcuts_group_folders.svg"),
                                         tmpList = QList<QAction *>()
                                                 << addFolderAction
                                                 << deleteFolderAction
                                                 << setRootIndexAction
                                                 << expandAllNodesAction
                                                 << colapseAllNodesAction
                                                 << openContainingFolderAction
                                                 << setFolderAsNotCompletedAction
                                                 << setFolderAsCompletedAction
                                                 << setFolderAsReadAction
                                                 << setFolderAsUnreadAction
                                                 << setFolderAsMangaAction
                                                 << setFolderAsNormalAction
                                                 << updateCurrentFolderAction
                                                 << rescanXMLFromCurrentFolderAction);
    allActions << tmpList;

    editShortcutsDialog->addActionsGroup("Lists", QIcon(":/images/shortcuts_group_folders.svg"), // TODO change icon
                                         tmpList = QList<QAction *>()
                                                 << addReadingListAction
                                                 << deleteReadingListAction
                                                 << addLabelAction
                                                 << renameListAction);
    allActions << tmpList;

    editShortcutsDialog->addActionsGroup("General", QIcon(":/images/shortcuts_group_general.svg"),
                                         tmpList = QList<QAction *>()
                                                 << backAction
                                                 << forwardAction
                                                 << focusSearchLineAction
                                                 << focusComicsViewAction
                                                 << helpAboutAction
                                                 << optionsAction
                                                 << serverConfigAction
                                                 << showEditShortcutsAction
                                                 << quitAction);

    allActions << tmpList;

    editShortcutsDialog->addActionsGroup("Libraries", QIcon(":/images/shortcuts_group_libraries.svg"),
                                         tmpList = QList<QAction *>()
                                                 << createLibraryAction
                                                 << openLibraryAction
                                                 << exportComicsInfoAction
                                                 << importComicsInfoAction
                                                 << exportLibraryAction
                                                 << importLibraryAction
                                                 << updateLibraryAction
                                                 << renameLibraryAction
                                                 << removeLibraryAction
                                                 << rescanLibraryForXMLInfoAction
                                                 << showLibraryInfo);

    allActions << tmpList;

    editShortcutsDialog->addActionsGroup("Visualization", QIcon(":/images/shortcuts_group_visualization.svg"),
                                         tmpList = QList<QAction *>()
                                                 << showHideMarksAction
                                                 << toogleShowRecentIndicatorAction
#ifndef Q_OS_MACOS
                                                 << toggleFullScreenAction // Think about what to do in macos if the default theme is used
#endif
                                                 << toggleComicsViewAction);

    allActions << tmpList;

    ShortcutsManager::getShortcutsManager().registerActions(allActions);
}

void LibraryWindowActions::disableComicsActions(bool disabled)
{
    // if there aren't comics, no fullscreen option will be available
#ifndef Q_OS_MACOS
    toggleFullScreenAction->setDisabled(disabled);
#endif
    // edit toolbar
    openComicAction->setDisabled(disabled);
    editSelectedComicsAction->setDisabled(disabled);
    selectAllComicsAction->setDisabled(disabled);
    asignOrderAction->setDisabled(disabled);
    setAsReadAction->setDisabled(disabled);
    setAsNonReadAction->setDisabled(disabled);
    setNormalAction->setDisabled(disabled);
    setMangaAction->setDisabled(disabled);
    setWebComicAction->setDisabled(disabled);
    setWesternMangaAction->setDisabled(disabled);
    setYonkomaAction->setDisabled(disabled);
    // setAllAsReadAction->setDisabled(disabled);
    // setAllAsNonReadAction->setDisabled(disabled);
    showHideMarksAction->setDisabled(disabled);
    deleteMetadataAction->setDisabled(disabled);
    deleteComicsAction->setDisabled(disabled);
    // context menu
    openContainingFolderComicAction->setDisabled(disabled);
    resetComicRatingAction->setDisabled(disabled);

    getInfoAction->setDisabled(disabled);

    updateCurrentFolderAction->setDisabled(disabled);
}
void LibraryWindowActions::disableLibrariesActions(bool disabled)
{
    updateLibraryAction->setDisabled(disabled);
    renameLibraryAction->setDisabled(disabled);
    removeLibraryAction->setDisabled(disabled);
    exportComicsInfoAction->setDisabled(disabled);
    importComicsInfoAction->setDisabled(disabled);
    exportLibraryAction->setDisabled(disabled);
    rescanLibraryForXMLInfoAction->setDisabled(disabled);
    // importLibraryAction->setDisabled(disabled);
}

void LibraryWindowActions::disableNoUpdatedLibrariesActions(bool disabled)
{
    updateLibraryAction->setDisabled(disabled);
    exportComicsInfoAction->setDisabled(disabled);
    importComicsInfoAction->setDisabled(disabled);
    exportLibraryAction->setDisabled(disabled);
    rescanLibraryForXMLInfoAction->setDisabled(disabled);
}

void LibraryWindowActions::disableFoldersActions(bool disabled)
{
    setRootIndexAction->setDisabled(disabled);
    expandAllNodesAction->setDisabled(disabled);
    colapseAllNodesAction->setDisabled(disabled);

    openContainingFolderAction->setDisabled(disabled);

    updateFolderAction->setDisabled(disabled);
    rescanXMLFromCurrentFolderAction->setDisabled(disabled);
}

void LibraryWindowActions::disableAllActions()
{
    disableComicsActions(true);
    disableLibrariesActions(true);
    disableFoldersActions(true);
}
