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
#include "scrapper_lineedit.h"
#include "title_header.h"
#include "series_question.h"
#include "search_single_comic.h"
#include "search_volume.h"
#include "select_comic.h"
#include "select_volume.h"

#include "response_parser.h"


ComicVineDialog::ComicVineDialog(QWidget *parent) :
	QDialog(parent),comicVineClient(new ComicVineClient)
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
	setFixedSize(672,529);
}

void ComicVineDialog::doStackedWidgets()
{
	doLoading();
	content->addWidget(seriesQuestion = new SeriesQuestion);
	content->addWidget(searchSingleComic = new SearchSingleComic);
	content->addWidget(searchVolume = new SearchVolume);
	content->addWidget(selectVolume = new SelectVolume);
}

void ComicVineDialog::doConnections()
{
	connect(closeButton,SIGNAL(clicked()),this,SLOT(close()));
	connect(nextButton,SIGNAL(clicked()),this,SLOT(goNext()));
	connect(searchButton,SIGNAL(clicked()),this,SLOT(search()));

	connect(comicVineClient,SIGNAL(searchResult(QString)),this,SLOT(debugClientResults(QString)));
}

void ComicVineDialog::goNext()
{
	//
	if(content->currentWidget() == seriesQuestion)
	{
		if(seriesQuestion->getYes())
		{
			QString volumeSearchString = comics[0].getParentFolderName();

			if(volumeSearchString.isEmpty())
				showSearchVolume();
			else
			{
				showLoading();
				comicVineClient->search(volumeSearchString);
			}

			status = Volume;
		}
		else
		{
			ComicDB comic = comics[currentIndex];
			QString title = comic.getTitleOrPath();
			titleHeader->setSubTitle(tr("comic %1 of %2 - %3").arg(currentIndex+1).arg(comics.length()).arg(title));
			showLoading();

			comicVineClient->search(title);

			status = SingleComicInSeries;
		}
	}
	else if (content->currentWidget() == searchSingleComic) {

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
		QString title = singleComic.getTitleOrPath();
		titleHeader->setSubTitle(title);
		showLoading();

		comicVineClient->search(title);

		status = SingleComic;
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

	QString debug = QString("%1 \n %2").arg(p.getNumResults()).arg(string);

	QMessageBox::information(0,"-Response-", debug);

	switch(status)
	{
	case SingleComic:
		showSearchSingleComic();
		break;
	case Volume:
		showSearchVolume();
		break;
	case SingleComicInSeries:
		showSearchSingleComic();
		break;
	}


}

void ComicVineDialog::showSeriesQuestion()
{
	content->setCurrentWidget(seriesQuestion);
	backButton->setHidden(true);
	skipButton->setHidden(true);
	nextButton->setVisible(true);
	searchButton->setHidden(true);
	closeButton->setVisible(true);
}

void ComicVineDialog::showSearchSingleComic()
{
	content->setCurrentWidget(searchSingleComic);
	backButton->setHidden(true);
	skipButton->setHidden(true);
	nextButton->setHidden(true);
	searchButton->setVisible(true);
	closeButton->setVisible(true);
}

void ComicVineDialog::showSearchVolume()
{
	content->setCurrentWidget(searchVolume);
	backButton->setHidden(true);
	nextButton->setHidden(true);
	searchButton->setVisible(true);
	closeButton->setVisible(true);

	if (status == SingleComicInSeries)
		skipButton->setVisible(true);
	else
		skipButton->setHidden(true);
}

void ComicVineDialog::showSelectVolume()
{
	content->setCurrentWidget(selectVolume);
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
	switch (status) {
	case Volume:
		launchSearchVolume();
		break;
	default:
		launchSearchComic();
		break;
	}
}

void ComicVineDialog::launchSearchVolume()
{
	showLoading();
	//TODO: check if volume info is empty.
	comicVineClient->search(searchVolume->getVolumeInfo());
}

void ComicVineDialog::launchSearchComic()
{
	showLoading();

	QString volumeInfo = searchSingleComic->getVolumeInfo();
	QString comicInfo = searchSingleComic->getComicInfo();
	int comicNumber = searchSingleComic->getComicNumber();

	if(comicInfo.isEmpty() && comicNumber == -1)
		comicVineClient->search(volumeInfo);
}

