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

	QLabel * label = new QLabel(tr("Please, sort the list of comics info on the right until it matches your comics."));
	label->setStyleSheet(labelStylesheet);

	QLabel * sortLabel = new QLabel(tr("sort comic info to match your comic files"));
	sortLabel->setStyleSheet(labelStylesheet);

	moveUpButtonCL = new ScrapperToolButton(ScrapperToolButton::LEFT);
	moveUpButtonCL->setIcon(QIcon(":/images/comic_vine/rowUp.png"));
	moveDownButtonCL = new ScrapperToolButton(ScrapperToolButton::RIGHT);
	moveDownButtonCL->setIcon(QIcon(":/images/comic_vine/rowDown.png"));
	moveUpButtonIL = new ScrapperToolButton(ScrapperToolButton::LEFT);
	moveUpButtonIL->setIcon(QIcon(":/images/comic_vine/rowUp.png"));
	moveDownButtonIL = new ScrapperToolButton(ScrapperToolButton::RIGHT);
	moveDownButtonIL->setIcon(QIcon(":/images/comic_vine/rowDown.png"));

	QVBoxLayout * l = new QVBoxLayout;
	QHBoxLayout * content = new QHBoxLayout;
	QHBoxLayout * sortButtonsLayout = new QHBoxLayout;

	tableFiles = new ScraperTableView();
	tableVolumeComics = new ScraperTableView();

	tableFiles->setFixedSize(407,341);
	tableVolumeComics->setFixedSize(407,341);
	content->addWidget(tableFiles,0,Qt::AlignLeft|Qt::AlignTop);
	content->addWidget(tableVolumeComics,0,Qt::AlignRight|Qt::AlignTop);
	//content->addWidget(tableVolumes,0,Qt::AlignRight|Qt::AlignTop);

	connect(tableVolumeComics->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(synchronizeScroll(int)));
	connect(tableFiles->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(synchronizeScroll(int)));

	connect(tableVolumeComics, SIGNAL(pressed(QModelIndex)), tableFiles, SLOT(setCurrentIndex(QModelIndex)));
	connect(tableFiles, SIGNAL(pressed(QModelIndex)), tableVolumeComics, SLOT(setCurrentIndex(QModelIndex)));

	sortButtonsLayout->addWidget(moveUpButtonCL);
	sortButtonsLayout->addWidget(ScrapperToolButton::getSeparator());
	sortButtonsLayout->addWidget(moveDownButtonCL);
	sortButtonsLayout->addStretch();
	sortButtonsLayout->addWidget(sortLabel);
	sortButtonsLayout->addStretch();
	sortButtonsLayout->addWidget(moveUpButtonIL);
	sortButtonsLayout->addWidget(ScrapperToolButton::getSeparator());
	sortButtonsLayout->addWidget(moveDownButtonIL);
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
