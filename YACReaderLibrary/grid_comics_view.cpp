#include "grid_comics_view.h"

#include "QsLog.h"
#include "comic.h"
#include "comic_db.h"
#include "comic_files_manager.h"
#include "cover_utils.h"
#include "current_comic_view_helper.h"
#include "folder_model.h"
#include "grid_content_model.h"
#include "reading_list_model.h"
#include "yacreader_comic_info_helper.h"
#include "yacreader_comics_selection_helper.h"
#include "yacreader_global_gui.h"

#include <QApplication>
#include <QBuffer>
#include <QDrag>
#include <QHBoxLayout>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWidget>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <algorithm>

namespace {
QString pixmapDataUrl(const QPixmap &pixmap)
{
    if (pixmap.isNull())
        return { };

    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    return QStringLiteral("data:image/png;base64,") + QString::fromLatin1(data.toBase64());
}
} // namespace

GridComicsView::GridComicsView(QWidget *parent)
    : ComicsView(parent), toolbar(nullptr), coverSizeSliderWidget(nullptr), coverSizeSlider(nullptr), coverSizeSliderAction(nullptr), showInfoSeparatorAction(nullptr), startSeparatorAction(nullptr), filterEnabled(false), contentModel(new GridContentModel(this)), viewStateTimer(new QTimer(this)), smallZoomLabel(nullptr), bigZoomLabel(nullptr)
{
    qmlRegisterUncreatableType<GridContentModel>("com.yacreader.GridContentModel", 1, 0, "GridContentModel", QStringLiteral("GridContentModel is provided by GridComicsView"));

    settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat, this);
    settings->beginGroup("libraryConfig");

    // view->setFocusPolicy(Qt::TabFocus);

    selectionHelper = new YACReaderComicsSelectionHelper(this);
    connect(selectionHelper, &YACReaderComicsSelectionHelper::selectionChanged, this, [this]() {
        emit comicSelectionStateChanged(selectionHelper->numItemsSelected() > 0);
        emit selectedComicsInfoChanged();
    });

    comicInfoHelper = new YACReaderComicInfoHelper(this);

    QQmlContext *ctxt = view->rootContext();

    // fonts settings (not theme-dependent)
    int fontSize = QApplication::font().pointSize();
    if (fontSize == -1)
        fontSize = QApplication::font().pixelSize();
    ctxt->setContextProperty("fontSize", fontSize);
    ctxt->setContextProperty("fontFamily", QApplication::font().family());
    ctxt->setContextProperty("fontSpacing", 0.5);

    ctxt->setContextProperty("backgroundImage", QUrl());
    ctxt->setContextProperty("backgroundBlurOpacity", 0.0);
    ctxt->setContextProperty("backgroundBlurRadius", 0.0);
    ctxt->setContextProperty("backgroundBlurVisible", QVariant(false));

    auto model = new ComicModel();
    selectionHelper->setModel(model);
    contentModel->setComicModel(model);
    connect(contentModel, &QAbstractItemModel::modelReset, this, [this]() {
        if (focusedFolderIndex.isValid())
            setFocusedFolder(focusedFolderIndex.row());
        else
            clearFocusedFolder();
    });
    connect(contentModel, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
        const auto focusedRow = focusedFolderRow();
        if (focusedRow >= topLeft.row() && focusedRow <= bottomRight.row())
            setFocusedFolder(focusedRow);
    });
    ctxt->setContextProperty("comicsList", contentModel);
    ctxt->setContextProperty("comicsSelection", selectionHelper->selectionModel());
    ctxt->setContextProperty("contextMenuHelper", this);
    ctxt->setContextProperty("comicsSelectionHelper", selectionHelper);
    ctxt->setContextProperty("currentIndexHelper", this);
    ctxt->setContextProperty("comicRatingHelper", this);
    ctxt->setContextProperty("dragManager", this);
    ctxt->setContextProperty("dropManager", this);
    ctxt->setContextProperty("comicOpener", this);
    rootContinueReadingModelStorage = std::make_unique<ComicModel>();
    globalContinueReadingEnabled = settings->value(DISPLAY_GLOBAL_CONTINUE_READING_IN_GRID_VIEW, true).toBool();
    contentModel->setMixFoldersAndComics(settings->value(COMICS_GRID_MIX_FOLDERS_AND_COMICS, true).toBool());
    contentModel->setStartComicsOnNewRow(settings->value(COMICS_GRID_START_COMICS_ON_NEW_ROW, false).toBool());

    viewStateTimer->setSingleShot(true);
    connect(viewStateTimer, &QTimer::timeout, this, &GridComicsView::applyPendingViewState);

    bool showInfo = settings->value(COMICS_GRID_SHOW_INFO, false).toBool();
    ctxt->setContextProperty("showInfo", showInfo);

    ctxt->setContextProperty("currentComic", &currentComic);
    ctxt->setContextProperty("currentComicInfo", &currentComic.info);

    showInfoAction = new QAction(tr("Show info"), this);
    showInfoAction->setCheckable(true);
    showInfoAction->setChecked(showInfo);
    connect(showInfoAction, &QAction::toggled, this, &GridComicsView::updateInfoPanelVisibility);

    updateCoversSizeInContext(YACREADER_MIN_COVER_WIDTH, ctxt);

    // Seed theme globals before loading QML so the first binding pass does not
    // resolve them as undefined and spam startup warnings.
    initTheme(this);

    view->setSource(QUrl("qrc:/qml/GridComicsView.qml"));

    if (auto *rootObject = view->rootObject()) {
        auto infoContainer = rootObject->findChild<QObject *>("infoContainer");
        QQmlProperty(infoContainer, "width").write(settings->value(COMICS_GRID_INFO_WIDTH, 350));
    }

    setShowMarks(true); // TODO save this in settings

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
    coverSizeSliderWidget = new QWidget(this);
    coverSizeSliderWidget->setFixedWidth(200);
    coverSizeSlider = new QSlider();
    coverSizeSlider->setOrientation(Qt::Horizontal);
    coverSizeSlider->setRange(YACREADER_MIN_GRID_ZOOM_WIDTH, YACREADER_MAX_GRID_ZOOM_WIDTH);

    const auto &comicsToolbar = theme.comicsViewToolbar;

    auto horizontalLayout = new QHBoxLayout();
    smallZoomLabel = new QLabel();
    smallZoomLabel->setPixmap(comicsToolbar.smallGridZoomIcon.pixmap(18, 18));
    horizontalLayout->addWidget(smallZoomLabel);
    horizontalLayout->addWidget(coverSizeSlider, 0, Qt::AlignVCenter);
    bigZoomLabel = new QLabel();
    bigZoomLabel->setPixmap(comicsToolbar.bigGridZoomIcon.pixmap(18, 18));
    horizontalLayout->addWidget(bigZoomLabel);
    horizontalLayout->addSpacing(10);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    coverSizeSliderWidget->setLayout(horizontalLayout);
    // TODO add shortcuts (ctrl-+ and ctrl-- for zooming in out, + ctrl-0 for reseting the zoom)

    connect(coverSizeSlider, &QAbstractSlider::valueChanged, this, &GridComicsView::setCoversSize);

    int coverSize = settings->value(COMICS_GRID_COVER_SIZES, YACREADER_MIN_COVER_WIDTH).toInt();

    coverSizeSlider->setValue(coverSize);
    setCoversSize(coverSize);
}

