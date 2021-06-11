#include "grid_comics_view.h"

#include <QtQuick>
#include <QQuickWidget>
#include <QtWidgets>

#include "comic.h"
#include "comic_files_manager.h"
#include "QsLog.h"
#include "yacreader_global.h"
#include "yacreader_tool_bar_stretch.h"
#include "comic_db.h"
#include "yacreader_comics_selection_helper.h"
#include "yacreader_comic_info_helper.h"
#include "current_comic_view_helper.h"

//values relative to visible cells
const unsigned int YACREADER_MIN_GRID_ZOOM_WIDTH = 156;
const unsigned int YACREADER_MAX_GRID_ZOOM_WIDTH = 312;

//GridView cells
const unsigned int YACREADER_MIN_CELL_CUSTOM_HEIGHT = 295;
const unsigned int YACREADER_MIN_CELL_CUSTOM_WIDTH = 185;

//Covers
const unsigned int YACREADER_MAX_COVER_HEIGHT = 236;
const unsigned int YACREADER_MIN_COVER_WIDTH = YACREADER_MIN_GRID_ZOOM_WIDTH;

//visible cells (realCell in qml), grid cells size is used to create faux inner margings
const unsigned int YACREADER_MIN_ITEM_HEIGHT = YACREADER_MAX_COVER_HEIGHT + 51; //51 is the height of the bottom rectangle used for title and other info
const unsigned int YACREADER_MIN_ITEM_WIDTH = YACREADER_MIN_COVER_WIDTH;

GridComicsView::GridComicsView(QWidget *parent)
    : ComicsView(parent), filterEnabled(false)
{
    settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat, this);
    settings->beginGroup("libraryConfig");

    qmlRegisterType<ComicModel>("com.yacreader.ComicModel", 1, 0, "ComicModel");
    qmlRegisterType<ComicDB>("com.yacreader.ComicDB", 1, 0, "ComicDB");
    qmlRegisterType<ComicInfo>("com.yacreader.ComicInfo", 1, 0, "ComicInfo");

    view = new QQuickWidget(this);
    view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    connect(
            view, &QQuickWidget::statusChanged,
            [=](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error) {
                    QLOG_ERROR() << view->errors();
                }
            });

    //view->setFocusPolicy(Qt::TabFocus);

    selectionHelper = new YACReaderComicsSelectionHelper(this);
    connect(selectionHelper, &YACReaderComicsSelectionHelper::selectionChanged, this, &GridComicsView::dummyUpdater);

    comicInfoHelper = new YACReaderComicInfoHelper(this);

    QQmlContext *ctxt = view->rootContext();

    LibraryUITheme theme;
#ifdef Q_OS_MAC
    theme = Light;
#else
    theme = Dark;
#endif

    if (theme == Light) {
        ctxt->setContextProperty("backgroundColor", "#F6F6F6");
        ctxt->setContextProperty("cellColor", "#FFFFFF");
        ctxt->setContextProperty("selectedColor", "#FFFFFF");
        ctxt->setContextProperty("selectedBorderColor", "#007AFF");
        ctxt->setContextProperty("borderColor", "#DBDBDB");
        ctxt->setContextProperty("titleColor", "#121212");
        ctxt->setContextProperty("textColor", "#636363");
        //fonts settings
        ctxt->setContextProperty("fontSize", 11);
        ctxt->setContextProperty("fontFamily", QApplication::font().family());
        ctxt->setContextProperty("fontSpacing", 0.5);

        //info - copy/pasted from info_comics_view TODO create helpers for setting the UI config
        ctxt->setContextProperty("infoBackgroundColor", "#FFFFFF");
        ctxt->setContextProperty("topShadow", QUrl());
        ctxt->setContextProperty("infoShadow", "info-shadow-light.png");
        ctxt->setContextProperty("infoIndicator", "info-indicator-light.png");

        ctxt->setContextProperty("infoTextColor", "#404040");
        ctxt->setContextProperty("infoTitleColor", "#2E2E2E");

        ctxt->setContextProperty("ratingUnselectedColor", "#DEDEDE");
        ctxt->setContextProperty("ratingSelectedColor", "#2B2B2B");

        ctxt->setContextProperty("favUncheckedColor", "#DEDEDE");
        ctxt->setContextProperty("favCheckedColor", "#E84852");

        ctxt->setContextProperty("readTickUncheckedColor", "#DEDEDE");
        ctxt->setContextProperty("readTickCheckedColor", "#E84852");
    } else {
        ctxt->setContextProperty("backgroundColor", "#2A2A2A");
        ctxt->setContextProperty("cellColor", "#212121");
        ctxt->setContextProperty("selectedColor", "#121212");
        ctxt->setContextProperty("selectedBorderColor", "#121212");
        ctxt->setContextProperty("borderColor", "#121212");
        ctxt->setContextProperty("titleColor", "#FFFFFF");
        ctxt->setContextProperty("textColor", "#A8A8A8");
        ctxt->setContextProperty("dropShadow", false);
        //fonts settings
        int fontSize = QApplication::font().pointSize();
        if (fontSize == -1)
            fontSize = QApplication::font().pixelSize();
        ctxt->setContextProperty("fontSize", fontSize);
        ctxt->setContextProperty("fontFamily", QApplication::font().family());
        ctxt->setContextProperty("fontSpacing", 0.5);

        //info - copy/pasted from info_comics_view TODO create helpers for setting the UI config
        ctxt->setContextProperty("infoBackgroundColor", "#2E2E2E");
        ctxt->setContextProperty("topShadow", "info-top-shadow.png");
        ctxt->setContextProperty("infoShadow", "info-shadow.png");
        ctxt->setContextProperty("infoIndicator", "info-indicator.png");

        ctxt->setContextProperty("infoTextColor", "#B0B0B0");
        ctxt->setContextProperty("infoTitleColor", "#FFFFFF");

        ctxt->setContextProperty("ratingUnselectedColor", "#1C1C1C");
        ctxt->setContextProperty("ratingSelectedColor", "#FFFFFF");

        ctxt->setContextProperty("favUncheckedColor", "#1C1C1C");
        ctxt->setContextProperty("favCheckedColor", "#E84852");

        ctxt->setContextProperty("readTickUncheckedColor", "#1C1C1C");
        ctxt->setContextProperty("readTickCheckedColor", "#E84852");
    }

