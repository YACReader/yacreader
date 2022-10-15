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
#include "folder_content_view.h"
#include "yacreader_search_line_edit.h"
#include "yacreader_global.h"
#include "empty_label_widget.h"
#include "empty_special_list.h"
#include "yacreader_content_views_manager.h"

#include "QsLog.h"

YACReaderNavigationController::YACReaderNavigationController(LibraryWindow *parent, YACReaderContentViewsManager *contentViewsManager)
    : QObject(parent), libraryWindow(parent), contentViewsManager(contentViewsManager)
{
    setupConnections();
}

void YACReaderNavigationController::selectedFolder(const QModelIndex &mi)
{
    // A proxy is used
    QModelIndex modelIndex = libraryWindow->foldersModelProxy->mapToSource(mi);

    // update history
    libraryWindow->historyController->updateHistory(YACReaderLibrarySourceContainer(modelIndex, YACReaderLibrarySourceContainer::Folder));

    // when a folder is selected the search mode has to be reset
    if (libraryWindow->exitSearchMode()) {
        libraryWindow->foldersView->scrollTo(modelIndex, QAbstractItemView::PositionAtTop);
        libraryWindow->foldersView->setCurrentIndex(modelIndex);
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
    // Get FolderItem
    qulonglong folderId = folderModelIndexToID(modelIndex);

    // check comics in folder with id = folderId
    libraryWindow->comicsModel->setupFolderModelData(folderId, libraryWindow->foldersModel->getDatabase());

    // configure views
    if (libraryWindow->comicsModel->rowCount() > 0) {
        // updateView
        contentViewsManager->comicsView->setModel(libraryWindow->comicsModel);
        contentViewsManager->showComicsView();
        libraryWindow->disableComicsActions(false);
    } else {
        // showEmptyFolder
        loadEmptyFolderInfo(modelIndex);
        contentViewsManager->showFolderContentView();
        libraryWindow->disableComicsActions(true);
    }

    // libraryWindow->updateFoldersViewConextMenu(modelIndex);

    // if a folder is selected, listsView selection must be cleared
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

    // if a list is selected, foldersView selection must be cleared
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

    contentViewsManager->comicsView->setModel(libraryWindow->comicsModel);

    if (libraryWindow->comicsModel->rowCount() > 0) {
        contentViewsManager->showComicsView();
        libraryWindow->disableComicsActions(false);
    } else {
        // setup empty special list widget
        switch (type) {
        case ReadingListModel::Favorites:
            contentViewsManager->emptySpecialList->setPixmap(QPixmap(":/images/empty_favorites.png"));
            contentViewsManager->emptySpecialList->setText(tr("No favorites"));
            break;
        case ReadingListModel::Reading:
            contentViewsManager->emptySpecialList->setPixmap(QPixmap(":/images/empty_current_readings.png"));
            contentViewsManager->emptySpecialList->setText(tr("You are not reading anything yet, come on!!"));
            break;
        }

        contentViewsManager->showEmptySpecialList();
        libraryWindow->disableComicsActions(true);
    }
}

void YACReaderNavigationController::loadLabelInfo(const QModelIndex &modelIndex)
{
    qulonglong id = modelIndex.data(ReadingListModel::IDRole).toULongLong();
    // check comics in label with id = id
    libraryWindow->comicsModel->setupLabelModelData(id, libraryWindow->foldersModel->getDatabase());
    contentViewsManager->comicsView->setModel(libraryWindow->comicsModel);

    // configure views
    if (libraryWindow->comicsModel->rowCount() > 0) {
        // updateView
        contentViewsManager->showComicsView();
        libraryWindow->disableComicsActions(false);
    } else {
        // showEmptyFolder
        // loadEmptyLabelInfo(); //there is no info in an empty label by now, TODO design something
        contentViewsManager->emptyLabelWidget->setColor((YACReader::LabelColors)modelIndex.data(ReadingListModel::LabelColorRole).toInt());
        contentViewsManager->showEmptyLabelView();
        libraryWindow->disableComicsActions(true);
    }
}

void YACReaderNavigationController::loadReadingListInfo(const QModelIndex &modelIndex)
{
    qulonglong id = modelIndex.data(ReadingListModel::IDRole).toULongLong();
    // check comics in label with id = id
    libraryWindow->comicsModel->setupReadingListModelData(id, libraryWindow->foldersModel->getDatabase());
    contentViewsManager->comicsView->setModel(libraryWindow->comicsModel);

    // configure views
    if (libraryWindow->comicsModel->rowCount() > 0) {
        // updateView
        contentViewsManager->showComicsView();
        libraryWindow->disableComicsActions(false);
    } else {
        contentViewsManager->showEmptyReadingListWidget();
        libraryWindow->disableComicsActions(true);
    }
}

void YACReaderNavigationController::selectedList(const QModelIndex &mi)
{
    // A proxy is used
    QModelIndex modelIndex = libraryWindow->listsModelProxy->mapToSource(mi);

    // update history
    libraryWindow->historyController->updateHistory(YACReaderLibrarySourceContainer(modelIndex, YACReaderLibrarySourceContainer::List));

    // when a list is selected the search mode has to be reset
    if (libraryWindow->exitSearchMode()) {

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
    // TODO NO searching allowed, just disable backward/forward actions in searching mode
    // when a folder or a list is selected the search mode has to be reset
    libraryWindow->exitSearchMode();
    loadIndexFromHistory(sourceContainer);
    libraryWindow->setToolbarTitle(sourceContainer.getSourceModelIndex());
}

void YACReaderNavigationController::loadIndexFromHistory(const YACReaderLibrarySourceContainer &sourceContainer)
{
    QModelIndex sourceMI = sourceContainer.getSourceModelIndex();
    switch (sourceContainer.getType()) {
    case YACReaderLibrarySourceContainer::Folder: {
        QModelIndex mi = libraryWindow->foldersModelProxy->mapFromSource(sourceMI);
        libraryWindow->foldersView->scrollTo(mi, QAbstractItemView::PositionAtTop);
        // currentIndexChanged is about to be emited, but we don't want it to end in YACReaderHistoryController::updateHistory
        disconnect(libraryWindow->foldersView, &YACReaderTreeView::currentIndexChanged, this, &YACReaderNavigationController::selectedFolder);
        libraryWindow->foldersView->setCurrentIndex(mi);
        connect(libraryWindow->foldersView, &YACReaderTreeView::currentIndexChanged, this, &YACReaderNavigationController::selectedFolder);
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
        QLOG_ERROR() << "Cannot load a source container of type None";
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
    auto readingComicsModel = new ComicModel();

    auto isRoot = !modelIndex.isValid();

    if (isRoot) {
        readingComicsModel->setupReadingModelData(libraryWindow->foldersModel->getDatabase());
    }

    contentViewsManager->folderContentView->setContinueReadingModel(readingComicsModel);

    auto subFolderModel = libraryWindow->foldersModel->getSubfoldersModel(modelIndex);
    contentViewsManager->folderContentView->setModel(modelIndex, subFolderModel);
}

void YACReaderNavigationController::loadPreviousStatus()
{
    YACReaderLibrarySourceContainer sourceContainer = libraryWindow->historyController->currentSourceContainer();
    loadIndexFromHistory(sourceContainer);
}

void YACReaderNavigationController::setupConnections()
{
    // we need YACReaderTreeView::currentIndexChanged to be able to navigate the folders tree using the keyboard cursors
    connect(libraryWindow->foldersView, &YACReaderTreeView::currentIndexChanged, this, &YACReaderNavigationController::selectedFolder);
    connect(libraryWindow->foldersView, &YACReaderTreeView::clicked, this, &YACReaderNavigationController::selectedFolder);
    connect(libraryWindow->listsView, &QAbstractItemView::clicked, this, &YACReaderNavigationController::selectedList);
    connect(libraryWindow->historyController, &YACReaderHistoryController::modelIndexSelected, this, &YACReaderNavigationController::selectedIndexFromHistory);
    connect(contentViewsManager->folderContentView, &FolderContentView::subfolderSelected, this, &YACReaderNavigationController::selectSubfolder);
    connect(contentViewsManager->folderContentView, &FolderContentView::openComic, libraryWindow, QOverload<const ComicDB &, const ComicModel::Mode>::of(&LibraryWindow::openComic));
    connect(contentViewsManager->folderContentView, &FolderContentView::openFolderContextMenu, libraryWindow, &LibraryWindow::showGridFoldersContextMenu);
    connect(contentViewsManager->folderContentView, &FolderContentView::openContinueReadingComicContextMenu, libraryWindow, &LibraryWindow::showContinueReadingContextMenu);
    connect(libraryWindow->comicsModel, &ComicModel::isEmpty, this, &YACReaderNavigationController::reselectCurrentSource);
}

qulonglong YACReaderNavigationController::folderModelIndexToID(const QModelIndex &mi)
{
    if (!mi.isValid())
        return 1;

    auto folderItem = static_cast<FolderItem *>(mi.internalPointer());
    if (folderItem != nullptr)
        return folderItem->id;

    return 1;
}
