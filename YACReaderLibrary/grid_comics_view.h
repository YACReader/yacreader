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
    void setToolBar(QToolBar *toolBar) override;
    void setModel(ComicModel *model) override;
    void setCurrentIndex(const QModelIndex &index) override;
    QModelIndex currentIndex() override;
    QItemSelectionModel *selectionModel() override;
    void scrollTo(const QModelIndex &mi, QAbstractItemView::ScrollHint hint) override;
    void toFullScreen() override;
    void toNormal() override;
    void updateConfig(QSettings *settings) override;
    void enableFilterMode(bool enabled) override;
    QSize sizeHint();
    QByteArray getMimeDataFromSelection();
    void updateCurrentComicView() override;

public slots:
    //ComicsView
    void setShowMarks(bool show) override;
    void selectAll() override;
    void selectIndex(int index) override;
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
    void closeEvent(QCloseEvent *event) override;
    void createCoverSizeSliderWidget();
};

#endif // GRID_COMICS_VIEW_H
