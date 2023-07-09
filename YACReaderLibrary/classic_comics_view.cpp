#include "classic_comics_view.h"

#include "yacreader_global.h"

#include "QStackedWidget"

#include "comic_flow_widget.h"
#include "QsLog.h"
#include "shortcuts_manager.h"
#include "yacreader_table_view.h"
#include "yacreader_tool_bar_stretch.h"

ClassicComicsView::ClassicComicsView(QWidget *parent)
    : ComicsView(parent), searching(false)
{
    auto layout = new QHBoxLayout;

    settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat); // TODO unificar la creación del fichero de config con el servidor
    settings->beginGroup("libraryConfig");
    // FLOW-----------------------------------------------------------------------
    //---------------------------------------------------------------------------
// FORCE_ANGLE is not used here, because ComicFlowWidgetGL will use OpenGL ES in the future
#ifndef NO_OPENGL
    if ((settings->value(USE_OPEN_GL).toBool() == true))
        comicFlow = new ComicFlowWidgetGL(0);
    else
        comicFlow = new ComicFlowWidgetSW(0);
#else
    comicFlow = new ComicFlowWidgetSW(0);
#endif
    comicFlow->updateConfig(settings);
    comicFlow->setFocusPolicy(Qt::StrongFocus);
    comicFlow->setShowMarks(true);
    setFocusProxy(comicFlow);

    comicFlow->setFocus(Qt::OtherFocusReason);

    comicFlow->setContextMenuPolicy(Qt::CustomContextMenu);

    // layout-----------------------------------------------
    sVertical = new QSplitter(Qt::Vertical); // spliter derecha

    stack = new QStackedWidget;
    stack->addWidget(comicFlow);
    setupSearchingIcon();
    stack->addWidget(searchingIcon);

    sVertical->addWidget(stack);
    comics = new QWidget;
    auto comicsLayout = new QVBoxLayout;
    comicsLayout->setSpacing(0);
    comicsLayout->setContentsMargins(0, 0, 0, 0);
    // TODO ComicsView:(set toolbar) comicsLayout->addWidget(editInfoToolBar);

    tableView = new YACReaderTableView;
    tableView->verticalHeader()->hide();
    tableView->setFocusPolicy(Qt::StrongFocus);
    comicsLayout->addWidget(tableView);
    comics->setLayout(comicsLayout);
    sVertical->addWidget(comics);

    tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    // config--------------------------------------------------
    if (settings->contains(COMICS_VIEW_HEADERS))
        tableView->horizontalHeader()->restoreState(settings->value(COMICS_VIEW_HEADERS).toByteArray());

    // connections---------------------------------------------
    connect(tableView, &QAbstractItemView::clicked, this, &ClassicComicsView::centerComicFlow);
    connect(tableView, &QAbstractItemView::doubleClicked, this, &ClassicComicsView::selectedComicForOpening);
    connect(comicFlow, &ComicFlowWidget::centerIndexChanged, this, &ClassicComicsView::updateTableView);
    connect(tableView, &YACReaderTableView::comicRated, this, &ComicsView::comicRated);
    connect(comicFlow, &ComicFlowWidget::selected, this, &ComicsView::selected);
    connect(tableView->horizontalHeader(), &QHeaderView::sectionMoved, this, &ClassicComicsView::saveTableHeadersStatus);
    connect(tableView->horizontalHeader(), &QHeaderView::sectionResized, this, &ClassicComicsView::saveTableHeadersStatus);
    connect(comicFlow, &QWidget::customContextMenuRequested, this, &ClassicComicsView::requestedViewContextMenu);
    connect(tableView, &QWidget::customContextMenuRequested, this, &ClassicComicsView::requestedItemContextMenu);
    layout->addWidget(sVertical);
    setLayout(layout);

    layout->setContentsMargins(0, 0, 0, 0);

#ifdef Y_MAC_UI
    sVertical->setCollapsible(1, false);
#endif

    if (settings->contains(COMICS_VIEW_FLOW_SPLITTER_STATUS))
        sVertical->restoreState(settings->value(COMICS_VIEW_FLOW_SPLITTER_STATUS).toByteArray());

    // hide flow widgets
    hideFlowViewAction = new QAction(this);
    hideFlowViewAction->setText(tr("Hide comic flow"));
    hideFlowViewAction->setData(HIDE_COMIC_VIEW_ACTION_YL);
    hideFlowViewAction->setShortcut(ShortcutsManager::getShortcutsManager().getShortcut(HIDE_COMIC_VIEW_ACTION_YL));
    hideFlowViewAction->setIcon(QIcon(":/images/comics_view_toolbar/hideComicFlow.svg"));
    hideFlowViewAction->setCheckable(true);
    hideFlowViewAction->setChecked(false);

    connect(hideFlowViewAction, &QAction::toggled, this, &ClassicComicsView::hideComicFlow);
}

