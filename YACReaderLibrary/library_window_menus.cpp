#include "library_window_menus.h"

#include "comic_management_coordinator.h"
#include "comic_model.h"
#include "feature_flags.h"
#include "folder_management_coordinator.h"
#include "folder_model.h"
#include "grid_comics_view.h"
#include "library_window_actions.h"
#include "organize_files_coordinator.h"
#include "reading_list_item.h"
#include "reading_list_model.h"
#include "theme.h"
#include "yacreader_content_views_manager.h"
#include "yacreader_folders_view.h"
#include "yacreader_global_gui.h"
#include "yacreader_library_list_widget.h"

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>

#include <utility>

namespace {
struct TypeActions {
    QAction *comic;
    QAction *manga;
    QAction *westernManga;
    QAction *webComic;
    QAction *yonkoma;
};

TypeActions addTypeActions(QMenu *menu)
{
    TypeActions typeActions {
        new QAction(LibraryWindowMenus::tr("comic"), menu),
        new QAction(LibraryWindowMenus::tr("manga"), menu),
        new QAction(LibraryWindowMenus::tr("western manga (left to right)"), menu),
        new QAction(LibraryWindowMenus::tr("web comic"), menu),
        new QAction(LibraryWindowMenus::tr("4koma (top to botom)"), menu)
    };

    const QList<QAction *> actions { typeActions.comic, typeActions.manga, typeActions.westernManga, typeActions.webComic, typeActions.yonkoma };
    for (auto *action : actions) {
        action->setCheckable(true);
        menu->addAction(action);
    }

    return typeActions;
}

void setCheckedType(const TypeActions &actions, YACReader::FileType type)
{
    actions.comic->setChecked(type == YACReader::FileType::Comic);
    actions.manga->setChecked(type == YACReader::FileType::Manga);
    actions.westernManga->setChecked(type == YACReader::FileType::WesternManga);
    actions.webComic->setChecked(type == YACReader::FileType::WebComic);
    actions.yonkoma->setChecked(type == YACReader::FileType::Yonkoma);
}

void connectTypeActions(const TypeActions &actions, QObject *context, const std::function<void(YACReader::FileType)> &handler)
{
    QObject::connect(actions.comic, &QAction::triggered, context, [handler] { handler(YACReader::FileType::Comic); });
    QObject::connect(actions.manga, &QAction::triggered, context, [handler] { handler(YACReader::FileType::Manga); });
    QObject::connect(actions.westernManga, &QAction::triggered, context, [handler] { handler(YACReader::FileType::WesternManga); });
    QObject::connect(actions.webComic, &QAction::triggered, context, [handler] { handler(YACReader::FileType::WebComic); });
    QObject::connect(actions.yonkoma, &QAction::triggered, context, [handler] { handler(YACReader::FileType::Yonkoma); });
}
}

LibraryWindowMenus::LibraryWindowMenus(QMainWindow *window,
                                       LibraryWindowActions &actions,
                                       YACReaderLibraryListWidget *selectedLibrary,
                                       YACReaderFoldersView *foldersView,
                                       YACReaderContentViewsManager *contentViewsManager,
                                       FolderModel *foldersModel,
                                       FolderModelProxy *foldersModelProxy,
                                       ReadingListModel *listsModel,
                                       FolderManagementCoordinator *folderManagementCoordinator,
                                       ComicManagementCoordinator *comicManagementCoordinator,
                                       OrganizeFilesCoordinator *organizeFilesCoordinator,
                                       ComicSelectionProvider comicSelectionProvider,
                                       LibraryIdProvider libraryIdProvider,
                                       LibraryPathProvider libraryPathProvider,
                                       ThemeProvider themeProvider)
    : QObject(window), window(window), actions(actions), selectedLibrary(selectedLibrary), foldersView(foldersView), contentViewsManager(contentViewsManager), foldersModel(foldersModel), foldersModelProxy(foldersModelProxy), listsModel(listsModel), folderManagementCoordinator(folderManagementCoordinator), comicManagementCoordinator(comicManagementCoordinator), organizeFilesCoordinator(organizeFilesCoordinator), comicSelectionProvider(std::move(comicSelectionProvider)), libraryIdProvider(std::move(libraryIdProvider)), libraryPathProvider(std::move(libraryPathProvider)), themeProvider(std::move(themeProvider))
{
}

