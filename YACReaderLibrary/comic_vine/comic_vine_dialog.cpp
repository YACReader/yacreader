#include "comic_vine_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QRadioButton>
#include <QMessageBox>
#include <QTableView>
#include <QtConcurrent/QtConcurrentRun>
#include <QSqlDatabase>
#include <QtScript>
#include "data_base_management.h"

#include "yacreader_busy_widget.h"
#include "comic_vine_client.h"
#include "scraper_lineedit.h"
#include "title_header.h"
#include "series_question.h"
#include "search_single_comic.h"
#include "search_volume.h"
#include "select_comic.h"
#include "select_volume.h"
#include "sort_volume_comics.h"
#include "db_helper.h"
#include "response_parser.h"



ComicVineDialog::ComicVineDialog(QWidget *parent) :
	QDialog(parent)
{
	doLayout();
	doStackedWidgets();
	doConnections();
}

void ComicVineDialog::doLayout()
{
	setStyleSheet(""
		"QDialog {background-color: #404040; }"
		"");

	QString dialogButtonsStyleSheet = "QPushButton {border: 1px solid #242424; background: #2e2e2e; color:white; padding: 5px 26px 5px 26px; font-size:12px;font-family:Arial; font-weight:bold;}";

	skipButton = new QPushButton(tr("skip"));
	backButton = new QPushButton(tr("back"));
	nextButton = new QPushButton(tr("next"));
	searchButton = new QPushButton(tr("search"));
	closeButton = new QPushButton(tr("close"));

	skipButton->setStyleSheet(dialogButtonsStyleSheet);
	backButton->setStyleSheet(dialogButtonsStyleSheet);
	nextButton->setStyleSheet(dialogButtonsStyleSheet);
	searchButton->setStyleSheet(dialogButtonsStyleSheet);
	closeButton->setStyleSheet(dialogButtonsStyleSheet);

	content = new QStackedWidget(this);

	QVBoxLayout * mainLayout = new QVBoxLayout;

	QHBoxLayout * buttonLayout = new QHBoxLayout;

	buttonLayout->addStretch();
	buttonLayout->addWidget(skipButton);
	buttonLayout->addWidget(backButton);
	buttonLayout->addWidget(nextButton);
	buttonLayout->addWidget(searchButton);
	buttonLayout->addWidget(closeButton);
	buttonLayout->setContentsMargins(0,0,0,0);

	mainLayout->addWidget(titleHeader = new TitleHeader);
	mainLayout->addWidget(content);
	mainLayout->addStretch();
	mainLayout->addLayout(buttonLayout);

	mainLayout->setContentsMargins(26,16,26,11);

	setLayout(mainLayout);
	setFixedSize(872,529);

	setWindowTitle("Comic Vine Scraper (beta)");
}

void ComicVineDialog::doStackedWidgets()
{
	doLoading();
	content->addWidget(seriesQuestionWidget = new SeriesQuestion);
	content->addWidget(searchSingleComicWidget = new SearchSingleComic);
	content->addWidget(searchVolumeWidget = new SearchVolume);
	content->addWidget(selectVolumeWidget = new SelectVolume);
	content->addWidget(selectComicWidget = new SelectComic);
	content->addWidget(sortVolumeComicsWidget = new SortVolumeComics);
}

void ComicVineDialog::doConnections()
{
	connect(closeButton,SIGNAL(clicked()),this,SLOT(close()));
	connect(nextButton,SIGNAL(clicked()),this,SLOT(goNext()));
	connect(backButton,SIGNAL(clicked()),this,SLOT(goBack()));
	connect(searchButton,SIGNAL(clicked()),this,SLOT(search()));
}

void ComicVineDialog::goNext()
{
	//
	if(content->currentWidget() == seriesQuestionWidget)
	{
		if(seriesQuestionWidget->getYes())
		{
			QString volumeSearchString = comics[0].getParentFolderName();

			if(volumeSearchString.isEmpty())
				showSearchVolume();
			else
			{
				showLoading();
				searchVolume(volumeSearchString);
				status = AutoSearching;
			}

			mode = Volume;
		}
		else
		{
			ComicDB comic = comics[currentIndex];
			QString title = comic.getTitleOrFileName();
			titleHeader->setSubTitle(tr("comic %1 of %2 - %3").arg(currentIndex+1).arg(comics.length()).arg(title));
			showLoading();

			searchVolume(title);
			status = AutoSearching;
			mode = SingleComicInList;
		}
	}
	else if (content->currentWidget() == selectVolumeWidget) {
		showLoading();

		status = GettingVolumeComics;

		ComicVineClient * comicVineClient = new ComicVineClient;
		if(mode == Volume)
			connect(comicVineClient,SIGNAL(volumeComicsInfo(QString)),this,SLOT(showSortVolumeComics(QString)));
		else
			connect(comicVineClient,SIGNAL(volumeComicsInfo(QString)),this,SLOT(showSelectComic(QString)));
		connect(comicVineClient,SIGNAL(timeOut()),this,SLOT(queryTimeOut()));
		connect(comicVineClient,SIGNAL(finished()),comicVineClient,SLOT(deleteLater()));
		comicVineClient->getVolumeComicsInfo(selectVolumeWidget->getSelectedVolumeId());
	} else if (content->currentWidget() == sortVolumeComicsWidget) {
		showLoading();

		//ComicDB-ComicVineID
		QList<QPair<ComicDB,QString> > matchingInfo = sortVolumeComicsWidget->getMatchingInfo();
		QtConcurrent::run(this, &ComicVineDialog::getComicsInfo,matchingInfo);
	}
}

