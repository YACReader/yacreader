#include "info_comics_view.h"

#include "yacreader_global.h"
#include "theme_manager.h"

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
    : ComicsView(parent), flow(nullptr), list(nullptr)
{

    // container->setFocusPolicy(Qt::StrongFocus);

    QQmlContext *ctxt = view->rootContext();

    // Apply theme colors
    initTheme(this);

    ctxt->setContextProperty("backgroundImage", QUrl());
    ctxt->setContextProperty("comicsList", new ComicModel());

    view->setSource(QUrl("qrc:/qml/InfoComicsView.qml"));

    selectionHelper = new YACReaderComicsSelectionHelper(this);
    comicInfoHelper = new YACReaderComicInfoHelper(this);

    auto l = new QVBoxLayout;
    l->addWidget(view);
    this->setLayout(l);

    setContentsMargins(0, 0, 0, 0);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

    setShowMarks(true);

    QLOG_TRACE() << "InfoComicsView";
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

    if (flow != nullptr) {
        flow->disconnect();
    }

    if (list != nullptr) {
        list->disconnect();
    }

    auto rootObject = dynamic_cast<QQuickItem *>(view->rootObject());
    flow = rootObject->findChild<QQuickItem *>("flow", Qt::FindChildrenRecursively);
    list = rootObject->findChild<QQuickItem *>("list", Qt::FindChildrenRecursively);

    // QML signals only work with old style signal slot syntax
    connect(flow, SIGNAL(currentCoverChanged(int)), this, SLOT(updateInfoForIndex(int))); // clazy:exclude=old-style-connect
    connect(flow, SIGNAL(currentCoverChanged(int)), this, SLOT(setCurrentIndex(int))); // clazy:exclude=old-style-connect
}

void InfoComicsView::setCurrentIndex(const QModelIndex &index)
{
    if (list != nullptr) {
        QQmlProperty(list, "currentIndex").write(index.row());
    }

    setCurrentIndex(index.row());
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

void InfoComicsView::applyTheme(const Theme &theme)
{
    QQmlContext *ctxt = view->rootContext();
    const auto &giv = theme.gridAndInfoView;

    // Info panel colors
    // Cache-bust the SVG file URLs so QML's image cache doesn't serve stale
    // files when the theme is updated (the same file path is rewritten each time).
    const QString bust = QString::number(QDateTime::currentMSecsSinceEpoch());
    auto svgUrl = [&bust](const QString &path) {
        QUrl url = QUrl::fromLocalFile(path);
        url.setQuery(bust);
        return url;
    };
    ctxt->setContextProperty("infoBackgroundColor", giv.infoBackgroundColor);
    ctxt->setContextProperty("topShadow", svgUrl(giv.topShadow));
    ctxt->setContextProperty("infoShadow", svgUrl(giv.infoShadow));
    ctxt->setContextProperty("infoIndicator", svgUrl(giv.infoIndicator));
    ctxt->setContextProperty("infoMetadataTextColor", giv.infoMetadataTextColor);
    ctxt->setContextProperty("infoTextColor", giv.infoTextColor);

    // Rating and favorite colors
    ctxt->setContextProperty("ratingUnselectedColor", giv.ratingUnselectedColor);
    ctxt->setContextProperty("ratingSelectedColor", giv.ratingSelectedColor);
    ctxt->setContextProperty("favUncheckedColor", giv.favUncheckedColor);
    ctxt->setContextProperty("favCheckedColor", giv.favCheckedColor);
    ctxt->setContextProperty("readTickUncheckedColor", giv.readTickUncheckedColor);
    ctxt->setContextProperty("readTickCheckedColor", giv.readTickCheckedColor);

    ctxt->setContextProperty("showDropShadow", QVariant(giv.showDropShadow));
    ctxt->setContextProperty("backgroundBlurOverlayColor", giv.backgroundBlurOverlayColor);

    // Info panel scrollbar, comic cover border, links
    ctxt->setContextProperty("infoScrollbarColor", giv.infoScrollbarColor);
    ctxt->setContextProperty("comicCoverBorderColor", giv.comicCoverBorderColor);
    ctxt->setContextProperty("themeLinkColor", giv.linkColor);
    ctxt->setContextProperty("themeLinkColorStr", giv.linkColor.name());
}
