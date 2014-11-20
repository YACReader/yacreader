#include "yacreader_navigation_controller.h"

#include <QModelIndex>

#include "library_window.h"
#include "yacreader_folders_view.h"
#include "yacreader_reading_lists_view.h"
#include "folder_item.h"
#include "yacreader_history_controller.h"
#include "comic_model.h"
#include "folder_model.h"
#include "comics_view.h"
#include "empty_folder_widget.h"
#include "yacreader_search_line_edit.h"

YACReaderNavigationController::YACReaderNavigationController(LibraryWindow *parent) :
    QObject(parent),libraryWindow(parent)
{
    setupConnections();
}

void YACReaderNavigationController::selectedFolder(const QModelIndex &mi)
{
    //A proxy is used
    QModelIndex modelIndex = libraryWindow->foldersModelProxy->mapToSource(mi);

    //update history
    libraryWindow->historyController->updateHistory(modelIndex);

    if(libraryWindow->status == LibraryWindow::Searching)
    {
        //when a folder is selected the search mode has to be reset
        libraryWindow->searchEdit->clearText();
        libraryWindow->clearSearchFilter();
        libraryWindow->foldersView->scrollTo(mi,QAbstractItemView::PositionAtTop);
        libraryWindow->foldersView->setCurrentIndex(mi);
    }

    loadFolderInfo(modelIndex);

}

void YACReaderNavigationController::reselectCurrentFolder()
{
    selectedFolder(libraryWindow->foldersView->currentIndex());
}

void YACReaderNavigationController::loadFolderInfo(const QModelIndex &modelIndex)
{
    //Get FolderItem
    qulonglong folderId = folderModelIndexToID(modelIndex);

    //check comics in folder with id = folderId
    libraryWindow->comicsModel->setupModelData(folderId,libraryWindow->foldersModel->getDatabase());
    libraryWindow->comicsView->setModel(libraryWindow->comicsModel);

    //configure views
    if(libraryWindow->comicsModel->rowCount() > 0)
    {
        //updateView
        libraryWindow->showComicsView();
        libraryWindow->disableComicsActions(false);
    }
    else{
        //showEmptyFolder
        loadEmptyFolderInfo(modelIndex);
        libraryWindow->showEmptyFolderView();
        libraryWindow->disableComicsActions(true);
    }
}

void YACReaderNavigationController::selectedList(const QModelIndex &mi)
{

}

void YACReaderNavigationController::selectedIndexFromHistory(const QModelIndex &sourceMI)
{
    //TODO NO searching allowed, just disable backward/forward actions in searching mode
    if(libraryWindow->status == LibraryWindow::Searching)
    {
        //when a folder is selected the search mode has to be reset
        libraryWindow->searchEdit->clearText();
        libraryWindow->clearSearchFilter();
    }

    //TODO more info about mi is needed (folder, lists...)
    QModelIndex mi = libraryWindow->foldersModelProxy->mapFromSource(sourceMI);
    libraryWindow->foldersView->scrollTo(mi,QAbstractItemView::PositionAtTop);
    libraryWindow->foldersView->setCurrentIndex(mi);
    loadFolderInfo(sourceMI);
}

void YACReaderNavigationController::selectSubfolder(const QModelIndex &sourceMIParent, int child)
{
    QModelIndex dest = libraryWindow->foldersModel->index(child,0,sourceMIParent);
    libraryWindow->foldersView->setCurrentIndex(libraryWindow->foldersModelProxy->mapFromSource(dest));
    libraryWindow->historyController->updateHistory(dest);
    loadFolderInfo(dest);
}

void YACReaderNavigationController::loadEmptyFolderInfo(const QModelIndex &modelIndex)
{
    QStringList subfolders;
    subfolders = libraryWindow->foldersModel->getSubfoldersNames(modelIndex);
    libraryWindow->emptyFolderWidget->setSubfolders(modelIndex,subfolders);
}

void YACReaderNavigationController::loadPreviousStatus()
{
    QModelIndex sourceMI = libraryWindow->historyController->currentIndex();
    QModelIndex mi = libraryWindow->foldersModelProxy->mapFromSource(sourceMI);
    libraryWindow->foldersView->scrollTo(mi,QAbstractItemView::PositionAtTop);
    libraryWindow->foldersView->setCurrentIndex(mi);
    loadFolderInfo(sourceMI);
}

void YACReaderNavigationController::setupConnections()
{
    connect(libraryWindow->foldersView,SIGNAL(clicked(QModelIndex)),this,SLOT(selectedFolder(QModelIndex)));
    connect(libraryWindow->listsView,SIGNAL(clicked(QModelIndex)),this,SLOT(selectedList(QModelIndex)));
    connect(libraryWindow->historyController,SIGNAL(modelIndexSelected(QModelIndex)),this,SLOT(selectedIndexFromHistory(QModelIndex)));
    connect(libraryWindow->emptyFolderWidget,SIGNAL(subfolderSelected(QModelIndex,int)),this,SLOT(selectSubfolder(QModelIndex,int)));
}

qulonglong YACReaderNavigationController::folderModelIndexToID(const QModelIndex &mi)
{
    if(!mi.isValid())
        return 1;

    FolderItem * folderItem = static_cast<FolderItem *>(mi.internalPointer());
    if(folderItem != 0)
        return folderItem->id;

    return 1;
}