void LibraryWindowMenus::setupMenus()
{
    connect(foldersView, &QWidget::customContextMenuRequested, this, &LibraryWindowMenus::showFoldersContextMenu);
    auto gridView = contentViewsManager->gridView();
    connect(gridView, &GridComicsView::openFolderContextMenu, this, [this, gridView](const QPoint &point, const Folder &folder) {
        showGridFoldersContextMenu(gridView->mapToGlobal(point), folder);
    });
    connect(gridView, &GridComicsView::openContinueReadingComicContextMenu, this, [this, gridView](const QPoint &point, const ComicDB &comic) {
        showContinueReadingContextMenu(gridView->mapToGlobal(point), comic);
    });

    foldersView->addAction(actions.addFolderAction);
    foldersView->addAction(actions.renameFolderAction);
    foldersView->addAction(actions.deleteFolderAction);
    YACReader::addSperator(foldersView);

    foldersView->addAction(actions.openContainingFolderAction);
    foldersView->addAction(actions.updateFolderAction);
    YACReader::addSperator(foldersView);

    foldersView->addAction(actions.setFolderAsNotCompletedAction);
    foldersView->addAction(actions.setFolderAsCompletedAction);
    YACReader::addSperator(foldersView);

    foldersView->addAction(actions.setFolderAsReadAction);
    foldersView->addAction(actions.setFolderAsUnreadAction);
    YACReader::addSperator(foldersView);

    foldersView->addAction(actions.setFolderAsNormalAction);
    foldersView->addAction(actions.setFolderAsMangaAction);
    foldersView->addAction(actions.setFolderAsWesternMangaAction);
    foldersView->addAction(actions.setFolderAsWebComicAction);
    foldersView->addAction(actions.setFolderAsYonkomaAction);
    YACReader::addSperator(foldersView);

    foldersView->addAction(actions.setFolderCoverAction);
    foldersView->addAction(actions.deleteCustomFolderCoverAction);

    selectedLibrary->addAction(actions.updateLibraryAction);
    selectedLibrary->addAction(actions.renameLibraryAction);
    selectedLibrary->addAction(actions.removeLibraryAction);
    YACReader::addSperator(selectedLibrary);

    auto typeMenu = new QMenu(tr("Set type"), selectedLibrary);
    const auto typeActions = addTypeActions(typeMenu);
    connectTypeActions(typeActions, this, [this](YACReader::FileType type) { emit currentLibraryTypeChangeRequested(type); });
    connect(typeMenu, &QMenu::aboutToShow, this, [this, typeActions] { setCheckedType(typeActions, foldersModel->getRootFolder().type); });

    selectedLibrary->addAction(typeMenu->menuAction());
    YACReader::addSperator(selectedLibrary);

    selectedLibrary->addAction(actions.rescanLibraryForXMLInfoAction);
    selectedLibrary->addAction(actions.repairLibraryAction);
    YACReader::addSperator(selectedLibrary);

    selectedLibrary->addAction(actions.backupLibraryAction);
    selectedLibrary->addAction(actions.restoreLibraryAction);
    YACReader::addSperator(selectedLibrary);

    selectedLibrary->addAction(actions.exportComicsInfoAction);
    selectedLibrary->addAction(actions.importComicsInfoAction);
    YACReader::addSperator(selectedLibrary);

    selectedLibrary->addAction(actions.exportLibraryAction);
    selectedLibrary->addAction(actions.importLibraryAction);
    YACReader::addSperator(selectedLibrary);

    selectedLibrary->addAction(actions.openLibraryFolderAction);
    selectedLibrary->addAction(actions.showLibraryInfo);

#ifdef Q_OS_MACOS
    auto menuBar = window->menuBar();

    auto libraryMenu = new QMenu(tr("Library"), menuBar);
    libraryMenu->addAction(actions.updateLibraryAction);
    libraryMenu->addAction(actions.renameLibraryAction);
    libraryMenu->addAction(actions.removeLibraryAction);
    libraryMenu->addSeparator();
    libraryMenu->addMenu(typeMenu);
    libraryMenu->addSeparator();
    libraryMenu->addAction(actions.rescanLibraryForXMLInfoAction);
    libraryMenu->addAction(actions.repairLibraryAction);
    libraryMenu->addSeparator();
    libraryMenu->addAction(actions.backupLibraryAction);
    libraryMenu->addAction(actions.restoreLibraryAction);
    libraryMenu->addSeparator();
    libraryMenu->addAction(actions.exportComicsInfoAction);
    libraryMenu->addAction(actions.importComicsInfoAction);
    libraryMenu->addSeparator();
    libraryMenu->addAction(actions.exportLibraryAction);
    libraryMenu->addAction(actions.importLibraryAction);
    libraryMenu->addSeparator();
    libraryMenu->addAction(actions.openLibraryFolderAction);
    libraryMenu->addAction(actions.showLibraryInfo);

    auto folderMenu = new QMenu(tr("Folder"), menuBar);
    folderMenu->addAction(actions.openContainingFolderAction);
    folderMenu->addAction(actions.renameFolderAction);
    folderMenu->addAction(actions.updateFolderAction);
    folderMenu->addSeparator();
    folderMenu->addAction(actions.rescanXMLFromCurrentFolderAction);
    folderMenu->addSeparator();
    folderMenu->addAction(actions.setFolderAsNotCompletedAction);
    folderMenu->addAction(actions.setFolderAsCompletedAction);
    folderMenu->addSeparator();
    folderMenu->addAction(actions.setFolderAsReadAction);
    folderMenu->addAction(actions.setFolderAsUnreadAction);
    folderMenu->addSeparator();
    folderMenu->addAction(actions.setFolderAsNormalAction);
    folderMenu->addAction(actions.setFolderAsMangaAction);
    folderMenu->addAction(actions.setFolderAsWesternMangaAction);
    folderMenu->addAction(actions.setFolderAsWebComicAction);
    folderMenu->addAction(actions.setFolderAsYonkomaAction);
    folderMenu->addSeparator();
    folderMenu->addAction(actions.setFolderCoverAction);
    folderMenu->addAction(actions.deleteCustomFolderCoverAction);

    auto comicMenu = new QMenu(tr("Comic"), menuBar);
    comicMenu->addAction(actions.openContainingFolderComicAction);
    comicMenu->addSeparator();
    comicMenu->addAction(actions.resetComicRatingAction);

    menuBar->addMenu(libraryMenu);
    menuBar->addMenu(folderMenu);
    menuBar->addMenu(comicMenu);
#endif
}

