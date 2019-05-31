#include "yacreader_navigation_controller.h"

#include <QModelIndex>

#include "library_window.h"
#include "yacreader_folders_view.h"
#include "yacreader_reading_lists_view.h"
#include "folder_item.h"
#include "yacreader_history_controller.h"
#include "comic_model.h"
#include "folder_model.h"
#include "reading_list_model.h"
#include "comics_view.h"
#include "empty_folder_widget.h"
#include "yacreader_search_line_edit.h"
#include "yacreader_global.h"
#include "empty_label_widget.h"
#include "empty_special_list.h"
#include "yacreader_comics_views_manager.h"

#include "QsLog.h"

YACReaderNavigationController::YACReaderNavigationController(LibraryWindow *parent, YACReaderComicsViewsManager *comicsViewsManager)
    : QObject(parent), libraryWindow(parent), comicsViewsManager(comicsViewsManager)
{
    setupConnections();
}

void YACReaderNavigationController::selectedFolder(const QModelIndex &mi)
{
    //A proxy is used
    QModelIndex modelIndex = libraryWindow->foldersModelProxy->mapToSource(mi);

    //update history
    libraryWindow->historyController->updateHistory(YACReaderLibrarySourceContainer(modelIndex, YACReaderLibrarySourceContainer::Folder));

    if (libraryWindow->status == LibraryWindow::Searching) {
        //when a folder is selected the search mode has to be reset
        libraryWindow->searchEdit->clearText();
        libraryWindow->clearSearchFilter();
        libraryWindow->foldersView->scrollTo(mi, QAbstractItemView::PositionAtTop);
        libraryWindow->foldersView->setCurrentIndex(mi);
    }

    loadFolderInfo(modelIndex);

    libraryWindow->setToolbarTitle(modelIndex);
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
    libraryWindow->comicsModel->setupFolderModelData(folderId, libraryWindow->foldersModel->getDatabase());

    //configure views
    if (libraryWindow->comicsModel->rowCount() > 0) {
        //updateView
        comicsViewsManager->comicsView->setModel(libraryWindow->comicsModel);
        comicsViewsManager->showComicsView();
        libraryWindow->disableComicsActions(false);
    } else {
        //showEmptyFolder
        loadEmptyFolderInfo(modelIndex);
        comicsViewsManager->showEmptyFolderView();
        libraryWindow->disableComicsActions(true);
    }

    //libraryWindow->updateFoldersViewConextMenu(modelIndex);

    //if a folder is selected, listsView selection must be cleared
    libraryWindow->listsView->clearSelection();
}

void YACReaderNavigationController::loadListInfo(const QModelIndex &modelIndex)
{
    switch (modelIndex.data(ReadingListModel::TypeListsRole).toInt()) {
    case ReadingListModel::SpecialList:
        loadSpecialListInfo(modelIndex);
        break;

    case ReadingListModel::Label:
        loadLabelInfo(modelIndex);
        break;

    case ReadingListModel::ReadingList:
        loadReadingListInfo(modelIndex);
        break;
    }

    //if a list is selected, foldersView selection must be cleared
    libraryWindow->foldersView->clearSelection();
}

void YACReaderNavigationController::loadSpecialListInfo(const QModelIndex &modelIndex)
{
    ReadingListModel::TypeSpecialList type = (ReadingListModel::TypeSpecialList)modelIndex.data(ReadingListModel::SpecialListTypeRole).toInt();

    switch (type) {
    case ReadingListModel::Favorites:
        libraryWindow->comicsModel->setupFavoritesModelData(libraryWindow->foldersModel->getDatabase());
        break;
    case ReadingListModel::Reading:
        libraryWindow->comicsModel->setupReadingModelData(libraryWindow->foldersModel->getDatabase());
        break;
    }

    comicsViewsManager->comicsView->setModel(libraryWindow->comicsModel);

    if (libraryWindow->comicsModel->rowCount() > 0) {
        comicsViewsManager->showComicsView();
        libraryWindow->disableComicsActions(false);
    } else {
        //setup empty special list widget
        switch (type) {
        case ReadingListModel::Favorites:
            comicsViewsManager->emptySpecialList->setPixmap(QPixmap(":/images/empty_favorites.png"));
            comicsViewsManager->emptySpecialList->setText(tr("No favorites"));
            break;
        case ReadingListModel::Reading:
            comicsViewsManager->emptySpecialList->setPixmap(QPixmap(":/images/empty_current_readings.png"));
            comicsViewsManager->emptySpecialList->setText(tr("You are not reading anything yet, come on!!"));
            break;
        }

        comicsViewsManager->showEmptySpecialList();
        libraryWindow->disableComicsActions(true);
    }
}

void YACReaderNavigationController::loadLabelInfo(const QModelIndex &modelIndex)
{
    qulonglong id = modelIndex.data(ReadingListModel::IDRole).toULongLong();
    //check comics in label with id = id
    libraryWindow->comicsModel->setupLabelModelData(id, libraryWindow->foldersModel->getDatabase());
    comicsViewsManager->comicsView->setModel(libraryWindow->comicsModel);

    //configure views
    if (libraryWindow->comicsModel->rowCount() > 0) {
        //updateView
        comicsViewsManager->showComicsView();
        libraryWindow->disableComicsActions(false);
    } else {
        //showEmptyFolder
        //loadEmptyLabelInfo(); //there is no info in an empty label by now, TODO design something
        comicsViewsManager->emptyLabelWidget->setColor((YACReader::LabelColors)modelIndex.data(ReadingListModel::LabelColorRole).toInt());
        comicsViewsManager->showEmptyLabelView();
        libraryWindow->disableComicsActions(true);
    }
}

