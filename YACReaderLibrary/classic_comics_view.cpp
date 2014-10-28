#include "classic_comics_view.h"

#include "yacreader_table_view.h"

#include "comic_flow_widget.h"
#include "QsLog.h"

#include "QStackedWidget"

ClassicComicsView::ClassicComicsView(QWidget *parent)
    :ComicsView(parent),searching(false)
{
    QHBoxLayout * layout = new QHBoxLayout;

    settings = new QSettings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
    settings->beginGroup("libraryConfig");
    //FLOW-----------------------------------------------------------------------
    //---------------------------------------------------------------------------

    if(QGLFormat::hasOpenGL() && (settings->value(USE_OPEN_GL).toBool() == true))
        comicFlow = new ComicFlowWidgetGL(0);
    else
        comicFlow = new ComicFlowWidgetSW(0);

    comicFlow->updateConfig(settings);
    comicFlow->setFocusPolicy(Qt::StrongFocus);
    comicFlow->setShowMarks(true);
    setFocusProxy(comicFlow);

    comicFlow->setFocus(Qt::OtherFocusReason);

    comicFlow->setContextMenuPolicy(Qt::ActionsContextMenu);

    //TODO!!! set actions....
    //comicFlow->addAction(toggleFullScreenAction);
    //comicFlow->addAction(openComicAction);

    //END FLOW----


    //layout-----------------------------------------------
    sVertical = new QSplitter(Qt::Vertical);  //spliter derecha

    stack = new QStackedWidget;
    stack->addWidget(comicFlow);
    setupSearchingIcon();
    stack->addWidget(searchingIcon);


    sVertical->addWidget(stack);
    comics = new QWidget;
    QVBoxLayout * comicsLayout = new QVBoxLayout;
    comicsLayout->setSpacing(0);
    comicsLayout->setContentsMargins(0,0,0,0);
    //TODO ComicsView:(set toolbar) comicsLayout->addWidget(editInfoToolBar);

    tableView = new YACReaderTableView;
    tableView->verticalHeader()->hide();
    tableView->setFocusPolicy(Qt::StrongFocus);
    comicsLayout->addWidget(tableView);
    comics->setLayout(comicsLayout);
    sVertical->addWidget(comics);

    //config--------------------------------------------------
    if(settings->contains(COMICS_VIEW_HEADERS))
        tableView->horizontalHeader()->restoreState(settings->value(COMICS_VIEW_HEADERS).toByteArray());

    //connections---------------------------------------------
    connect(tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(centerComicFlow(QModelIndex)));
    connect(tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(selectedComicForOpening(QModelIndex)));
    connect(comicFlow, SIGNAL(centerIndexChanged(int)), this, SLOT(updateTableView(int)));
    connect(tableView, SIGNAL(comicRated(int,QModelIndex)), this, SIGNAL(comicRated(int,QModelIndex)));
    connect(comicFlow, SIGNAL(selected(uint)), this, SIGNAL(selected(uint)));
    connect(tableView->horizontalHeader(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(saveTableHeadersStatus()));
    layout->addWidget(sVertical);
    setLayout(layout);

    layout->setMargin(0);

#ifdef Q_OS_MAC
    sVertical->setCollapsible(1,false);
#endif

    if(settings->contains(COMICS_VIEW_FLOW_SPLITTER_STATUS))
        sVertical->restoreState(settings->value(COMICS_VIEW_FLOW_SPLITTER_STATUS).toByteArray());


}

void ClassicComicsView::setToolBar(QToolBar *toolBar)
{
    static_cast<QVBoxLayout *>(comics->layout())->insertWidget(0,toolBar);
}

void ClassicComicsView::setModel(ComicModel *model)
{

    ComicsView::setModel(model);

    if(model == NULL)
    {
        comicFlow->clear();
    }
    else
    {
        connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(applyModelChanges(QModelIndex,QModelIndex,QVector<int>)),Qt::UniqueConnection);
        connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(removeItemsFromFlow(QModelIndex,int,int)),Qt::UniqueConnection);

        tableView->setModel(model);

        tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    #if QT_VERSION >= 0x050000
        tableView->horizontalHeader()->setSectionsMovable(true);
    #else
        tableView->horizontalHeader()->setMovable(true);
    #endif
        //TODO parametrizar la configuración de las columnas
        for(int i = 0;i<tableView->horizontalHeader()->count();i++)
            tableView->horizontalHeader()->hideSection(i);

        tableView->horizontalHeader()->showSection(ComicModel::Number);
        tableView->horizontalHeader()->showSection(ComicModel::Title);
        tableView->horizontalHeader()->showSection(ComicModel::FileName);
        tableView->horizontalHeader()->showSection(ComicModel::NumPages);
        tableView->horizontalHeader()->showSection(ComicModel::Hash); //Size is part of the Hash...TODO add Columns::Size to Columns
        tableView->horizontalHeader()->showSection(ComicModel::ReadColumn);
        tableView->horizontalHeader()->showSection(ComicModel::CurrentPage);
        tableView->horizontalHeader()->showSection(ComicModel::Rating);

        //debido a un bug, qt4 no es capaz de ajustar el ancho teniendo en cuenta todas la filas (no sólo las visibles)
        //así que se ecala la primera vez y después se deja el control al usuario.
        //if(!settings->contains(COMICS_VIEW_HEADERS))
        tableView->resizeColumnsToContents();
        tableView->horizontalHeader()->setStretchLastSection(true);

        QStringList paths = model->getPaths(model->getCurrentPath());//TODO ComicsView: get currentpath from somewhere currentPath());
        comicFlow->setImagePaths(paths);
        comicFlow->setMarks(model->getReadList());
        //comicFlow->setFocus(Qt::OtherFocusReason);
    }

    if(settings->contains(COMICS_VIEW_HEADERS))
        tableView->horizontalHeader()->restoreState(settings->value(COMICS_VIEW_HEADERS).toByteArray());


}

