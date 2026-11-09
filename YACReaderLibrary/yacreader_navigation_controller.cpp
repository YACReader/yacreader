#include "yacreader_navigation_controller.h"

#include "QsLog.h"
#include "comic_model.h"
#include "comics_view.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "empty_label_widget.h"
#include "empty_special_list.h"
#include "folder_item.h"
#include "folder_model.h"
#include "grid_comics_view.h"
#include "grid_content_model.h"
#include "library_search_coordinator.h"
#include "library_window.h"
#include "reading_list_model.h"
#include "theme_manager.h"
#include "yacreader_content_views_manager.h"
#include "yacreader_folders_view.h"
#include "yacreader_global.h"
#include "yacreader_history_controller.h"
#include "yacreader_library_list_widget.h"
#include "yacreader_reading_lists_view.h"

#include <QModelIndex>
#include <QSqlDatabase>

#include <memory>

YACReaderNavigationController::YACReaderNavigationController(LibraryWindow *parent, YACReaderContentViewsManager *contentViewsManager, LibrarySearchCoordinator *librarySearchCoordinator)
    : QObject(parent), libraryWindow(parent), contentViewsManager(contentViewsManager), librarySearchCoordinator(librarySearchCoordinator)
{
    setupConnections();
}

void YACReaderNavigationController::selectedFolder(const QModelIndex &proxyIndex)
{
    const QModelIndex folderIndex = libraryWindow->foldersModelProxy->mapToSource(proxyIndex);

    if (!restoringHistorySelection) {
        recordCurrentViewState();
        libraryWindow->historyController->updateHistory(YACReaderLibrarySourceContainer(folderIndex, YACReaderLibrarySourceContainer::Folder));
    }

    // when a folder is selected the search mode has to be reset
    if (librarySearchCoordinator->exitSearchMode()) {
        libraryWindow->foldersView->scrollTo(folderIndex, QAbstractItemView::PositionAtTop);
        libraryWindow->foldersView->setCurrentIndex(folderIndex);
    }

    loadFolderContent(folderIndex);

    libraryWindow->setToolbarTitle(folderIndex);
}

void YACReaderNavigationController::navigateToFolder(const QModelIndex &sourceIndex)
{
    if (!sourceIndex.isValid())
        return;

    const QModelIndex proxyIndex = libraryWindow->foldersModelProxy->mapFromSource(sourceIndex);
    if (!proxyIndex.isValid())
        return;

    disconnect(libraryWindow->foldersView, &YACReaderTreeView::currentIndexChanged, this, &YACReaderNavigationController::selectedFolder);
    libraryWindow->foldersView->setCurrentIndex(proxyIndex);
    connect(libraryWindow->foldersView, &YACReaderTreeView::currentIndexChanged, this, &YACReaderNavigationController::selectedFolder);

    selectedFolder(proxyIndex);
}

void YACReaderNavigationController::reselectCurrentFolder()
{
    selectedFolder(libraryWindow->foldersView->currentIndex());
}

void YACReaderNavigationController::loadFolderContent(const QModelIndex &folderIndex)
{
    loadedFolder = folderIndex;

    const qulonglong folderId = folderIdForIndex(folderIndex);
    const bool isRoot = folderId == FolderModel::RootFolderId;

    libraryWindow->comicsModel->setupFolderModelData(folderId, libraryWindow->foldersModel->getDatabase());

    if (isRoot) {
        loadRootContinueReading();
    } else {
        contentViewsManager->gridView()->clearRootContinueReadingModel();
    }

    const auto libraryName = libraryWindow->selectedLibrary->currentText();
    const auto libraryInfo = isRoot ? DBHelper::getLibraryInfoData(libraryWindow->libraries.getUuid(libraryName)) : QVariantMap();
    contentViewsManager->gridView()->setFolderModel(libraryWindow->foldersModel, folderIndex, libraryName, libraryInfo);

    if (libraryWindow->comicsModel->rowCount() > 0) {
        contentViewsManager->comicsView->setModel(libraryWindow->comicsModel);
        contentViewsManager->showComicsView();
        libraryWindow->setComicActionsDisabled(false);
    } else if (libraryWindow->foldersModel->rowCount(folderIndex) > 0) {
        // Folder has subfolders, so show the unified content grid.
        contentViewsManager->gridView()->setModel(libraryWindow->comicsModel);
        contentViewsManager->showFoldersOnlyGrid();
        libraryWindow->setComicActionsDisabled(true);
    } else {
        contentViewsManager->showEmptyFolder();
        libraryWindow->setComicActionsDisabled(true);
    }
    // if a folder is selected, listsView selection must be cleared
    libraryWindow->listsView->clearSelection();
}

