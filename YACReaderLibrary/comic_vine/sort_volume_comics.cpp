#include "sort_volume_comics.h"

#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QScrollBar>

#include "scrapper_tableview.h"
#include "local_comic_list_model.h"

SortVolumeComics::SortVolumeComics(QWidget *parent) :
	QWidget(parent)
{
	QString labelStylesheet = "QLabel {color:white; font-size:12px;font-family:Arial;}";

	QLabel * label = new QLabel(tr("Please, sort the list of comics info on the right until it matches your comics."));
	label->setStyleSheet(labelStylesheet);

	QLabel * sortLabel = new QLabel(tr("sort comic info to match your comic files"));
	moveUpButton = new QPushButton;
	moveDownButton = new QPushButton;

	QVBoxLayout * l = new QVBoxLayout;
	QHBoxLayout * content = new QHBoxLayout;
	QHBoxLayout * sortButtonsLayout = new QHBoxLayout;

	tableFiles = new ScrapperTableView();
	tableVolumeComics = new ScrapperTableView();

	tableFiles->setFixedSize(407,341);
	tableVolumeComics->setFixedSize(407,341);
	content->addWidget(tableFiles,0,Qt::AlignLeft|Qt::AlignTop);
	content->addWidget(tableVolumeComics,0,Qt::AlignRight|Qt::AlignTop);
	//content->addWidget(tableVolumes,0,Qt::AlignRight|Qt::AlignTop);

	connect(tableVolumeComics->verticalScrollBar(), SIGNAL(valueChanged(int)), tableFiles->verticalScrollBar(), SLOT(setValue(int)));

	sortButtonsLayout->addStretch();
	sortButtonsLayout->addWidget(sortLabel);
	sortButtonsLayout->addWidget(moveUpButton);
	sortButtonsLayout->addWidget(moveDownButton);

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

	tableFiles->setModel(localComicsModel);
}