void ClassicComicsView::hideComicFlow(bool hide)
{
    if (hide) {
        QList<int> sizes;
        sizes.append(0);
        int total = sVertical->sizes().at(0) + sVertical->sizes().at(1);
        sizes.append(total);
        sVertical->setSizes(sizes);
    } else {
        QList<int> sizes;
        int total = sVertical->sizes().at(0) + sVertical->sizes().at(1);
        sizes.append(2 * total / 3);
        sizes.append(total / 3);
        sVertical->setSizes(sizes);
    }
}

// the toolbar has to be populated
void ClassicComicsView::setToolBar(QToolBar *toolBar)
{
    static_cast<QVBoxLayout *>(comics->layout())->insertWidget(0, toolBar);
    this->toolbar = toolBar;

    startSeparatorAction = toolBar->addSeparator();
    toolBar->addAction(hideFlowViewAction);
}

void ClassicComicsView::setModel(ComicModel *model)
{
    ComicsView::setModel(model);

    if (model == NULL) {
        comicFlow->clear();
    } else {
        connect(model, &QAbstractItemModel::dataChanged, this, &ClassicComicsView::applyModelChanges, Qt::UniqueConnection);
        connect(model, &QAbstractItemModel::rowsRemoved, this, &ClassicComicsView::removeItemsFromFlow, Qt::UniqueConnection);
        // TODO: Missing method resortCovers?
        connect(model, &ComicModel::resortedIndexes, comicFlow, &ComicFlowWidget::resortCovers, Qt::UniqueConnection);
        connect(model, &ComicModel::newSelectedIndex, this, &ClassicComicsView::setCurrentIndex, Qt::UniqueConnection);

        tableView->setModel(model);
        if (model->rowCount() > 0)
            tableView->setCurrentIndex(model->index(0, 0));

        tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
#if QT_VERSION >= 0x050000
        tableView->horizontalHeader()->setSectionsMovable(true);
#else
        tableView->horizontalHeader()->setMovable(true);
#endif
        // TODO parametrizar la configuración de las columnas
        /*if(!settings->contains(COMICS_VIEW_HEADERS))
        {*/
        for (int i = 0; i < tableView->horizontalHeader()->count(); i++)
            tableView->horizontalHeader()->hideSection(i);

        tableView->horizontalHeader()->showSection(ComicModel::Number);
        tableView->horizontalHeader()->showSection(ComicModel::Title);
        tableView->horizontalHeader()->showSection(ComicModel::FileName);
        tableView->horizontalHeader()->showSection(ComicModel::NumPages);
        tableView->horizontalHeader()->showSection(ComicModel::Hash); // Size is part of the Hash...TODO add Columns::Size to Columns
        tableView->horizontalHeader()->showSection(ComicModel::ReadColumn);
        tableView->horizontalHeader()->showSection(ComicModel::CurrentPage);
        tableView->horizontalHeader()->showSection(ComicModel::PublicationDate);
        tableView->horizontalHeader()->showSection(ComicModel::Rating);
        //}

        // debido a un bug, qt4 no es capaz de ajustar el ancho teniendo en cuenta todas la filas (no sólo las visibles)
        // así que se ecala la primera vez y después se deja el control al usuario.
        // if(!settings->contains(COMICS_VIEW_HEADERS))

        QStringList paths = model->getPaths(model->getCurrentPath()); // TODO ComicsView: get currentpath from somewhere currentPath());
        comicFlow->setImagePaths(paths);
        comicFlow->setMarks(model->getReadList());
        // comicFlow->setFocus(Qt::OtherFocusReason);

        if (settings->contains(COMICS_VIEW_HEADERS))
            tableView->horizontalHeader()->restoreState(settings->value(COMICS_VIEW_HEADERS).toByteArray());

        tableView->resizeColumnsToContents();

        tableView->horizontalHeader()->setStretchLastSection(true);
    }
}

void ClassicComicsView::setCurrentIndex(const QModelIndex &index)
{
    tableView->setCurrentIndex(index);
    centerComicFlow(index);
}