void YACReaderNavigationController::loadListContent(const QModelIndex &listIndex)
{
    contentViewsManager->gridView()->clearFolderModel();
    switch (listIndex.data(ReadingListModel::TypeListsRole).toInt()) {
    case ReadingListModel::SpecialList:
        loadSpecialListContent(listIndex);
        break;

    case ReadingListModel::Label:
        loadLabelContent(listIndex);
        break;

    case ReadingListModel::ReadingList:
        loadReadingListContent(listIndex);
        break;

    case ReadingListModel::SmartList:
        loadSmartListContent(listIndex);
        break;

    case ReadingListModel::ReadingListFolder:
    case ReadingListModel::SmartListFolder:
        loadListFolderContent(listIndex);
        break;
    }
    contentViewsManager->gridView()->setCurrentList(listIndex);
    // if a list is selected, foldersView selection must be cleared
    libraryWindow->foldersView->clearSelection();
}

void YACReaderNavigationController::loadSpecialListContent(const QModelIndex &listIndex)
{
    const auto type = static_cast<ReadingListModel::TypeSpecialList>(listIndex.data(ReadingListModel::SpecialListTypeRole).toInt());

    switch (type) {
    case ReadingListModel::TypeSpecialList::Favorites:
        libraryWindow->comicsModel->setupFavoritesModelData(libraryWindow->foldersModel->getDatabase());
        break;
    case ReadingListModel::TypeSpecialList::Reading:
        libraryWindow->comicsModel->setupReadingModelData(libraryWindow->foldersModel->getDatabase());
        break;
    case ReadingListModel::TypeSpecialList::Recent:
        libraryWindow->comicsModel->setupRecentModelData(libraryWindow->foldersModel->getDatabase());
        break;
    }

    contentViewsManager->comicsView->setModel(libraryWindow->comicsModel);

    if (libraryWindow->comicsModel->rowCount() > 0) {
        contentViewsManager->showComicsView();
        libraryWindow->setComicActionsDisabled(false);
    } else {
        contentViewsManager->showEmptySpecialList(type);
        libraryWindow->setComicActionsDisabled(true);
    }
}

void YACReaderNavigationController::loadLabelContent(const QModelIndex &listIndex)
{
    const qulonglong id = listIndex.data(ReadingListModel::IDRole).toULongLong();
    // check comics in label with id = id
    libraryWindow->comicsModel->setupLabelModelData(id, libraryWindow->foldersModel->getDatabase());
    contentViewsManager->comicsView->setModel(libraryWindow->comicsModel);

    // configure views
    if (libraryWindow->comicsModel->rowCount() > 0) {
        // updateView
        contentViewsManager->showComicsView();
        libraryWindow->setComicActionsDisabled(false);
    } else {
        // showEmptyFolder
        // loadEmptyLabelInfo(); //there is no info in an empty label by now, TODO design something
        contentViewsManager->showEmptyLabel(static_cast<YACReader::LabelColors>(listIndex.data(ReadingListModel::LabelColorRole).toInt()));
        libraryWindow->setComicActionsDisabled(true);
    }
}

