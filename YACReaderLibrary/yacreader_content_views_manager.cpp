#include "yacreader_content_views_manager.h"

#include "library_window.h"

#include "classic_comics_view.h"
#include "grid_comics_view.h"
#include "info_comics_view.h"
#include "comics_view_transition.h"
#include "folder_content_view.h"
#include "empty_label_widget.h"
#include "empty_special_list.h"
#include "empty_reading_list_widget.h"
#include "no_search_results_widget.h"

#include "yacreader_sidebar.h"

//--
#include "yacreader_search_line_edit.h"
#include "options_dialog.h"

YACReaderContentViewsManager::YACReaderContentViewsManager(QSettings *settings, LibraryWindow *parent)
    : QObject(parent), libraryWindow(parent), classicComicsView(nullptr), gridComicsView(nullptr), infoComicsView(nullptr)
{
    comicsViewStack = new QStackedWidget();

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
        comicsView = gridComicsView = new GridComicsView();
        connect(libraryWindow->optionsDialog, &YACReaderOptionsDialog::optionsChanged, gridComicsView, &GridComicsView::updateBackgroundConfig);
        comicsViewStatus = Grid;
        break;
    }

    doComicsViewConnections();

    comicsViewStack->addWidget(comicsViewTransition = new ComicsViewTransition());
    comicsViewStack->addWidget(folderContentView = new FolderContentView());
    comicsViewStack->addWidget(emptyLabelWidget = new EmptyLabelWidget());
    comicsViewStack->addWidget(emptySpecialList = new EmptySpecialListWidget());
    comicsViewStack->addWidget(emptyReadingList = new EmptyReadingListWidget());
    comicsViewStack->addWidget(noSearchResultsWidget = new NoSearchResultsWidget());

    comicsViewStack->addWidget(comicsView);

    comicsViewStack->setCurrentWidget(comicsView);

    // connections
    connect(folderContentView, &FolderContentView::copyComicsToCurrentFolder, libraryWindow, &LibraryWindow::copyAndImportComicsToCurrentFolder);
    connect(folderContentView, &FolderContentView::moveComicsToCurrentFolder, libraryWindow, &LibraryWindow::moveAndImportComicsToCurrentFolder);
}

QWidget *YACReaderContentViewsManager::containerWidget()
{
    return comicsViewStack;
}

void YACReaderContentViewsManager::updateCurrentComicView()
{
    if (comicsViewStack->currentWidget() == comicsView) {
        comicsView->updateCurrentComicView();
    }
}

