#include "yacreader_comics_views_manager.h"

#include "library_window.h"

#include "classic_comics_view.h"
#include "grid_comics_view.h"
#include "info_comics_view.h"
#include "comics_view_transition.h"
#include "empty_folder_widget.h"
#include "empty_label_widget.h"
#include "empty_special_list.h"
#include "empty_reading_list_widget.h"
#include "no_search_results_widget.h"

#include "yacreader_sidebar.h"

//--
#include "yacreader_search_line_edit.h"
#include "options_dialog.h"

YACReaderComicsViewsManager::YACReaderComicsViewsManager(QSettings *settings, LibraryWindow *parent)
    : QObject(parent), libraryWindow(parent), classicComicsView(nullptr), gridComicsView(nullptr), infoComicsView(nullptr)
{
    comicsViewStack = new QStackedWidget();

    switch ((YACReader::ComicsViewStatus)settings->value(COMICS_VIEW_STATUS).toInt()) {
    case Flow:
        comicsView = classicComicsView = new ClassicComicsView();
        comicsViewStatus = Flow;
        break;

    case Grid:
        comicsView = gridComicsView = new GridComicsView();
        connect(libraryWindow->optionsDialog, &YACReaderOptionsDialog::optionsChanged, gridComicsView, &GridComicsView::updateBackgroundConfig);
        comicsViewStatus = Grid;
        break;

    case Info:
        comicsView = infoComicsView = new InfoComicsView();
        comicsViewStatus = Info;
        break;

    default:
        comicsView = classicComicsView = new ClassicComicsView();
        comicsViewStatus = Flow;
    }

    doComicsViewConnections();

    comicsViewStack->addWidget(comicsViewTransition = new ComicsViewTransition());
    comicsViewStack->addWidget(emptyFolderWidget = new EmptyFolderWidget());
    comicsViewStack->addWidget(emptyLabelWidget = new EmptyLabelWidget());
    comicsViewStack->addWidget(emptySpecialList = new EmptySpecialListWidget());
    comicsViewStack->addWidget(emptyReadingList = new EmptyReadingListWidget());
    comicsViewStack->addWidget(noSearchResultsWidget = new NoSearchResultsWidget());

    comicsViewStack->addWidget(comicsView);

    comicsViewStack->setCurrentWidget(comicsView);

    // connections
    connect(emptyFolderWidget, &EmptyFolderWidget::copyComicsToCurrentFolder, libraryWindow, &LibraryWindow::copyAndImportComicsToCurrentFolder);
    connect(emptyFolderWidget, &EmptyFolderWidget::moveComicsToCurrentFolder, libraryWindow, &LibraryWindow::moveAndImportComicsToCurrentFolder);
}

QWidget *YACReaderComicsViewsManager::containerWidget()
{
    return comicsViewStack;
}

void YACReaderComicsViewsManager::updateCurrentComicView()
{
    if (comicsViewStack->currentWidget() == comicsView) {
        comicsView->updateCurrentComicView();
    }
}

void YACReaderComicsViewsManager::showComicsView()
{
    comicsViewStack->setCurrentWidget(comicsView);

    // BUG, ugly workaround for glitch when QOpenGLWidget (flow) is used just after any other widget in the views stack
    // Somehow QOpenGLWidget is messing with the rendering of the side bar (wrong buffer swapping)
    libraryWindow->sideBar->update();
}

void YACReaderComicsViewsManager::showEmptyFolderView()
{
    comicsViewStack->setCurrentWidget(emptyFolderWidget);
}

void YACReaderComicsViewsManager::showEmptyLabelView()
{
    comicsViewStack->setCurrentWidget(emptyLabelWidget);
}

void YACReaderComicsViewsManager::showEmptySpecialList()
{
    comicsViewStack->setCurrentWidget(emptySpecialList);
}

void YACReaderComicsViewsManager::showEmptyReadingListWidget()
{
    comicsViewStack->setCurrentWidget(emptyReadingList);
}

void YACReaderComicsViewsManager::showNoSearchResultsView()
{
    comicsViewStack->setCurrentWidget(noSearchResultsWidget);
}

// TODO recover the current comics selection and restore it in the destination
void YACReaderComicsViewsManager::toggleComicsView()
{
    if (comicsViewStack->currentWidget() == comicsView) {
        QTimer::singleShot(0, this, &YACReaderComicsViewsManager::showComicsViewTransition);
        QTimer::singleShot(100, this, &YACReaderComicsViewsManager::_toggleComicsView);
    } else {
        _toggleComicsView();
    }
}

void YACReaderComicsViewsManager::focusComicsViewViaShortcut()
{
    comicsView->focusComicsNavigation(Qt::ShortcutFocusReason);
}

// PROTECTED

void YACReaderComicsViewsManager::disconnectComicsViewConnections(ComicsView *widget)
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

void YACReaderComicsViewsManager::doComicsViewConnections()
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

void YACReaderComicsViewsManager::switchToComicsView(ComicsView *from, ComicsView *to)
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

void YACReaderComicsViewsManager::showComicsViewTransition()
{
    comicsViewStack->setCurrentWidget(comicsViewTransition);
}

void YACReaderComicsViewsManager::_toggleComicsView()
{
    switch (comicsViewStatus) {
    case Flow: {
        QIcon icoViewsButton;
        icoViewsButton.addFile(":/images/main_toolbar/info.png", QSize(), QIcon::Normal);
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
        icoViewsButton.addFile(":/images/main_toolbar/flow.png", QSize(), QIcon::Normal);
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
        icoViewsButton.addFile(":/images/main_toolbar/grid.png", QSize(), QIcon::Normal);
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
