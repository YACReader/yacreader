#ifndef LIBRARY_SEARCH_COORDINATOR_H
#define LIBRARY_SEARCH_COORDINATOR_H

#include "comic_query_result_processor.h"
#include "folder_query_result_processor.h"

#include <QObject>

#include <functional>
#include <memory>

class ComicItem;
class ComicModel;
class FolderItem;
class FolderModel;
class FolderModelProxy;
class YACReaderContentViewsManager;
class YACReaderFoldersView;

class LibrarySearchCoordinator : public QObject
{
    Q_OBJECT

public:
    using ClearSearchInput = std::function<void()>;

    LibrarySearchCoordinator(FolderModel *foldersModel,
                             FolderModelProxy *foldersModelProxy,
                             ComicModel *comicsModel,
                             YACReaderFoldersView *foldersView,
                             YACReaderContentViewsManager *contentViewsManager,
                             ClearSearchInput clearSearchInput,
                             QObject *parent = nullptr);

    bool isSearching() const;
    bool exitSearchMode();

public slots:
    void search(const QString &filter);

signals:
    void previousNavigationStateRequested();
    void comicActionsDisabledChanged(bool disabled);

private slots:
    void applyComicResults(QList<ComicItem *> *data, const QString &databasePath);
    void applyFolderResults(QMap<unsigned long long, FolderItem *> *filteredItems, FolderItem *root);

private:
    void clearResults();

    FolderModel *foldersModel;
    FolderModelProxy *foldersModelProxy;
    ComicModel *comicsModel;
    YACReaderFoldersView *foldersView;
    YACReaderContentViewsManager *contentViewsManager;
    ClearSearchInput clearSearchInput;
    YACReader::ComicQueryResultProcessor comicQueryResultProcessor;
    std::unique_ptr<YACReader::FolderQueryResultProcessor> folderQueryResultProcessor;
    bool searching { false };
};

#endif // LIBRARY_SEARCH_COORDINATOR_H
