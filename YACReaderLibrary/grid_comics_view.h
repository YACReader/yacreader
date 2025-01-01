#ifndef GRID_COMICS_VIEW_H
#define GRID_COMICS_VIEW_H

#include "comics_view.h"

#include <QModelIndex>

#include "comic_db.h"

class QAbstractListModel;
class QItemSelectionModel;
class QQuickWidget;
class QQmlContext;

class YACReaderToolBarStretch;
class YACReaderComicsSelectionHelper;
class YACReaderComicInfoHelper;

// values relative to visible cells
const unsigned int YACREADER_MIN_GRID_ZOOM_WIDTH = 156;
const unsigned int YACREADER_MAX_GRID_ZOOM_WIDTH = 312;

// GridView cells
const unsigned int YACREADER_MIN_CELL_CUSTOM_HEIGHT = 295;
const unsigned int YACREADER_MIN_CELL_CUSTOM_WIDTH = 185;

// Covers
const unsigned int YACREADER_MAX_COVER_HEIGHT = 236;
const unsigned int YACREADER_MIN_COVER_WIDTH = YACREADER_MIN_GRID_ZOOM_WIDTH;

// visible cells (realCell in qml), grid cells size is used to create faux inner margings
const unsigned int YACREADER_MIN_ITEM_HEIGHT = YACREADER_MAX_COVER_HEIGHT + 51; // 51 is the height of the bottom rectangle used for title and other info
const unsigned int YACREADER_MIN_ITEM_WIDTH = YACREADER_MIN_COVER_WIDTH;

class GridComicsView : public ComicsView
{
    Q_OBJECT
public:
    explicit GridComicsView(QWidget *parent = nullptr);
    ~GridComicsView() override;
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
    QByteArray getMimeDataFromSelection();
    void updateCurrentComicView() override;
    void focusComicsNavigation(Qt::FocusReason reason) override;
    void reloadContent() override;

public slots:
    // ComicsView
    void setShowMarks(bool show) override;
    void selectAll() override;
    void selectIndex(int index) override;
    void triggerOpenCurrentComic();
    void updateSettings();
    void updateBackgroundConfig();
    void showInfo();

protected slots:
    void setCurrentIndex(int index);
    // QML - double clicked item
    void selectedItem(int index);

    // QML - rating
    void rate(int index, int rating);
    // QML - dragManager
    void startDrag();
    // QML - dropManager
    bool canDropUrls(const QList<QUrl> &urls, Qt::DropAction action);
    bool canDropFormats(const QString &formats);
    void droppedFiles(const QList<QUrl> &urls, Qt::DropAction action);
    void droppedComicsForResortingAt(const QString &data, int index);
    // QML - context menu
    void requestedContextMenu(const QPoint &point);

    void setCoversSize(int width);
    void updateCoversSizeInContext(int width, QQmlContext *ctxt);

    void dummyUpdater(); // TODO remove this

    void setCurrentComicIfNeeded();

    void resetScroll();

    virtual void showEvent(QShowEvent *event) override;

signals:
    void onScrollToOrigin();

private:
    QSettings *settings;
    QToolBar *toolbar;
    QWidget *coverSizeSliderWidget;
    QSlider *coverSizeSlider;
    QAction *coverSizeSliderAction;
    QAction *showInfoAction;
    QAction *showInfoSeparatorAction;
    QAction *startSeparatorAction;

    bool filterEnabled;

    YACReaderComicsSelectionHelper *selectionHelper;
    YACReaderComicInfoHelper *comicInfoHelper;

    ComicDB currentComic;

    bool dummy;
    void closeEvent(QCloseEvent *event) override;
    void createCoverSizeSliderWidget();
};

#endif // GRID_COMICS_VIEW_H