void YACReaderContentViewsManager::updateContinueReadingView()
{
    if (comicsViewStack->currentWidget() == folderContentView) {
        folderContentView->reloadContinueReadingModel();
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
    comicsViewStack->setCurrentWidget(comicsView);

    // BUG, ugly workaround for glitch when QOpenGLWidget (flow) is used just after any other widget in the views stack
    // Somehow QOpenGLWidget is messing with the rendering of the side bar (wrong buffer swapping)
    libraryWindow->sideBar->update();
}

void YACReaderContentViewsManager::showFolderContentView()
{
    comicsViewStack->setCurrentWidget(folderContentView);
}

void YACReaderContentViewsManager::showEmptyLabelView()
{
    comicsViewStack->setCurrentWidget(emptyLabelWidget);
}

void YACReaderContentViewsManager::showEmptySpecialList()
{
    comicsViewStack->setCurrentWidget(emptySpecialList);
}

void YACReaderContentViewsManager::showEmptyReadingListWidget()
{
    comicsViewStack->setCurrentWidget(emptyReadingList);
}

void YACReaderContentViewsManager::showNoSearchResultsView()
{
    comicsViewStack->setCurrentWidget(noSearchResultsWidget);
}

// TODO recover the current comics selection and restore it in the destination
void YACReaderContentViewsManager::toggleComicsView()
{
    if (comicsViewStack->currentWidget() == comicsView) {
        QTimer::singleShot(0, this, &YACReaderContentViewsManager::showComicsViewTransition);
        QTimer::singleShot(100, this, &YACReaderContentViewsManager::_toggleComicsView);
    } else {
        _toggleComicsView();
    }
}

void YACReaderContentViewsManager::focusComicsViewViaShortcut()
{
    comicsView->focusComicsNavigation(Qt::ShortcutFocusReason);
}

// PROTECTED

void YACReaderContentViewsManager::disconnectComicsViewConnections(ComicsView *widget)
{
    disconnect(widget, &ComicsView::comicRated, libraryWindow->comicsModel, &ComicModel::updateRating);
    disconnect(libraryWindow->showHideMarksAction, &QAction::toggled, widget, &ComicsView::setShowMarks);
    disconnect(widget, &ComicsView::selected, libraryWindow, QOverload<>::of(&LibraryWindow::openComic));
    disconnect(widget, &ComicsView::openComic, libraryWindow, QOverload<const ComicDB &, const ComicModel::Mode>::of(&LibraryWindow::openComic));
    disconnect(libraryWindow->selectAllComicsAction, &QAction::triggered, widget, &ComicsView::selectAll);
    disconnect(comicsView, &ComicsView::copyComicsToCurrentFolder, libraryWindow, &LibraryWindow::copyAndImportComicsToCurrentFolder);
    disconnect(comicsView, &ComicsView::moveComicsToCurrentFolder, libraryWindow, &LibraryWindow::moveAndImportComicsToCurrentFolder);
    disconnect(comicsView, &ComicsView::customContextMenuViewRequested, libraryWindow, &LibraryWindow::showComicsViewContextMenu);
    disconnect(comicsView, &ComicsView::customContextMenuItemRequested, libraryWindow, &LibraryWindow::showComicsItemContextMenu);
}

void YACReaderContentViewsManager::doComicsViewConnections()
{
    connect(comicsView, &ComicsView::comicRated, libraryWindow->comicsModel, &ComicModel::updateRating);
    connect(libraryWindow->showHideMarksAction, &QAction::toggled, comicsView, &ComicsView::setShowMarks);
    connect(comicsView, &ComicsView::selected, libraryWindow, QOverload<>::of(&LibraryWindow::openComic));
    connect(comicsView, &ComicsView::openComic, libraryWindow, QOverload<const ComicDB &, const ComicModel::Mode>::of(&LibraryWindow::openComic));

    connect(libraryWindow->selectAllComicsAction, &QAction::triggered, comicsView, &ComicsView::selectAll);

    connect(comicsView, &ComicsView::customContextMenuViewRequested, libraryWindow, &LibraryWindow::showComicsViewContextMenu);
    connect(comicsView, &ComicsView::customContextMenuItemRequested, libraryWindow, &LibraryWindow::showComicsItemContextMenu);
    // Drops
    connect(comicsView, &ComicsView::copyComicsToCurrentFolder, libraryWindow, &LibraryWindow::copyAndImportComicsToCurrentFolder);
    connect(comicsView, &ComicsView::moveComicsToCurrentFolder, libraryWindow, &LibraryWindow::moveAndImportComicsToCurrentFolder);
}

void YACReaderContentViewsManager::switchToComicsView(ComicsView *from, ComicsView *to)
{
    // setup views
    disconnectComicsViewConnections(from);
    from->close();

    comicsView = to;
    doComicsViewConnections();

    comicsView->setToolBar(libraryWindow->editInfoToolBar);

    comicsViewStack->removeWidget(from);
    comicsViewStack->addWidget(comicsView);

    // delete from; No need to delete the previews view, because all views are going to be kept in memory

    // load content into current view
    libraryWindow->loadCoversFromCurrentModel();

    if (!libraryWindow->searchEdit->text().isEmpty()) {
        comicsView->enableFilterMode(true);
    }
}

void YACReaderContentViewsManager::showComicsViewTransition()
{
    comicsViewStack->setCurrentWidget(comicsViewTransition);
}

void YACReaderContentViewsManager::_toggleComicsView()
{
    switch (comicsViewStatus) {
    case Flow: {
        QIcon icoViewsButton;
        icoViewsButton.addFile(addExtensionToIconPath(":/images/main_toolbar/info"), QSize(), QIcon::Normal);
        libraryWindow->toggleComicsViewAction->setIcon(icoViewsButton);
#ifdef Q_OS_MAC
        libraryWindow->libraryToolBar->updateViewSelectorIcon(icoViewsButton);
#endif
        if (gridComicsView == nullptr)
            gridComicsView = new GridComicsView();

        switchToComicsView(classicComicsView, gridComicsView);
        connect(libraryWindow->optionsDialog, &YACReaderOptionsDialog::optionsChanged, gridComicsView, &GridComicsView::updateBackgroundConfig);
        comicsViewStatus = Grid;

        break;
    }

    case Grid: {
        QIcon icoViewsButton;
        icoViewsButton.addFile(addExtensionToIconPath(":/images/main_toolbar/flow"), QSize(), QIcon::Normal);
        libraryWindow->toggleComicsViewAction->setIcon(icoViewsButton);
#ifdef Q_OS_MAC
        libraryWindow->libraryToolBar->updateViewSelectorIcon(icoViewsButton);
#endif
        if (infoComicsView == nullptr)
            infoComicsView = new InfoComicsView();

        switchToComicsView(gridComicsView, infoComicsView);
        comicsViewStatus = Info;

        break;
    }

    case Info: {
        QIcon icoViewsButton;
        icoViewsButton.addFile(addExtensionToIconPath(":/images/main_toolbar/grid"), QSize(), QIcon::Normal);
        libraryWindow->toggleComicsViewAction->setIcon(icoViewsButton);
#ifdef Q_OS_MAC
        libraryWindow->libraryToolBar->updateViewSelectorIcon(icoViewsButton);
#endif
        if (classicComicsView == nullptr)
            classicComicsView = new ClassicComicsView();

        switchToComicsView(infoComicsView, classicComicsView);
        comicsViewStatus = Flow;

        break;
    }
    }

    libraryWindow->settings->setValue(COMICS_VIEW_STATUS, comicsViewStatus);

    if (comicsViewStack->currentWidget() == comicsViewTransition)
        showComicsView();
}
