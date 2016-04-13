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

//--
#include "yacreader_search_line_edit.h"
#include "options_dialog.h"

YACReaderComicsViewsManager::YACReaderComicsViewsManager(QSettings *settings, LibraryWindow *parent)
    : QObject(parent), libraryWindow(parent)
{
    comicsViewStack = new QStackedWidget();

    switch ((YACReader::ComicsViewStatus)settings->value(COMICS_VIEW_STATUS).toInt())
    {
    case Flow:
        comicsView = classicComicsView = new ClassicComicsView();
        comicsViewStatus = Flow;
        break;

    case Grid:
        comicsView = gridComicsView = new GridComicsView();
        connect(libraryWindow->optionsDialog, SIGNAL(optionsChanged()), gridComicsView, SLOT(updateBackgroundConfig()));
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

    //connections

    connect(emptyFolderWidget, SIGNAL(copyComicsToCurrentFolder(QList<QPair<QString, QString> >)), libraryWindow, SLOT(copyAndImportComicsToCurrentFolder(QList<QPair<QString, QString> >)));
    connect(emptyFolderWidget, SIGNAL(moveComicsToCurrentFolder(QList<QPair<QString, QString> >)), libraryWindow, SLOT(moveAndImportComicsToCurrentFolder(QList<QPair<QString, QString> >)));
}

QWidget * YACReaderComicsViewsManager::containerWidget()
{
    return comicsViewStack;
}

void YACReaderComicsViewsManager::showComicsView()
{
    comicsViewStack->setCurrentWidget(comicsView);
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

//TODO recover the current comics selection and restore it in the destination
void YACReaderComicsViewsManager::toggleComicsView()
{
    if(comicsViewStack->currentWidget()==comicsView) {
        QTimer::singleShot(0,this,SLOT(showComicsViewTransition()));
        QTimer::singleShot(100,this,SLOT(_toggleComicsView()));
    } else
    {
        _toggleComicsView();
    }
}

//PROTECTED

void YACReaderComicsViewsManager::disconnectComicsViewConnections(ComicsView * widget)
{
    disconnect(widget, SIGNAL(comicRated(int,QModelIndex)), libraryWindow->comicsModel, SLOT(updateRating(int,QModelIndex)));
    disconnect(libraryWindow->showHideMarksAction,SIGNAL(toggled(bool)),widget,SLOT(setShowMarks(bool)));
    disconnect(widget,SIGNAL(selected(unsigned int)),libraryWindow,SLOT(openComic()));
    disconnect(widget,SIGNAL(doubleClicked(QModelIndex)),libraryWindow,SLOT(openComic()));
    disconnect(libraryWindow->selectAllComicsAction,SIGNAL(triggered()),widget,SLOT(selectAll()));
    disconnect(comicsView, SIGNAL(copyComicsToCurrentFolder(QList<QPair<QString, QString> >)), libraryWindow, SLOT(copyAndImportComicsToCurrentFolder(QList<QPair<QString, QString> >)));
    disconnect(comicsView, SIGNAL(moveComicsToCurrentFolder(QList<QPair<QString, QString> >)), libraryWindow, SLOT(moveAndImportComicsToCurrentFolder(QList<QPair<QString, QString> >)));
    disconnect(comicsView,SIGNAL(customContextMenuViewRequested(QPoint)),libraryWindow,SLOT(showComicsViewContextMenu(QPoint)));
    disconnect(comicsView,SIGNAL(customContextMenuItemRequested(QPoint)),libraryWindow,SLOT(showComicsItemContextMenu(QPoint)));
}

void YACReaderComicsViewsManager::doComicsViewConnections()
{
    connect(comicsView, SIGNAL(comicRated(int,QModelIndex)), libraryWindow->comicsModel, SLOT(updateRating(int,QModelIndex)));
    connect(libraryWindow->showHideMarksAction,SIGNAL(toggled(bool)),comicsView,SLOT(setShowMarks(bool)));
    connect(comicsView,SIGNAL(selected(unsigned int)),libraryWindow,SLOT(openComic()));
    connect(comicsView,SIGNAL(doubleClicked(QModelIndex)),libraryWindow,SLOT(openComic()));
    connect(libraryWindow->selectAllComicsAction,SIGNAL(triggered()),comicsView,SLOT(selectAll()));

    connect(comicsView,SIGNAL(customContextMenuViewRequested(QPoint)),libraryWindow,SLOT(showComicsViewContextMenu(QPoint)));
    connect(comicsView,SIGNAL(customContextMenuItemRequested(QPoint)),libraryWindow,SLOT(showComicsItemContextMenu(QPoint)));
    //Drops
    connect(comicsView, SIGNAL(copyComicsToCurrentFolder(QList<QPair<QString, QString> >)), libraryWindow, SLOT(copyAndImportComicsToCurrentFolder(QList<QPair<QString, QString> >)));
    connect(comicsView, SIGNAL(moveComicsToCurrentFolder(QList<QPair<QString, QString> >)), libraryWindow, SLOT(moveAndImportComicsToCurrentFolder(QList<QPair<QString, QString> >)));
}

void YACReaderComicsViewsManager::switchToComicsView(ComicsView * from, ComicsView * to)
{
    //setup views
    disconnectComicsViewConnections(from);
    from->close();

    comicsView = to;
    doComicsViewConnections();

    comicsView->setToolBar(libraryWindow->editInfoToolBar);

    comicsViewStack->removeWidget(from);
    comicsViewStack->addWidget(comicsView);

    delete from;

    //load content into current view
    libraryWindow->loadCoversFromCurrentModel();

    if(!libraryWindow->searchEdit->text().isEmpty())
    {
       comicsView->enableFilterMode(true);
    }
}

void YACReaderComicsViewsManager::showComicsViewTransition()
{
    comicsViewStack->setCurrentWidget(comicsViewTransition);
}

void YACReaderComicsViewsManager::_toggleComicsView()
{
    switch(comicsViewStatus)
    {
    case Flow:
    {
        QIcon icoViewsButton;
        icoViewsButton.addFile(":/images/main_toolbar/info.png", QSize(), QIcon::Normal);
        libraryWindow->toggleComicsViewAction->setIcon(icoViewsButton);
#ifdef Q_OS_MAC
        libraryWindow->libraryToolBar->updateViewSelectorIcon(icoViewsButton);
#endif
        switchToComicsView(classicComicsView, gridComicsView = new GridComicsView());
        connect(libraryWindow->optionsDialog, SIGNAL(optionsChanged()), gridComicsView, SLOT(updateBackgroundConfig()));
        comicsViewStatus = Grid;

        break;
    }

    case Grid:
    {
        QIcon icoViewsButton;
        icoViewsButton.addFile(":/images/main_toolbar/flow.png", QSize(), QIcon::Normal);
        libraryWindow->toggleComicsViewAction->setIcon(icoViewsButton);
#ifdef Q_OS_MAC
        libraryWindow->libraryToolBar->updateViewSelectorIcon(icoViewsButton);
#endif
        switchToComicsView(gridComicsView, infoComicsView = new InfoComicsView());
        comicsViewStatus = Info;

        break;
    }

    case Info:
    {
        QIcon icoViewsButton;
        icoViewsButton.addFile(":/images/main_toolbar/grid.png", QSize(), QIcon::Normal);
        libraryWindow->toggleComicsViewAction->setIcon(icoViewsButton);
#ifdef Q_OS_MAC
        libraryWindow->libraryToolBar->updateViewSelectorIcon(icoViewsButton);
#endif
        switchToComicsView(infoComicsView, classicComicsView = new ClassicComicsView());
        comicsViewStatus = Flow;

        break;
    }
    }

    libraryWindow->settings->setValue(COMICS_VIEW_STATUS, comicsViewStatus);

    showComicsView();
}
