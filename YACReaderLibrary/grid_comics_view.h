#ifndef GRID_COMICS_VIEW_H
#define GRID_COMICS_VIEW_H

#include "comics_view.h"

#include <QModelIndex>

#include "comic_db.h"

class QAbstractListModel;
class QItemSelectionModel;
class QQuickWidget;

class YACReaderToolBarStretch;
class YACReaderComicsSelectionHelper;
class YACReaderComicInfoHelper;

class GridComicsView : public ComicsView
{
    Q_OBJECT
public:
    explicit GridComicsView(QWidget *parent = 0);
    virtual ~GridComicsView();
    void setToolBar(QToolBar *toolBar);
    void setModel(ComicModel *model);
    void setCurrentIndex(const QModelIndex &index);
    QModelIndex currentIndex();
    QItemSelectionModel *selectionModel();
    void scrollTo(const QModelIndex &mi, QAbstractItemView::ScrollHint hint);
    void toFullScreen();
    void toNormal();
    void updateConfig(QSettings *settings);
    void enableFilterMode(bool enabled);
    QSize sizeHint();
    QByteArray getMimeDataFromSelection();
    void updateCurrentComicView();

public slots:
    //ComicsView
    void setShowMarks(bool show);
    void selectAll();
    void selectIndex(int index);
    void triggerOpenCurrentComic();

    void updateBackgroundConfig();

    void showInfo();

protected slots:
    void setCurrentIndex(int index);
    //QML - double clicked item
    void selectedItem(int index);

    //QML - rating
    void rate(int index, int rating);
    //QML - dragManager
    void startDrag();
    //QML - dropManager
    bool canDropUrls(const QList<QUrl> &urls, Qt::DropAction action);
    bool canDropFormats(const QString &formats);
    void droppedFiles(const QList<QUrl> &urls, Qt::DropAction action);
    void droppedComicsForResortingAt(const QString &data, int index);
    //QML - context menu
    void requestedContextMenu(const QPoint &point);

    void setCoversSize(int width);

    void dummyUpdater(); //TODO remove this

    void setCurrentComicIfNeeded();

    void resetScroll();

signals:
    void onScrollToOrigin();

private:
    QSettings *settings;
    QToolBar *toolbar;
    YACReaderToolBarStretch *toolBarStretch;
    QAction *toolBarStretchAction;
    QWidget *coverSizeSliderWidget;
    QSlider *coverSizeSlider;
    QAction *coverSizeSliderAction;
    QAction *showInfoAction;
    QAction *showInfoSeparatorAction;

    bool filterEnabled;

    YACReaderComicsSelectionHelper *selectionHelper;
    YACReaderComicInfoHelper *comicInfoHelper;

    ComicDB currentComic;

    bool dummy;
    void closeEvent(QCloseEvent *event);
    void createCoverSizeSliderWidget();
};

#endif // GRID_COMICS_VIEW_H
