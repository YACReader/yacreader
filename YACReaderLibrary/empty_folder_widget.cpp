#include "empty_folder_widget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPainter>
#include <QStringListModel>

#include "comic.h"
#include "comic_files_manager.h"
#include "QsLog.h"

#include "theme.h"

void testListView(QListView *l)
{
    QStringListModel *slm = new QStringListModel(QStringList() << "Lorem ipsum"
                                                               << "Hailer skualer"
                                                               << "Mumbaluba X"
                                                               << "Finger layden"
                                                               << "Pacum tactus filer"
                                                               << "Aposum"
                                                               << "En"
                                                               << "Lorem ipsum"
                                                               << "Hailer skualer"
                                                               << "Mumbaluba X"
                                                               << "Finger layden"
                                                               << "Pacum tactus filer"
                                                               << "Aposum"
                                                               << "En");
    l->setModel(slm);
}

class ListviewDelegate : public QStyledItemDelegate
{
public:
    ListviewDelegate()
        : QStyledItemDelegate() {}

    virtual ~ListviewDelegate() {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        painter->save();

        QFontMetrics fm(option.font);
        QString text = qvariant_cast<QString>(index.data(Qt::DisplayRole));

        QRect textRect = option.rect;

        textRect.setLeft(std::max(0, (option.rect.size().width() - fm.width(text)) / 2));

        painter->drawText(textRect, text);

        painter->restore();

        //TODO add mouse hover style ??
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        QFontMetrics fm(option.font);
        QString text = qvariant_cast<QString>(index.data(Qt::DisplayRole));

        return QSize(fm.width(text), fm.height());
    }
};

EmptyFolderWidget::EmptyFolderWidget(QWidget *parent)
    : EmptyContainerInfo(parent), subfoldersModel(new QStringListModel())
{
    QVBoxLayout *layout = setUpDefaultLayout(false);

    auto theme = Theme::currentTheme();

    iconLabel->setPixmap(QPixmap(theme.emptyFolderWidgetImage));
    titleLabel->setText(tr("Subfolders in this folder"));

    foldersView = new QListView();
    foldersView->setAttribute(Qt::WA_MacShowFocusRect, false);
    foldersView->setItemDelegate(new ListviewDelegate);

    foldersView->setStyleSheet(theme.emptyFolderWidgetStyle);

    foldersView->setSizePolicy(QSizePolicy ::Expanding, QSizePolicy ::Expanding);

    layout->addSpacing(12);
    layout->addWidget(foldersView, 1);
    layout->addStretch();
    layout->setMargin(0);
    layout->setSpacing(0);

    setContentsMargins(0, 0, 0, 0);

    setStyleSheet(QString("QWidget {background:%1}").arg(backgroundColor));

    setSizePolicy(QSizePolicy ::Expanding, QSizePolicy ::Expanding);

    setAcceptDrops(true);

    connect(foldersView, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
}

void EmptyFolderWidget::setSubfolders(const QModelIndex &mi, const QStringList &foldersNames)
{
    parent = mi;
    subfoldersModel->setStringList(foldersNames);
    foldersView->setModel(subfoldersModel);

    if (foldersNames.isEmpty()) {
        titleLabel->setText(tr("Empty folder") + QString("<p style='color:rgb(150,150,150);font-size:14px;font-weight:normal;'>%1</p>").arg(tr("Drag and drop folders and comics here")));
    } else {
        titleLabel->setText(tr("Subfolders in this folder"));
    }
}

void EmptyFolderWidget::onItemClicked(const QModelIndex &mi)
{
    emit subfolderSelected(parent, mi.row());
}

//TODO remove repeated code in drag & drop support....
void EmptyFolderWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> urlList;

    if (event->mimeData()->hasUrls() && event->dropAction() == Qt::CopyAction) {
        urlList = event->mimeData()->urls();
        QString currentPath;
        foreach (QUrl url, urlList) {
            //comics or folders are accepted, folders' content is validate in dropEvent (avoid any lag before droping)
            currentPath = url.toLocalFile();
            if (Comic::fileIsComic(currentPath) || QFileInfo(currentPath).isDir()) {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void EmptyFolderWidget::dropEvent(QDropEvent *event)
{
    QLOG_DEBUG() << "drop in emptyfolder" << event->dropAction();

    bool validAction = event->dropAction() == Qt::CopyAction; // || event->dropAction() & Qt::MoveAction;  TODO move

    if (validAction) {

        QList<QPair<QString, QString>> droppedFiles = ComicFilesManager::getDroppedFiles(event->mimeData()->urls());

        if (event->dropAction() == Qt::CopyAction) {
            QLOG_DEBUG() << "copy in emptyfolder:" << droppedFiles;
            emit copyComicsToCurrentFolder(droppedFiles);
        } else if (event->dropAction() & Qt::MoveAction) {
            QLOG_DEBUG() << "move in emptyfolder:" << droppedFiles;
            emit moveComicsToCurrentFolder(droppedFiles);
        }

        event->acceptProposedAction();
    }
}