// a folder of lists has no comics: it shows its folders and lists as cards
void YACReaderNavigationController::loadListFolderContent(const QModelIndex &listIndex)
{
    libraryWindow->setComicActionsDisabled(true);
    const auto *model = listIndex.model();
    if (model->rowCount(listIndex) == 0) {
        contentViewsManager->showEmptyListFolder();
        return;
    }

    QList<GridListCard> cards;
    QList<ReadingListId> readingLists;
    const auto &art = ThemeManager::instance().getCurrentTheme().emptyContainer;
    for (int row = 0; row < model->rowCount(listIndex); ++row) {
        const auto child = model->index(row, 0, listIndex);
        GridListCard card;
        card.listType = child.data(ReadingListModel::TypeListsRole).toInt();
        card.id = child.data(ReadingListModel::IDRole).toULongLong();
        card.name = child.data(Qt::DisplayRole).toString();
        switch (card.listType) {
        case ReadingListModel::ReadingList:
            card.kind = GridListCard::ReadingList;
            card.icon = art.readingListArtUrl;
            readingLists.append(ReadingListId(card.id));
            break;
        case ReadingListModel::SmartList:
            card.kind = GridListCard::SmartList;
            card.icon = art.smartListArtUrl;
            break;
        default:
            card.kind = GridListCard::Folder;
            break;
        }
        cards.append(card);
    }

    const auto dataPath = libraryWindow->foldersModel->getDatabase();
    std::optional<QHash<qulonglong, QString>> hashes;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        connectionName = db.connectionName();
        hashes = DBHelper::readingListCoverHashes(readingLists, db);
    }
    QSqlDatabase::removeDatabase(connectionName);
    for (auto &card : cards) {
        if (card.kind == GridListCard::ReadingList && hashes && hashes->contains(card.id))
            card.cover = QUrl::fromLocalFile(YACReader::LibraryPaths::coverPathFromLibraryDataPath(dataPath, hashes->value(card.id)));
    }

    contentViewsManager->gridView()->setListCards(cards);
    contentViewsManager->showFoldersOnlyGrid();
}

// opening a card selects its row in the sidebar, inside its folders
void YACReaderNavigationController::openListCard(int listType, qulonglong id)
{
    const auto sourceIndex = libraryWindow->listsModel->indexOf(listType, id);
    if (!sourceIndex.isValid())
        return;

    const auto proxyIndex = libraryWindow->listsModelProxy->mapFromSource(sourceIndex);
    for (auto parent = proxyIndex.parent(); parent.isValid(); parent = parent.parent())
        libraryWindow->listsView->expand(parent);
    libraryWindow->listsView->scrollTo(proxyIndex);
    libraryWindow->listsView->setCurrentIndex(proxyIndex);
    selectedList(proxyIndex);
}

// an empty smart list is a search with no results
void YACReaderNavigationController::loadSmartListContent(const QModelIndex &listIndex)
{
    const qulonglong id = listIndex.data(ReadingListModel::IDRole).toULongLong();
    libraryWindow->comicsModel->setupSmartListModelData(id, libraryWindow->foldersModel->getDatabase());
    contentViewsManager->comicsView->setModel(libraryWindow->comicsModel);

    if (libraryWindow->comicsModel->rowCount() > 0) {
        contentViewsManager->showComicsView();
        libraryWindow->setComicActionsDisabled(false);
    } else {
        contentViewsManager->showNoSearchResults();
        libraryWindow->setComicActionsDisabled(true);
    }
}

void YACReaderNavigationController::loadReadingListContent(const QModelIndex &listIndex)
{
    const qulonglong id = listIndex.data(ReadingListModel::IDRole).toULongLong();
    // check comics in label with id = id
    libraryWindow->comicsModel->setupReadingListModelData(id, libraryWindow->foldersModel->getDatabase());
    contentViewsManager->comicsView->setModel(libraryWindow->comicsModel);

    // configure views
    if (libraryWindow->comicsModel->rowCount() > 0) {
        // updateView
        contentViewsManager->showComicsView();
        libraryWindow->setComicActionsDisabled(false);
    } else {
        contentViewsManager->showEmptyReadingList();
        libraryWindow->setComicActionsDisabled(true);
    }
}