void GridComicsView::setToolBar(QToolBar *toolBar)
{
    static_cast<QVBoxLayout *>(this->layout())->insertWidget(1, toolBar);
    toolbar = toolBar;

    if (!coverSizeSliderWidget)
        createCoverSizeSliderWidget();

    if (!startSeparatorAction) {
        startSeparatorAction = new QAction(this);
        startSeparatorAction->setSeparator(true);
    }
    if (!showInfoSeparatorAction) {
        showInfoSeparatorAction = new QAction(this);
        showInfoSeparatorAction->setSeparator(true);
    }
    if (!coverSizeSliderAction) {
        auto *sliderAction = new QWidgetAction(this);
        sliderAction->setDefaultWidget(coverSizeSliderWidget);
        coverSizeSliderAction = sliderAction;
    }

    const auto actions = toolbar->actions();
    if (!actions.contains(startSeparatorAction))
        toolbar->addAction(startSeparatorAction);
    if (!actions.contains(showInfoAction))
        toolbar->addAction(showInfoAction);
    if (!actions.contains(showInfoSeparatorAction))
        toolbar->addAction(showInfoSeparatorAction);
    if (!actions.contains(coverSizeSliderAction))
        toolbar->addAction(coverSizeSliderAction);
}

void GridComicsView::releaseToolBar()
{
    if (!toolbar)
        return;

    toolbar->removeAction(startSeparatorAction);
    toolbar->removeAction(showInfoAction);
    toolbar->removeAction(showInfoSeparatorAction);
    toolbar->removeAction(coverSizeSliderAction);
}

void GridComicsView::saveViewConfig()
{
    int infoWidth = 0;
    if (auto *rootObject = view->rootObject()) {
        auto infoContainer = rootObject->findChild<QObject *>("infoContainer", Qt::FindChildrenRecursively);
        infoWidth = QQmlProperty(infoContainer, "width").read().toInt();
    }

    if (coverSizeSlider)
        settings->setValue(COMICS_GRID_COVER_SIZES, coverSizeSlider->value());
    settings->setValue(COMICS_GRID_SHOW_INFO, showInfoAction->isChecked());
    settings->setValue(COMICS_GRID_INFO_WIDTH, infoWidth);
}

