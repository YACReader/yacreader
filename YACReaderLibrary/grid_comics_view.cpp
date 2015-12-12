#include "grid_comics_view.h"

#include <QtQuick>
#include <QtWidgets>

#include "comic.h"
#include "comic_files_manager.h"
#include "QsLog.h"
#include "yacreader_global.h"
#include "yacreader_tool_bar_stretch.h"

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


GridComicsView::GridComicsView(QWidget *parent) :
    ComicsView(parent),_selectionModel(NULL)
{
    settings = new QSettings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini", QSettings::IniFormat, this);
    settings->beginGroup("libraryConfig");

    qmlRegisterType<ComicModel>("comicModel",1,0,"TableModel");

    view = new QQuickView();
    container = QWidget::createWindowContainer(view, this);

    container->setMinimumSize(200, 200);
    container->setFocusPolicy(Qt::TabFocus);
    view->setSource(QUrl("qrc:/qml/GridComicsView.qml"));

    createCoverSizeSliderWidget();

    int coverSize = settings->value(COMICS_GRID_COVER_SIZES, YACREADER_MIN_COVER_WIDTH).toInt();

    coverSizeSlider->setValue(coverSize);
    setCoversSize(coverSize);

    QQmlContext *ctxt = view->rootContext();

#ifdef Q_OS_MAC
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

#else
    ctxt->setContextProperty("backgroundColor", "#2A2A2A");
    ctxt->setContextProperty("cellColor", "#212121");
    ctxt->setContextProperty("selectedColor", "#121212");
    ctxt->setContextProperty("selectedBorderColor", "#121212");
    ctxt->setContextProperty("borderColor", "#121212");
    ctxt->setContextProperty("titleColor", "#FFFFFF");
    ctxt->setContextProperty("textColor", "#A8A8A8");
    ctxt->setContextProperty("dropShadow",false);
    //fonts settings
    int fontSize = QApplication::font().pointSize();
    if(fontSize == -1)
        fontSize = QApplication::font().pixelSize();
    ctxt->setContextProperty("fontSize", fontSize);
    ctxt->setContextProperty("fontFamily", QApplication::font().family());
    ctxt->setContextProperty("fontSpacing", 0.5);
#endif

    setShowMarks(true);//TODO save this in settings

    QVBoxLayout * l = new QVBoxLayout;
    l->addWidget(container);
    this->setLayout(l);

    setContentsMargins(0,0,0,0);
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(0);

    QLOG_INFO() << "GridComicsView";
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

    QHBoxLayout * horizontalLayout = new QHBoxLayout();
    QLabel * smallLabel = new QLabel();
    smallLabel->setPixmap(QPixmap(":/images/comics_view_toolbar/small_size_grid_zoom.png"));
    horizontalLayout->addWidget(smallLabel);
    horizontalLayout->addWidget(coverSizeSlider, 0, Qt::AlignVCenter);
    QLabel * bigLabel = new QLabel();
    bigLabel->setPixmap(QPixmap(":/images/comics_view_toolbar/big_size_grid_zoom.png"));
    horizontalLayout->addWidget(bigLabel);
    horizontalLayout->addSpacing(10);
    horizontalLayout->setMargin(0);

    coverSizeSliderWidget->setLayout(horizontalLayout);
    //TODO add shortcuts (ctrl-+ and ctrl-- for zooming in out, + ctrl-0 for reseting the zoom)

    connect(coverSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(setCoversSize(int)));
}

void GridComicsView::setToolBar(QToolBar *toolBar)
{
    QLOG_INFO() << "setToolBar";
    static_cast<QVBoxLayout *>(this->layout())->insertWidget(1,toolBar);
    this->toolbar = toolBar;

     toolBarStretchAction = toolBar->addWidget(toolBarStretch);
     coverSizeSliderAction = toolBar->addWidget(coverSizeSliderWidget);
}

void GridComicsView::setModel(ComicModel *model)
{
    QLOG_INFO() << "setModel";

    QQmlContext *ctxt = view->rootContext();

    //there is only one mothel in the system
    ComicsView::setModel(model);
    if(this->model != NULL)
    {
        QLOG_INFO() << "xxx";

        if(_selectionModel != NULL)
            delete _selectionModel;
        _selectionModel = new QItemSelectionModel(this->model);

        ctxt->setContextProperty("comicsList", this->model);
        ctxt->setContextProperty("comicsSelection", _selectionModel);
        ctxt->setContextProperty("contextMenuHelper",this);
        ctxt->setContextProperty("comicsSelectionHelper", this);
        ctxt->setContextProperty("comicRatingHelper", this);
        ctxt->setContextProperty("dummyValue", true);
        ctxt->setContextProperty("dragManager", this);
        ctxt->setContextProperty("dropManager", this);

        updateBackgroundConfig();

        if(model->rowCount()>0)
            setCurrentIndex(model->index(0,0));
    }
}

