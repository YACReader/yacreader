#include "comic_vine_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QRadioButton>
#include <QMessageBox>
#include <QTableView>
#if QT_VERSION >= 0x050000
	#include <QtConcurrent/QtConcurrentRun>
#else
	#include <QtConcurrentRun>
#endif
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
	connect(skipButton,SIGNAL(clicked()),this,SLOT(goToNextComic()));
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
                showLoading(tr("Looking for volume..."));
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

            showLoading(tr("Looking for volume..."));
			searchVolume(title);
			status = AutoSearching;
			mode = SingleComicInList;
		}
	}
	else if (content->currentWidget() == selectVolumeWidget) {
        showLoading(tr("Retrieving volume info..."));

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
        int count = selectVolumeWidget->getSelectedVolumeNumIssues();
        QString publisher = selectVolumeWidget->getSelectedVolumePublisher();
        QtConcurrent::run(this, &ComicVineDialog::getComicsInfo,matchingInfo,count,publisher);
    } else if (content->currentWidget() == selectComicWidget)
    {
        showLoading();
        QString comicId = selectComicWidget->getSelectedComicId();
        int count = selectVolumeWidget->getSelectedVolumeNumIssues();
        QString publisher = selectVolumeWidget->getSelectedVolumePublisher();
        QtConcurrent::run(this, &ComicVineDialog::getComicInfo,comicId,count,publisher);
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
    case SelectingComic:
        if(mode == SingleComic)
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
        showLoading(tr("Looking for volume..."));

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
    loadingMessage = new QLabel;

    loadingMessage->setStyleSheet("QLabel {color:white; font-size:12px;font-family:Arial;}");

	l->addStretch();
	l->addWidget(bw,0,Qt::AlignHCenter);
    l->addStretch();
    l->addWidget(loadingMessage);


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

	if(mode == SingleComicInList)
		skipButton->setVisible(true);
	else
		skipButton->setHidden(true);
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

	if(mode == SingleComicInList)
		skipButton->setVisible(true);
	else
		skipButton->setHidden(true);
}

void ComicVineDialog::showSearchVolume()
{
	status = AskingForInfo;
	content->setCurrentWidget(searchVolumeWidget);
	backButton->setHidden(true);
	nextButton->setHidden(true);
	searchButton->setVisible(true);
	closeButton->setVisible(true);
	toggleSkipButton();
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
	toggleSkipButton();
}

void ComicVineDialog::showSelectComic(const QString &json)
{
    status = SelectingComic;

	content->setCurrentWidget(selectComicWidget);
	selectComicWidget->load(json);

    backButton->setVisible(true);
    nextButton->setVisible(true);
    searchButton->setHidden(true);
    closeButton->setVisible(true);
	toggleSkipButton();
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
	toggleSkipButton();
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

void ComicVineDialog::getComicsInfo(QList<QPair<ComicDB, QString> > & matchingInfo, int count,const QString & publisher)
{
	QPair<ComicDB, QString> p;
	QList<ComicDB> comics;
	foreach (p, matchingInfo) {
		ComicVineClient * comicVineClient = new ComicVineClient;
		//connect(comicVineClient,SIGNAL(searchResult(QString)),this,SLOT(debugClientResults(QString)));
		//connect(comicVineClient,SIGNAL(timeOut()),this,SLOT(queryTimeOut()));
		//connect(comicVineClient,SIGNAL(finished()),comicVineClient,SLOT(deleteLater()));
		QByteArray result = comicVineClient->getComicDetail(p.second); //TODO check timeOut or Connection error

        comics.push_back(parseComicInfo(p.first,result,count,publisher)); //TODO check result error

        setLoadingMessage(tr("Retrieving tags for : %1").arg(p.first.getFileName()));
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

void ComicVineDialog::getComicInfo(const QString &comicId, int count, const QString &publisher)
{

    ComicVineClient * comicVineClient = new ComicVineClient;
    QByteArray result = comicVineClient->getComicDetail(comicId); //TODO check timeOut or Connection error

    ComicDB comic = parseComicInfo(comics[currentIndex],result,count,publisher); //TODO check result error

    setLoadingMessage(tr("Retrieving tags for : %1").arg(comics[currentIndex].getFileName()));

    QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
    db.open();
    db.transaction();

    DBHelper::update(&(comic.info),db);

    db.commit();
    db.close();
    QSqlDatabase::removeDatabase(databasePath);

    if(mode == SingleComic || currentIndex == (comics.count()-1))
    {
        close();
        emit accepted();
    } else
    {
	   goToNextComic();
    }
}

ComicDB ComicVineDialog::parseComicInfo(ComicDB & comic, const QString & json, int count, const QString & publisher)
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
            //QString count; //get from select volume


			QString volume = result.property("volume").property("name").toString();
			QString storyArc; //story_arc
			QString arcNumber; //??
            QString arcCount; //count_of_issue_appearances -> NO

			QString genere; //no

            QMap<QString,QString> authors = getAuthors(result.property("person_credits"));

            QString writer = QStringList(authors.values("writer")).join("\n");
            QString penciller = QStringList(authors.values("penciller")).join("\n");
            QString inker = QStringList(authors.values("inker")).join("\n");
            QString colorist = QStringList(authors.values("colorist")).join("\n");
            QString letterer = QStringList(authors.values("letterer")).join("\n");
            QString coverArtist = QStringList(authors.values("cover")).join("\n");

			QString date = result.property("cover_date").toString();

            //QString publisher; //get from select volume
			QString format; //no
			bool color; //no
			QString ageRating; //no

            QString synopsis = result.property("description").toString().remove(QRegExp("<[^>]*>")); //description
            QString characters = getCharacters(result.property("character_credits"));

			comic.info.setTitle(title);

			comic.info.setNumber(number.toInt());
            comic.info.setCount(count);

            comic.info.setWriter(writer);
            comic.info.setPenciller(penciller);
            comic.info.setInker(inker);
            comic.info.setColorist(colorist);
            comic.info.setLetterer(letterer);
            comic.info.setCoverArtist(coverArtist);

			QStringList tempList = date.split("-");
			std::reverse(tempList.begin(),tempList.end());
			comic.info.setDate(tempList.join("/"));
			comic.info.setVolume(volume);

            comic.info.setPublisher(publisher);

            comic.info.setSynopsis(synopsis);
            comic.info.setCharacters(characters);
		}
	}
    return comic;
}

