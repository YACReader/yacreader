#include "yacreader_content_views_manager.h"

#include "classic_comics_view.h"
#include "comic_management_coordinator.h"
#include "comics_view_transition.h"
#include "empty_folder_widget.h"
#include "empty_label_widget.h"
#include "empty_reading_list_widget.h"
#include "empty_special_list.h"
#include "grid_comics_view.h"
#include "info_comics_view.h"
#include "library_window.h"
#include "library_window_menus.h"
#include "no_search_results_widget.h"
#include "options_dialog.h"
#include "yacreader_options_dialog.h"
#include "yacreader_sidebar.h"

#include <utility>

YACReaderContentViewsManager::YACReaderContentViewsManager(QSettings *settings, LibraryWindow *parent)
    : QObject(parent), libraryWindow(parent), classicComicsView(nullptr), gridComicsView(nullptr), infoComicsView(nullptr), toolbarOwner(nullptr), comicManagementCoordinator(nullptr), libraryWindowMenus(nullptr)
{
    comicsViewStack = new QStackedWidget();
    gridComicsView = new GridComicsView();

    switch ((YACReader::ComicsViewStatus)settings->value(COMICS_VIEW_STATUS).toInt()) {
    case Flow:
        comicsView = classicComicsView = new ClassicComicsView();
        comicsViewStatus = Flow;
        break;

    case Info:
        comicsView = infoComicsView = new InfoComicsView();
        comicsViewStatus = Info;
        break;

    case Grid:
    default:
        comicsView = gridComicsView;
        comicsViewStatus = Grid;
        break;
    }

    connectComicsViewConnections(comicsView);
    toolbarOwner = comicsView;
    connect(gridComicsView, &GridComicsView::comicSelectionStateChanged, this, [this](bool hasSelection) {
        if (comicsViewStack->currentWidget() == gridComicsView)
            libraryWindow->actions.setComicSelectionActionsEnabled(hasSelection);
    });
    connect(libraryWindow->optionsDialog, &YACReaderOptionsDialog::optionsChanged, gridComicsView, &GridComicsView::updateSettings);

    comicsViewStack->addWidget(comicsViewTransition = new ComicsViewTransition());
    comicsViewStack->addWidget(emptyLabelWidget = new EmptyLabelWidget());
    comicsViewStack->addWidget(emptySpecialList = new EmptySpecialListWidget());
    comicsViewStack->addWidget(emptyReadingList = new EmptyReadingListWidget());
    comicsViewStack->addWidget(emptyFolderWidget = new EmptyFolderWidget());
    comicsViewStack->addWidget(noSearchResultsWidget = new NoSearchResultsWidget());

    ensureInStack(comicsView);
    ensureInStack(gridComicsView);

    comicsViewStack->setCurrentWidget(comicsView);

    initTheme(this);
}

void YACReaderContentViewsManager::setComicManagementCoordinator(ComicManagementCoordinator *coordinator)
{
    if (comicManagementCoordinator == coordinator)
        return;

    if (comicManagementCoordinator != nullptr) {
        disconnect(comicsView, &ComicsView::selected, comicManagementCoordinator, &ComicManagementCoordinator::openCurrentComic);
        disconnect(comicsView, &ComicsView::openComic, comicManagementCoordinator, &ComicManagementCoordinator::openComic);
        disconnect(comicsView, &ComicsView::copyComicsToCurrentFolder, comicManagementCoordinator, &ComicManagementCoordinator::copyAndImportComicsToCurrentFolder);
        disconnect(comicsView, &ComicsView::moveComicsToCurrentFolder, comicManagementCoordinator, &ComicManagementCoordinator::moveAndImportComicsToCurrentFolder);
        disconnect(comicsView, &ComicsView::customComicCoverRequested, comicManagementCoordinator, &ComicManagementCoordinator::setCustomCover);
    }

    comicManagementCoordinator = coordinator;
    if (comicManagementCoordinator != nullptr) {
        connect(comicsView, &ComicsView::selected, comicManagementCoordinator, &ComicManagementCoordinator::openCurrentComic, Qt::UniqueConnection);
        connect(comicsView, &ComicsView::openComic, comicManagementCoordinator, &ComicManagementCoordinator::openComic, Qt::UniqueConnection);
        connect(comicsView, &ComicsView::copyComicsToCurrentFolder, comicManagementCoordinator, &ComicManagementCoordinator::copyAndImportComicsToCurrentFolder, Qt::UniqueConnection);
        connect(comicsView, &ComicsView::moveComicsToCurrentFolder, comicManagementCoordinator, &ComicManagementCoordinator::moveAndImportComicsToCurrentFolder, Qt::UniqueConnection);
        connect(comicsView, &ComicsView::customComicCoverRequested, comicManagementCoordinator, &ComicManagementCoordinator::setCustomCover, Qt::UniqueConnection);
    }
}

