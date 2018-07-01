#include "sort_volume_comics.h"

#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QAction>

#include "scraper_tableview.h"
#include "local_comic_list_model.h"
#include "volume_comics_model.h"

SortVolumeComics::SortVolumeComics(QWidget *parent) :
	ScraperSelector(parent)
{
	QString labelStylesheet = "QLabel {color:white; font-size:12px;font-family:Arial;}";

	QLabel * label = new QLabel(tr("Please, sort the list of comics on the left until it matches the comics' information."));
	label->setStyleSheet(labelStylesheet);

	QLabel * sortLabel = new QLabel(tr("sort comics to match comic information"));
	sortLabel->setStyleSheet(labelStylesheet);

	moveUpButtonCL = new ScrapperToolButton(ScrapperToolButton::LEFT);
	moveUpButtonCL->setIcon(QIcon(":/images/comic_vine/rowUp.png"));
	moveUpButtonCL->setAutoRepeat(true);
	moveDownButtonCL = new ScrapperToolButton(ScrapperToolButton::RIGHT);
	moveDownButtonCL->setIcon(QIcon(":/images/comic_vine/rowDown.png"));
	moveDownButtonCL->setAutoRepeat(true);
	//moveUpButtonIL = new ScrapperToolButton(ScrapperToolButton::LEFT);
	//moveUpButtonIL->setIcon(QIcon(":/images/comic_vine/rowUp.png"));
	//moveDownButtonIL = new ScrapperToolButton(ScrapperToolButton::RIGHT);
	//moveDownButtonIL->setIcon(QIcon(":/images/comic_vine/rowDown.png"));

	connect(moveUpButtonCL,SIGNAL(clicked()),this,SLOT(moveUpCL()));
	connect(moveDownButtonCL,SIGNAL(clicked()),this,SLOT(moveDownCL()));
	//connect(moveUpButtonIL,SIGNAL(clicked()),this,SLOT(moveUpIL()));
	//connect(moveUpButtonIL,SIGNAL(clicked()),this,SLOT(moveDownIL()));

	QVBoxLayout * l = new QVBoxLayout;
    QGridLayout * content = new QGridLayout;
	QHBoxLayout * sortButtonsLayout = new QHBoxLayout;

	tableFiles = new ScraperTableView();
	tableVolumeComics = new ScraperTableView();

	tableFiles->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableFiles->setSelectionMode(QAbstractItemView::ContiguousSelection);

	//content->addWidget(tableVolumes,0,Qt::AlignRight|Qt::AlignTop);

	connect(tableVolumeComics->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(synchronizeScroll(int)));
	connect(tableFiles->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(synchronizeScroll(int)));

	//connect(tableVolumeComics, SIGNAL(pressed(QModelIndex)), tableFiles, SLOT(setCurrentIndex(QModelIndex)));
	//connect(tableFiles, SIGNAL(pressed(QModelIndex)), tableVolumeComics, SLOT(setCurrentIndex(QModelIndex)));

	paginator->setCustomLabel(tr("issues"));
	paginator->setMinimumWidth(422);

	sortButtonsLayout->addWidget(moveUpButtonCL);
	sortButtonsLayout->addWidget(ScrapperToolButton::getSeparator());
	sortButtonsLayout->addWidget(moveDownButtonCL);
	sortButtonsLayout->addSpacing(10);
	sortButtonsLayout->addWidget(sortLabel);
    sortButtonsLayout->addStretch();
    sortButtonsLayout->setSpacing(0);

    content->addWidget(tableFiles, 0, 0);
    content->addWidget(tableVolumeComics, 0, 1);
    content->addLayout(sortButtonsLayout, 1, 0);
    content->addWidget(paginator, 1, 1);

    content->setRowStretch(0, 1);

    l->addSpacing(15);
    l->addWidget(label, 0);
	l->addSpacing(5);
    l->addLayout(content, 1);
    l->addLayout(sortButtonsLayout, 0);

	l->setContentsMargins(0,0,0,0);
	setLayout(l);
	setContentsMargins(0,0,0,0);

    //rows actions
    QAction * removeItemFromList = new QAction(tr("remove selected comics"),this);
    QAction * restoreAllItems = new QAction(tr("restore all removed comics"),this);
    //QAction * restoreItems = new QAction(tr("restore removed comics"),this);

    tableFiles->setContextMenuPolicy(Qt::ActionsContextMenu);
    tableFiles->addAction(removeItemFromList);
    tableFiles->addAction(restoreAllItems);
    //tableFiles->addAction(restoreItems);

    connect(removeItemFromList,SIGNAL(triggered()),this,SLOT(removeSelectedComics()));
    connect(restoreAllItems,SIGNAL(triggered()),this,SLOT(restoreAllComics()));
    //connect(restoreItems,SIGNAL(triggered()),this,SLOT(showRemovedComicsSelector()));
}

