#include "select_volume.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QModelIndex>
#include <QScrollArea>
#include <QDesktopServices>
#include <QHeaderView>

#include "scraper_tableview.h"

#include <QtScript>

#include "volumes_model.h"
#include "comic_vine_client.h"
#include "scraper_scroll_label.h"

SelectVolume::SelectVolume(QWidget *parent)
	:QWidget(parent),model(0)
{
	QString labelStylesheet = "QLabel {color:white; font-size:12px;font-family:Arial;}";

	QLabel * label = new QLabel(tr("Please, select the right series for your comic."));
	label->setStyleSheet(labelStylesheet);

	QVBoxLayout * l = new QVBoxLayout;
	QWidget * leftWidget = new QWidget;
	QVBoxLayout * left = new QVBoxLayout;
	QHBoxLayout * content = new QHBoxLayout;

	//widgets
	cover = new QLabel();
	cover->setScaledContents(true);
	cover->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
	cover->setMinimumSize(168,168*5.0/3);
	cover->setStyleSheet("QLabel {background-color: #2B2B2B; color:white; font-size:12px; font-family:Arial; }");
	detailLabel = new ScraperScrollLabel(this);

	tableVolumes = new ScraperTableView(this);
	//connections
	connect(tableVolumes,SIGNAL(clicked(QModelIndex)),this,SLOT(loadVolumeInfo(QModelIndex)));

	left->addWidget(cover);
	left->addWidget(detailLabel,1);
	left->addStretch();
	leftWidget->setMaximumWidth(180);
	leftWidget->setLayout(left);
	left->setContentsMargins(0,0,0,0);
	leftWidget->setContentsMargins(0,0,0,0);

	content->addWidget(leftWidget);
	content->addWidget(tableVolumes,0,Qt::AlignRight|Qt::AlignTop);

	l->addSpacing(15);
	l->addWidget(label);
	l->addSpacing(5);
	l->addLayout(content);
	l->addStretch();

	l->setContentsMargins(0,0,0,0);
	setLayout(l);
	setContentsMargins(0,0,0,0);
}

void SelectVolume::load(const QString & json)
{
	VolumesModel * tempM = new VolumesModel();
	tempM->load(json);
	tableVolumes->setModel(tempM);

	tableVolumes->resizeColumnsToContents();

	tableVolumes->setFixedSize(619,341);

	if(model != 0)
		delete model;

	model = tempM;

	if(model->rowCount()>0)
	{
		tableVolumes->selectRow(0);
		loadVolumeInfo(model->index(0,0));
	}

	tableVolumes->setColumnWidth(0,350);
}

SelectVolume::~SelectVolume() {}

void SelectVolume::loadVolumeInfo(const QModelIndex & mi)
{
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
	return model->getVolumeId(tableVolumes->currentIndex());
}