void YACReaderContentViewsManager::setLibraryWindowMenus(LibraryWindowMenus *menus)
{
    if (libraryWindowMenus == menus)
        return;

    if (libraryWindowMenus != nullptr) {
        disconnect(comicsView, &ComicsView::customContextMenuViewRequested, libraryWindowMenus, &LibraryWindowMenus::showComicsViewContextMenu);
        disconnect(comicsView, &ComicsView::customContextMenuItemRequested, libraryWindowMenus, &LibraryWindowMenus::showComicsItemContextMenu);
    }

    libraryWindowMenus = menus;
    if (libraryWindowMenus != nullptr) {
        connect(comicsView, &ComicsView::customContextMenuViewRequested, libraryWindowMenus, &LibraryWindowMenus::showComicsViewContextMenu, Qt::UniqueConnection);
        connect(comicsView, &ComicsView::customContextMenuItemRequested, libraryWindowMenus, &LibraryWindowMenus::showComicsItemContextMenu, Qt::UniqueConnection);
    }
}

QWidget *YACReaderContentViewsManager::containerWidget()
{
    return comicsViewStack;
}

GridComicsView *YACReaderContentViewsManager::gridView() const
{
    return gridComicsView;
}

bool YACReaderContentViewsManager::isComicsViewVisible() const
{
    return comicsViewStack->currentWidget() == comicsView;
}

void YACReaderContentViewsManager::prepareToClose()
{
    const auto saveIfInactive = [this](ComicsView *view) {
        if (view && view != comicsView)
            view->saveViewConfig();
    };

    saveIfInactive(classicComicsView);
    saveIfInactive(gridComicsView);
    saveIfInactive(infoComicsView);

    comicsView->close();
}

ContentViewState YACReaderContentViewsManager::captureViewState() const
{
    const auto *view = qobject_cast<ComicsView *>(comicsViewStack->currentWidget());
    return view ? view->captureViewState() : ContentViewState { };
}

void YACReaderContentViewsManager::restoreViewState(const ContentViewState &state)
{
    if (auto *view = qobject_cast<ComicsView *>(comicsViewStack->currentWidget()))
        view->restoreViewState(state);
}

void YACReaderContentViewsManager::updateCurrentComicView()
{
    if (comicsViewStack->currentWidget() == comicsView) {
        comicsView->updateCurrentComicView();
    }
}

void YACReaderContentViewsManager::toFullscreen()
{
    if (comicsViewStack->currentWidget() == comicsView) {
        comicsView->toFullScreen();
        return;
    }
}

void YACReaderContentViewsManager::toNormal()
{
    if (comicsViewStack->currentWidget() == comicsView) {
        comicsView->toNormal();
        return;
    }
}

void YACReaderContentViewsManager::showComicsView()
{
    setToolBarOwner(comicsView);

    showStackWidget(comicsView, true);

    // TODO: check if this is still needed in the rhi implementation
    // BUG, ugly workaround for glitch when QOpenGLWidget (flow) is used just after any other widget in the views stack
    // Somehow QOpenGLWidget is messing with the rendering of the side bar (wrong buffer swapping)
    libraryWindow->sideBar->update();
}

void YACReaderContentViewsManager::showFoldersOnlyGrid()
{
    setToolBarOwner(gridComicsView);
    connectComicsViewConnections(gridComicsView);
    ensureInStack(gridComicsView);
    showStackWidget(gridComicsView, false);
}

void YACReaderContentViewsManager::showEmptyLabel(YACReader::LabelColors color)
{
    emptyLabelWidget->setColor(color);
    showStackWidget(emptyLabelWidget, true);
}

void YACReaderContentViewsManager::showEmptySpecialList(ReadingListModel::TypeSpecialList type)
{
    switch (type) {
    case ReadingListModel::TypeSpecialList::Favorites:
        emptySpecialList->showFavorites();
        break;
    case ReadingListModel::TypeSpecialList::Reading:
        emptySpecialList->showReading();
        break;
    case ReadingListModel::TypeSpecialList::Recent:
        emptySpecialList->showRecent();
        break;
    }

    showStackWidget(emptySpecialList, true);
}

void YACReaderContentViewsManager::showEmptyReadingList()
{
    showStackWidget(emptyReadingList, true);
}