void SortVolumeComics::setData(QList<ComicDB> & comics, const QString &json, const QString &vID)
{
	//set up models
	localComicsModel = new LocalComicListModel;
	localComicsModel->load(comics);

	volumeComicsModel = new VolumeComicsModel;
	volumeComicsModel->load(json);

	int numLocalComics = localComicsModel->rowCount();
	int numVolumeComics = volumeComicsModel->rowCount();

	if(numLocalComics > numVolumeComics)
		volumeComicsModel->addExtraRows(numLocalComics - numVolumeComics);
	if(numLocalComics < numVolumeComics)
		localComicsModel->addExtraRows(numVolumeComics - numLocalComics);

	tableFiles->setModel(localComicsModel);
	tableVolumeComics->setModel(volumeComicsModel);

	tableVolumeComics->resizeColumnToContents(0);

	ScraperSelector::load(json,vID);
}

void SortVolumeComics::synchronizeScroll(int pos)
{
	void * senderObject = sender();

	if(senderObject == 0) //invalid call
		return;

	QScrollBar * tableVolumeComicsScrollBar = tableVolumeComics->verticalScrollBar();
	QScrollBar * tableFilesScrollBar = tableFiles->verticalScrollBar();

	if(senderObject == tableVolumeComicsScrollBar)
	{
		disconnect(tableFilesScrollBar,SIGNAL(valueChanged(int)),this,0);
		tableFilesScrollBar->setValue(pos);
		connect(tableFilesScrollBar, SIGNAL(valueChanged(int)), this, SLOT(synchronizeScroll(int)));
	}
	else
	{
		disconnect(tableVolumeComicsScrollBar,SIGNAL(valueChanged(int)),this,0);
		tableVolumeComicsScrollBar->setValue(pos);
		connect(tableVolumeComicsScrollBar, SIGNAL(valueChanged(int)), this, SLOT(synchronizeScroll(int)));
	}
}

void SortVolumeComics::moveUpCL()
{
	QList<QModelIndex> selection = tableFiles->selectionModel()->selectedIndexes();

	if(selection.count() == 0)
		return;

	localComicsModel->moveSelectionUp(selection);

	selection = tableFiles->selectionModel()->selectedIndexes();
	tableFiles->scrollTo(selection.first());
}

void SortVolumeComics::moveDownCL()
{
	QList<QModelIndex> selection = tableFiles->selectionModel()->selectedIndexes();

	if(selection.count() > 0)
    {
		localComicsModel->moveSelectionDown(selection);

        selection = tableFiles->selectionModel()->selectedIndexes();
        tableFiles->scrollTo(selection.last());
    }
}

void SortVolumeComics::moveUpIL()
{

}

void SortVolumeComics::moveDownIL()
{

}

void SortVolumeComics::removeSelectedComics()
{
    QList<QModelIndex> selection = tableFiles->selectionModel()->selectedIndexes();

    localComicsModel->removeComics(selection);
}

void SortVolumeComics::restoreAllComics()
{
    localComicsModel->restoreAll();
}

void SortVolumeComics::showRemovedComicsSelector()
{

}

QList<QPair<ComicDB, QString> > SortVolumeComics::getMatchingInfo()
{
	QList<ComicDB> comicList = localComicsModel->getData();
	QList<QPair<ComicDB, QString> > l;

	int index = 0;

	QString id;
	foreach(ComicDB c, comicList)
	{
		id = volumeComicsModel->getComicId(index);
		if(!c.getFileName().isEmpty() && !id.isEmpty()) //there is a valid comic, and valid comic ID
		{
			l.push_back(QPair<ComicDB, QString>(c,id));
		}
		index++;
	}

	return l;
}
