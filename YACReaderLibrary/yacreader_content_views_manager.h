#ifndef YACREADERCONTENTVIEWSMANAGER_H
#define YACREADERCONTENTVIEWSMANAGER_H

#include "content_view_state.h"
#include "reading_list_model.h"
#include "themable.h"
#include "yacreader_global_gui.h"

#include <QSettings>
#include <QStackedWidget>

class LibraryWindow;

class ComicsView;
class ComicModel;
class ClassicComicsView;
class GridComicsView;
class InfoComicsView;
class ComicsViewTransition;
class EmptyLabelWidget;
class EmptySpecialListWidget;
class EmptyReadingListWidget;
class EmptyFolderWidget;
class NoSearchResultsWidget;
class FolderModel;
class ComicManagementCoordinator;
class LibraryWindowMenus;

using namespace YACReader;

class YACReaderContentViewsManager : public QObject, protected Themable
{
    Q_OBJECT
public:
    explicit YACReaderContentViewsManager(QSettings *settings, LibraryWindow *parent = nullptr);

    QWidget *containerWidget();
    GridComicsView *gridView() const;
    bool isComicsViewVisible() const;
    void prepareToClose();
    ContentViewState captureViewState() const;
    void restoreViewState(const ContentViewState &state);
    void setComicManagementCoordinator(ComicManagementCoordinator *coordinator);
    void setLibraryWindowMenus(LibraryWindowMenus *menus);

    ComicsView *comicsView;

    ComicsViewTransition *comicsViewTransition;

    void updateCurrentComicView();

    void toFullscreen();
    void toNormal();

protected:
    QStackedWidget *comicsViewStack;
    LibraryWindow *libraryWindow;

    ComicsViewStatus comicsViewStatus;

    ClassicComicsView *classicComicsView;
    GridComicsView *gridComicsView;
    InfoComicsView *infoComicsView;
    ComicsView *toolbarOwner;
    ComicManagementCoordinator *comicManagementCoordinator;
    LibraryWindowMenus *libraryWindowMenus;

    EmptyLabelWidget *emptyLabelWidget;
    EmptySpecialListWidget *emptySpecialList;
    EmptyReadingListWidget *emptyReadingList;
    EmptyFolderWidget *emptyFolderWidget;
    NoSearchResultsWidget *noSearchResultsWidget;

    void applyTheme(const Theme &theme) override;

public slots:
    void toggleComicsView();
    void focusComicsViewViaShortcut();

    void showComicsView();
    void showFoldersOnlyGrid();
    void showEmptyLabel(YACReader::LabelColors color);
    void showEmptySpecialList(ReadingListModel::TypeSpecialList type);
    void showEmptyReadingList();
    void showEmptyFolder();
    void showNoSearchResults();

protected slots:
    void showComicsViewTransition();

    void disconnectComicsViewConnections(ComicsView *widget);
    void connectComicsViewConnections(ComicsView *view);

    void switchToNextComicsView(const ContentViewState &viewState);
    void switchToComicsView(ComicsView *from, ComicsView *to, const ContentViewState &viewState);
    void setToolBarOwner(ComicsView *view);
    void setViewSelectorEnabled(bool enabled);
    void updateViewSelectorIcon(const Theme &theme);
    void ensureInStack(ComicsView *view);
    void showStackWidget(QWidget *widget, bool viewSelectorEnabled);
    void updateComicActionsForCurrentView();
};

#endif // YACREADERCONTENTVIEWSMANAGER_H