void YACReaderContentViewsManager::showEmptyFolder()
{
    showStackWidget(emptyFolderWidget, false);
}

void YACReaderContentViewsManager::showNoSearchResults()
{
    showStackWidget(noSearchResultsWidget, true);
}

void YACReaderContentViewsManager::toggleComicsView()
{
    const auto viewState = captureViewState();
    if (comicsViewStack->currentWidget() == comicsView) {
        QTimer::singleShot(0, this, &YACReaderContentViewsManager::showComicsViewTransition);
        QTimer::singleShot(100, this, [this, viewState]() { switchToNextComicsView(viewState); });
    } else {
        switchToNextComicsView(viewState);
    }
}

void YACReaderContentViewsManager::focusComicsViewViaShortcut()
{
    if (auto *currentView = qobject_cast<ComicsView *>(comicsViewStack->currentWidget()))
        currentView->focusComicsNavigation(Qt::ShortcutFocusReason);
}

// PROTECTED

void YACReaderContentViewsManager::disconnectComicsViewConnections(ComicsView *widget)
{
    disconnect(widget, &ComicsView::comicRated, libraryWindow->comicsModel, &ComicModel::updateRating);
    disconnect(libraryWindow->actions.showHideMarksAction, &QAction::toggled, widget, &ComicsView::setShowMarks);
    disconnect(libraryWindow->actions.selectAllComicsAction, &QAction::triggered, widget, &ComicsView::selectAll);
    if (comicManagementCoordinator != nullptr) {
        disconnect(widget, &ComicsView::selected, comicManagementCoordinator, &ComicManagementCoordinator::openCurrentComic);
        disconnect(widget, &ComicsView::openComic, comicManagementCoordinator, &ComicManagementCoordinator::openComic);
        disconnect(widget, &ComicsView::copyComicsToCurrentFolder, comicManagementCoordinator, &ComicManagementCoordinator::copyAndImportComicsToCurrentFolder);
        disconnect(widget, &ComicsView::moveComicsToCurrentFolder, comicManagementCoordinator, &ComicManagementCoordinator::moveAndImportComicsToCurrentFolder);
        disconnect(widget, &ComicsView::customComicCoverRequested, comicManagementCoordinator, &ComicManagementCoordinator::setCustomCover);
    }
    if (libraryWindowMenus != nullptr) {
        disconnect(widget, &ComicsView::customContextMenuViewRequested, libraryWindowMenus, &LibraryWindowMenus::showComicsViewContextMenu);
        disconnect(widget, &ComicsView::customContextMenuItemRequested, libraryWindowMenus, &LibraryWindowMenus::showComicsItemContextMenu);
    }
}

void YACReaderContentViewsManager::connectComicsViewConnections(ComicsView *view)
{
    connect(view, &ComicsView::comicRated, libraryWindow->comicsModel, &ComicModel::updateRating, Qt::UniqueConnection);
    connect(libraryWindow->actions.showHideMarksAction, &QAction::toggled, view, &ComicsView::setShowMarks, Qt::UniqueConnection);

    connect(libraryWindow->actions.selectAllComicsAction, &QAction::triggered, view, &ComicsView::selectAll, Qt::UniqueConnection);

    if (libraryWindowMenus != nullptr) {
        connect(view, &ComicsView::customContextMenuViewRequested, libraryWindowMenus, &LibraryWindowMenus::showComicsViewContextMenu, Qt::UniqueConnection);
        connect(view, &ComicsView::customContextMenuItemRequested, libraryWindowMenus, &LibraryWindowMenus::showComicsItemContextMenu, Qt::UniqueConnection);
    }
    // Drops
    if (comicManagementCoordinator != nullptr) {
        connect(view, &ComicsView::selected, comicManagementCoordinator, &ComicManagementCoordinator::openCurrentComic, Qt::UniqueConnection);
        connect(view, &ComicsView::openComic, comicManagementCoordinator, &ComicManagementCoordinator::openComic, Qt::UniqueConnection);
        connect(view, &ComicsView::copyComicsToCurrentFolder, comicManagementCoordinator, &ComicManagementCoordinator::copyAndImportComicsToCurrentFolder, Qt::UniqueConnection);
        connect(view, &ComicsView::moveComicsToCurrentFolder, comicManagementCoordinator, &ComicManagementCoordinator::moveAndImportComicsToCurrentFolder, Qt::UniqueConnection);
        connect(view, &ComicsView::customComicCoverRequested, comicManagementCoordinator, &ComicManagementCoordinator::setCustomCover, Qt::UniqueConnection);
    }
}