void GridComicsView::setModel(ComicModel *model)
{
    if (model == nullptr)
        return;

    // Keep the previous frame visible while QML resets the model. The pending
    // origin/anchor is applied before painting is enabled again.
    view->setUpdatesEnabled(false);

    clearFocusedFolder();
    disconnect(modelDataChangedConnection);
    disconnect(modelFavoritesChangedConnection);
    ComicsView::setModel(model);

    modelDataChangedConnection = connect(model, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
        if (selectionHelper->numItemsSelected() > 1) {
            const auto selectedRows = selectionHelper->selectedRows();
            const bool selectedComicChanged = std::any_of(selectedRows.cbegin(), selectedRows.cend(), [topLeft, bottomRight](const QModelIndex &index) {
                return index.row() >= topLeft.row() && index.row() <= bottomRight.row();
            });
            if (selectedComicChanged)
                emit selectedComicsInfoChanged();
        }

        if (!showInfoAction->isChecked() || focusedFolderIndex.isValid())
            return;

        const auto index = currentIndex();
        if (index.isValid() && index.row() >= topLeft.row() && index.row() <= bottomRight.row())
            updateInfoForIndex(index.row());
    });

    modelFavoritesChangedConnection = connect(model, &ComicModel::favoritesChanged, this, [this](const QList<qulonglong> &comicIds) {
        if (!showInfoAction->isChecked() || focusedFolderIndex.isValid())
            return;

        const auto index = currentIndex();
        if (index.isValid() && comicIds.contains(index.data(ComicModel::IdRole).toULongLong()))
            updateInfoForIndex(index.row());
    });

    updateCurrentComicBanner();

    selectionHelper->setModel(model);
    comicInfoHelper->setModel(model);
    contentModel->setComicModel(model);

    if (model->getMode() != ComicModel::Folder)
        clearFolderModel();

    QQmlContext *ctxt = view->rootContext();

    ctxt->setContextProperty("comicsList", contentModel);
    ctxt->setContextProperty("comicsSelection", selectionHelper->selectionModel());
    ctxt->setContextProperty("contextMenuHelper", this);
    ctxt->setContextProperty("comicsSelectionHelper", selectionHelper);
    ctxt->setContextProperty("currentIndexHelper", this);
    ctxt->setContextProperty("comicRatingHelper", this);
    ctxt->setContextProperty("dragManager", this);
    ctxt->setContextProperty("dropManager", this);
    ctxt->setContextProperty("comicInfoHelper", comicInfoHelper);

    auto *root = view->rootObject();
    auto grid = root ? root->findChild<QQuickItem *>(QStringLiteral("grid")) : nullptr;

    if (grid != nullptr)
        grid->setProperty("currentIndex", -1);

    updateBackgroundConfig();

    selectionHelper->clear();
    updateInfoForIndex(-1);

    pendingViewState.reset();
    viewStateTimer->start(0);
}

void GridComicsView::updateBackgroundConfig()
{
    if (this->model == nullptr)
        return;

    QQmlContext *ctxt = view->rootContext();

    // backgroun image configuration
    const bool useBackgroundImage = settings->value(USE_BACKGROUND_IMAGE_IN_GRID_VIEW, true).toBool();
    const bool hasBackgroundComic = this->model->rowCount() > 0;
    const bool showBackgroundImage = useBackgroundImage && hasBackgroundComic;

    if (showBackgroundImage) {
        float opacity = settings->value(OPACITY_BACKGROUND_IMAGE_IN_GRID_VIEW, 0.2).toFloat();
        float blurRadius = settings->value(BLUR_RADIUS_BACKGROUND_IMAGE_IN_GRID_VIEW, 75).toInt();

        const auto selectedIndex = currentIndex();
        int row = settings->value(USE_SELECTED_COMIC_COVER_AS_BACKGROUND_IMAGE_IN_GRID_VIEW, false).toBool() && selectedIndex.isValid() ? selectedIndex.row() : 0;

        ctxt->setContextProperty("backgroundImage", this->model->data(this->model->index(row, 0), ComicModel::CoverPathRole));
        ctxt->setContextProperty("backgroundBlurOpacity", opacity);
        ctxt->setContextProperty("backgroundBlurRadius", blurRadius);
        ctxt->setContextProperty("backgroundBlurVisible", true);
    } else {
        ctxt->setContextProperty("backgroundImage", QUrl());
        ctxt->setContextProperty("backgroundBlurOpacity", 0.0);
        ctxt->setContextProperty("backgroundBlurRadius", 0.0);
        ctxt->setContextProperty("backgroundBlurVisible", QVariant(false));
    }

    // Use theme colors for cell and selected colors
    const auto &giv = theme.gridAndInfoView;
    ctxt->setContextProperty("backgroundColor", showBackgroundImage ? giv.backgroundBlurOverlayColor : giv.backgroundColor);
    ctxt->setContextProperty("cellColor", showBackgroundImage ? giv.cellColorWithBackground : giv.cellColor);
    ctxt->setContextProperty("cellSelectedColor", giv.cellSelectedColor);
}

void GridComicsView::updateInfoPanelVisibility()
{
    QQmlContext *ctxt = view->rootContext();
    ctxt->setContextProperty("showInfo", showInfoAction->isChecked());

    if (!focusedFolderIndex.isValid())
        updateInfoForIndex(currentIndex().row());
}

void GridComicsView::setCurrentIndex(const QModelIndex &index)
{
    clearFocusedFolder();
    selectionHelper->selectOnly(index.row());

    auto *root = view->rootObject();
    auto grid = root ? root->findChild<QQuickItem *>(QStringLiteral("grid")) : nullptr;
    if (grid)
        grid->setProperty("currentIndex", contentModel->viewRowForComicRow(index.row()));

    if (settings->value(USE_SELECTED_COMIC_COVER_AS_BACKGROUND_IMAGE_IN_GRID_VIEW, false).toBool())
        updateBackgroundConfig();

    if (showInfoAction->isChecked())
        updateInfoForIndex(index.row());
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
        if (currentComicBannerVisible) {
            currentComicBannerVisible = false;
            emit currentComicBannerVisibleChanged();
        }
        ctxt->setContextProperty("currentComic", nullptr);
    } else {
        updateCurrentComicBanner();
    }
}

void GridComicsView::selectAll()
{
    clearFocusedFolder();
    selectionHelper->selectAll();
}

void GridComicsView::selectIndex(int index)
{
    clearFocusedFolder();
    selectionHelper->selectIndex(index);
}

void GridComicsView::triggerOpenCurrentComic()
{
    if (model == nullptr) {
        return;
    }

    emit openComic(currentComic, model->getMode());
}