void GridComicsView::updateBackgroundConfig()
{
    if(this->model == NULL)
        return;

    QQmlContext *ctxt = view->rootContext();

    //backgroun image configuration
    bool useBackgroundImage = settings->value(USE_BACKGROUND_IMAGE_IN_GRID_VIEW, true).toBool();

    if(useBackgroundImage)
    {
        float opacity = settings->value(OPACITY_BACKGROUND_IMAGE_IN_GRID_VIEW, 0.2).toFloat();
        float blurRadius = settings->value(BLUR_RADIUS_BACKGROUND_IMAGE_IN_GRID_VIEW, 75).toInt();

        ctxt->setContextProperty("backgroundImage", this->model->data(this->model->index(0, 0), ComicModel::CoverPathRole));
        ctxt->setContextProperty("backgroundBlurOpacity", opacity);
        ctxt->setContextProperty("backgroundBlurRadius", blurRadius);
        ctxt->setContextProperty("backgroundBlurVisible", true);
    }
    else
    {
        ctxt->setContextProperty("backgroundImage", QVariant());
        ctxt->setContextProperty("backgroundBlurOpacity", 0);
        ctxt->setContextProperty("backgroundBlurRadius", 0);
        ctxt->setContextProperty("backgroundBlurVisible", false);
    }

#ifdef Q_OS_MAC
    ctxt->setContextProperty("cellColor", useBackgroundImage?"#99FFFFFF":"#FFFFFF");
    ctxt->setContextProperty("selectedColor", "#FFFFFF");
#else
    ctxt->setContextProperty("cellColor", useBackgroundImage?"#99212121":"#212121");
    ctxt->setContextProperty("selectedColor", "#121212");
#endif
}

void GridComicsView::setCurrentIndex(const QModelIndex &index)
{
    QLOG_INFO() << "setCurrentIndex";
    _selectionModel->clear();
    _selectionModel->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    view->rootContext()->setContextProperty("dummyValue", true);
}

QModelIndex GridComicsView::currentIndex()
{
        QLOG_INFO() << "currentIndex";
    QModelIndexList indexes = _selectionModel->selectedRows();
    if(indexes.length()>0)
        return indexes[0];

    this->selectIndex(0);
    return _selectionModel->selectedRows()[0];
}

QItemSelectionModel *GridComicsView::selectionModel()
{
    QLOG_INFO() << "selectionModel";
    QModelIndexList indexes = _selectionModel->selectedRows();
    if(indexes.length()==0)
        this->selectIndex(0);

    return _selectionModel;
}

void GridComicsView::scrollTo(const QModelIndex &mi, QAbstractItemView::ScrollHint hint)
{
    QLOG_INFO() << "scrollTo";
}

void GridComicsView::toFullScreen()
{
    QLOG_INFO() << "toFullScreen";
    toolbar->hide();
}

void GridComicsView::toNormal()
{
    QLOG_INFO() << "toNormal";
    toolbar->show();
}

void GridComicsView::updateConfig(QSettings *settings)
{
    QLOG_INFO() << "updateConfig";
}

void GridComicsView::enableFilterMode(bool enabled)
{

}

void GridComicsView::selectAll()
{
    QLOG_INFO() << "selectAll";
    QModelIndex top = model->index(0, 0);
    QModelIndex bottom = model->index(model->rowCount()-1, 0);
    QItemSelection selection(top, bottom);
    _selectionModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    view->rootContext()->setContextProperty("dummyValue", true);
}

void GridComicsView::rate(int index, int rating)
{
    QLOG_INFO() << "Comic "<< index << "rated" << rating;
    model->updateRating(rating,model->index(index,0));
}

void GridComicsView::requestedContextMenu(const QPoint &point)
{
    emit customContextMenuViewRequested(point);
}