#ifdef Q_OS_MAC

#else

#endif

    ctxt->setContextProperty("backgroundImage", QUrl());
    ctxt->setContextProperty("backgroundBlurOpacity", 0.0);
    ctxt->setContextProperty("backgroundBlurRadius", 0.0);
    ctxt->setContextProperty("backgroundBlurVisible", false);

    auto model = new ComicModel();
    selectionHelper->setModel(model);
    ctxt->setContextProperty("comicsList", model);
    ctxt->setContextProperty("comicsSelection", selectionHelper->selectionModel());
    ctxt->setContextProperty("contextMenuHelper", this);
    ctxt->setContextProperty("comicsSelectionHelper", selectionHelper);
    ctxt->setContextProperty("currentIndexHelper", this);
    ctxt->setContextProperty("comicRatingHelper", this);
    ctxt->setContextProperty("dummyValue", true);
    ctxt->setContextProperty("dragManager", this);
    ctxt->setContextProperty("dropManager", this);
    ctxt->setContextProperty("comicOpener", this);

    bool showInfo = settings->value(COMICS_GRID_SHOW_INFO, false).toBool();
    ctxt->setContextProperty("showInfo", showInfo);

    view->setSource(QUrl("qrc:/qml/GridComicsView.qml"));

    auto rootObject = dynamic_cast<QObject *>(view->rootObject());
    auto infoContainer = rootObject->findChild<QObject *>("infoContainer");

    QQmlProperty(infoContainer, "width").write(settings->value(COMICS_GRID_INFO_WIDTH, 350));

    showInfoAction = new QAction(tr("Show info"), this);
    showInfoAction->setIcon(QIcon(":/images/comics_view_toolbar/show_comic_info.png"));
    showInfoAction->setCheckable(true);
    showInfoAction->setChecked(showInfo);
    connect(showInfoAction, &QAction::toggled, this, &GridComicsView::showInfo);

    setShowMarks(true); //TODO save this in settings

    auto l = new QVBoxLayout;
    l->addWidget(view);
    this->setLayout(l);

    setContentsMargins(0, 0, 0, 0);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

    QLOG_TRACE() << "GridComicsView";
}

GridComicsView::~GridComicsView()
{
    delete view;
}

