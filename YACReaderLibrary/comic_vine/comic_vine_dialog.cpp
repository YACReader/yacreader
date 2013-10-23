#include "comic_vine_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QRadioButton>
#include <QMessageBox>
#include <QTableView>

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

		ComicVineClient * comicVineClient = new ComicVineClient;
		connect(comicVineClient,SIGNAL(volumeComicsInfo(QString)),this,SLOT(showSortVolumeComics(QString)));
		connect(comicVineClient,SIGNAL(finished()),comicVineClient,SLOT(deleteLater()));
		comicVineClient->getVolumeComicsInfo(selectVolumeWidget->getSelectedVolumeId());
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
		QMessageBox::critical(0,"Error from ComicVine", "-");
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
	status = SelectingSeries;

	content->setCurrentWidget(selectVolumeWidget);

	backButton->setVisible(true);
	nextButton->setVisible(true);
	searchButton->setHidden(true);
	closeButton->setVisible(true);

	selectVolumeWidget->load(json);
}

void ComicVineDialog::showSelectComic(const QString &json)
{
	content->setCurrentWidget(selectComicWidget);
	selectComicWidget->load(json);
}

void ComicVineDialog::showSortVolumeComics(const QString &json)
{
	content->setCurrentWidget(sortVolumeComicsWidget);
	sortVolumeComicsWidget->setData(comics, json);
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

