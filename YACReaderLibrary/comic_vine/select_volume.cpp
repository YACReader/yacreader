#include "select_volume.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QModelIndex>
#include <QScrollArea>
#include <QDesktopServices>
#include <QHeaderView>
#include <QToolButton>
#include <QSortFilterProxyModel>

#include "scraper_tableview.h"

#include <QtScript>

#include "volumes_model.h"
#include "comic_vine_client.h"
#include "scraper_scroll_label.h"

#include "response_parser.h"
#include "scraper_results_paginator.h"

SelectVolume::SelectVolume(QWidget *parent)
	:ScraperSelector(parent),model(0)
{
	proxyModel = new QSortFilterProxyModel;

	QString labelStylesheet = "QLabel {color:white; font-size:12px;font-family:Arial;}";

	QLabel * label = new QLabel(tr("Please, select the right series for your comic."));
	label->setStyleSheet(labelStylesheet);

	QVBoxLayout * l = new QVBoxLayout;
	QWidget * leftWidget = new QWidget;
	QVBoxLayout * left = new QVBoxLayout;
    QGridLayout * content = new QGridLayout;

	//widgets
	cover = new QLabel();
	cover->setScaledContents(true);
	cover->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
	cover->setMinimumSize(168,168*5.0/3);
	cover->setStyleSheet("QLabel {background-color: #2B2B2B; color:white; font-size:12px; font-family:Arial; }");
    detailLabel = new ScraperScrollLabel();

    tableVolumes = new ScraperTableView();
	tableVolumes->setSortingEnabled(true);
#if QT_VERSION >= 0x050000
	tableVolumes->horizontalHeader()->setSectionsClickable(true);
#else
    tableVolumes->horizontalHeader()->setClickable(true);
#endif
	//tableVolumes->horizontalHeader()->setSortIndicatorShown(false);
	connect(tableVolumes->horizontalHeader(),SIGNAL(sectionClicked(int)), tableVolumes, SLOT(sortByColumn(int)));
	//connections
	connect(tableVolumes,SIGNAL(clicked(QModelIndex)),this,SLOT(loadVolumeInfo(QModelIndex)));

	paginator->setCustomLabel(tr("volumes"));

	left->addWidget(cover);
	left->addWidget(detailLabel,1);
	leftWidget->setMaximumWidth(180);
	leftWidget->setLayout(left);
	left->setContentsMargins(0,0,0,0);
	leftWidget->setContentsMargins(0,0,0,0);

    content->addWidget(leftWidget, 0, 0);
    content->addWidget(tableVolumes, 0, 1);
    content->addWidget(paginator, 1, 1);

    content->setColumnStretch(1, 1);
    content->setRowStretch(0, 1);

    l->addSpacing(15);
	l->addWidget(label);
    l->addSpacing(5);
    l->addLayout(content);

	l->setContentsMargins(0,0,0,0);
	setLayout(l);
	setContentsMargins(0,0,0,0);
}

void SelectVolume::load(const QString & json, const QString & searchString)
{
	VolumesModel * tempM = new VolumesModel();
	tempM->load(json);
	//tableVolumes->setModel(tempM);

	proxyModel->setSourceModel( tempM );
	tableVolumes->setModel(proxyModel);
	tableVolumes->sortByColumn(0,Qt::AscendingOrder);
	tableVolumes->resizeColumnsToContents();

	if(model != 0)
		delete model;

	model = tempM;

	if(model->rowCount()>0)
	{
		tableVolumes->selectRow(0);
		loadVolumeInfo(proxyModel->index(0,0));
    }

    tableVolumes->setColumnWidth(0,350);

	ScraperSelector::load(json,searchString);
}

SelectVolume::~SelectVolume() {}

void SelectVolume::loadVolumeInfo(const QModelIndex & omi)
{
	QModelIndex mi = proxyModel->mapToSource(omi);
	QString coverURL = model->getCoverURL(mi);
	QString id =  model->getVolumeId(mi);

	QString loadingStyle = "<font color='#AAAAAA'>%1</font>";
	cover->setText(loadingStyle.arg(tr("loading cover")));
	detailLabel->setAltText(loadingStyle.arg(tr("loading description")));

	ComicVineClient * comicVineClient = new ComicVineClient;
	connect(comicVineClient,SIGNAL(seriesCover(const QByteArray &)),this,SLOT(setCover(const QByteArray &)));
	connect(comicVineClient,SIGNAL(finished()),comicVineClient,SLOT(deleteLater()));
	comicVineClient->getSeriesCover(coverURL);

	ComicVineClient * comicVineClient2 = new ComicVineClient;
	connect(comicVineClient2,SIGNAL(seriesDetail(QString)),this,SLOT(setDescription(QString)));
	connect(comicVineClient2,SIGNAL(finished()),comicVineClient2,SLOT(deleteLater()));
	comicVineClient2->getSeriesDetail(id);
}

void SelectVolume::setCover(const QByteArray & data)
{
	QPixmap p;
	p.loadFromData(data);
	int w = p.width();
	int h = p.height();

	cover->setPixmap(p);
	float aspectRatio = static_cast<float>(w)/h;

	cover->setFixedSize(180,static_cast<int>(180/aspectRatio));

	cover->update();
}

void SelectVolume::setDescription(const QString & jsonDetail)
{
	QScriptEngine engine;
	QScriptValue sc;
	sc = engine.evaluate("(" + jsonDetail + ")");

	if (!sc.property("error").isValid() && sc.property("error").toString() != "OK")
	{
		qDebug("Error detected");
	}
	else
	{

		QScriptValue descriptionValues = sc.property("results").property("description");
		bool valid = !descriptionValues.isNull() && descriptionValues.isValid();
		detailLabel->setText(valid?descriptionValues.toString().replace("<a","<a style = 'color:#827A68; text-decoration:none;'"):tr("description unavailable"));
	}
}

QString SelectVolume::getSelectedVolumeId()
{
	return model->getVolumeId(proxyModel->mapToSource(tableVolumes->currentIndex()));
}

int SelectVolume::getSelectedVolumeNumIssues()
{
	return model->getNumIssues(proxyModel->mapToSource(tableVolumes->currentIndex()));
}

QString SelectVolume::getSelectedVolumePublisher()
{
	return model->getPublisher(proxyModel->mapToSource(tableVolumes->currentIndex()));
}


