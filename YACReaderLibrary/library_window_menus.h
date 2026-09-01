#ifndef LIBRARY_WINDOW_MENUS_H
#define LIBRARY_WINDOW_MENUS_H

#include "comic_db.h"
#include "folder.h"
#include "yacreader_global.h"

#include <QModelIndex>
#include <QObject>

#include <functional>

class ComicManagementCoordinator;
class FolderManagementCoordinator;
class FolderModel;
class FolderModelProxy;
class LibraryWindowActions;
class OrganizeFilesCoordinator;
class QMainWindow;
class QMenu;
class QPoint;
class ReadingListModel;
struct Theme;
class YACReaderContentViewsManager;
class YACReaderFoldersView;
class YACReaderLibraryListWidget;

class LibraryWindowMenus : public QObject
{
    Q_OBJECT

public:
    using ComicSelectionProvider = std::function<QModelIndexList()>;
    using LibraryIdProvider = std::function<qulonglong()>;
    using LibraryPathProvider = std::function<QString()>;
    using ThemeProvider = std::function<const Theme &()>;

    explicit LibraryWindowMenus(QMainWindow *window,
                                LibraryWindowActions &actions,
                                YACReaderLibraryListWidget *selectedLibrary,
                                YACReaderFoldersView *foldersView,
                                YACReaderContentViewsManager *contentViewsManager,
                                FolderModel *foldersModel,
                                FolderModelProxy *foldersModelProxy,
                                ReadingListModel *listsModel,
                                FolderManagementCoordinator *folderManagementCoordinator,
                                ComicManagementCoordinator *comicManagementCoordinator,
                                OrganizeFilesCoordinator *organizeFilesCoordinator,
                                ComicSelectionProvider comicSelectionProvider,
                                LibraryIdProvider libraryIdProvider,
                                LibraryPathProvider libraryPathProvider,
                                ThemeProvider themeProvider);

    void setupMenus();

public slots:
    void showComicsViewContextMenu(const QPoint &point);
    void showComicsItemContextMenu(const QPoint &point);
    void showGridFoldersContextMenu(const QPoint &point, const Folder &folder);
    void showContinueReadingContextMenu(const QPoint &point, const ComicDB &comic);
    void showFoldersContextMenu(const QPoint &point);

signals:
    void currentLibraryTypeChangeRequested(YACReader::FileType type);
    void folderUpdateRequested(const QModelIndex &folder);
    void folderXmlRescanRequested(const QModelIndex &folder);

private:
    void showComicsContextMenu(const QPoint &point, bool showFullScreenAction);
    void setupAddToSubmenu(QMenu &menu);

    QMainWindow *window;
    LibraryWindowActions &actions;
    YACReaderLibraryListWidget *selectedLibrary;
    YACReaderFoldersView *foldersView;
    YACReaderContentViewsManager *contentViewsManager;
    FolderModel *foldersModel;
    FolderModelProxy *foldersModelProxy;
    ReadingListModel *listsModel;
    FolderManagementCoordinator *folderManagementCoordinator;
    ComicManagementCoordinator *comicManagementCoordinator;
    OrganizeFilesCoordinator *organizeFilesCoordinator;
    ComicSelectionProvider comicSelectionProvider;
    LibraryIdProvider libraryIdProvider;
    LibraryPathProvider libraryPathProvider;
    ThemeProvider themeProvider;
};

#endif // LIBRARY_WINDOW_MENUS_H
