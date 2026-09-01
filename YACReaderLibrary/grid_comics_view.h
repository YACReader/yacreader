#ifndef GRID_COMICS_VIEW_H
#define GRID_COMICS_VIEW_H

#include "comic_db.h"
#include "comics_view.h"
#include "themable.h"

#include <QLabel>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QVariantMap>

#include <memory>
#include <optional>

class QAbstractListModel;
class QItemSelectionModel;
class QQuickWidget;
class QQmlContext;
class QTimer;

class YACReaderToolBarStretch;
class YACReaderComicsSelectionHelper;
class YACReaderComicInfoHelper;
class GridContentModel;
class FolderModel;
class Folder;

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

class GridComicsView : public ComicsView, protected Themable
{
    Q_OBJECT
    Q_PROPERTY(ComicModel *rootContinueReadingModel READ rootContinueReadingModel NOTIFY rootContinueReadingModelChanged)
    Q_PROPERTY(bool rootFolder READ isRootFolder NOTIFY rootFolderChanged)
    Q_PROPERTY(bool globalContinueReadingEnabled READ isGlobalContinueReadingEnabled NOTIFY globalContinueReadingEnabledChanged)
    Q_PROPERTY(bool currentComicBannerVisible READ isCurrentComicBannerVisible NOTIFY currentComicBannerVisibleChanged)
    Q_PROPERTY(int focusedFolderRow READ focusedFolderRow NOTIFY focusedFolderChanged)
    Q_PROPERTY(QVariantMap focusedFolderInfo READ folderInfoForFocusedFolder NOTIFY focusedFolderChanged)
    Q_PROPERTY(QVariantMap currentLocationInfo READ locationInfo NOTIFY currentLocationInfoChanged)
    Q_PROPERTY(bool hasComicSelection READ hasComicSelection NOTIFY comicSelectionStateChanged)
    Q_PROPERTY(int selectedComicCount READ selectedComicCount NOTIFY selectedComicsInfoChanged)
    Q_PROPERTY(QVariantMap selectedComicsInfo READ selectedComicsInfo NOTIFY selectedComicsInfoChanged)
public:
    explicit GridComicsView(QWidget *parent = nullptr);
    ComicModel *rootContinueReadingModel() const;
    bool isRootFolder() const;
    bool isGlobalContinueReadingEnabled() const;
    bool isCurrentComicBannerVisible() const;
    int focusedFolderRow() const;
    QVariantMap folderInfoForFocusedFolder() const;
    QVariantMap locationInfo() const;
    bool hasComicSelection() const;
    int selectedComicCount() const;
    QVariantMap selectedComicsInfo() const;
    void setFolderModel(FolderModel *model, const QModelIndex &folderIndex, const QString &rootName = { }, const QVariantMap &libraryInfo = { });
    void clearFolderModel();
    void setCurrentList(const QModelIndex &listIndex);
    void setModel(ComicModel *model) override;
    void setRootContinueReadingModel(std::unique_ptr<ComicModel> model);
    void clearRootContinueReadingModel();
    void reloadRootContinueReadingModel();

    Q_INVOKABLE void requestOpenLibraryFolder();
    Q_INVOKABLE void openFolder(int viewRow);
    Q_INVOKABLE void focusItem(int viewRow);
    Q_INVOKABLE void clearFolderFocus();
    Q_INVOKABLE void selectComicRange(int from, int to);
    Q_INVOKABLE int viewRowForComicRow(int sourceRow) const;
    Q_INVOKABLE void setGridColumnCount(int columns);
    Q_INVOKABLE int nearestSelectableRow(int viewRow, int direction) const;
    Q_INVOKABLE void openContinueReadingComic(int sourceRow);
    Q_INVOKABLE void requestContinueReadingComicContextMenu(const QPoint &point, int sourceRow);

protected:
    void applyTheme(const Theme &theme) override;
    ~GridComicsView() override;
    void setToolBar(QToolBar *toolBar) override;
    void releaseToolBar() override;
    void saveViewConfig() override;
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
    ContentViewState captureViewState() const override;
    void restoreViewState(const ContentViewState &state) override;

public slots:
    // ComicsView
    void setShowMarks(bool show) override;
    void selectAll() override;
    void selectIndex(int index) override;
    void triggerOpenCurrentComic();
    void updateSettings();
    void updateBannerSettings();
    void updateBackgroundConfig();
    void updateInfoPanelVisibility();

protected slots:
    // QML - double clicked item
    void activateItem(int viewRow);

    // QML - rating
    void rate(int index, int rating);
    // QML - dragManager
    void startDrag();
    // QML - dropManager
    bool canDropUrls(const QList<QUrl> &urls, Qt::DropAction action);
    bool canDropImage(const QList<QUrl> &urls);
    bool canDropFormats(const QStringList &formats);
    void droppedFiles(const QList<QUrl> &urls, Qt::DropAction action);
    void droppedImageAt(const QList<QUrl> &urls, int viewRow);
    void droppedComicsForResortingAt(const QString &data, int index);
    // QML - context menu
    void requestItemContextMenu(const QPoint &point, int viewRow);

    void setCoversSize(int width);
    void updateCoversSizeInContext(int width, QQmlContext *ctxt);

    void updateCurrentComicBanner();

    void applyPendingViewState();

    virtual void showEvent(QShowEvent *event) override;

signals:
    void onScrollToOrigin();
    void folderSelected(const QModelIndex &index);
    void openFolderContextMenu(const QPoint &point, const Folder &folder);
    void openContinueReadingComicContextMenu(const QPoint &point, const ComicDB &comic);
    void comicSelectionStateChanged(bool hasSelection);
    void selectedComicsInfoChanged();
    void rootContinueReadingModelChanged();
    void rootFolderChanged();
    void globalContinueReadingEnabledChanged();
    void currentComicBannerVisibleChanged();
    void focusedFolderChanged();
    void currentLocationInfoChanged();
    void openLibraryFolderRequested();

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
    GridContentModel *contentModel;
    std::unique_ptr<ComicModel> rootContinueReadingModelStorage;
    bool rootFolder = false;
    bool globalContinueReadingEnabled = true;
    bool currentComicBannerVisible = false;
    QPersistentModelIndex focusedFolderIndex;
    QVariantMap focusedFolderInfo;
    QVariantMap currentLocationInfo;
    QTimer *viewStateTimer;
    std::optional<ContentViewState> pendingViewState;
    QMetaObject::Connection modelDataChangedConnection;
    QMetaObject::Connection modelFavoritesChangedConnection;

    ComicDB currentComic;

    void closeEvent(QCloseEvent *event) override;
    void createCoverSizeSliderWidget();
    QVariantMap makeFolderInfo(const Folder &folder, const QVariant &cover) const;
    void updateCurrentListIcon();
    void setFocusedFolder(int viewRow);
    void clearFocusedFolder();
    int viewRowForItem(const ContentItemRef &item) const;

    // Zoom slider labels (for theming)
    QLabel *smallZoomLabel;
    QLabel *bigZoomLabel;
};

#endif // GRID_COMICS_VIEW_H
