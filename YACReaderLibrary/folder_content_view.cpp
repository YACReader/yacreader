#include "folder_content_view.h"

#include "folder_model.h"
#include "grid_comics_view.h"
#include "yacreader_global_gui.h"
#include "yacreader_tool_bar_stretch.h"

#include "QsLog.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>

using namespace YACReader;

FolderContentView::FolderContentView(QWidget *parent)
    : QWidget { parent }, parent(QModelIndex()), comicModel(new ComicModel()), folderModel(new FolderModel())
{
    qmlRegisterType<FolderModel>("com.yacreader.FolderModel", 1, 0, "FolderModel");

    settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat, this);
    settings->beginGroup("libraryConfig");

    view = new QQuickWidget();

    // QQuickWidget requires rendering into OpenGL framebuffer objects
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    view->quickWindow()->setGraphicsApi(QSGRendererInterface::OpenGL);
#endif

    view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    connect(
            view, &QQuickWidget::statusChanged,
            [=](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error) {
                    QLOG_ERROR() << view->errors();
                }
            });

    coverSizeSliderWidget = new QWidget(this);
    coverSizeSliderWidget->setFixedWidth(200);
    coverSizeSlider = new QSlider(coverSizeSliderWidget);
    coverSizeSlider->setOrientation(Qt::Horizontal);
    coverSizeSlider->setRange(YACREADER_MIN_GRID_ZOOM_WIDTH, YACREADER_MAX_GRID_ZOOM_WIDTH);

    auto horizontalLayout = new QHBoxLayout();
    QLabel *smallLabel = new QLabel();
    smallLabel->setPixmap(hdpiPixmap(":/images/comics_view_toolbar/small_size_grid_zoom.svg", QSize(18, 18)));
    horizontalLayout->addWidget(smallLabel);
    horizontalLayout->addWidget(coverSizeSlider, 0, Qt::AlignVCenter);
    QLabel *bigLabel = new QLabel();
    bigLabel->setPixmap(hdpiPixmap(":/images/comics_view_toolbar/big_size_grid_zoom.svg", QSize(18, 18)));
    horizontalLayout->addWidget(bigLabel);
    horizontalLayout->addSpacing(10);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    coverSizeSliderWidget->setLayout(horizontalLayout);

    connect(coverSizeSlider, &QAbstractSlider::valueChanged, this, &FolderContentView::setCoversSize);

    toolbar = new QToolBar();
    toolbar->addWidget(new YACReaderToolBarStretch);
    toolbar->addWidget(coverSizeSliderWidget);

    auto l = new QVBoxLayout;
    setContentsMargins(0, 0, 0, 0);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    l->addWidget(view);
    l->addWidget(toolbar);
    this->setLayout(l);

    QQmlContext *ctxt = view->rootContext();

    LibraryUITheme theme;
#ifdef Q_OS_MAC
    theme = Light;
#else
    theme = Dark;
#endif

    if (theme == Light) {
        ctxt->setContextProperty("continueReadingBackgroundColor", "#E8E8E8");
        ctxt->setContextProperty("continueReadingColor", "#000000");

        ctxt->setContextProperty("backgroundColor", "#F6F6F6");
        ctxt->setContextProperty("cellColor", "#FFFFFF");
        ctxt->setContextProperty("selectedColor", "#FFFFFF");
        ctxt->setContextProperty("selectedBorderColor", "#007AFF");
        ctxt->setContextProperty("borderColor", "#DBDBDB");
        ctxt->setContextProperty("titleColor", "#121212");
        ctxt->setContextProperty("textColor", "#636363");
        // fonts settings
        ctxt->setContextProperty("fontSize", 11);
        ctxt->setContextProperty("fontFamily", QApplication::font().family());
        ctxt->setContextProperty("fontSpacing", 0.5);

        // info - copy/pasted from info_comics_view TODO create helpers for setting the UI config
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
        ctxt->setContextProperty("continueReadingBackgroundColor", "#88000000");
        ctxt->setContextProperty("continueReadingColor", "#FFFFFF");

        ctxt->setContextProperty("backgroundColor", "#2A2A2A");
        ctxt->setContextProperty("cellColor", "#212121");
        ctxt->setContextProperty("selectedColor", "#121212");
        ctxt->setContextProperty("selectedBorderColor", "#121212");
        ctxt->setContextProperty("borderColor", "#121212");
        ctxt->setContextProperty("titleColor", "#FFFFFF");
        ctxt->setContextProperty("textColor", "#A8A8A8");
        ctxt->setContextProperty("dropShadow", QVariant(false));
        // fonts settings
        int fontSize = QApplication::font().pointSize();
        if (fontSize == -1)
            fontSize = QApplication::font().pixelSize();
        ctxt->setContextProperty("fontSize", fontSize);
        ctxt->setContextProperty("fontFamily", QApplication::font().family());
        ctxt->setContextProperty("fontSpacing", 0.5);

        // info - copy/pasted from info_comics_view TODO create helpers for setting the UI config
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

    updateCoversSizeInContext(YACREADER_MIN_COVER_WIDTH, ctxt);

    ctxt->setContextProperty("comicsList", comicModel.get());
    ctxt->setContextProperty("foldersList", folderModel);

    ctxt->setContextProperty("showCurrentComic", QVariant(false));

    ctxt->setContextProperty("openHelper", this);
    ctxt->setContextProperty("contextMenuHelper", this);

    view->setSource(QUrl("qrc:/qml/FolderContentView.qml"));
}