void LibraryWindowMenus::showComicsViewContextMenu(const QPoint &point)
{
    showComicsContextMenu(point, true);
}

void LibraryWindowMenus::showComicsItemContextMenu(const QPoint &point)
{
    showComicsContextMenu(point, false);
}

void LibraryWindowMenus::showComicsContextMenu(const QPoint &point, bool showFullScreenAction)
{
    const auto selection = comicSelectionProvider();
    auto menu = new QMenu(window);
    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);

    auto typeMenu = new QMenu(tr("Set type"), menu);
    const auto typeActions = addTypeActions(typeMenu);
    connectTypeActions(typeActions, menu, [this](YACReader::FileType type) { comicManagementCoordinator->setSelectedComicsType(type); });
    if (selection.size() == 1)
        setCheckedType(typeActions, selection.constFirst().data(ComicModel::TypeRole).value<YACReader::FileType>());

    menu->addAction(actions.openComicAction);
    menu->addAction(actions.saveCoversToAction);
    menu->addSeparator();
    menu->addAction(actions.openContainingFolderComicAction);
    menu->addAction(actions.updateCurrentFolderAction);
    menu->addSeparator();
    menu->addAction(actions.editSelectedComicsAction);
    menu->addAction(actions.getInfoAction);
    menu->addAction(actions.asignOrderAction);
    if (YACReader::FeatureFlags::organizeFiles) {
        menu->addSeparator();
        menu->addAction(actions.renameComicsFilesAction);
        menu->addAction(actions.organizeComicsFilesAction);
    }
    menu->addSeparator();
    menu->addAction(actions.selectAllComicsAction);
    menu->addSeparator();
    menu->addAction(actions.setAsReadAction);
    menu->addAction(actions.setAsNonReadAction);
    menu->addSeparator();
    menu->addMenu(typeMenu);
    menu->addSeparator();
    menu->addAction(actions.resetComicRatingAction);
    menu->addSeparator();
    menu->addAction(actions.deleteMetadataAction);
    if (comicManagementCoordinator->hasCustomCoverInSelection()) {
        auto resetCustomCoverAction = menu->addAction(tr("Delete custom cover"));
        connect(resetCustomCoverAction, &QAction::triggered, menu, [this] { comicManagementCoordinator->resetSelectedCustomCovers(); });
    }
    menu->addSeparator();
    menu->addAction(actions.deleteComicsAction);
    menu->addSeparator();
    menu->addAction(actions.addToMenuAction);
    auto subMenu = new QMenu(menu);
    setupAddToSubmenu(*subMenu);