void GridComicsView::updateSettings()
{
    contentModel->setMixFoldersAndComics(settings->value(COMICS_GRID_MIX_FOLDERS_AND_COMICS, true).toBool());
    contentModel->setStartComicsOnNewRow(settings->value(COMICS_GRID_START_COMICS_ON_NEW_ROW, false).toBool());
    if (currentLocationInfo.value(QStringLiteral("kind")).toString() == QStringLiteral("recent")) {
        currentLocationInfo.insert(QStringLiteral("recentDays"), settings->value(NUM_DAYS_TO_CONSIDER_RECENT, 1).toInt());
        emit currentLocationInfoChanged();
    }
    updateBannerSettings();
    updateBackgroundConfig();
}

void GridComicsView::updateBannerSettings()
{
    const bool enabled = settings->value(DISPLAY_GLOBAL_CONTINUE_READING_IN_GRID_VIEW, true).toBool();
    if (globalContinueReadingEnabled != enabled) {
        globalContinueReadingEnabled = enabled;
        emit globalContinueReadingEnabledChanged();
    }

    updateCurrentComicBanner();
}

void GridComicsView::rate(int index, int rating)
{
    model->updateRating(rating, model->index(index, 0));
}

void GridComicsView::requestItemContextMenu(const QPoint &point, int viewRow)
{
    if (contentModel->isFolderRow(viewRow)) {
        emit openFolderContextMenu(point, contentModel->folderAt(viewRow));
        return;
    }

    emit customContextMenuViewRequested(point);
}

void GridComicsView::requestOpenLibraryFolder()
{
    emit openLibraryFolderRequested();
}

void GridComicsView::setFolderModel(FolderModel *model, const QModelIndex &folderIndex, const QString &rootName, const QVariantMap &libraryInfo)
{
    clearFocusedFolder();
    contentModel->setFolderModel(model, folderIndex);
    const bool selectedFolderIsRoot = !folderIndex.isValid();
    if (rootFolder != selectedFolderIsRoot) {
        rootFolder = selectedFolderIsRoot;
        emit rootFolderChanged();
    }

    if (selectedFolderIsRoot) {
        currentLocationInfo = libraryInfo;
        currentLocationInfo.insert(QStringLiteral("kind"), QStringLiteral("library"));
        currentLocationInfo.insert(QStringLiteral("name"), rootName);
    } else {
        const auto folder = model->getFolder(folderIndex);
        const auto cover = folder.customImage.isEmpty() ? model->getCoverUrlPathForComicHash(folder.firstChildHash) : model->getCoverUrlPathForFolderId(folder.id);
        currentLocationInfo = makeFolderInfo(folder, cover);
    }
    emit currentLocationInfoChanged();
}

void GridComicsView::clearFolderModel()
{
    clearFocusedFolder();
    contentModel->clearFolderModel();
    if (rootFolder) {
        rootFolder = false;
        emit rootFolderChanged();
    }
}

void GridComicsView::setCurrentList(const QModelIndex &listIndex)
{
    const auto listType = static_cast<ReadingListModel::TypeList>(listIndex.data(ReadingListModel::TypeListsRole).toInt());
    QString kind;
    int labelColor = -1;
    int recentDays = 0;
    int sublistCount = 0;

    switch (listType) {
    case ReadingListModel::SpecialList: {
        const auto specialType = static_cast<ReadingListModel::TypeSpecialList>(listIndex.data(ReadingListModel::SpecialListTypeRole).toInt());
        switch (specialType) {
        case ReadingListModel::TypeSpecialList::Favorites:
            kind = QStringLiteral("favorites");
            break;
        case ReadingListModel::TypeSpecialList::Reading:
            kind = QStringLiteral("reading");
            break;
        case ReadingListModel::TypeSpecialList::Recent:
            kind = QStringLiteral("recent");
            recentDays = settings->value(NUM_DAYS_TO_CONSIDER_RECENT, 1).toInt();
            break;
        }
        break;
    }
    case ReadingListModel::Label: {
        kind = QStringLiteral("tag");
        labelColor = listIndex.data(ReadingListModel::LabelColorRole).toInt();
        break;
    }
    case ReadingListModel::ReadingList: {
        kind = QStringLiteral("readingList");
        sublistCount = listIndex.model()->rowCount(listIndex);
        break;
    }
    case ReadingListModel::Separator:
        return;
    }

    currentLocationInfo = {
        { QStringLiteral("kind"), kind },
        { QStringLiteral("name"), listIndex.data(Qt::DisplayRole).toString() },
        { QStringLiteral("itemCount"), model ? model->rowCount() : 0 },
        { QStringLiteral("labelColor"), labelColor },
        { QStringLiteral("recentDays"), recentDays },
        { QStringLiteral("sublistCount"), sublistCount },
    };
    updateCurrentListIcon();
    emit currentLocationInfoChanged();
}

void GridComicsView::updateCurrentListIcon()
{
    const auto kind = currentLocationInfo.value(QStringLiteral("kind")).toString();
    QPixmap icon;

    if (kind == QStringLiteral("favorites"))
        icon = theme.emptyContainer.emptyFavoritesIcon;
    else if (kind == QStringLiteral("reading"))
        icon = theme.emptyContainer.emptyCurrentReadingsIcon;
    else if (kind == QStringLiteral("recent"))
        icon = theme.emptyContainer.emptyRecentIcon;
    else if (kind == QStringLiteral("tag"))
        icon = theme.emptyContainer.emptyLabelIcons.value(currentLocationInfo.value(QStringLiteral("labelColor")).toInt());
    else if (kind == QStringLiteral("readingList"))
        icon = theme.emptyContainer.emptyReadingListIcon;
    else
        return;

    currentLocationInfo.insert(QStringLiteral("icon"), pixmapDataUrl(icon));
}