void GridComicsView::setCoversSize(int width)
{
    QQmlContext *ctxt = view->rootContext();

    QQuickItem * grid = view->rootObject()->findChild<QQuickItem *>(QStringLiteral("grid"));

    if(grid != 0)
    {
        QLOG_INFO() << "method invoked";
        QVariant cellCustomWidth = (width * YACREADER_MIN_CELL_CUSTOM_WIDTH) / YACREADER_MIN_GRID_ZOOM_WIDTH;
        QMetaObject::invokeMethod(grid, "calculateCellWidths",
                                  Q_ARG(QVariant, cellCustomWidth));
    }

    int cellBottomMarging = 8 * (1 + 2*(1 - (float(YACREADER_MAX_GRID_ZOOM_WIDTH - width) / (YACREADER_MAX_GRID_ZOOM_WIDTH - YACREADER_MIN_GRID_ZOOM_WIDTH))) );

    ctxt->setContextProperty("cellCustomHeight", ((width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH) + 51 + cellBottomMarging);
    ctxt->setContextProperty("cellCustomWidth", (width * YACREADER_MIN_CELL_CUSTOM_WIDTH) / YACREADER_MIN_COVER_WIDTH );

    ctxt->setContextProperty("itemWidth", width);
    ctxt->setContextProperty("itemHeight", ((width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH) + 51);

    ctxt->setContextProperty("coverWidth", width);
    ctxt->setContextProperty("coverHeight", (width * YACREADER_MAX_COVER_HEIGHT) / YACREADER_MIN_COVER_WIDTH);
}

QSize GridComicsView::sizeHint()
{
        QLOG_INFO() << "sizeHint";
        return QSize(1280,768);
}

QByteArray GridComicsView::getMimeDataFromSelection()
{
    QByteArray data;

    QMimeData * mimeData = model->mimeData(_selectionModel->selectedIndexes());
    data = mimeData->data(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat);

    delete mimeData;

    return data;
}

void GridComicsView::startDrag()
{
    QLOG_DEBUG() << "performDrag";
    QDrag *drag = new QDrag(this);
    drag->setMimeData(model->mimeData(_selectionModel->selectedRows()));
    drag->setPixmap(QPixmap(":/images/comics_view_toolbar/openInYACReader.png")); //TODO add better image

    /*Qt::DropAction dropAction =*/ drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
}

bool GridComicsView::canDropUrls(const QList<QUrl> &urls, Qt::DropAction action)
{
    if(action == Qt::CopyAction)
    {
        QString currentPath;
        foreach (QUrl url, urls)
        {
            //comics or folders are accepted, folders' content is validate in dropEvent (avoid any lag before droping)
            currentPath = url.toLocalFile();
            if(Comic::fileIsComic(currentPath) || QFileInfo(currentPath).isDir())
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

    if(validAction)
    {
        QList<QPair<QString, QString> > droppedFiles = ComicFilesManager::getDroppedFiles(urls);
        emit copyComicsToCurrentFolder(droppedFiles);
    }
}

void GridComicsView::droppedComicsForResortingAt(const QString &data, int index)
{
    model->dropMimeData(model->mimeData(_selectionModel->selectedRows()), Qt::MoveAction, index, 0, QModelIndex());
}

//helper
void GridComicsView::selectIndex(int index)
{
    QLOG_INFO() << "selectIndex" << index;
    if(_selectionModel != NULL && model!=NULL)
    {
        _selectionModel->select(model->index(index,0),QItemSelectionModel::Select | QItemSelectionModel::Rows);
        view->rootContext()->setContextProperty("dummyValue", true);
    }
}

void GridComicsView::setCurrentIndex(int index)
{
    setCurrentIndex(model->index(index,0));
}

void GridComicsView::deselectIndex(int index)
{
    if(_selectionModel != NULL && model!=NULL)
    {
        _selectionModel->select(model->index(index,0),QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
        view->rootContext()->setContextProperty("dummyValue", true);
    }
}

bool GridComicsView::isSelectedIndex(int index)
{
    if(_selectionModel != NULL && model!=NULL)
    {
        QModelIndex mi = model->index(index,0);
        return _selectionModel->isSelected(mi);
    }
    return false;
}

void GridComicsView::clear()
{
    QLOG_INFO() << "clear";
    if(_selectionModel != NULL)
    {
        _selectionModel->clear();

        QQmlContext *ctxt = view->rootContext();
        ctxt->setContextProperty("dummyValue", true);
    }
    //model->forceClear();
}

void GridComicsView::selectedItem(int index)
{
    emit doubleClicked(model->index(index,0));
}

int GridComicsView::numItemsSelected()
{
    if(_selectionModel != NULL)
    {
        return _selectionModel->selectedRows().length();
    }

    return 0;
}

int GridComicsView::lastSelectedIndex()
{
    if(_selectionModel != NULL)
    {
        QLOG_INFO() << "last selected index " << _selectionModel->selectedRows().last().row();
        return _selectionModel->selectedRows().last().row();
    }

    return -1;
}

void GridComicsView::setShowMarks(bool show)
{
    QLOG_INFO() << "setShowMarks";
    QQmlContext *ctxt = view->rootContext();
    ctxt->setContextProperty("show_marks", show);
}

void GridComicsView::closeEvent(QCloseEvent *event)
{
    toolbar->removeAction(toolBarStretchAction);
    toolbar->removeAction(coverSizeSliderAction);

    QLOG_INFO() << "closeEvent";
    QObject *object = view->rootObject();
    QMetaObject::invokeMethod(object, "exit");
    container->close();
    view->close();
    event->accept();
    ComicsView::closeEvent(event);

    //save settings
    settings->setValue(COMICS_GRID_COVER_SIZES, coverSizeSlider->value());
}
