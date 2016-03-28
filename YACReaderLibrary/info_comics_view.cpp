#include "info_comics_view.h"

#include <QtQuick>

#include "comic_model.h"

#include "QsLog.h"

InfoComicsView::InfoComicsView(QWidget *parent)
    :ComicsView(parent)
{
    qmlRegisterType<ComicModel>("com.yacreader.ComicModel",1,0,"ComicModel");

    view = new QQuickView();
    container = QWidget::createWindowContainer(view, this);

    container->setFocusPolicy(Qt::StrongFocus);

    view->setSource(QUrl("qrc:/qml/InfoComicsView.qml"));

    QVBoxLayout * l = new QVBoxLayout;
    l->addWidget(container);
    this->setLayout(l);

    setContentsMargins(0,0,0,0);
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(0);

    QLOG_TRACE() << "GridComicsView";
}

void InfoComicsView::setToolBar(QToolBar *toolBar)
{
    toolBar->setParent(this);

    int FIXME;
}

void InfoComicsView::setModel(ComicModel *model)
{
    if(model == NULL)
        return;

    ComicsView::setModel(model);

    QQmlContext *ctxt = view->rootContext();

    /*if(_selectionModel != NULL)
        delete _selectionModel;

    _selectionModel = new QItemSelectionModel(model);*/

    //TODO fix crash in the following line on comics views switch

    int row = currentIndex().row();
    ctxt->setContextProperty("comicsList", model);
    ctxt->setContextProperty("backgroundImage", this->model->data(this->model->index(0, 0), ComicModel::CoverPathRole));

    /*ctxt->setContextProperty("comicsSelection", _selectionModel);
    ctxt->setContextProperty("contextMenuHelper",this);
    ctxt->setContextProperty("comicsSelectionHelper", this);
    ctxt->setContextProperty("comicRatingHelper", this);
    ctxt->setContextProperty("dummyValue", true);
    ctxt->setContextProperty("dragManager", this);
    ctxt->setContextProperty("dropManager", this);

    updateBackgroundConfig();*/

    if(model->rowCount()>0)
        setCurrentIndex(model->index(0,0));
}

void InfoComicsView::setCurrentIndex(const QModelIndex &index)
{
    int FIXME;
}

QModelIndex InfoComicsView::currentIndex()
{
    int FIXME;

    return QModelIndex();
}

QItemSelectionModel *InfoComicsView::selectionModel()
{
    int FIXME;

    return 0;
}

void InfoComicsView::scrollTo(const QModelIndex &mi, QAbstractItemView::ScrollHint hint)
{
    int FIXME;
}

void InfoComicsView::toFullScreen()
{
    int FIXME;
}

void InfoComicsView::toNormal()
{
    int FIXME;
}

void InfoComicsView::updateConfig(QSettings *settings)
{
    int FIXME;
}

void InfoComicsView::enableFilterMode(bool enabled)
{
    int FIXME;
}

void InfoComicsView::selectIndex(int index)
{
    int FIXME;
}

void InfoComicsView::setShowMarks(bool show)
{
    int FIXME;
}

void InfoComicsView::selectAll()
{
    int FIXME;
}