#ifndef Q_OS_MACOS
    if (showFullScreenAction) {
        menu->addSeparator();
        menu->addAction(actions.toggleFullScreenAction);
    }
#else
    Q_UNUSED(showFullScreenAction);
#endif

    menu->popup(contentViewsManager->comicsView->mapToGlobal(point));
}

void LibraryWindowMenus::showGridFoldersContextMenu(const QPoint &point, const Folder &folder)
{
    auto menu = new QMenu(window);
    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);

    const auto folderId = folder.id;
    const auto libraryPath = libraryPathProvider();
    const auto &theme = themeProvider();

    auto openContainingFolderAction = new QAction(tr("Open folder..."), menu);
    openContainingFolderAction->setIcon(theme.menuIcons.openContainingFolderIcon);
    auto updateFolderAction = new QAction(tr("Update folder"), menu);
    updateFolderAction->setIcon(theme.menuIcons.updateCurrentFolderIcon);
    auto renameFolderAction = new QAction(tr("Rename folder"), menu);
    renameFolderAction->setIcon(theme.sidebarIcons.renameListIcon);
    auto renameFilesAction = new QAction(tr("Rename files..."), menu);
    auto organizeFilesAction = new QAction(tr("Organize into folders..."), menu);
    auto rescanLibraryForXMLInfoAction = new QAction(tr("Rescan library for XML info"), menu);
    auto setFolderAsNotCompletedAction = new QAction(tr("Set as uncompleted"), menu);
    auto setFolderAsCompletedAction = new QAction(tr("Set as completed"), menu);
    auto setFolderAsReadAction = new QAction(tr("Set as read"), menu);
    auto setFolderAsUnreadAction = new QAction(tr("Set as unread"), menu);
    auto setFolderCoverAction = new QAction(tr("Set custom cover"), menu);
    auto deleteCustomFolderCoverAction = new QAction(tr("Delete custom cover"), menu);

    menu->addAction(openContainingFolderAction);
    menu->addAction(renameFolderAction);
    menu->addAction(updateFolderAction);
    if (YACReader::FeatureFlags::organizeFiles) {
        menu->addSeparator();
        menu->addAction(renameFilesAction);
        menu->addAction(organizeFilesAction);
    }
    menu->addSeparator();
    menu->addAction(rescanLibraryForXMLInfoAction);
    menu->addSeparator();
    menu->addAction(folder.completed ? setFolderAsNotCompletedAction : setFolderAsCompletedAction);
    menu->addSeparator();
    menu->addAction(folder.finished ? setFolderAsUnreadAction : setFolderAsReadAction);
    menu->addSeparator();

    auto typeMenu = new QMenu(tr("Set type"), menu);
    const auto typeActions = addTypeActions(typeMenu);
    setCheckedType(typeActions, folder.type);
    menu->addMenu(typeMenu);

    connect(openContainingFolderAction, &QAction::triggered, menu, [this, folderId, libraryPath] { folderManagementCoordinator->openFolder(folderId, libraryPath); });
    connect(updateFolderAction, &QAction::triggered, menu, [this, folder] { emit folderUpdateRequested(foldersModel->getIndexFromFolder(folder)); });
    connect(renameFolderAction, &QAction::triggered, menu, [this, folderId, libraryPath] { folderManagementCoordinator->renameFolder(folderId, libraryPath); });
    connect(renameFilesAction, &QAction::triggered, menu, [this, folder] { organizeFilesCoordinator->renameFolder(foldersModel->getIndexFromFolder(folder)); });
    connect(organizeFilesAction, &QAction::triggered, menu, [this, folder] { organizeFilesCoordinator->organizeFolder(foldersModel->getIndexFromFolder(folder)); });
    connect(rescanLibraryForXMLInfoAction, &QAction::triggered, menu, [this, folder] { emit folderXmlRescanRequested(foldersModel->getIndexFromFolder(folder)); });
    connect(setFolderAsNotCompletedAction, &QAction::triggered, menu, [this, folderId, libraryPath] { folderManagementCoordinator->setFolderCompleted(folderId, libraryPath, false); });
    connect(setFolderAsCompletedAction, &QAction::triggered, menu, [this, folderId, libraryPath] { folderManagementCoordinator->setFolderCompleted(folderId, libraryPath, true); });
    connect(setFolderAsReadAction, &QAction::triggered, menu, [this, folderId, libraryPath] { folderManagementCoordinator->setFolderRead(folderId, libraryPath, true); });
    connect(setFolderAsUnreadAction, &QAction::triggered, menu, [this, folderId, libraryPath] { folderManagementCoordinator->setFolderRead(folderId, libraryPath, false); });
    connectTypeActions(typeActions, menu, [this, folderId, libraryPath](YACReader::FileType type) { folderManagementCoordinator->setFolderType(folderId, libraryPath, type); });
    connect(setFolderCoverAction, &QAction::triggered, menu, [this, folderId, libraryPath] { folderManagementCoordinator->selectAndSetCustomCover(folderId, libraryPath); });
    connect(deleteCustomFolderCoverAction, &QAction::triggered, menu, [this, folderId, libraryPath] { folderManagementCoordinator->resetCustomCover(folderId, libraryPath); });

    menu->addSeparator();
    menu->addAction(setFolderCoverAction);
    if (!folder.customImage.isEmpty())
        menu->addAction(deleteCustomFolderCoverAction);

    menu->popup(point);
}

