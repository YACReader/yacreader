#include "comics_view.h"

#include "QsLog.h"
#include "comic.h"
#include "comic_db.h"
#include "comic_files_manager.h"

#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>

#include <utility>

ComicsView::ComicsView(QWidget *parent)
    : QWidget(parent), model(nullptr), comicDB(nullptr)
{
    qmlRegisterType<ComicModel>("com.yacreader.ComicModel", 1, 0, "ComicModel");
    qmlRegisterType<ComicDB>("com.yacreader.ComicDB", 1, 0, "ComicDB");
    qmlRegisterType<ComicInfo>("com.yacreader.ComicInfo", 1, 0, "ComicInfo");

    // QML renders into its own native window. A QQuickWidget would make Qt compose the whole
    // library window through RHI and copy its full backing store on every repaint (QTBUG-120565),
    // which makes the library slow on large screens.
    view = new QQuickView();

    view->setResizeMode(QQuickView::SizeRootObjectToView);
    connect(
            view, &QQuickView::statusChanged, this,
            [=, this](QQuickView::Status status) {
                if (status == QQuickView::Error) {
                    QLOG_ERROR() << view->errors();
                }
            });

    // No parent: the QML based views add the container to their layout
    container = QWidget::createWindowContainer(view);
    container->setFocusPolicy(Qt::StrongFocus);
    view->installEventFilter(this);

    comicDB = new ComicDB();
    auto comicInfo = &(comicDB->info);
    QQmlContext *ctxt = view->rootContext();

    ctxt->setContextProperty("comic", comicDB);
    ctxt->setContextProperty("comicInfo", comicInfo);

    ctxt->setContextProperty("comic_info_index", 0);

    setAcceptDrops(true);
}

ComicsView::~ComicsView()
{
    // Views that do not show QML (e.g. the classic view) never add the container to a layout
    if (container->parentWidget() == nullptr)
        delete container;
}

bool ComicsView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == view && event->type() == QEvent::MouseButtonPress) {
        // A click does not give keyboard focus to an embedded child window (Windows only
        // activates the top-level window), so move the focus to the view here.
        if (!container->hasFocus())
            container->setFocus(Qt::MouseFocusReason);
        else if (QGuiApplication::focusWindow() != view)
            view->requestActivate();
    }

    return QWidget::eventFilter(watched, event);
}

void ComicsView::setModel(ComicModel *m)
{
    model = m;
}

void ComicsView::reloadContent()
{
    if (model != nullptr) {
        model->reload();
        updateInfoForIndex(currentIndex().row());
    }
}

void ComicsView::updateInfoForIndex(int index)
{
    QQmlContext *ctxt = view->rootContext();

    // Clear the member before destroying the object. Deleting ComicDB notifies
    // QML and can re-enter this method; an invalid index must also not leave a
    // dangling pointer that is deleted again when the next comic is selected.
    delete std::exchange(comicDB, nullptr);

    if ((index < 0) || (index >= model->rowCount())) {
        ctxt->setContextProperty("comic", nullptr);
        ctxt->setContextProperty("comicInfo", nullptr);
        ctxt->setContextProperty("comic_info_index", -1);
        return;
    }

    comicDB = new ComicDB(model->getComic(this->model->index(index, 0)));
    ComicInfo *comicInfo = &(comicDB->info);
    comicInfo->isFavorite = model->isFavorite(model->index(index, 0));

    ctxt->setContextProperty("comic", comicDB);
    ctxt->setContextProperty("comicInfo", comicInfo);

    ctxt->setContextProperty("comic_info_index", index);
}

bool ComicsView::canImportDrop(const QDropEvent *event) const
{
    if (model != nullptr && model->canDropMimeData(event->mimeData(), event->proposedAction(), 0, 0, QModelIndex()))
        return true;

    QLOG_TRACE() << "dragEnterEvent";
    if (event->mimeData()->hasUrls() && event->dropAction() == Qt::CopyAction) {
        const auto urlList = event->mimeData()->urls();
        QString currentPath;
        for (const auto &url : urlList) {
            // comics or folders are accepted, folders' content is validate in dropEvent (avoid any lag before droping)
            currentPath = url.toLocalFile();
            if (Comic::fileIsComic(currentPath) || QFileInfo(currentPath).isDir())
                return true;
        }
    }

    return false;
}

void ComicsView::dragEnterEvent(QDragEnterEvent *event)
{
    if (canImportDrop(event))
        event->acceptProposedAction();
}

void ComicsView::dropEvent(QDropEvent *event)
{
    importDrop(event);
}

void ComicsView::importDrop(QDropEvent *event)
{
    QLOG_DEBUG() << "drop" << event->dropAction();

    bool validAction = event->dropAction() == Qt::CopyAction; // || event->dropAction() & Qt::MoveAction;  TODO move

    if (event->mimeData()->hasUrls() && validAction) {

        QList<QPair<QString, QString>> droppedFiles = ComicFilesManager::getDroppedFiles(event->mimeData()->urls());

        if (event->dropAction() == Qt::CopyAction) {
            QLOG_DEBUG() << "copy :" << droppedFiles;
            emit copyComicsToCurrentFolder(droppedFiles);
        } else if (event->dropAction() & Qt::MoveAction) {
            QLOG_DEBUG() << "move :" << droppedFiles;
            emit moveComicsToCurrentFolder(droppedFiles);
        }

        event->acceptProposedAction();
    }
}
