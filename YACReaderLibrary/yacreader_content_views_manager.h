#ifndef YACREADERCONTENTVIEWSMANAGER_H
#define YACREADERCONTENTVIEWSMANAGER_H

#include <QSettings>
#include <QStackedWidget>

#include "yacreader_global_gui.h"
#include "themable.h"

class LibraryWindow;

class ComicsView;
class ClassicComicsView;
class GridComicsView;
class InfoComicsView;
class ComicsViewTransition;
class FolderContentView;
class EmptyLabelWidget;
class EmptySpecialListWidget;
class EmptyReadingListWidget;
class EmptyFolderWidget;
class NoSearchResultsWidget;

using namespace YACReader;

class YACReaderContentViewsManager : public QObject, protected Themable
{
    Q_OBJECT
public:
    explicit YACReaderContentViewsManager(QSettings *settings, LibraryWindow *parent = nullptr);

    QWidget *containerWidget();

    ComicsView *comicsView;

    ComicsViewTransition *comicsViewTransition;

    FolderContentView *folderContentView;
    EmptyLabelWidget *emptyLabelWidget;
    EmptySpecialListWidget *emptySpecialList;
    EmptyReadingListWidget *emptyReadingList;
    EmptyFolderWidget *emptyFolderWidget;

    NoSearchResultsWidget *noSearchResultsWidget;

    void updateCurrentContentView();
    void updateCurrentComicView();
    void updateContinueReadingView();

    void toFullscreen();
    void toNormal();

protected:
    QStackedWidget *comicsViewStack;
    LibraryWindow *libraryWindow;

    ComicsViewStatus comicsViewStatus;

    ClassicComicsView *classicComicsView;
    GridComicsView *gridComicsView;
    InfoComicsView *infoComicsView;

    void applyTheme(const Theme &theme) override;

signals:

public slots:
    void toggleComicsView();
    void focusComicsViewViaShortcut();

    void showComicsView();
    void showFolderContentView();
    void showEmptyLabelView();
    void showEmptySpecialList();
    void showEmptyReadingListWidget();
    void showEmptyFolderWidget();
    void showNoSearchResultsView();

protected slots:
    void showComicsViewTransition();
    void _toggleComicsView();

    void disconnectComicsViewConnections(ComicsView *widget);
    void doComicsViewConnections();

    void switchToComicsView(ComicsView *from, ComicsView *to);
};

#endif // YACREADERCONTENTVIEWSMANAGER_H