void ClassicComicsView::setCurrentIndex(const QModelIndex &index)
{
    tableView->setCurrentIndex(index);
    //TODO ComicsView: scroll comicFlow to index
}

QModelIndex ClassicComicsView::currentIndex()
{
    return tableView->currentIndex();
}

QItemSelectionModel *ClassicComicsView::selectionModel()
{
    return tableView->selectionModel();
}

void ClassicComicsView::scrollTo(const QModelIndex & mi, QAbstractItemView::ScrollHint hint)
{
    comicFlow->setCenterIndex(mi.row());
}

void ClassicComicsView::toFullScreen()
{
    comicFlow->hide();
    comicFlow->setCenterIndex(comicFlow->centerIndex());
    comics->hide();

    //showFullScreen() //parent windows

    comicFlow->show();
    comicFlow->setFocus(Qt::OtherFocusReason);
}

void ClassicComicsView::toNormal()
{
    comicFlow->hide();
    comicFlow->setCenterIndex(comicFlow->centerIndex());
    comicFlow->render();
    comics->show();
    comicFlow->show();
}

void ClassicComicsView::updateConfig(QSettings *settings)
{
    comicFlow->updateConfig(settings);
}

void ClassicComicsView::setItemActions(const QList<QAction *> &actions)
{
    tableView->addActions(actions);
}

void ClassicComicsView::setViewActions(const QList<QAction *> &actions)
{
    comicFlow->addActions(actions);
}

void ClassicComicsView::enableFilterMode(bool enabled)
{
    if(enabled)
    {
        comicFlow->clear();
        if(previousSplitterStatus.isEmpty())
            previousSplitterStatus = sVertical->saveState();
        sVertical->setSizes(QList<int> () << 100 << 10000000);
        showSearchingIcon();
    }else
    {
        hideSearchingIcon();
        sVertical->restoreState(previousSplitterStatus);
        previousSplitterStatus.clear();
    }

    //sVertical->setCollapsible(0,!enabled);
    searching = enabled;
}

void ClassicComicsView::selectAll()
{
    tableView->selectAll();
}

void ClassicComicsView::selectedComicForOpening(const QModelIndex &mi)
{
    emit selected(mi.row());
}

void ClassicComicsView::setShowMarks(bool show)
{
    comicFlow->setShowMarks(show);
}

void ClassicComicsView::centerComicFlow(const QModelIndex & mi)
{
    comicFlow->showSlide(mi.row());
    comicFlow->setFocus(Qt::OtherFocusReason);
}

void ClassicComicsView::updateTableView(int i)
{
    QModelIndex mi = model->index(i,2);
    tableView->setCurrentIndex(mi);
    tableView->scrollTo(mi,QAbstractItemView::EnsureVisible);
}

void ClassicComicsView::saveTableHeadersStatus()
{
    settings->setValue(COMICS_VIEW_HEADERS,tableView->horizontalHeader()->saveState());
}

void ClassicComicsView::saveSplitterStatus()
{
    if(!searching)
        settings->setValue(COMICS_VIEW_FLOW_SPLITTER_STATUS, sVertical->saveState());
}

void ClassicComicsView::applyModelChanges(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    if(roles.contains(ComicModel::ReadColumnRole))
    {
        comicFlow->setMarks(model->getReadList());
        comicFlow->updateMarks();
    }
}

void ClassicComicsView::removeItemsFromFlow(const QModelIndex &parent, int from, int to)
{
    Q_UNUSED(parent);
    for(int i = from; i<=to; i++)
        comicFlow->remove(i);
}

void ClassicComicsView::closeEvent(QCloseEvent *event)
{
    saveTableHeadersStatus();
    saveSplitterStatus();
    ComicsView::closeEvent(event);
}

void ClassicComicsView::setupSearchingIcon()
{
    searchingIcon = new QWidget(comicFlow);

    QHBoxLayout * h = new QHBoxLayout;

    QPixmap p(":/images/searching_icon.png");
    QLabel * l = new QLabel(searchingIcon);
    l->setPixmap(p);
    l->setFixedSize(p.size());
    h->addWidget(l,0,Qt::AlignCenter);
    searchingIcon->setLayout(h);

    QPalette pal(searchingIcon->palette());
    pal.setColor(QPalette::Background, Qt::black);
    searchingIcon->setAutoFillBackground(true);
    searchingIcon->setPalette(pal);

    hideSearchingIcon();
}

void ClassicComicsView::showSearchingIcon()
{
    stack->setCurrentWidget(searchingIcon);
}

void ClassicComicsView::hideSearchingIcon()
{
    stack->setCurrentWidget(comicFlow);
}