void GridComicsView::setRootContinueReadingModel(std::unique_ptr<ComicModel> model)
{
    rootContinueReadingModelStorage = std::move(model);
    emit rootContinueReadingModelChanged();
}

void GridComicsView::clearRootContinueReadingModel()
{
    setRootContinueReadingModel(nullptr);
}

ComicModel *GridComicsView::rootContinueReadingModel() const
{
    return rootContinueReadingModelStorage.get();
}

bool GridComicsView::isRootFolder() const
{
    return rootFolder;
}

bool GridComicsView::isGlobalContinueReadingEnabled() const
{
    return globalContinueReadingEnabled;
}

bool GridComicsView::isCurrentComicBannerVisible() const
{
    return currentComicBannerVisible;
}

int GridComicsView::focusedFolderRow() const
{
    return focusedFolderIndex.isValid() ? focusedFolderIndex.row() : -1;
}

QVariantMap GridComicsView::folderInfoForFocusedFolder() const
{
    return focusedFolderInfo;
}

QVariantMap GridComicsView::locationInfo() const
{
    return currentLocationInfo;
}

bool GridComicsView::hasComicSelection() const
{
    return selectionHelper->numItemsSelected() > 0;
}

int GridComicsView::selectedComicCount() const
{
    return selectionHelper->numItemsSelected();
}

QVariantMap GridComicsView::selectedComicsInfo() const
{
    return selectionHelper->selectionInfo();
}

void GridComicsView::reloadRootContinueReadingModel()
{
    if (rootFolder && rootContinueReadingModelStorage)
        rootContinueReadingModelStorage->reloadContinueReading();
}

ContentViewState GridComicsView::captureViewState() const
{
    ContentViewState state;
    auto *rootObject = view->rootObject();
    auto *scrollView = rootObject ? rootObject->findChild<QObject *>(QStringLiteral("topScrollView"), Qt::FindChildrenRecursively) : nullptr;
    if (!scrollView)
        return state;

    QVariant position;
    QMetaObject::invokeMethod(scrollView, "capturePosition", Q_RETURN_ARG(QVariant, position));
    const auto values = position.toMap();
    const auto viewRow = values.value(QStringLiteral("viewRow"), -1).toInt();

    state.offset = values.value(QStringLiteral("offset")).toReal();
    state.itemExtent = values.value(QStringLiteral("itemExtent")).toReal();
    if (model && model->rowCount() > 0)
        state.fallbackComicRow = qBound(0, contentModel->sourceComicRow(viewRow), model->rowCount() - 1);

    if (values.value(QStringLiteral("header")).toBool()) {
        state.topItem.kind = ContentItemRef::Header;
    } else if (viewRow >= 0 && viewRow < contentModel->rowCount()) {
        const auto index = contentModel->index(viewRow, 0);
        const auto kind = contentModel->data(index, GridContentModel::ItemKindRole).toInt();
        state.topItem.kind = kind == GridContentModel::FolderItem ? ContentItemRef::Folder : ContentItemRef::Comic;
        state.topItem.id = contentModel->data(index, GridContentModel::IdRole).toULongLong();
    }

    if (focusedFolderIndex.isValid()) {
        state.currentItem.kind = ContentItemRef::Folder;
        state.currentItem.id = focusedFolderIndex.data(FolderModel::IdRole).toULongLong();
    } else if (const auto index = selectionHelper->currentIndex(); index.isValid()) {
        state.currentItem.kind = ContentItemRef::Comic;
        state.currentItem.id = index.data(ComicModel::IdRole).toULongLong();
    }

    return state;
}

void GridComicsView::restoreViewState(const ContentViewState &state)
{
    pendingViewState = state;
    viewStateTimer->start(0);
}

void GridComicsView::openContinueReadingComic(int sourceRow)
{
    if (!rootContinueReadingModelStorage || sourceRow < 0 || sourceRow >= rootContinueReadingModelStorage->rowCount())
        return;

    emit openComic(rootContinueReadingModelStorage->getComic(rootContinueReadingModelStorage->index(sourceRow, 0)), ComicModel::Folder);
}

void GridComicsView::requestContinueReadingComicContextMenu(const QPoint &point, int sourceRow)
{
    if (!rootContinueReadingModelStorage || sourceRow < 0 || sourceRow >= rootContinueReadingModelStorage->rowCount())
        return;

    emit openContinueReadingComicContextMenu(point, rootContinueReadingModelStorage->getComic(rootContinueReadingModelStorage->index(sourceRow, 0)));
}

void GridComicsView::openFolder(int viewRow)
{
    const QPersistentModelIndex sourceIndex(contentModel->sourceFolderIndex(viewRow));
    if (!sourceIndex.isValid())
        return;

    // setupFolderModelData()/setFolderModel() reset the model that owns the QML
    // delegate. Defer navigation until Qt Quick finishes dispatching the event.
    QTimer::singleShot(0, this, [this, sourceIndex]() {
        if (sourceIndex.isValid())
            emit folderSelected(sourceIndex);
    });
}

void GridComicsView::focusItem(int viewRow)
{
    if (contentModel->isSpacerRow(viewRow))
        return;

    if (contentModel->isFolderRow(viewRow)) {
        selectionHelper->clear();
        setFocusedFolder(viewRow);
        return;
    }

    const auto sourceRow = contentModel->sourceComicRow(viewRow);
    if (sourceRow >= 0 && model && sourceRow < model->rowCount())
        setCurrentIndex(model->index(sourceRow, 0));
}