void YACReaderNavigationController::loadReadingListInfo(const QModelIndex &modelIndex)
{
    qulonglong id = modelIndex.data(ReadingListModel::IDRole).toULongLong();
    //check comics in label with id = id
    libraryWindow->comicsModel->setupReadingListModelData(id, libraryWindow->foldersModel->getDatabase());
    comicsViewsManager->comicsView->setModel(libraryWindow->comicsModel);

    //configure views
    if (libraryWindow->comicsModel->rowCount() > 0) {
        //updateView
        comicsViewsManager->showComicsView();
        libraryWindow->disableComicsActions(false);
    } else {
        comicsViewsManager->showEmptyReadingListWidget();
        libraryWindow->disableComicsActions(true);
    }
}

void YACReaderNavigationController::selectedList(const QModelIndex &mi)
{
    //A proxy is used
    QModelIndex modelIndex = libraryWindow->listsModelProxy->mapToSource(mi);

    //update history
    libraryWindow->historyController->updateHistory(YACReaderLibrarySourceContainer(modelIndex, YACReaderLibrarySourceContainer::List));

    if (libraryWindow->status == LibraryWindow::Searching) {
        //when a list is selected the search mode has to be reset
        libraryWindow->searchEdit->clearText();
        libraryWindow->clearSearchFilter();
        libraryWindow->listsView->scrollTo(mi, QAbstractItemView::PositionAtTop);
        libraryWindow->listsView->setCurrentIndex(mi);
    }

    loadListInfo(modelIndex);

    libraryWindow->setToolbarTitle(modelIndex);
}

void YACReaderNavigationController::reselectCurrentList()
{
    selectedList(libraryWindow->listsView->currentIndex());
}

void YACReaderNavigationController::reselectCurrentSource()
{
    if (!libraryWindow->listsView->selectionModel()->selectedRows().isEmpty()) {
        reselectCurrentList();
    } else {
        reselectCurrentFolder();
    }
}

void YACReaderNavigationController::selectedIndexFromHistory(const YACReaderLibrarySourceContainer &sourceContainer)
{
    //TODO NO searching allowed, just disable backward/forward actions in searching mode
    if (libraryWindow->status == LibraryWindow::Searching) {
        //when a folder is selected the search mode has to be reset
        libraryWindow->searchEdit->clearText();
        libraryWindow->clearSearchFilter();
    }

    loadIndexFromHistory(sourceContainer);
}

void YACReaderNavigationController::loadIndexFromHistory(const YACReaderLibrarySourceContainer &sourceContainer)
{
    QModelIndex sourceMI = sourceContainer.getSourceModelIndex();
    switch (sourceContainer.getType()) {
    case YACReaderLibrarySourceContainer::Folder: {
        QModelIndex mi = libraryWindow->foldersModelProxy->mapFromSource(sourceMI);
        libraryWindow->foldersView->scrollTo(mi, QAbstractItemView::PositionAtTop);
        libraryWindow->foldersView->setCurrentIndex(mi);
        loadFolderInfo(sourceMI);
        break;
    }
    case YACReaderLibrarySourceContainer::List: {
        QModelIndex mi = libraryWindow->listsModelProxy->mapFromSource(sourceMI);
        libraryWindow->listsView->scrollTo(mi, QAbstractItemView::PositionAtTop);
        libraryWindow->listsView->setCurrentIndex(mi);
        loadListInfo(sourceMI);
        break;
    }
    case YACReaderLibrarySourceContainer::None:
        break;
    }
}

void YACReaderNavigationController::selectSubfolder(const QModelIndex &sourceMIParent, int child)
{
    QModelIndex dest = libraryWindow->foldersModel->index(child, 0, sourceMIParent);
    libraryWindow->foldersView->setCurrentIndex(libraryWindow->foldersModelProxy->mapFromSource(dest));
    libraryWindow->historyController->updateHistory(YACReaderLibrarySourceContainer(dest, YACReaderLibrarySourceContainer::Folder));
    loadFolderInfo(dest);
}

void YACReaderNavigationController::loadEmptyFolderInfo(const QModelIndex &modelIndex)
{
    QStringList subfolders;
    subfolders = libraryWindow->foldersModel->getSubfoldersNames(modelIndex);
    comicsViewsManager->emptyFolderWidget->setSubfolders(modelIndex, subfolders);
}

void YACReaderNavigationController::loadPreviousStatus()
{
    YACReaderLibrarySourceContainer sourceContainer = libraryWindow->historyController->currentSourceContainer();
    loadIndexFromHistory(sourceContainer);
}

void YACReaderNavigationController::setupConnections()
{
    connect(libraryWindow->foldersView, SIGNAL(clicked(QModelIndex)), this, SLOT(selectedFolder(QModelIndex)));
    connect(libraryWindow->listsView, SIGNAL(clicked(QModelIndex)), this, SLOT(selectedList(QModelIndex)));
    connect(libraryWindow->historyController, SIGNAL(modelIndexSelected(YACReaderLibrarySourceContainer)), this, SLOT(selectedIndexFromHistory(YACReaderLibrarySourceContainer)));
    connect(comicsViewsManager->emptyFolderWidget, SIGNAL(subfolderSelected(QModelIndex, int)), this, SLOT(selectSubfolder(QModelIndex, int)));
    connect(libraryWindow->comicsModel, SIGNAL(isEmpty()), this, SLOT(reselectCurrentSource()));
}

qulonglong YACReaderNavigationController::folderModelIndexToID(const QModelIndex &mi)
{
    if (!mi.isValid())
        return 1;

    auto folderItem = static_cast<FolderItem *>(mi.internalPointer());
    if (folderItem != 0)
        return folderItem->id;

    return 1;
}
