#ifndef YACREADER_NAVIGATION_CONTROLLER_H
#define YACREADER_NAVIGATION_CONTROLLER_H

#include "content_view_state.h"

#include <QObject>
#include <QPersistentModelIndex>

#include <optional>

class LibraryWindow;
class LibrarySearchCoordinator;
class YACReaderLibrarySourceContainer;
class YACReaderContentViewsManager;

class YACReaderNavigationController : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderNavigationController(LibraryWindow *parent, YACReaderContentViewsManager *contentViewsManager, LibrarySearchCoordinator *librarySearchCoordinator);

public slots:
    void selectedFolder(const QModelIndex &proxyIndex);
    void navigateToFolder(const QModelIndex &sourceIndex);
    void reselectCurrentFolder();
    void selectedList(const QModelIndex &proxyIndex);
    void reselectCurrentList();

    void reselectCurrentSource();
    void beginCurrentSourceRefresh();
    void cancelCurrentSourceRefresh();
    void refreshCurrentSource();

    // history navigation
    void backward();
    void forward();
    void selectedIndexFromHistory(const YACReaderLibrarySourceContainer &sourceContainer);
    void loadIndexFromHistory(const YACReaderLibrarySourceContainer &sourceContainer);

    void loadFolderContent(const QModelIndex &folderIndex);
    void loadListContent(const QModelIndex &listIndex);
    void loadSpecialListContent(const QModelIndex &listIndex);
    void loadLabelContent(const QModelIndex &listIndex);
    void loadReadingListContent(const QModelIndex &listIndex);

    void loadPreviousStatus();
    void reloadRootContinueReading();

private:
    void setupConnections();
    void loadRootContinueReading();
    void recordCurrentViewState();

    LibraryWindow *libraryWindow;
    YACReaderContentViewsManager *contentViewsManager;
    LibrarySearchCoordinator *librarySearchCoordinator;
    bool restoringHistorySelection = false;
    std::optional<ContentViewState> pendingRefreshViewState;
    QPersistentModelIndex loadedFolder;

    qulonglong folderIdForIndex(const QModelIndex &folderIndex) const;
};

#endif // YACREADER_NAVIGATION_CONTROLLER_H