void YACReaderNavigationController::selectedList(const QModelIndex &proxyIndex)
{
    const QModelIndex listIndex = libraryWindow->listsModelProxy->mapToSource(proxyIndex);

    recordCurrentViewState();
    libraryWindow->historyController->updateHistory(YACReaderLibrarySourceContainer(listIndex, YACReaderLibrarySourceContainer::List));

    // when a list is selected the search mode has to be reset
    if (librarySearchCoordinator->exitSearchMode()) {

        libraryWindow->listsView->scrollTo(proxyIndex, QAbstractItemView::PositionAtTop);
        libraryWindow->listsView->setCurrentIndex(proxyIndex);
    }

    loadListContent(listIndex);

    libraryWindow->setToolbarTitle(listIndex);
}

void YACReaderNavigationController::reselectCurrentList()
{
    selectedList(libraryWindow->listsView->currentIndex());
}

void YACReaderNavigationController::reselectCurrentSource()
{
    if (!libraryWindow->hasLoadedLibraryModels())
        return;

    if (!libraryWindow->listsView->selectionModel()->selectedRows().isEmpty()) {
        reselectCurrentList();
    } else {
        reselectCurrentFolder();
    }
}

void YACReaderNavigationController::beginCurrentSourceRefresh()
{
    pendingRefreshViewState = contentViewsManager->captureViewState();
}

void YACReaderNavigationController::cancelCurrentSourceRefresh()
{
    pendingRefreshViewState.reset();
}

void YACReaderNavigationController::refreshCurrentSource()
{
    if (!libraryWindow->hasLoadedLibraryModels()) {
        pendingRefreshViewState.reset();
        return;
    }

    // Reloading resets the models used by every content view. Keep the view-specific
    // state outside that operation so each view can restore its stable item anchor
    // once the refreshed source has been populated.
    const auto viewState = pendingRefreshViewState.value_or(contentViewsManager->captureViewState());
    pendingRefreshViewState.reset();

    if (librarySearchCoordinator->isSearching()) {
        libraryWindow->comicsModel->reload();

        if (contentViewsManager->isComicsViewVisible())
            contentViewsManager->comicsView->reloadContent();

        contentViewsManager->restoreViewState(viewState);
        return;
    }

    if (!libraryWindow->listsView->selectionModel()->selectedRows().isEmpty()) {
        auto currentListIndex = libraryWindow->listsModelProxy->mapToSource(libraryWindow->listsView->currentIndex());
        if (currentListIndex.isValid()) {
            loadListContent(currentListIndex);
            contentViewsManager->restoreViewState(viewState);
            return;
        }
    }

    // The folder on screen is the one to reload. The tree selection can point
    // somewhere else (a right click on the tree selects a folder without
    // navigating to it), and using it here would move the content view to
    // another folder, without going through the navigation and history code.
    loadFolderContent(loadedFolder);
    contentViewsManager->restoreViewState(viewState);
}

void YACReaderNavigationController::backward()
{
    libraryWindow->historyController->backward(contentViewsManager->captureViewState());
}

void YACReaderNavigationController::forward()
{
    libraryWindow->historyController->forward(contentViewsManager->captureViewState());
}

void YACReaderNavigationController::selectedIndexFromHistory(const YACReaderLibrarySourceContainer &sourceContainer)
{
    // TODO NO searching allowed, just disable backward/forward actions in searching mode
    // when a folder or a list is selected the search mode has to be reset
    librarySearchCoordinator->exitSearchMode();
    restoringHistorySelection = true;
    loadIndexFromHistory(sourceContainer);
    contentViewsManager->restoreViewState(sourceContainer.getViewState());
    restoringHistorySelection = false;
    libraryWindow->setToolbarTitle(sourceContainer.getSourceModelIndex());
}

