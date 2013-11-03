#include "sort_volume_comics.h"

#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QScrollBar>

#include "scraper_tableview.h"
#include "local_comic_list_model.h"
#include "volume_comics_model.h"

SortVolumeComics::SortVolumeComics(QWidget *parent) :
	QWidget(parent)
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
	QHBoxLayout * content = new QHBoxLayout;
	QHBoxLayout * sortButtonsLayout = new QHBoxLayout;

	tableFiles = new ScraperTableView();
	tableVolumeComics = new ScraperTableView();

	tableFiles->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableFiles->setSelectionMode(QAbstractItemView::ContiguousSelection);

	tableFiles->setFixedSize(407,341);
	tableVolumeComics->setFixedSize(407,341);
	content->addWidget(tableFiles,0,Qt::AlignLeft|Qt::AlignTop);
	content->addWidget(tableVolumeComics,0,Qt::AlignRight|Qt::AlignTop);
	//content->addWidget(tableVolumes,0,Qt::AlignRight|Qt::AlignTop);

	connect(tableVolumeComics->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(synchronizeScroll(int)));
	connect(tableFiles->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(synchronizeScroll(int)));

	//connect(tableVolumeComics, SIGNAL(pressed(QModelIndex)), tableFiles, SLOT(setCurrentIndex(QModelIndex)));
	//connect(tableFiles, SIGNAL(pressed(QModelIndex)), tableVolumeComics, SLOT(setCurrentIndex(QModelIndex)));

	sortButtonsLayout->addWidget(moveUpButtonCL);
	sortButtonsLayout->addWidget(ScrapperToolButton::getSeparator());
	sortButtonsLayout->addWidget(moveDownButtonCL);
	sortButtonsLayout->addSpacing(10);
	//sortButtonsLayout->addStretch();
	sortButtonsLayout->addWidget(sortLabel);
	//sortButtonsLayout->addStretch();
	//sortButtonsLayout->addWidget(moveUpButtonIL);
	//sortButtonsLayout->addWidget(ScrapperToolButton::getSeparator());
	//sortButtonsLayout->addWidget(moveDownButtonIL);
	sortButtonsLayout->setSpacing(0);

	l->addSpacing(15);
	l->addWidget(label);
	l->addSpacing(5);
	l->addLayout(content);
	l->addLayout(sortButtonsLayout);
	l->addStretch();

	l->setContentsMargins(0,0,0,0);
	setLayout(l);
	setContentsMargins(0,0,0,0);
}

void SortVolumeComics::setData(QList<ComicDB> & comics, const QString &json)
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
		localComicsModel->moveSelectionDown(selection);

	selection = tableFiles->selectionModel()->selectedIndexes();
	tableFiles->scrollTo(selection.last());
}

void SortVolumeComics::moveUpIL()
{

}

void SortVolumeComics::moveDownIL()
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