void FolderContentView::setModel(const QModelIndex &parent, FolderModel *model)
{
    this->parent = parent;
    QQmlContext *ctxt = view->rootContext();

    ctxt->setContextProperty("foldersList", model);

    // when the root folder is set, FolderModel just returns itself in `getSubfoldersModel`, I need to measure the performance of create a deep copy...
    if (folderModel->isSubfolder) {
        delete folderModel;
    }
    folderModel = model;

    auto grid = view->rootObject()->findChild<QQuickItem *>(QStringLiteral("grid"));

    if (grid != nullptr) {
        grid->setProperty("currentIndex", 0);
    }
}

void FolderContentView::setContinueReadingModel(ComicModel *model)
{
    QQmlContext *ctxt = view->rootContext();

    ctxt->setContextProperty("comicsList", model);
    this->comicModel.reset(model);

    auto list = view->rootObject()->findChild<QQuickItem *>(QStringLiteral("list"));

    if (list != nullptr) {
        list->setProperty("currentIndex", 0);
    }
}

void FolderContentView::reloadContinueReadingModel()
{
    if (!folderModel->isSubfolder) {
        comicModel->reloadContinueReading();
    }
}

void FolderContentView::openFolder(int index)
{
    emit subfolderSelected(this->parent, index);
}

void FolderContentView::openComicFromContinueReadingList(int index)
{
    auto comic = comicModel->getComic(comicModel->index(index, 0));
    emit openComic(comic, ComicModel::Folder);
}

void FolderContentView::requestedFolderContextMenu(QPoint point, int index)
{
    auto folder = folderModel->getFolder(folderModel->index(index, 0));
    emit openFolderContextMenu(point, folder);
}

void FolderContentView::requestedContinueReadingComicContextMenu(QPoint point, int index)
{
    auto comic = comicModel->getComic(comicModel->index(index, 0));
    emit openContinueReadingComicContextMenu(point, comic);
}

void FolderContentView::updateCoversSizeInContext(int width, QQmlContext *ctxt)
{
    int cellBottomMarging = 8 * (1 + 2 * (1 - (float(YACREADER_MAX_GRID_ZOOM_WIDTH - width) / (YACREADER_MAX_GRID_ZOOM_WIDTH - YACREADER_MIN_GRID_ZOOM_WIDTH))));

    ctxt->setContextProperty("cellCustomHeight", ((width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH) + 51 + cellBottomMarging);
    ctxt->setContextProperty("cellCustomWidth", (width * YACREADER_MIN_CELL_CUSTOM_WIDTH) / YACREADER_MIN_COVER_WIDTH);

    ctxt->setContextProperty("itemWidth", width);
    ctxt->setContextProperty("itemHeight", ((width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH) + 51);

    ctxt->setContextProperty("coverWidth", width);
    ctxt->setContextProperty("coverHeight", (width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH);
}

void FolderContentView::setCoversSize(int width)
{
    QQmlContext *ctxt = view->rootContext();

    auto grid = view->rootObject()->findChild<QQuickItem *>(QStringLiteral("grid"));

    if (grid != 0) {
        QVariant cellCustomWidth = (width * YACREADER_MIN_CELL_CUSTOM_WIDTH) / YACREADER_MIN_GRID_ZOOM_WIDTH;
        QMetaObject::invokeMethod(grid, "calculateCellWidths",
                                  Q_ARG(QVariant, cellCustomWidth));
    }

    updateCoversSizeInContext(width, ctxt);

    settings->setValue(COMICS_GRID_COVER_SIZES, coverSizeSlider->value());
}

void FolderContentView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    int coverSize = settings->value(COMICS_GRID_COVER_SIZES, YACREADER_MIN_COVER_WIDTH).toInt();

    coverSizeSlider->setValue(coverSize);
    setCoversSize(coverSize);
}

void FolderContentView::dragEnterEvent(QDragEnterEvent *event)
{
}

void FolderContentView::dropEvent(QDropEvent *event)
{
}