void YACReaderNavigationController::loadIndexFromHistory(const YACReaderLibrarySourceContainer &sourceContainer)
{
    QModelIndex sourceMI = sourceContainer.getSourceModelIndex();
    switch (sourceContainer.getType()) {
    case YACReaderLibrarySourceContainer::Folder: {
        if (!sourceMI.isValid()) {
            libraryWindow->setRootIndex(); // TODO: we do a double update, without it the continue reading list height comes later and causes a small flash
            break;
        }

        QModelIndex mi = libraryWindow->foldersModelProxy->mapFromSource(sourceMI);
        libraryWindow->foldersView->scrollTo(mi, QAbstractItemView::PositionAtTop);
        // currentIndexChanged is about to be emited, but we don't want it to end in YACReaderHistoryController::updateHistory
        disconnect(libraryWindow->foldersView, &YACReaderTreeView::currentIndexChanged, this, &YACReaderNavigationController::selectedFolder);
        libraryWindow->foldersView->setCurrentIndex(mi);
        connect(libraryWindow->foldersView, &YACReaderTreeView::currentIndexChanged, this, &YACReaderNavigationController::selectedFolder);
        loadFolderContent(sourceMI);
        break;
    }
    case YACReaderLibrarySourceContainer::List: {
        QModelIndex mi = libraryWindow->listsModelProxy->mapFromSource(sourceMI);
        libraryWindow->listsView->scrollTo(mi, QAbstractItemView::PositionAtTop);
        libraryWindow->listsView->setCurrentIndex(mi);
        loadListContent(sourceMI);
        break;
    }
    case YACReaderLibrarySourceContainer::None:
        QLOG_ERROR() << "Cannot load a source container of type None";
        break;
    }
}

void YACReaderNavigationController::loadRootContinueReading()
{
    auto readingComicsModel = std::make_unique<ComicModel>();

    readingComicsModel->setupReadingModelData(libraryWindow->foldersModel->getDatabase());

    contentViewsManager->gridView()->setRootContinueReadingModel(std::move(readingComicsModel));
}

void YACReaderNavigationController::reloadRootContinueReading()
{
    contentViewsManager->gridView()->reloadRootContinueReadingModel();
}

void YACReaderNavigationController::loadPreviousStatus()
{
    YACReaderLibrarySourceContainer sourceContainer = libraryWindow->historyController->currentSourceContainer();
    loadIndexFromHistory(sourceContainer);
}

void YACReaderNavigationController::setupConnections()
{
    auto *gridView = contentViewsManager->gridView();

    // we need YACReaderTreeView::currentIndexChanged to be able to navigate the folders tree using the keyboard cursors
    connect(libraryWindow->foldersView, &YACReaderTreeView::currentIndexChanged, this, &YACReaderNavigationController::selectedFolder);
    connect(libraryWindow->foldersView, &YACReaderTreeView::clicked, this, &YACReaderNavigationController::selectedFolder);
    connect(libraryWindow->listsView, &QAbstractItemView::clicked, this, &YACReaderNavigationController::selectedList);
    connect(contentViewsManager->gridView(), &GridComicsView::listCardOpened, this, &YACReaderNavigationController::openListCard);
    // the cards of the folder on screen follow the moves and renames of what it holds
    const auto refreshListFolder = [this] {
        const auto *selection = libraryWindow->listsView->selectionModel();
        if (selection == nullptr || selection->selectedRows().isEmpty())
            return;
        if (libraryWindow->listsModel->listFolder(libraryWindow->listsModelProxy->mapToSource(libraryWindow->listsView->currentIndex())))
            refreshCurrentSource();
    };
    connect(libraryWindow->listsModel, &QAbstractItemModel::rowsMoved, this, refreshListFolder);
    connect(libraryWindow->listsModel, &QAbstractItemModel::dataChanged, this, refreshListFolder);
    connect(libraryWindow->historyController, &YACReaderHistoryController::modelIndexSelected, this, &YACReaderNavigationController::selectedIndexFromHistory);
    connect(gridView, &GridComicsView::folderSelected, this, &YACReaderNavigationController::navigateToFolder);
    connect(libraryWindow->comicsModel, &ComicModel::isEmpty, this, &YACReaderNavigationController::reselectCurrentSource);
}

void YACReaderNavigationController::recordCurrentViewState()
{
    libraryWindow->historyController->recordViewStateForCurrentEntry(contentViewsManager->captureViewState());
}

qulonglong YACReaderNavigationController::folderIdForIndex(const QModelIndex &folderIndex) const
{
    if (!folderIndex.isValid())
        return FolderModel::RootFolderId;

    auto folderItem = static_cast<FolderItem *>(folderIndex.internalPointer());
    if (folderItem != nullptr)
        return folderItem->folder.id.value();

    return FolderModel::RootFolderId;
}