QModelIndex ClassicComicsView::currentIndex()
{
    return tableView->currentIndex();
}

QItemSelectionModel *ClassicComicsView::selectionModel()
{
    return tableView->selectionModel();
}

void ClassicComicsView::scrollTo(const QModelIndex &mi, QAbstractItemView::ScrollHint hint)
{
    Q_UNUSED(hint);

    comicFlow->setCenterIndex(mi.row());
}

void ClassicComicsView::toFullScreen()
{
    comicFlow->hide();
    comicFlow->setCenterIndex(comicFlow->centerIndex());
    comics->hide();

    // showFullScreen() //parent windows

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

void ClassicComicsView::enableFilterMode(bool enabled)
{
    if (enabled) {
        comicFlow->clear();
        if (previousSplitterStatus.isEmpty())
            previousSplitterStatus = sVertical->saveState();
        sVertical->setSizes(QList<int>() << 100 << 10000000);
        showSearchingIcon();
    } else {
        hideSearchingIcon();
        sVertical->restoreState(previousSplitterStatus);
        previousSplitterStatus.clear();
    }

    // sVertical->setCollapsible(0,!enabled);
    searching = enabled;
}

void ClassicComicsView::selectIndex(int index)
{
    tableView->selectRow(index);
}

void ClassicComicsView::updateCurrentComicView()
{
}

void ClassicComicsView::focusComicsNavigation(Qt::FocusReason reason)
{
    const bool comicFlowVisible = stack->currentWidget() == comicFlow && sVertical->sizes().constFirst() != 0;
    if (comicFlowVisible)
        comicFlow->setFocus(reason);
    else // Let the user navigate the table.
        tableView->setFocus(reason);
}

void ClassicComicsView::selectAll()
{
    tableView->selectAll();
}

void ClassicComicsView::selectedComicForOpening(const QModelIndex &mi)
{
    emit selected(mi.row());
}

void ClassicComicsView::requestedViewContextMenu(const QPoint &point)
{
    emit customContextMenuViewRequested(comicFlow->mapTo(this, point));
}

void ClassicComicsView::requestedItemContextMenu(const QPoint &point)
{
    emit customContextMenuItemRequested(tableView->mapTo(this, point));
}

void ClassicComicsView::setShowMarks(bool show)
{
    comicFlow->setShowMarks(show);
}

void ClassicComicsView::centerComicFlow(const QModelIndex &mi)
{
    comicFlow->showSlide(mi.row());
    comicFlow->setFocus(Qt::OtherFocusReason);
}

void ClassicComicsView::updateTableView(int i)
{
    if (model == nullptr) {
        return;
    }

    QModelIndex mi = model->index(i, 2);
    tableView->setCurrentIndex(mi);
    tableView->scrollTo(mi, QAbstractItemView::EnsureVisible);
}

void ClassicComicsView::saveTableHeadersStatus()
{
    settings->setValue(COMICS_VIEW_HEADERS, tableView->horizontalHeader()->saveState());
}

void ClassicComicsView::saveSplitterStatus()
{
    settingsMutex.lock();
    if (!searching)
        settings->setValue(COMICS_VIEW_FLOW_SPLITTER_STATUS, sVertical->saveState());
    settingsMutex.unlock();
}

void ClassicComicsView::applyModelChanges(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    if (roles.contains(ComicModel::ReadColumnRole)) {
        comicFlow->setMarks(model->getReadList());
        comicFlow->updateMarks();
    }
}

void ClassicComicsView::removeItemsFromFlow(const QModelIndex &parent, int from, int to)
{
    Q_UNUSED(parent);
    for (int i = from; i <= to; i++)
        comicFlow->remove(i);
}

void ClassicComicsView::closeEvent(QCloseEvent *event)
{
    toolbar->removeAction(startSeparatorAction);
    toolbar->removeAction(hideFlowViewAction);

    saveTableHeadersStatus();
    saveSplitterStatus();
    ComicsView::closeEvent(event);
}

void ClassicComicsView::setupSearchingIcon()
{
    searchingIcon = new QWidget(comicFlow);

    auto h = new QHBoxLayout;

    QPixmap p(":/images/searching_icon.png");
    QLabel *l = new QLabel(searchingIcon);
    l->setPixmap(p);
    l->setFixedSize(p.size());
    h->addWidget(l, 0, Qt::AlignCenter);
    searchingIcon->setLayout(h);

    searchingIcon->setStyleSheet(QString("QWidget {border : none; background-color: #000000;}"));

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