void GridComicsView::selectComicRange(int from, int to)
{
    clearFocusedFolder();

    if (from > to)
        std::swap(from, to);

    const auto firstComic = qMax(from, contentModel->viewRowForComicRow(0));
    const auto lastComic = qMin(to, contentModel->rowCount() - 1);
    for (auto row = firstComic; row <= lastComic; ++row)
        selectionHelper->selectIndex(contentModel->sourceComicRow(row));
}

int GridComicsView::viewRowForComicRow(int sourceRow) const
{
    return contentModel->viewRowForComicRow(sourceRow);
}

void GridComicsView::setGridColumnCount(int columns)
{
    contentModel->setGridColumnCount(columns);
}

int GridComicsView::nearestSelectableRow(int viewRow, int direction) const
{
    if (!contentModel->isSpacerRow(viewRow))
        return viewRow;

    return direction < 0 ? contentModel->visibleFolderCount() - 1 : contentModel->viewRowForComicRow(0);
}

void GridComicsView::setCoversSize(int width)
{
    QQmlContext *ctxt = view->rootContext();

    auto *root = view->rootObject();
    auto grid = root ? root->findChild<QQuickItem *>(QStringLiteral("grid")) : nullptr;

    if (grid != 0) {
        QVariant cellCustomWidth = (width * YACREADER_MIN_CELL_CUSTOM_WIDTH) / YACREADER_MIN_GRID_ZOOM_WIDTH;
        QMetaObject::invokeMethod(grid, "calculateCellWidths",
                                  Q_ARG(QVariant, cellCustomWidth));
    }

    updateCoversSizeInContext(width, ctxt);

    settings->setValue(COMICS_GRID_COVER_SIZES, coverSizeSlider->value());
}

