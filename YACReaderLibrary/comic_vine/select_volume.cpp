#include "select_volume.h"

#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QModelIndex>
#include <QScrollArea>
#include <QDesktopServices>

#include <QtScript>

#include "volumes_model.h"
#include "comic_vine_client.h"

SelectVolume::SelectVolume(QWidget *parent)
	:QWidget(parent),model(0)
{
	QString labelStylesheet = "QLabel {color:white; font-size:12px;font-family:Arial;}";
	QString tableStylesheet = "QTableView {color:white; border:0px;alternate-background-color: #2E2E2E;background-color: #2B2B2B; outline: 0px;}"
			"QTableView::item {outline: 0px; border: 0px; color:#FFFFFF;}"
			"QTableView::item:selected {outline: 0px; background-color: #555555;  }"
			"QHeaderView::section:horizontal {background-color:#292929; border-bottom:1px solid #1F1F1F; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #292929, stop: 1 #1F1F1F); border-left:none; border-top:none; padding:4px; color:#ebebeb;}"
			"QHeaderView::section:vertical {border-bottom: 1px solid #DFDFDF;border-top: 1px solid #FEFEFE;}"
			"QScrollBar:vertical { border: none; background: #2B2B2B; width: 3px; margin: 0; }"
			"QScrollBar:horizontal { border: none; background: #2B2B2B; height: 3px; margin: 0; }"
			"QScrollBar::handle:vertical { background: #DDDDDD; width: 7px; min-height: 20px; }"
			"QScrollBar::handle:horizontal { background: #DDDDDD; width: 7px; min-height: 20px; }"
			"QScrollBar::add-line:vertical { border: none; background: #404040; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"
			"QScrollBar::sub-line:vertical {  border: none; background: #404040; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
			"QScrollBar::add-line:horizontal { border: none; background: #404040; width: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 0 3px 0;}"
			"QScrollBar::sub-line:horizontal {  border: none; background: #404040; width: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 0 3px 0;}"
			"QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
			"QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"
			"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical, QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {background: none; }";

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
	detailLabel = new QLabel();
	detailLabel->setStyleSheet("QLabel {background-color: #2B2B2B; color:white; font-size:12px; font-family:Arial; }");
	detailLabel->setWordWrap(true);

	detailLabel->setMinimumSize(168,12);

	connect(detailLabel,SIGNAL(linkActivated(QString)),this,SLOT(openLink(QString)));

	QScrollArea * scroll = new QScrollArea(this);
	scroll->setWidget(detailLabel);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setStyleSheet(
				"QScrollArea {background-color:#2B2B2B; border:none;}"
				"QScrollBar:vertical { border: none; background: #2B2B2B; width: 3px; margin: 0; }"
				"QScrollBar:horizontal { border: none; background: #2B2B2B; height: 3px; margin: 0; }"
				"QScrollBar::handle:vertical { background: #DDDDDD; width: 7px; min-height: 20px; }"
				"QScrollBar::handle:horizontal { background: #DDDDDD; width: 7px; min-height: 20px; }"
				"QScrollBar::add-line:vertical { border: none; background: #404040; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"
				"QScrollBar::sub-line:vertical {  border: none; background: #404040; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
				"QScrollBar::add-line:horizontal { border: none; background: #404040; width: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 0 3px 0;}"
				"QScrollBar::sub-line:horizontal {  border: none; background: #404040; width: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 0 3px 0;}"
				"QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
				"QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"
				"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical, QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {background: none; }"

			);
	//scroll->setWidgetResizable(true);
	//iScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	//iScroll->show();

	tableVolumes = new QTableView();
	tableVolumes->setStyleSheet(tableStylesheet);

	tableVolumes->setShowGrid(false);
#if QT_VERSION >= 0x050000
	tableVolumes->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#else
	tableVolumes->verticalHeader()->setResizeMode(QHeaderView::Fixed);
#endif

	//comicView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	tableVolumes->horizontalHeader()->setStretchLastSection(true);
#if QT_VERSION >= 0x050000
	tableVolumes->horizontalHeader()->setSectionsClickable(false);
#else
	tableVolumes->horizontalHeader()->setClickable(false);
#endif
	//comicView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	tableVolumes->verticalHeader()->setDefaultSectionSize(24);
#if QT_VERSION >= 0x050000
	tableVolumes->verticalHeader()->setSectionsClickable(false); //TODO comportamiento anómalo
#else
	tableVolumes->verticalHeader()->setClickable(false); //TODO comportamiento anómalo
#endif

	tableVolumes->setCornerButtonEnabled(false);

	tableVolumes->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableVolumes->setSelectionMode(QAbstractItemView::ExtendedSelection);

	tableVolumes->setAlternatingRowColors(true);

	tableVolumes->verticalHeader()->hide();

	tableVolumes->setSelectionMode(QAbstractItemView::SingleSelection);

	//connections
	connect(tableVolumes,SIGNAL(clicked(QModelIndex)),this,SLOT(loadVolumeInfo(QModelIndex)));

	left->addWidget(cover);
	left->addWidget(scroll,1);
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

	tableVolumes->setFixedSize(419,341);

	if(tempM->rowCount()>0)
	{
		tableVolumes->selectRow(0);
		loadVolumeInfo(tempM->index(0,0));
	}

	if(model != 0)
		delete model;
	else
		model = tempM;
}

SelectVolume::~SelectVolume() {}

void SelectVolume::loadVolumeInfo(const QModelIndex & mi)
{
	QStringList * data = static_cast<QStringList *>(mi.internalPointer());
	QString coverURL = data->at(VolumesModel::COVER_URL);
	QString deck = data->at(VolumesModel::DECK);
	QString id = data->at(VolumesModel::ID);

	//cover->setText(coverURL);
	//detailLabel->setText(deck);
	QString loadingStyle = "<font color='#AAAAAA'>%1</font>";
	cover->setText(loadingStyle.arg(tr("loading cover")));
	detailLabel->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
	detailLabel->setText(loadingStyle.arg(tr("loading description")));
	detailLabel->adjustSize();

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
		detailLabel->setAlignment(Qt::AlignTop|Qt::AlignLeft);
		detailLabel->setText(sc.property("results").property("description").toString().replace("<a","<a style = 'color:#827A68; text-decoration:none;'"));
		detailLabel->adjustSize();
	}
}

void SelectVolume::openLink(const QString & link)
{
	QDesktopServices::openUrl(QUrl("http://www.comicvine.com"+link));
}