QString ComicVineDialog::getCharacters(const QScriptValue &json_characters)
{
    QString characters;

    QScriptValueIterator it(json_characters);
    QScriptValue resultsValue;
    while (it.hasNext()) {
        it.next();
        if(it.flags() & QScriptValue::SkipInEnumeration)
            continue;
        resultsValue = it.value();

        characters += resultsValue.property("name").toString() + "\n";
    }

    return characters;
}

QMap<QString, QString> ComicVineDialog::getAuthors(const QScriptValue &json_authors)
{
    QMap<QString, QString> authors;

    QScriptValueIterator it(json_authors);
    QScriptValue resultsValue;
    while (it.hasNext()) {
        it.next();
        if(it.flags() & QScriptValue::SkipInEnumeration)
            continue;
        resultsValue = it.value();

        QString authorName = resultsValue.property("name").toString();

        QStringList roles = resultsValue.property("role").toString().split(",");
        foreach(QString role, roles)
        {
            if(role.trimmed() == "writer")
                authors.insertMulti("writer",authorName);
            else if(role.trimmed() == "inker")
                authors.insertMulti("inker",authorName);
            else if(role.trimmed() == "penciler" || role.trimmed() == "penciller")
                authors.insertMulti("penciller",authorName);
            else if(role.trimmed() == "colorist")
                authors.insertMulti("colorist",authorName);
            else if(role.trimmed() == "letterer")
                authors.insertMulti("letterer",authorName);
            else if(role.trimmed() == "cover")
                authors.insertMulti("cover",authorName);
        }
    }

	return authors;
}

void ComicVineDialog::toggleSkipButton()
{
	if (mode == SingleComicInList)
		skipButton->setVisible(true);
	else
		skipButton->setHidden(true);
}

void ComicVineDialog::goToNextComic()
{
	if(mode == SingleComic || currentIndex == (comics.count()-1))
	{
		close();
		emit accepted();
		return;
	}

	currentIndex++;

	showSearchSingleComic();

	ComicDB comic = comics[currentIndex];
	QString title = comic.getTitleOrFileName();
	titleHeader->setSubTitle(tr("comic %1 of %2 - %3").arg(currentIndex+1).arg(comics.length()).arg(title));
}

void ComicVineDialog::showLoading(const QString &message)
{
	content->setCurrentIndex(0);
    loadingMessage->setText(message);
	backButton->setHidden(true);
	skipButton->setHidden(true);
	nextButton->setHidden(true);
	searchButton->setHidden(true);
	closeButton->setVisible(true);
}

void ComicVineDialog::setLoadingMessage(const QString &message)
{
    loadingMessage->setText(message);
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
    showLoading(tr("Looking for volume..."));
	//TODO: check if volume info is empty.
	searchVolume(searchVolumeWidget->getVolumeInfo());
}

void ComicVineDialog::launchSearchComic()
{
    showLoading(tr("Looking for comic..."));

	QString volumeInfo = searchSingleComicWidget->getVolumeInfo();
	QString comicInfo = searchSingleComicWidget->getComicInfo();
	int comicNumber = searchSingleComicWidget->getComicNumber();

	if(comicInfo.isEmpty() && comicNumber == -1)
		searchVolume(volumeInfo);
}

