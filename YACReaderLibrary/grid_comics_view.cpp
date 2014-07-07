#include "grid_comics_view.h"

#include <QtWidgets>
#include <QtQuick>

#include "QsLog.h"

GridComicsView::GridComicsView(QWidget *parent) :
    ComicsView(parent),_selectionModel(NULL)
{
    qmlRegisterType<TableModel>("comicModel",1,0,"TableModel");

    view = new QQuickView();
    container = QWidget::createWindowContainer(view, this);

    container->setMinimumSize(200, 200);
    container->setFocusPolicy(Qt::TabFocus);
    view->setSource(QUrl("qrc:/qml/GridComicsView.qml"));

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

void GridComicsView::setToolBar(QToolBar *toolBar)
{
QLOG_INFO() << "setToolBar";
static_cast<QVBoxLayout *>(this->layout())->insertWidget(1,toolBar);
}

void GridComicsView::setModel(TableModel *model)
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
        ctxt->setContextProperty("comicsSelectionHelper", this);
        ctxt->setContextProperty("dummyValue", true);
    }

#ifdef Q_OS_MAC
    ctxt->setContextProperty("backgroundColor", "#FAFAFA");
    ctxt->setContextProperty("cellColor", "#EDEDED");
    ctxt->setContextProperty("selectedColor", "#DDDDDD");
    ctxt->setContextProperty("titleColor", "#121212");
    ctxt->setContextProperty("textColor", "#636363");
    ctxt->setContextProperty("dropShadow",true);
#else
    ctxt->setContextProperty("backgroundColor", "#2A2A2A");
    ctxt->setContextProperty("cellColor", "#212121");
    ctxt->setContextProperty("selectedColor", "#121212");
    ctxt->setContextProperty("titleColor", "#E6E6E6");
    ctxt->setContextProperty("textColor", "#E6E6E6");
    ctxt->setContextProperty("dropShadow",false);
#endif


}

void GridComicsView::setCurrentIndex(const QModelIndex &index)
{
    QLOG_INFO() << "setCurrentIndex";
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
}

void GridComicsView::toNormal()
{
    QLOG_INFO() << "toNormal";
}

void GridComicsView::updateConfig(QSettings *settings)
{
    QLOG_INFO() << "updateConfig";
}

void GridComicsView::setItemActions(const QList<QAction *> &actions)
{
    QLOG_INFO() << "setItemActions";
}

void GridComicsView::setViewActions(const QList<QAction *> &actions)
{
    //TODO generate QML Menu from actions
    QLOG_INFO() << "setViewActions";
    this->addActions(actions);

    //TODO this is completely unsafe, but QActions can't be used directly in QML
    if(actions.length()>17)
    {
        QQmlContext *ctxt = view->rootContext();

        ctxt->setContextProperty("openComicAction",actions[0]);

        ctxt->setContextProperty("openContainingFolderComicAction",actions[2]);

        ctxt->setContextProperty("resetComicRatingAction",actions[4]);

        ctxt->setContextProperty("editSelectedComicsAction",actions[6]);
        ctxt->setContextProperty("getInfoAction",actions[7]);
        ctxt->setContextProperty("asignOrderAction",actions[8]);

        ctxt->setContextProperty("selectAllComicsAction",actions[10]);

        ctxt->setContextProperty("setAsReadAction",actions[12]);
        ctxt->setContextProperty("setAsNonReadAction",actions[13]);
        ctxt->setContextProperty("showHideMarksAction",actions[14]);

        ctxt->setContextProperty("deleteComicsAction",actions[16]);

        ctxt->setContextProperty("toggleFullScreenAction",actions[18]);
    }
    else
        QLOG_ERROR() << "setViewActions invoked with the wrong number of actions";
}

QSize GridComicsView::sizeHint()
{
        QLOG_INFO() << "sizeHint";
    return QSize(1280,768);
}

//helper
void GridComicsView::selectIndex(int index)
{
    QLOG_INFO() << "selectIndex" << index;
    if(_selectionModel != NULL && model!=NULL)
        _selectionModel->select(model->index(index,0),QItemSelectionModel::Select | QItemSelectionModel::Rows);
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

void GridComicsView::setShowMarks(bool show)
{
    QLOG_INFO() << "setShowMarks";
    QQmlContext *ctxt = view->rootContext();
    ctxt->setContextProperty("show_marks", show);
}

void GridComicsView::closeEvent(QCloseEvent *event)
{
    QLOG_INFO() << "closeEvent";
    QObject *object = view->rootObject();
    QMetaObject::invokeMethod(object, "exit");
    container->close();
    view->close();
    event->accept();
    ComicsView::closeEvent(event);
}