void GridComicsView::createCoverSizeSliderWidget()
{
    toolBarStretch = new YACReaderToolBarStretch(this);
    coverSizeSliderWidget = new QWidget(this);
    coverSizeSliderWidget->setFixedWidth(200);
    coverSizeSlider = new QSlider();
    coverSizeSlider->setOrientation(Qt::Horizontal);
    coverSizeSlider->setRange(YACREADER_MIN_GRID_ZOOM_WIDTH, YACREADER_MAX_GRID_ZOOM_WIDTH);

    auto horizontalLayout = new QHBoxLayout();
    QLabel *smallLabel = new QLabel();
    smallLabel->setPixmap(QPixmap(":/images/comics_view_toolbar/small_size_grid_zoom.png"));
    horizontalLayout->addWidget(smallLabel);
    horizontalLayout->addWidget(coverSizeSlider, 0, Qt::AlignVCenter);
    QLabel *bigLabel = new QLabel();
    bigLabel->setPixmap(QPixmap(":/images/comics_view_toolbar/big_size_grid_zoom.png"));
    horizontalLayout->addWidget(bigLabel);
    horizontalLayout->addSpacing(10);
    horizontalLayout->setMargin(0);

    coverSizeSliderWidget->setLayout(horizontalLayout);
    //TODO add shortcuts (ctrl-+ and ctrl-- for zooming in out, + ctrl-0 for reseting the zoom)

    connect(coverSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(setCoversSize(int)));

    int coverSize = settings->value(COMICS_GRID_COVER_SIZES, YACREADER_MIN_COVER_WIDTH).toInt();

    coverSizeSlider->setValue(coverSize);
    setCoversSize(coverSize);
}

void GridComicsView::setToolBar(QToolBar *toolBar)
{
    static_cast<QVBoxLayout *>(this->layout())->insertWidget(1, toolBar);
    this->toolbar = toolBar;

    createCoverSizeSliderWidget();

    toolBarStretchAction = toolBar->addWidget(toolBarStretch);
    toolBar->addAction(showInfoAction);
    showInfoSeparatorAction = toolBar->addSeparator();
    coverSizeSliderAction = toolBar->addWidget(coverSizeSliderWidget);
}

void GridComicsView::setModel(ComicModel *model)
{
    if (model == NULL)
        return;

    ComicsView::setModel(model);

    setCurrentComicIfNeeded();

    selectionHelper->setModel(model);
    comicInfoHelper->setModel(model);

    QQmlContext *ctxt = view->rootContext();

    ctxt->setContextProperty("comicsList", model);
    ctxt->setContextProperty("comicsSelection", selectionHelper->selectionModel());
    ctxt->setContextProperty("contextMenuHelper", this);
    ctxt->setContextProperty("comicsSelectionHelper", selectionHelper);
    ctxt->setContextProperty("currentIndexHelper", this);
    ctxt->setContextProperty("comicRatingHelper", this);
    ctxt->setContextProperty("dummyValue", true);
    ctxt->setContextProperty("dragManager", this);
    ctxt->setContextProperty("dropManager", this);
    ctxt->setContextProperty("comicInfoHelper", comicInfoHelper);

    auto grid = view->rootObject()->findChild<QQuickItem *>(QStringLiteral("grid"));

    if (grid != nullptr) {
        grid->setProperty("currentIndex", 0);
    }

    updateBackgroundConfig();

    selectionHelper->clear();

    if (model->rowCount() > 0) {
        setCurrentIndex(model->index(0, 0));
        if (showInfoAction->isChecked())
            updateInfoForIndex(0);
    }

    //If the currentComicView was hidden before showing it sometimes the scroll view doesn't show it
    //this is a hacky solution...
    QTimer::singleShot(0, this, SLOT(resetScroll()));
}

void GridComicsView::updateBackgroundConfig()
{
    if (this->model == NULL)
        return;

    QQmlContext *ctxt = view->rootContext();

    //backgroun image configuration
    bool useBackgroundImage = settings->value(USE_BACKGROUND_IMAGE_IN_GRID_VIEW, true).toBool();

    if (useBackgroundImage && this->model->rowCount() > 0) {
        float opacity = settings->value(OPACITY_BACKGROUND_IMAGE_IN_GRID_VIEW, 0.2).toFloat();
        float blurRadius = settings->value(BLUR_RADIUS_BACKGROUND_IMAGE_IN_GRID_VIEW, 75).toInt();

        int row = settings->value(USE_SELECTED_COMIC_COVER_AS_BACKGROUND_IMAGE_IN_GRID_VIEW, false).toBool() ? currentIndex().row() : 0;

        ctxt->setContextProperty("backgroundImage", this->model->data(this->model->index(row, 0), ComicModel::CoverPathRole));
        ctxt->setContextProperty("backgroundBlurOpacity", opacity);
        ctxt->setContextProperty("backgroundBlurRadius", blurRadius);
        ctxt->setContextProperty("backgroundBlurVisible", true);
    } else {
        ctxt->setContextProperty("backgroundImage", QUrl());
        ctxt->setContextProperty("backgroundBlurOpacity", 0.0);
        ctxt->setContextProperty("backgroundBlurRadius", 0.0);
        ctxt->setContextProperty("backgroundBlurVisible", false);
    }

#ifdef Q_OS_MAC
    ctxt->setContextProperty("cellColor", useBackgroundImage ? "#99FFFFFF" : "#FFFFFF");
    ctxt->setContextProperty("selectedColor", "#FFFFFF");
#else
    ctxt->setContextProperty("cellColor", useBackgroundImage ? "#99212121" : "#212121");
    ctxt->setContextProperty("selectedColor", "#121212");
#endif
}