void ComicVineDialog::goBack()
{
	switch (status) {
	case SelectingSeries:
		if(mode == Volume)
			showSearchVolume();
		else
			showSearchSingleComic();
		break;
	case SortingComics:
		showSelectVolume();
		break;
	default:
		break;
	}
}

void ComicVineDialog::setComics(const QList<ComicDB> & comics)
{
	this->comics = comics;
}

void ComicVineDialog::show()
{
	QDialog::show();

	currentIndex = 0;

	if(comics.length() == 1)
	{
		ComicDB singleComic = comics[0];
		QString title = singleComic.getTitleOrFileName();
		titleHeader->setSubTitle(title);
		showLoading();

		searchVolume(title);
		status = AutoSearching;
		mode = SingleComic;

	}else if(comics.length()>1)
	{
		titleHeader->setSubTitle(tr("%1 comics selected").arg(comics.length()));
		showSeriesQuestion();
	}
}

void ComicVineDialog::doLoading()
{
	QWidget * w = new QWidget;
	QVBoxLayout * l = new QVBoxLayout;

	YACReaderBusyWidget * bw = new YACReaderBusyWidget;

	l->addStretch();
	l->addWidget(bw,0,Qt::AlignHCenter);
	l->addStretch();

	l->setContentsMargins(0,0,0,0);
	w->setLayout(l);
	w->setContentsMargins(0,0,0,0);

	content->addWidget(w);
}

void ComicVineDialog::debugClientResults(const QString & string)
{
	ResponseParser p;
	p.loadJSONResponse(string);
	//QMessageBox::information(0,"Result", QString("Number of results : %1").arg(p.getNumResults()));
	if(p.responseError())
	{
		QMessageBox::critical(0,"Error from ComicVine", "-");
		goBack();
	}
	else
	{
		switch(mode)
		{
		case SingleComic: case SingleComicInList:
			if(p.getNumResults() == 0)
				showSearchSingleComic();
			else
				if(status == SearchingVolume)
					showSelectVolume(string);
				else
					showSelectComic(string);
			break;
		case Volume:
			if(p.getNumResults() == 0)
				showSearchVolume();
			else
				showSelectVolume(string);
			break;
		}
	}
}

void ComicVineDialog::showSeriesQuestion()
{
	status = AskingForInfo;
	content->setCurrentWidget(seriesQuestionWidget);
	backButton->setHidden(true);
	skipButton->setHidden(true);
	nextButton->setVisible(true);
	searchButton->setHidden(true);
	closeButton->setVisible(true);
}

void ComicVineDialog::showSearchSingleComic()
{
	status = AskingForInfo;
	content->setCurrentWidget(searchSingleComicWidget);
	backButton->setHidden(true);
	skipButton->setHidden(true);
	nextButton->setHidden(true);
	searchButton->setVisible(true);
	closeButton->setVisible(true);
}

void ComicVineDialog::showSearchVolume()
{
	status = AskingForInfo;
	content->setCurrentWidget(searchVolumeWidget);
	backButton->setHidden(true);
	nextButton->setHidden(true);
	searchButton->setVisible(true);
	closeButton->setVisible(true);

	if (mode == SingleComicInList)
		skipButton->setVisible(true);
	else
		skipButton->setHidden(true);
}

void ComicVineDialog::showSelectVolume(const QString & json)
{
	showSelectVolume();
	selectVolumeWidget->load(json);
}

void ComicVineDialog::showSelectVolume()
{
	status = SelectingSeries;

	content->setCurrentWidget(selectVolumeWidget);

	backButton->setVisible(true);
	nextButton->setVisible(true);
	searchButton->setHidden(true);
	closeButton->setVisible(true);
}

void ComicVineDialog::showSelectComic(const QString &json)
{
	content->setCurrentWidget(selectComicWidget);
	selectComicWidget->load(json);
}

void ComicVineDialog::showSortVolumeComics(const QString &json)
{
	status = SortingComics;

	content->setCurrentWidget(sortVolumeComicsWidget);

	sortVolumeComicsWidget->setData(comics, json);

	backButton->setVisible(true);
	nextButton->setVisible(true);
	searchButton->setHidden(true);
	closeButton->setVisible(true);
}