void GridComicsView::updateCoversSizeInContext(int width, QQmlContext *ctxt)
{
    int cellBottomMarging = 8 * (1 + 2 * (1 - (float(YACREADER_MAX_GRID_ZOOM_WIDTH - width) / (YACREADER_MAX_GRID_ZOOM_WIDTH - YACREADER_MIN_GRID_ZOOM_WIDTH))));

    int infoHeight = 56;

    ctxt->setContextProperty("cellCustomHeight", ((width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH) + infoHeight + cellBottomMarging);
    ctxt->setContextProperty("cellCustomWidth", (width * YACREADER_MIN_CELL_CUSTOM_WIDTH) / YACREADER_MIN_COVER_WIDTH);

    ctxt->setContextProperty("itemWidth", width);
    ctxt->setContextProperty("itemHeight", ((width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH) + infoHeight);

    ctxt->setContextProperty("coverWidth", width);
    ctxt->setContextProperty("coverHeight", (width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH);
}

void GridComicsView::updateCurrentComicBanner()
{
    if (model == nullptr) {
        return;
    }

    bool found;
    currentComic = currentComicFromModel(model, found);

    QQmlContext *ctxt = view->rootContext();

    ComicModel::Mode mode = model->getMode();

    const bool showCurrentComic = found &&
            filterEnabled == false &&
            (mode == ComicModel::Mode::Folder || mode == ComicModel::Mode::ReadingList) &&
            settings->value(DISPLAY_CONTINUE_READING_IN_GRID_VIEW, true).toBool();

    ctxt->setContextProperty("currentComic", &currentComic);
    ctxt->setContextProperty("currentComicInfo", &(currentComic.info));
    if (currentComicBannerVisible != showCurrentComic) {
        currentComicBannerVisible = showCurrentComic;
        emit currentComicBannerVisibleChanged();
    }
}

void GridComicsView::clearFolderFocus()
{
    clearFocusedFolder();
}

QVariantMap GridComicsView::makeFolderInfo(const Folder &folder, const QVariant &cover) const
{
    QString typeName;
    switch (folder.type) {
    case YACReader::FileType::Manga:
        typeName = tr("Manga");
        break;
    case YACReader::FileType::WesternManga:
        typeName = tr("Western manga");
        break;
    case YACReader::FileType::WebComic:
        typeName = tr("Web comic");
        break;
    case YACReader::FileType::Yonkoma:
        typeName = tr("Yonkoma");
        break;
    case YACReader::FileType::Comic:
    default:
        typeName = tr("Comic");
        break;
    }

    const QVariant itemCount = folder.numChildren < 0 ? QVariant(tr("Unknown")) : QVariant(folder.numChildren);
    return {
        { QStringLiteral("kind"), QStringLiteral("folder") },
        { QStringLiteral("name"), folder.name },
        { QStringLiteral("path"), folder.path },
        { QStringLiteral("cover"), cover },
        { QStringLiteral("itemCount"), itemCount },
        { QStringLiteral("typeName"), typeName },
        { QStringLiteral("finished"), folder.finished },
        { QStringLiteral("completed"), folder.completed },
        { QStringLiteral("added"), folder.added },
        { QStringLiteral("updated"), folder.updated },
    };
}

void GridComicsView::setFocusedFolder(int viewRow)
{
    if (!contentModel->isFolderRow(viewRow)) {
        clearFocusedFolder();
        return;
    }

    const auto sourceIndex = contentModel->sourceFolderIndex(viewRow);
    if (!sourceIndex.isValid()) {
        clearFocusedFolder();
        return;
    }

    focusedFolderIndex = sourceIndex;
    focusedFolderInfo = makeFolderInfo(contentModel->folderAt(viewRow), sourceIndex.data(FolderModel::CoverPathRole));
    emit focusedFolderChanged();
}

void GridComicsView::clearFocusedFolder()
{
    if (!focusedFolderIndex.isValid() && focusedFolderInfo.isEmpty())
        return;

    focusedFolderIndex = QModelIndex();
    focusedFolderInfo.clear();
    emit focusedFolderChanged();
}

void GridComicsView::applyPendingViewState()
{
    auto *rootObject = view->rootObject();
    if (!rootObject) {
        view->setUpdatesEnabled(true);
        return;
    }
    auto scrollView = rootObject->findChild<QObject *>("topScrollView", Qt::FindChildrenRecursively);
    if (!scrollView) {
        view->setUpdatesEnabled(true);
        return;
    }

    if (!pendingViewState) {
        QMetaObject::invokeMethod(scrollView, "scrollToOrigin");
        view->setUpdatesEnabled(true);
        view->update();
        return;
    }

    const auto state = *pendingViewState;
    pendingViewState.reset();

    if (state.currentItem.kind != ContentItemRef::None) {
        const auto currentRow = viewRowForItem(state.currentItem);
        if (currentRow >= 0)
            focusItem(currentRow);
    }

    auto viewRow = viewRowForItem(state.topItem);
    if (state.topItem.kind == ContentItemRef::Header) {
        viewRow = -1;
    } else if (viewRow < 0 && contentModel->rowCount() > 0) {
        if (model && model->rowCount() > 0 && state.fallbackComicRow >= 0) {
            const auto comicRow = qBound(0, state.fallbackComicRow, model->rowCount() - 1);
            viewRow = contentModel->viewRowForComicRow(comicRow);
        } else {
            viewRow = 0;
        }
        viewRow = nearestSelectableRow(viewRow, 1);
    }

    QMetaObject::invokeMethod(scrollView, "restorePosition",
                              Q_ARG(QVariant, viewRow),
                              Q_ARG(QVariant, state.offset),
                              Q_ARG(QVariant, state.itemExtent));
    view->setUpdatesEnabled(true);
    view->update();
}

int GridComicsView::viewRowForItem(const ContentItemRef &item) const
{
    switch (item.kind) {
    case ContentItemRef::Comic:
        return contentModel->viewRowForComicId(item.id);
    case ContentItemRef::Folder:
        return contentModel->viewRowForFolderId(item.id);
    case ContentItemRef::None:
    case ContentItemRef::Header:
        return -1;
    }
    return -1;
}

void GridComicsView::showEvent(QShowEvent *event)
{
    ComicsView::showEvent(event);
    int coverSize = settings->value(COMICS_GRID_COVER_SIZES, YACREADER_MIN_COVER_WIDTH).toInt();

    coverSizeSlider->setValue(coverSize);
    setCoversSize(coverSize);
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
    updateCurrentComicBanner();
}

void GridComicsView::focusComicsNavigation(Qt::FocusReason reason)
{
    view->setFocus(reason);
}

void GridComicsView::reloadContent()
{
    ComicsView::reloadContent();
    updateCurrentComicView();
}

void GridComicsView::startDrag()
{
    auto drag = new QDrag(this);
    drag->setMimeData(model->mimeData(selectionHelper->selectedRows()));
    drag->setPixmap(theme.comicsViewToolbar.openInYACReaderIcon.pixmap(18, 18)); // TODO add better image

    /*Qt::DropAction dropAction =*/drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
}

bool GridComicsView::canDropUrls(const QList<QUrl> &urls, Qt::DropAction action)
{
    if (action == Qt::CopyAction) {
        QString currentPath;
        for (const auto &url : urls) {
            // comics or folders are accepted, folders' content is validate in dropEvent (avoid any lag before droping)
            currentPath = url.toLocalFile();
            if (Comic::fileIsComic(currentPath) || QFileInfo(currentPath).isDir())
                return true;
        }
    }
    return false;
}

bool GridComicsView::canDropImage(const QList<QUrl> &urls)
{
    return !YACReader::droppedImagePath(urls).isEmpty();
}

bool GridComicsView::canDropFormats(const QStringList &formats)
{
    return (formats.contains(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat) && model->canBeResorted());
}

void GridComicsView::droppedFiles(const QList<QUrl> &urls, Qt::DropAction action)
{
    bool validAction = action == Qt::CopyAction; // TODO add move

    if (validAction) {
        QList<QPair<QString, QString>> droppedFiles = ComicFilesManager::getDroppedFiles(urls);
        emit copyComicsToCurrentFolder(droppedFiles);
    }
}

void GridComicsView::droppedImageAt(const QList<QUrl> &urls, int viewRow)
{
    const auto imagePath = YACReader::droppedImagePath(urls);
    if (imagePath.isEmpty() || viewRow < 0 || viewRow >= contentModel->rowCount())
        return;

    if (contentModel->isFolderRow(viewRow)) {
        emit customFolderCoverRequested(contentModel->folderAt(viewRow).id, imagePath);
        return;
    }

    const auto comicRow = contentModel->sourceComicRow(viewRow);
    const auto comicIndex = model && comicRow >= 0 ? model->index(comicRow, 0) : QModelIndex();
    if (comicIndex.isValid())
        emit customComicCoverRequested(comicIndex.data(ComicModel::IdRole).toULongLong(), imagePath);
}

void GridComicsView::droppedComicsForResortingAt(const QString &data, int index)
{
    Q_UNUSED(data);

    const auto comicIndex = qBound(0, contentModel->sourceComicRow(index), model->rowCount());
    model->dropMimeData(model->mimeData(selectionHelper->selectedRows()), Qt::MoveAction, comicIndex, 0, QModelIndex());
}

void GridComicsView::activateItem(int viewRow)
{
    if (viewRow < 0 || viewRow >= contentModel->rowCount() || contentModel->isSpacerRow(viewRow))
        return;

    if (contentModel->isFolderRow(viewRow)) {
        openFolder(viewRow);
        return;
    }

    emit selected(contentModel->sourceComicRow(viewRow));
}

void GridComicsView::applyTheme(const Theme &theme)
{
    QQmlContext *ctxt = view->rootContext();
    const auto &giv = theme.gridAndInfoView;

    ctxt->setContextProperty("continueReadingBackgroundColor", giv.continueReadingBackgroundColor);
    ctxt->setContextProperty("continueReadingTextColor", giv.continueReadingTextColor);

    // Grid colors
    ctxt->setContextProperty("backgroundColor", giv.backgroundColor);
    ctxt->setContextProperty("backgroundBlurOverlayColor", giv.backgroundBlurOverlayColor);
    ctxt->setContextProperty("cellColor", giv.cellColor);
    ctxt->setContextProperty("cellSelectedColor", giv.cellSelectedColor);
    ctxt->setContextProperty("cellSelectedBorderColor", giv.cellSelectedBorderColor);
    ctxt->setContextProperty("borderColor", giv.borderColor);
    ctxt->setContextProperty("itemTitleColor", giv.itemTitleColor);
    ctxt->setContextProperty("itemDetailsColor", giv.itemDetailsColor);
    ctxt->setContextProperty("showDropShadow", QVariant(giv.showDropShadow));

    // Info panel colors
    ctxt->setContextProperty("infoBackgroundColor", giv.infoBackgroundColor);
    ctxt->setContextProperty("infoMetadataTextColor", giv.infoMetadataTextColor);
    ctxt->setContextProperty("infoTextColor", giv.infoTextColor);

    // Rating and favorite colors
    ctxt->setContextProperty("ratingUnselectedColor", giv.ratingUnselectedColor);
    ctxt->setContextProperty("ratingSelectedColor", giv.ratingSelectedColor);
    ctxt->setContextProperty("favUncheckedColor", giv.favUncheckedColor);
    ctxt->setContextProperty("favCheckedColor", giv.favCheckedColor);
    ctxt->setContextProperty("readTickUncheckedColor", giv.readTickUncheckedColor);
    ctxt->setContextProperty("readTickCheckedColor", giv.readTickCheckedColor);

    // Current comic banner
    ctxt->setContextProperty("currentComicBackgroundColor", giv.currentComicBackgroundColor);

    // New item indicator, button colors, links, scrollbars, cover borders, shadows
    ctxt->setContextProperty("newItemColor", giv.newItemColor);
    ctxt->setContextProperty("buttonColor", giv.buttonColor);
    ctxt->setContextProperty("buttonTextColor", giv.buttonTextColor);
    ctxt->setContextProperty("themeLinkColor", giv.linkColor);
    ctxt->setContextProperty("themeLinkColorStr", giv.linkColor.name());
    ctxt->setContextProperty("scrollbarColor", giv.scrollbarColor);
    ctxt->setContextProperty("scrollbarBorderColor", giv.scrollbarBorderColor);
    ctxt->setContextProperty("infoScrollbarColor", giv.infoScrollbarColor);
    ctxt->setContextProperty("comicCoverBorderColor", giv.comicCoverBorderColor);
    ctxt->setContextProperty("folderCoverBorderColor", giv.folderCoverBorderColor);
    ctxt->setContextProperty("placeholderFolder1Color", giv.placeholderFolder1Color);
    ctxt->setContextProperty("placeholderFolder1BorderColor", giv.placeholderFolder1BorderColor);
    ctxt->setContextProperty("placeholderFolder2Color", giv.placeholderFolder2Color);
    ctxt->setContextProperty("placeholderFolder2BorderColor", giv.placeholderFolder2BorderColor);
    ctxt->setContextProperty("currentComicCoverShadowColor", giv.currentComicCoverShadowColor);
    ctxt->setContextProperty("buttonShadowColor", giv.buttonShadowColor);

    // Update background config to apply theme cell colors
    updateBackgroundConfig();

    // Update show info action icon
    showInfoAction->setIcon(theme.comicsViewToolbar.showComicInfoIcon);

    // Update zoom slider icons (if they exist - created in setToolBar)
    if (smallZoomLabel) {
        smallZoomLabel->setPixmap(theme.comicsViewToolbar.smallGridZoomIcon.pixmap(18, 18));
    }
    if (bigZoomLabel) {
        bigZoomLabel->setPixmap(theme.comicsViewToolbar.bigGridZoomIcon.pixmap(18, 18));
    }

    const auto locationKind = currentLocationInfo.value(QStringLiteral("kind")).toString();
    if (locationKind == QStringLiteral("favorites") || locationKind == QStringLiteral("reading") || locationKind == QStringLiteral("recent") || locationKind == QStringLiteral("tag") || locationKind == QStringLiteral("readingList")) {
        updateCurrentListIcon();
        emit currentLocationInfoChanged();
    }
}

void GridComicsView::setShowMarks(bool show)
{
    QQmlContext *ctxt = view->rootContext();
    ctxt->setContextProperty("show_marks", show);
}

void GridComicsView::closeEvent(QCloseEvent *event)
{
    releaseToolBar();
    saveViewConfig();

    /*QObject *object = view->rootObject();
    QMetaObject::invokeMethod(object, "exit");
    container->close();
    view->close();*/

    event->accept();
    ComicsView::closeEvent(event);
}
