#ifndef YACREADERCOMICSVIEWSMANAGER_H
#define YACREADERCOMICSVIEWSMANAGER_H

#include <QtWidgets>

#include "yacreader_global_gui.h"

class LibraryWindow;

class ComicsView;
class ClassicComicsView;
class GridComicsView;
class InfoComicsView;
class ComicsViewTransition;
class EmptyFolderWidget;
class EmptyLabelWidget;
class EmptySpecialListWidget;
class EmptyReadingListWidget;
class NoSearchResultsWidget;

using namespace YACReader;

class YACReaderComicsViewsManager : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderComicsViewsManager(QSettings *settings, LibraryWindow *parent = nullptr);

    QWidget *containerWidget();

    ComicsView *comicsView;

    ComicsViewTransition *comicsViewTransition;

    EmptyFolderWidget *emptyFolderWidget;
    EmptyLabelWidget *emptyLabelWidget;
    EmptySpecialListWidget *emptySpecialList;
    EmptyReadingListWidget *emptyReadingList;

    NoSearchResultsWidget *noSearchResultsWidget;

    void updateCurrentComicView();

protected:
    QStackedWidget *comicsViewStack;
    LibraryWindow *libraryWindow;

    ComicsViewStatus comicsViewStatus;

    ClassicComicsView *classicComicsView;
    GridComicsView *gridComicsView;
    InfoComicsView *infoComicsView;

signals:

public slots:
    void toggleComicsView();

    void showComicsView();
    void showEmptyFolderView();
    void showEmptyLabelView();
    void showEmptySpecialList();
    void showEmptyReadingListWidget();
    void showNoSearchResultsView();

protected slots:
    void showComicsViewTransition();
    void _toggleComicsView();

    void disconnectComicsViewConnections(ComicsView *widget);
    void doComicsViewConnections();

    void switchToComicsView(ComicsView *from, ComicsView *to);
};

#endif // COMICSVIEWSMANAGER_H
