#include "folder_content_view.h"

#include "folder_model.h"
#include "grid_comics_view.h"
#include "yacreader_global.h"
#include "yacreader_global_gui.h"
#include "yacreader_tool_bar_stretch.h"
#include "comic.h"
#include "comic_files_manager.h"
#include "theme_manager.h"

#include "QsLog.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>

using namespace YACReader;

FolderContentView::FolderContentView(QAction *toogleRecentVisibilityAction, QWidget *parent)
    : QWidget { parent }, parent(QModelIndex()), comicModel(new ComicModel()), folderModel(new FolderModel()), smallZoomLabel(nullptr), bigZoomLabel(nullptr)
{
    qmlRegisterType<FolderModel>("com.yacreader.FolderModel", 1, 0, "FolderModel");

    settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat, this);
    settings->beginGroup("libraryConfig");

    view = new QQuickWidget();

    view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    connect(
            view, &QQuickWidget::statusChanged, this,
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

    connect(coverSizeSlider, &QAbstractSlider::valueChanged, this, &FolderContentView::setCoversSize);

    toolbar = new QToolBar();
    toolbar->setIconSize(QSize(18, 18));
    toolbar->addWidget(new YACReaderToolBarStretch);
    toolbar->addAction(toogleRecentVisibilityAction);
    toolbar->addSeparator();
    toolbar->addWidget(coverSizeSliderWidget);

    auto l = new QVBoxLayout;
    setContentsMargins(0, 0, 0, 0);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    l->addWidget(view);
    l->addWidget(toolbar);
    this->setLayout(l);

    QQmlContext *ctxt = view->rootContext();

    // fonts settings (not theme-dependent)
    int fontSize = QApplication::font().pointSize();
    if (fontSize == -1)
        fontSize = QApplication::font().pixelSize();
    ctxt->setContextProperty("fontSize", fontSize);
    ctxt->setContextProperty("fontFamily", QApplication::font().family());
    ctxt->setContextProperty("fontSpacing", 0.5);

    // Apply theme colors
    initTheme(this);

    updateCoversSizeInContext(YACREADER_MIN_COVER_WIDTH, ctxt);

    ctxt->setContextProperty("comicsList", comicModel.get());
    ctxt->setContextProperty("foldersList", folderModel);

    auto showContinueReading = settings->value(DISPLAY_GLOBAL_CONTINUE_READING_IN_GRID_VIEW, true).toBool();
    ctxt->setContextProperty("showContinueReading", QVariant(showContinueReading));

    ctxt->setContextProperty("openHelper", this);
    ctxt->setContextProperty("dropManager", this);
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

void FolderContentView::reloadContent()
{
    folderModel->reload();
    reloadContinueReadingModel();
}

void FolderContentView::reloadContinueReadingModel()
{
    if (!folderModel->isSubfolder) {
        comicModel->reloadContinueReading();
    }
}

void FolderContentView::setShowRecent(bool visible)
{
    folderModel->setShowRecent(visible);
}

void FolderContentView::setRecentRange(int days)
{
    folderModel->setRecentRange(days);
}

void FolderContentView::updateSettings()
{
    QQmlContext *ctxt = view->rootContext();

    auto showContinueReading = settings->value(DISPLAY_GLOBAL_CONTINUE_READING_IN_GRID_VIEW, true).toBool();
    ctxt->setContextProperty("showContinueReading", QVariant(showContinueReading));
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

bool FolderContentView::canDropUrls(const QList<QUrl> &urls, Qt::DropAction action)
{
    if (action == Qt::CopyAction) {
        QString currentPath;
        foreach (QUrl url, urls) {
            // comics or folders are accepted, folders' content is validate in dropEvent (avoid any lag before droping)
            currentPath = url.toLocalFile();
            if (Comic::fileIsComic(currentPath) || QFileInfo(currentPath).isDir())
                return true;
        }
    }
    return false;
}

bool FolderContentView::canDropFormats(const QString &formats)
{
    return true;
}

void FolderContentView::droppedFiles(const QList<QUrl> &urls, Qt::DropAction action)
{
    bool validAction = action == Qt::CopyAction; // TODO add move

    if (validAction) {
        QList<QPair<QString, QString>> droppedFiles = ComicFilesManager::getDroppedFiles(urls);
        emit copyComicsToCurrentFolder(droppedFiles);
    }
}

void FolderContentView::applyTheme(const Theme &theme)
{
    QQmlContext *ctxt = view->rootContext();
    const auto &qv = theme.qmlView;

    toolbar->setStyleSheet(theme.comicsViewToolbar.toolbarQSS);

    // Continue reading section colors
    ctxt->setContextProperty("continueReadingBackgroundColor", qv.continueReadingBackgroundColor);
    ctxt->setContextProperty("continueReadingColor", qv.continueReadingColor);

    // Grid colors
    ctxt->setContextProperty("backgroundColor", qv.backgroundColor);
    ctxt->setContextProperty("cellColor", qv.cellColor);
    ctxt->setContextProperty("selectedColor", qv.selectedColor);
    ctxt->setContextProperty("selectedBorderColor", qv.selectedBorderColor);
    ctxt->setContextProperty("borderColor", qv.borderColor);
    ctxt->setContextProperty("titleColor", qv.titleColor);
    ctxt->setContextProperty("textColor", qv.textColor);
    ctxt->setContextProperty("dropShadow", QVariant(qv.showDropShadow));

    // Info panel colors
    ctxt->setContextProperty("infoBackgroundColor", qv.infoBackgroundColor);
    ctxt->setContextProperty("topShadow", qv.topShadow.isEmpty() ? QUrl() : QUrl(qv.topShadow));
    ctxt->setContextProperty("infoShadow", qv.infoShadow);
    ctxt->setContextProperty("infoIndicator", qv.infoIndicator);
    ctxt->setContextProperty("infoTextColor", qv.infoTextColor);
    ctxt->setContextProperty("infoTitleColor", qv.infoTitleColor);

    // Rating and favorite colors
    ctxt->setContextProperty("ratingUnselectedColor", qv.ratingUnselectedColor);
    ctxt->setContextProperty("ratingSelectedColor", qv.ratingSelectedColor);
    ctxt->setContextProperty("favUncheckedColor", qv.favUncheckedColor);
    ctxt->setContextProperty("favCheckedColor", qv.favCheckedColor);
    ctxt->setContextProperty("readTickUncheckedColor", qv.readTickUncheckedColor);
    ctxt->setContextProperty("readTickCheckedColor", qv.readTickCheckedColor);

    // Update zoom slider icons
    if (smallZoomLabel) {
        smallZoomLabel->setPixmap(theme.comicsViewToolbar.smallGridZoomIcon.pixmap(18, 18));
    }
    if (bigZoomLabel) {
        bigZoomLabel->setPixmap(theme.comicsViewToolbar.bigGridZoomIcon.pixmap(18, 18));
    }
}
