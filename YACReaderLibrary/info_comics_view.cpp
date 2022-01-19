#include "info_comics_view.h"

#include <QtQuick>
#include <QQuickWidget>

#include "comic.h"
#include "comic_files_manager.h"
#include "comic_model.h"
#include "comic_db.h"
#include "yacreader_comic_info_helper.h"
#include "yacreader_comics_selection_helper.h"

#include "QsLog.h"

InfoComicsView::InfoComicsView(QWidget *parent)
    : ComicsView(parent)
{

    // container->setFocusPolicy(Qt::StrongFocus);

    QQmlContext *ctxt = view->rootContext();

    LibraryUITheme theme;
#ifdef Q_OS_MAC
    theme = Light;
#else
    theme = Dark;
#endif

    if (theme == Light) {
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

    view->setSource(QUrl("qrc:/qml/InfoComicsView.qml"));

    auto rootObject = dynamic_cast<QObject *>(view->rootObject());
    flow = rootObject->findChild<QObject *>("flow");
    list = rootObject->findChild<QObject *>("list");

    // QML signals only work with old style signal slot syntax
    connect(flow, SIGNAL(currentCoverChanged(int)), this, SLOT(updateInfoForIndex(int))); // clazy:exclude=old-style-connect
    connect(flow, SIGNAL(currentCoverChanged(int)), this, SLOT(setCurrentIndex(int))); // clazy:exclude=old-style-connect

    selectionHelper = new YACReaderComicsSelectionHelper(this);
    comicInfoHelper = new YACReaderComicInfoHelper(this);

    auto l = new QVBoxLayout;
    l->addWidget(view);
    this->setLayout(l);

    setContentsMargins(0, 0, 0, 0);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

    setShowMarks(true);

    QLOG_TRACE() << "GridComicsView";
}

InfoComicsView::~InfoComicsView()
{
    delete view;
}

void InfoComicsView::setToolBar(QToolBar *toolBar)
{
    static_cast<QVBoxLayout *>(this->layout())->insertWidget(1, toolBar);
    this->toolbar = toolBar;
}

void InfoComicsView::setModel(ComicModel *model)
{
    if (model == NULL)
        return;

    selectionHelper->setModel(model);
    comicInfoHelper->setModel(model);

    ComicsView::setModel(model);

    QQmlContext *ctxt = view->rootContext();

    /*if(_selectionModel != NULL)
        delete _selectionModel;

    _selectionModel = new QItemSelectionModel(model);*/

    ctxt->setContextProperty("comicsList", model);
    if (model->rowCount() > 0)
        ctxt->setContextProperty("backgroundImage", this->model->data(this->model->index(0, 0), ComicModel::CoverPathRole));
    else
        ctxt->setContextProperty("backgroundImage", QUrl());

    ctxt->setContextProperty("comicsSelection", selectionHelper->selectionModel());
    ctxt->setContextProperty("contextMenuHelper", this);
    ctxt->setContextProperty("currentIndexHelper", this);
    ctxt->setContextProperty("comicInfoHelper", comicInfoHelper);
    /*ctxt->setContextProperty("comicsSelectionHelper", this);
    ctxt->setContextProperty("dragManager", this);*/
    ctxt->setContextProperty("dropManager", this);

    if (model->rowCount() > 0) {
        setCurrentIndex(model->index(0, 0));
        updateInfoForIndex(0);
    }
}

void InfoComicsView::setCurrentIndex(const QModelIndex &index)
{
    QQmlProperty(list, "currentIndex").write(index.row());

    selectionHelper->clear();
    selectionHelper->selectIndex(index.row());
}

void InfoComicsView::setCurrentIndex(int index)
{
    selectionHelper->clear();
    selectionHelper->selectIndex(index);
}

QModelIndex InfoComicsView::currentIndex()
{
    return selectionHelper->currentIndex();
}

QItemSelectionModel *InfoComicsView::selectionModel()
{
    return selectionHelper->selectionModel();
}

void InfoComicsView::scrollTo(const QModelIndex &mi, QAbstractItemView::ScrollHint hint)
{
    Q_UNUSED(mi);
    Q_UNUSED(hint);
}

void InfoComicsView::toFullScreen()
{
    toolbar->hide();
}

void InfoComicsView::toNormal()
{
    toolbar->show();
}

void InfoComicsView::updateConfig(QSettings *settings)
{
    Q_UNUSED(settings);
}

void InfoComicsView::enableFilterMode(bool enabled)
{
    Q_UNUSED(enabled);
}

void InfoComicsView::selectIndex(int index)
{
    selectionHelper->selectIndex(index);
}

void InfoComicsView::updateCurrentComicView()
{
}

void InfoComicsView::focusComicsNavigation(Qt::FocusReason reason)
{
    view->setFocus(reason);
}

void InfoComicsView::setShowMarks(bool show)
{
    QQmlContext *ctxt = view->rootContext();
    ctxt->setContextProperty("show_marks", show);
}

void InfoComicsView::selectAll()
{
    selectionHelper->selectAll();
}

bool InfoComicsView::canDropUrls(const QList<QUrl> &urls, Qt::DropAction action)
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

void InfoComicsView::droppedFiles(const QList<QUrl> &urls, Qt::DropAction action)
{
    bool validAction = action == Qt::CopyAction; // TODO add move

    if (validAction) {
        QList<QPair<QString, QString>> droppedFiles = ComicFilesManager::getDroppedFiles(urls);
        emit copyComicsToCurrentFolder(droppedFiles);
    }
}

void InfoComicsView::requestedContextMenu(const QPoint &point)
{
    emit customContextMenuViewRequested(point);
}

void InfoComicsView::selectedItem(int index)
{
    emit selected(index);
}