void ComicVineDialog::queryTimeOut()
{
	QMessageBox::warning(this,"Comic Vine error", "Time out connecting to Comic Vine");

	switch (status) {
	case AutoSearching:
		if(mode == Volume)
			showSearchVolume();
		else
			showSearchSingleComic();
		break;
	case SearchingVolume:
		if(mode == Volume)
			showSearchVolume();
		else
			showSearchSingleComic();
		break;
	case SearchingSingleComic:
		showSearchSingleComic();
		break;
	case GettingVolumeComics:
		showSelectVolume();
		break;
	default:
		break;
	}
}

void ComicVineDialog::getComicsInfo(QList<QPair<ComicDB, QString> > & matchingInfo)
{
	QPair<ComicDB, QString> p;
	QList<ComicDB> comics;
	foreach (p, matchingInfo) {
		ComicVineClient * comicVineClient = new ComicVineClient;
		//connect(comicVineClient,SIGNAL(searchResult(QString)),this,SLOT(debugClientResults(QString)));
		//connect(comicVineClient,SIGNAL(timeOut()),this,SLOT(queryTimeOut()));
		//connect(comicVineClient,SIGNAL(finished()),comicVineClient,SLOT(deleteLater()));
		QByteArray result = comicVineClient->getComicDetail(p.second); //TODO check timeOut or Connection error

		comics.push_back(parseComicInfo(p.first,result)); //TODO check result error
	}

	QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
	db.open();
	db.transaction();
	foreach(ComicDB comic, comics)
	{
		DBHelper::update(&(comic.info),db);
	}
	db.commit();
	db.close();
	QSqlDatabase::removeDatabase(databasePath);

	close();
	emit accepted();
}

ComicDB ComicVineDialog::parseComicInfo(ComicDB & comic, const QString & json)
{
	QScriptEngine engine;
	QScriptValue sc;
	sc = engine.evaluate("(" + json + ")");

	if (!sc.property("error").isValid() && sc.property("error").toString() != "OK")
	{
		qDebug("Error detected");
	}
	else
	{
		int numResults = sc.property("number_of_total_results").toString().toInt(); //fix to weird behaviour using hasNext

		if(numResults > 0)
		{
			QScriptValue result = sc.property("results");

			QString title = result.property("name").toString();

			QString number = result.property("issue_number").toString();
			QString count; //get from select volume


			QString volume = result.property("volume").property("name").toString();
			QString storyArc; //story_arc
			QString arcNumber; //??
			QString arcCount; //count_of_issue_appearances

			QString genere; //no

			QString writer;
			QString penciller;
			QString inker;
			QString colorist;
			QString letterer;
			QString coverArtist;

			QString date = result.property("cover_date").toString();

			QString publisher; //get from select volume
			QString format; //no
			bool color; //no
			QString ageRating; //no

			QString synopsis = result.property("description").toString(); //description
			QString characters;

			comic.info.setTitle(title);

			comic.info.setNumber(number.toInt());

			QStringList tempList = date.split("-");
			std::reverse(tempList.begin(),tempList.end());
			comic.info.setDate(tempList.join("/"));
			comic.info.setVolume(volume);
		}
	}
	return comic;
}

void ComicVineDialog::showLoading()
{
	content->setCurrentIndex(0);
	backButton->setHidden(true);
	skipButton->setHidden(true);
	nextButton->setHidden(true);
	searchButton->setHidden(true);
	closeButton->setVisible(true);
}

void ComicVineDialog::search()
{
	switch (mode) {
	case Volume:
		launchSearchVolume();
		break;
	default:
		launchSearchComic();
		break;
	}
}

void ComicVineDialog::searchVolume(const QString &v)
{
	ComicVineClient * comicVineClient = new ComicVineClient;
	connect(comicVineClient,SIGNAL(searchResult(QString)),this,SLOT(debugClientResults(QString)));
	connect(comicVineClient,SIGNAL(timeOut()),this,SLOT(queryTimeOut()));
	connect(comicVineClient,SIGNAL(finished()),comicVineClient,SLOT(deleteLater()));
	comicVineClient->search(v);

	status = SearchingVolume;
}

void ComicVineDialog::launchSearchVolume()
{
	showLoading();
	//TODO: check if volume info is empty.
	searchVolume(searchVolumeWidget->getVolumeInfo());
}

void ComicVineDialog::launchSearchComic()
{
	showLoading();

	QString volumeInfo = searchSingleComicWidget->getVolumeInfo();
	QString comicInfo = searchSingleComicWidget->getComicInfo();
	int comicNumber = searchSingleComicWidget->getComicNumber();

	if(comicInfo.isEmpty() && comicNumber == -1)
		searchVolume(volumeInfo);
}