void YACReaderContentViewsManager::switchToComicsView(ComicsView *from, ComicsView *to, const ContentViewState &viewState)
{
    // setup views
    disconnectComicsViewConnections(from);
    from->saveViewConfig();
    from->hide();

    comicsView = to;
    connectComicsViewConnections(comicsView);

    setToolBarOwner(comicsView);

    comicsViewStack->removeWidget(from);
    ensureInStack(comicsView);

    // delete from; No need to delete the previews view, because all views are going to be kept in memory

    // load content into current view
    libraryWindow->loadCoversFromCurrentModel();

    if (!libraryWindow->searchText().isEmpty()) {
        comicsView->enableFilterMode(true);
    }

    to->restoreViewState(viewState);
    updateComicActionsForCurrentView();
}

void YACReaderContentViewsManager::ensureInStack(ComicsView *view)
{
    if (comicsViewStack->indexOf(view) < 0)
        comicsViewStack->addWidget(view);
}

void YACReaderContentViewsManager::showStackWidget(QWidget *widget, bool viewSelectorEnabled)
{
    // showFoldersOnlyGrid() lends the comics view connections to gridComicsView while
    // another view mode owns comicsView. Take them back as soon as the grid stops
    // being shown, otherwise it keeps reacting to comic actions while hidden.
    if (widget != gridComicsView && comicsView != gridComicsView)
        disconnectComicsViewConnections(gridComicsView);

    comicsViewStack->setCurrentWidget(widget);
    setViewSelectorEnabled(viewSelectorEnabled);
}

void YACReaderContentViewsManager::updateComicActionsForCurrentView()
{
    if (libraryWindow->comicsModel == nullptr)
        return;

    libraryWindow->setComicActionsDisabled(libraryWindow->comicsModel->rowCount() == 0);

    // Only the grid tracks a live comic selection, and it can have a folder focused
    // instead of a comic. Every other view keeps the comic actions available as long
    // as the current content has comics.
    if (comicsView == gridComicsView)
        libraryWindow->actions.setComicSelectionActionsEnabled(gridComicsView->hasComicSelection());
}

void YACReaderContentViewsManager::setToolBarOwner(ComicsView *view)
{
    if (!view || toolbarOwner == view)
        return;

    if (toolbarOwner)
        toolbarOwner->releaseToolBar();

    view->setToolBar(libraryWindow->editInfoToolBar);
    toolbarOwner = view;
}

void YACReaderContentViewsManager::setViewSelectorEnabled(bool enabled)
{
    libraryWindow->actions.toggleComicsViewAction->setEnabled(enabled);
}

void YACReaderContentViewsManager::updateViewSelectorIcon(const Theme &theme)
{
    const auto &mainToolbar = theme.mainToolbar;

    QIcon icon;
    switch (comicsViewStatus) {
    case Flow:
        icon = mainToolbar.gridIcon;
        break;
    case Grid:
        icon = mainToolbar.infoIcon;
        break;
    case Info:
        icon = mainToolbar.flowIcon;
        break;
    }

    libraryWindow->actions.toggleComicsViewAction->setIcon(icon);
#ifdef Y_MAC_UI
    libraryWindow->libraryToolBar->updateViewSelectorIcon(icon);
#endif
}

void YACReaderContentViewsManager::showComicsViewTransition()
{
    comicsViewStack->setCurrentWidget(comicsViewTransition);
}

void YACReaderContentViewsManager::switchToNextComicsView(const ContentViewState &viewState)
{
    switch (comicsViewStatus) {
    case Flow: {
        switchToComicsView(classicComicsView, gridComicsView, viewState);
        comicsViewStatus = Grid;

        break;
    }

    case Grid: {
        if (infoComicsView == nullptr)
            infoComicsView = new InfoComicsView();

        switchToComicsView(gridComicsView, infoComicsView, viewState);
        comicsViewStatus = Info;

        break;
    }

    case Info: {
        if (classicComicsView == nullptr)
            classicComicsView = new ClassicComicsView();

        switchToComicsView(infoComicsView, classicComicsView, viewState);
        comicsViewStatus = Flow;

        break;
    }
    }

    updateViewSelectorIcon(theme);
    libraryWindow->settings->setValue(COMICS_VIEW_STATUS, comicsViewStatus);

    if (comicsViewStack->currentWidget() == comicsViewTransition)
        showComicsView();
}

void YACReaderContentViewsManager::applyTheme(const Theme &theme)
{
    updateViewSelectorIcon(theme);
}