void GridComicsView::showInfo()
{
    QQmlContext *ctxt = view->rootContext();
    ctxt->setContextProperty("showInfo", showInfoAction->isChecked());

    updateInfoForIndex(currentIndex().row());
}

void GridComicsView::setCurrentIndex(const QModelIndex &index)
{
    selectionHelper->clear();
    selectionHelper->selectIndex(index.row());

    if (settings->value(USE_SELECTED_COMIC_COVER_AS_BACKGROUND_IMAGE_IN_GRID_VIEW, false).toBool())
        updateBackgroundConfig();

    if (showInfoAction->isChecked())
        updateInfoForIndex(index.row());
}

void GridComicsView::setCurrentIndex(int index)
{
    setCurrentIndex(model->index(index, 0));
}

QModelIndex GridComicsView::currentIndex()
{
    return selectionHelper->currentIndex();
}

QItemSelectionModel *GridComicsView::selectionModel()
{
    return selectionHelper->selectionModel();
}

void GridComicsView::scrollTo(const QModelIndex &mi, QAbstractItemView::ScrollHint hint)
{
    Q_UNUSED(mi);
    Q_UNUSED(hint);
}

void GridComicsView::toFullScreen()
{
    toolbar->hide();
}

void GridComicsView::toNormal()
{
    toolbar->show();
}

void GridComicsView::updateConfig(QSettings *settings)
{
    Q_UNUSED(settings);
}

void GridComicsView::enableFilterMode(bool enabled)
{
    filterEnabled = enabled;

    QQmlContext *ctxt = view->rootContext();

    if (enabled) {
        ctxt->setContextProperty("showCurrentComic", false);
        ctxt->setContextProperty("currentComic", nullptr);
    } else {
        setCurrentComicIfNeeded();
    }
}

void GridComicsView::selectAll()
{
    selectionHelper->selectAll();
}

void GridComicsView::selectIndex(int index)
{
    selectionHelper->selectIndex(index);
}

void GridComicsView::triggerOpenCurrentComic()
{
    emit openComic();
}

void GridComicsView::rate(int index, int rating)
{
    model->updateRating(rating, model->index(index, 0));
}

void GridComicsView::requestedContextMenu(const QPoint &point)
{
    emit customContextMenuViewRequested(point);
}