void LibraryWindowMenus::showContinueReadingContextMenu(const QPoint &point, const ComicDB &comic)
{
    QMenu menu;
    auto setAsUnreadAction = new QAction(tr("Set as unread"), &menu);
    setAsUnreadAction->setIcon(themeProvider().comicsViewToolbar.setAsUnreadIcon);
    menu.addAction(setAsUnreadAction);

    connect(setAsUnreadAction, &QAction::triggered, &menu, [this, comic] { comicManagementCoordinator->setComicUnread(libraryIdProvider(), comic); });
    menu.exec(point);
}

void LibraryWindowMenus::setupAddToSubmenu(QMenu &menu)
{
    menu.addAction(actions.addToFavoritesAction);
    actions.addToMenuAction->setMenu(&menu);

    const auto labels = listsModel->getLabels();
    if (!labels.isEmpty())
        menu.addSeparator();
    for (auto *label : labels) {
        auto action = new QAction(label->getIcon(), label->name(), &menu);
        menu.addAction(action);

        const auto labelId = label->getId();
        connect(action, &QAction::triggered, comicManagementCoordinator, [coordinator = comicManagementCoordinator, labelId] { coordinator->addSelectedComicsToLabel(labelId); });
    }
}

void LibraryWindowMenus::showFoldersContextMenu(const QPoint &point)
{
    const auto sourceIndex = foldersModelProxy->mapToSource(foldersView->indexAt(point));
    if (!sourceIndex.isValid())
        return;

    const auto folder = foldersModel->getFolder(sourceIndex);
    const TypeActions typeActions {
        actions.setFolderAsNormalAction,
        actions.setFolderAsMangaAction,
        actions.setFolderAsWesternMangaAction,
        actions.setFolderAsWebComicAction,
        actions.setFolderAsYonkomaAction
    };
    const QList<QAction *> checkableActions { typeActions.comic, typeActions.manga, typeActions.westernManga, typeActions.webComic, typeActions.yonkoma };
    for (auto *action : checkableActions)
        action->setCheckable(true);
    setCheckedType(typeActions, folder.type);

    QMenu menu;
    menu.addAction(actions.openContainingFolderAction);
    menu.addAction(actions.renameFolderAction);
    menu.addAction(actions.updateFolderAction);
    if (YACReader::FeatureFlags::organizeFiles) {
        menu.addSeparator();
        menu.addAction(actions.renameFilesAction);
        menu.addAction(actions.organizeFilesAction);
    }
    menu.addSeparator();
    menu.addAction(actions.rescanXMLFromCurrentFolderAction);
    menu.addSeparator();
    menu.addAction(folder.completed ? actions.setFolderAsNotCompletedAction : actions.setFolderAsCompletedAction);
    menu.addSeparator();
    menu.addAction(folder.finished ? actions.setFolderAsUnreadAction : actions.setFolderAsReadAction);
    menu.addSeparator();
    auto typeMenu = new QMenu(tr("Set type"), &menu);
    menu.addMenu(typeMenu);
    typeMenu->addActions(checkableActions);
    menu.addSeparator();
    menu.addAction(actions.setFolderCoverAction);
    if (!folder.customImage.isEmpty())
        menu.addAction(actions.deleteCustomFolderCoverAction);

    menu.exec(foldersView->mapToGlobal(point));
}
