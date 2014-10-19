#include "empty_folder_widget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPainter>
#include <QStringListModel>

#include "comic.h"
#include "comic_files_manager.h"
#include "QsLog.h"

void testListView(QListView * l)
{
    QStringListModel * slm = new QStringListModel(QStringList() << "Lorem ipsum" << "Hailer skualer"<< "Mumbaluba X" << "Finger layden" << "Pacum tactus filer" << "Aposum" << "En" << "Lorem ipsum" << "Hailer skualer" << "Mumbaluba X" << "Finger layden" << "Pacum tactus filer" << "Aposum" << "En" );
    l->setModel(slm);
}

EmptyFolderWidget::EmptyFolderWidget(QWidget *parent) :
    QWidget(parent),subfoldersModel(new QStringListModel())
{
    QVBoxLayout * layout = new QVBoxLayout;

    iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap(":/images/empty_folder.png"));
    iconLabel->setAlignment(Qt::AlignCenter);

    titleLabel = new QLabel("Subfolders in this folder");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel {color:#CCCCCC; font-size:24px;font-family:Arial;font-weight:bold;}");

    foldersView = new QListView();
    foldersView->setMinimumWidth(282);
    foldersView->setWrapping(true);

    foldersView->setStyleSheet("QListView {background-color:transparent; border: none; color:#858585; outline:0; font-size: 18px; font:bold; show-decoration-selected: 0; margin:0}"
                               "QListView::item:selected {background-color: #212121; color:#CCCCCC;}"
                               "QListView::item:hover {background-color:#212121; color:#CCCCCC; }"


                               "QScrollBar:vertical { border: none; background: #212121; width: 14px; margin: 0 10px 0 0; }"
                               "QScrollBar::handle:vertical { background: #858585; width: 14px; min-height: 20px; }"
                               "QScrollBar::add-line:vertical { border: none; background: #212121; height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"

                               "QScrollBar::sub-line:vertical {  border: none; background: #212121; height: 0px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                               "QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
                               "QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"

                               "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none; }"
                               "QScrollBar:horizontal{height:0px;}"
                               );

    foldersView->setSizePolicy(QSizePolicy ::Expanding , QSizePolicy ::Expanding );
    testListView(foldersView);

    layout->addSpacing(100);
    layout->addWidget(iconLabel);
    layout->addSpacing(30);
    layout->addWidget(titleLabel);
    layout->addSpacing(12);
    layout->addWidget(foldersView,1,Qt::AlignHCenter);
    layout->addStretch();
    layout->setMargin(0);
    layout->setSpacing(0);

    setContentsMargins(0,0,0,0);

    setStyleSheet("QWidget {background:#2A2A2A}");

    setSizePolicy(QSizePolicy ::Expanding , QSizePolicy ::Expanding );
    setLayout(layout);

    setAcceptDrops(true);

    connect(foldersView,SIGNAL(clicked(QModelIndex)),this,SLOT(onItemClicked(QModelIndex)));
}

void EmptyFolderWidget::setSubfolders(const QModelIndex &mi, const QStringList &foldersNames)
{
    parent = mi;
    subfoldersModel->setStringList(foldersNames);
    foldersView->setModel(subfoldersModel);
}

void EmptyFolderWidget::onItemClicked(const QModelIndex &mi)
{
    emit subfolderSelected(parent,mi.row());
}

void EmptyFolderWidget::paintEvent(QPaintEvent *)
{
    QPainter painter (this);
    painter.fillRect(0,0,width(),height(),QColor("#2A2A2A"));
}

//TODO remove repeated code in drag & drop support....
void EmptyFolderWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> urlList;

    if (event->mimeData()->hasUrls())
    {
        urlList = event->mimeData()->urls();
        QString currentPath;
        foreach (QUrl url, urlList)
        {
            //comics or folders are accepted, folders' content is validate in dropEvent (avoid any lag before droping)
            currentPath = url.toLocalFile();
            if(Comic::fileIsComic(currentPath) || QFileInfo(currentPath).isDir())
            {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void EmptyFolderWidget::dropEvent(QDropEvent *event)
{
    QLOG_DEBUG() << "drop in emptyfolder" << event->dropAction();

    bool validAction = event->dropAction() == Qt::CopyAction || event->dropAction() & Qt::MoveAction;

    if(validAction)
    {

        QList<QPair<QString, QString> > droppedFiles = ComicFilesManager::getDroppedFiles(event->mimeData()->urls());

        if(event->dropAction() == Qt::CopyAction)
        {
            QLOG_DEBUG() << "copy in emptyfolder:" << droppedFiles;
            emit copyComicsToCurrentFolder(droppedFiles);
        }
        else if(event->dropAction() & Qt::MoveAction)
        {
            QLOG_DEBUG() << "move in emptyfolder:" << droppedFiles;
            emit moveComicsToCurrentFolder(droppedFiles);
        }

        event->acceptProposedAction();
    }
}