void GridComicsView::setCoversSize(int width)
{
    QQmlContext *ctxt = view->rootContext();

    auto grid = view->rootObject()->findChild<QQuickItem *>(QStringLiteral("grid"));

    if (grid != 0) {
        QVariant cellCustomWidth = (width * YACREADER_MIN_CELL_CUSTOM_WIDTH) / YACREADER_MIN_GRID_ZOOM_WIDTH;
        QMetaObject::invokeMethod(grid, "calculateCellWidths",
                                  Q_ARG(QVariant, cellCustomWidth));
    }

    int cellBottomMarging = 8 * (1 + 2 * (1 - (float(YACREADER_MAX_GRID_ZOOM_WIDTH - width) / (YACREADER_MAX_GRID_ZOOM_WIDTH - YACREADER_MIN_GRID_ZOOM_WIDTH))));

    ctxt->setContextProperty("cellCustomHeight", ((width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH) + 51 + cellBottomMarging);
    ctxt->setContextProperty("cellCustomWidth", (width * YACREADER_MIN_CELL_CUSTOM_WIDTH) / YACREADER_MIN_COVER_WIDTH);

    ctxt->setContextProperty("itemWidth", width);
    ctxt->setContextProperty("itemHeight", ((width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH) + 51);

    ctxt->setContextProperty("coverWidth", width);
    ctxt->setContextProperty("coverHeight", (width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH);
}

void GridComicsView::dummyUpdater()
{
    QQmlContext *ctxt = view->rootContext();
    ctxt->setContextProperty("dummyValue", true);
}

void GridComicsView::setCurrentComicIfNeeded()
{
    bool found;
    currentComic = currentComicFromModel(model, found);

    QQmlContext *ctxt = view->rootContext();

    ComicModel::Mode mode = model->getMode();

    bool showCurrentComic = found &&
            filterEnabled == false &&
            (mode == ComicModel::Mode::Folder || mode == ComicModel::Mode::ReadingList) &&
            settings->value(DISPLAY_CONTINUE_READING_IN_GRID_VIEW, true).toBool();

    if (showCurrentComic) {
        ctxt->setContextProperty("currentComic", &currentComic);
        ctxt->setContextProperty("currentComicInfo", &(currentComic.info));
        ctxt->setContextProperty("showCurrentComic", true);
    } else {
        ctxt->setContextProperty("currentComic", &currentComic);
        ctxt->setContextProperty("currentComicInfo", &(currentComic.info));
        ctxt->setContextProperty("showCurrentComic", false);
        //ctxt->setContextProperty("currentComic", nullptr);
    }
}

void GridComicsView::resetScroll()
{
    auto rootObject = dynamic_cast<QObject *>(view->rootObject());
    auto scrollView = rootObject->findChild<QObject *>("topScrollView", Qt::FindChildrenRecursively);

    QMetaObject::invokeMethod(scrollView, "scrollToOrigin");
}

QByteArray GridComicsView::getMimeDataFromSelection()
{
    QByteArray data;

    QMimeData *mimeData = model->mimeData(selectionHelper->selectedIndexes());
    data = mimeData->data(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat);

    delete mimeData;

    return data;
}

void GridComicsView::updateCurrentComicView()
{
    setCurrentComicIfNeeded();
}

void GridComicsView::focusComicsNavigation(Qt::FocusReason reason)
{
    view->setFocus(reason);
}

void GridComicsView::startDrag()
{
    auto drag = new QDrag(this);
    drag->setMimeData(model->mimeData(selectionHelper->selectedRows()));
    drag->setPixmap(QPixmap(":/images/comics_view_toolbar/openInYACReader.png")); //TODO add better image

    /*Qt::DropAction dropAction =*/drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
}

bool GridComicsView::canDropUrls(const QList<QUrl> &urls, Qt::DropAction action)
{
    if (action == Qt::CopyAction) {
        QString currentPath;
        foreach (QUrl url, urls) {
            //comics or folders are accepted, folders' content is validate in dropEvent (avoid any lag before droping)
            currentPath = url.toLocalFile();
            if (Comic::fileIsComic(currentPath) || QFileInfo(currentPath).isDir())
                return true;
        }
    }
    return false;
}

bool GridComicsView::canDropFormats(const QString &formats)
{
    return (formats.contains(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat) && model->canBeResorted());
}

void GridComicsView::droppedFiles(const QList<QUrl> &urls, Qt::DropAction action)
{
    bool validAction = action == Qt::CopyAction; //TODO add move

    if (validAction) {
        QList<QPair<QString, QString>> droppedFiles = ComicFilesManager::getDroppedFiles(urls);
        emit copyComicsToCurrentFolder(droppedFiles);
    }
}

void GridComicsView::droppedComicsForResortingAt(const QString &data, int index)
{
    Q_UNUSED(data);

    model->dropMimeData(model->mimeData(selectionHelper->selectedRows()), Qt::MoveAction, index, 0, QModelIndex());
}

void GridComicsView::selectedItem(int index)
{
    emit selected(index);
}

void GridComicsView::setShowMarks(bool show)
{
    QQmlContext *ctxt = view->rootContext();
    ctxt->setContextProperty("show_marks", show);
}

void GridComicsView::closeEvent(QCloseEvent *event)
{
    toolbar->removeAction(toolBarStretchAction);
    toolbar->removeAction(showInfoAction);
    toolbar->removeAction(showInfoSeparatorAction);
    toolbar->removeAction(coverSizeSliderAction);

    auto rootObject = dynamic_cast<QObject *>(view->rootObject());
    auto infoContainer = rootObject->findChild<QObject *>("infoContainer", Qt::FindChildrenRecursively);

    int infoWidth = QQmlProperty(infoContainer, "width").read().toInt();

    /*QObject *object = view->rootObject();
    QMetaObject::invokeMethod(object, "exit");
    container->close();
    view->close();*/

    event->accept();
    ComicsView::closeEvent(event);

    //save settings
    settings->setValue(COMICS_GRID_COVER_SIZES, coverSizeSlider->value());
    settings->setValue(COMICS_GRID_SHOW_INFO, showInfoAction->isChecked());
    settings->setValue(COMICS_GRID_INFO_WIDTH, infoWidth);
}
