#include "comic_vine_dialog.h"
#include <QtWidgets>
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

#include "QsLog.h"

ComicVineDialog::ComicVineDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Window);

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

    auto mainLayout = new QVBoxLayout;

    auto buttonLayout = new QHBoxLayout;

    buttonLayout->addStretch();
    buttonLayout->addWidget(skipButton);
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(nextButton);
    buttonLayout->addWidget(searchButton);
    buttonLayout->addWidget(closeButton);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(titleHeader = new TitleHeader, 0);
    mainLayout->addWidget(content, 1);
    mainLayout->addLayout(buttonLayout, 0);

    mainLayout->setContentsMargins(26, 16, 26, 11);

    setLayout(mainLayout);

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
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(goNext()));
    connect(backButton, SIGNAL(clicked()), this, SLOT(goBack()));
    connect(searchButton, SIGNAL(clicked()), this, SLOT(search()));
    connect(skipButton, SIGNAL(clicked()), this, SLOT(goToNextComic()));

    connect(selectVolumeWidget, SIGNAL(loadPage(QString, int)), this, SLOT(searchVolume(QString, int)));
    connect(selectComicWidget, SIGNAL(loadPage(QString, int)), this, SLOT(getVolumeComicsInfo(QString, int)));
    connect(sortVolumeComicsWidget, SIGNAL(loadPage(QString, int)), this, SLOT(getVolumeComicsInfo(QString, int)));

    connect(this, SIGNAL(accepted()), this, SLOT(close()), Qt::QueuedConnection);
}

void ComicVineDialog::goNext()
{
    //
    if (content->currentWidget() == seriesQuestionWidget) {
        if (seriesQuestionWidget->getYes()) {
            QString volumeSearchString = comics[0].getParentFolderName();
            mode = Volume;

            if (volumeSearchString.isEmpty())
                showSearchVolume();
            else {
                status = AutoSearching;
                showLoading(tr("Looking for volume..."));
                searchVolume(volumeSearchString);
            }
        } else {
            status = AutoSearching;
            mode = SingleComicInList;
            ComicDB comic = comics[currentIndex];
            QString title = comic.getTitleOrFileName();
            titleHeader->setSubTitle(tr("comic %1 of %2 - %3").arg(currentIndex + 1).arg(comics.length()).arg(title));

            showLoading(tr("Looking for volume..."));
            searchVolume(title);
        }
    } else if (content->currentWidget() == selectVolumeWidget) {
        currentVolumeId = selectVolumeWidget->getSelectedVolumeId();
        getVolumeComicsInfo(currentVolumeId);

    } else if (content->currentWidget() == sortVolumeComicsWidget) {
        showLoading();

        //ComicDB-ComicVineID
        QList<QPair<ComicDB, QString>> matchingInfo = sortVolumeComicsWidget->getMatchingInfo();
        int count = selectVolumeWidget->getSelectedVolumeNumIssues();
        QString publisher = selectVolumeWidget->getSelectedVolumePublisher();
        QtConcurrent::run(this, &ComicVineDialog::getComicsInfo, matchingInfo, count, publisher);
    } else if (content->currentWidget() == selectComicWidget) {
        showLoading();
        QString comicId = selectComicWidget->getSelectedComicId();
        int count = selectVolumeWidget->getSelectedVolumeNumIssues();
        QString publisher = selectVolumeWidget->getSelectedVolumePublisher();
        QtConcurrent::run(this, &ComicVineDialog::getComicInfo, comicId, count, publisher);
    }
}

void ComicVineDialog::goBack()
{
    switch (status) {
    case SelectingSeries:
        if (mode == Volume)
            showSearchVolume();
        else
            showSearchSingleComic();
        break;
    case SortingComics:
        showSelectVolume();
        break;
    case SelectingComic:
        if (mode == SingleComic)
            showSelectVolume();
        break;
    case AutoSearching:
        if (mode == Volume)
            showSearchVolume();
        else
            showSearchSingleComic();
        break;
    default:
        if (mode == Volume)
            showSearchVolume();
        else
            showSearchSingleComic();
        break;
    }
}

void ComicVineDialog::setComics(const QList<ComicDB> &comics)
{
    this->comics = comics;
}

QSize ComicVineDialog::sizeHint() const
{
    int heightDesktopResolution = QApplication::desktop()->screenGeometry().height();
    int widthDesktopResolution = QApplication::desktop()->screenGeometry().width();
    int height, width;
    height = qMax(529, static_cast<int>(heightDesktopResolution * 0.5));
    width = height * 1.65;

    if (width > widthDesktopResolution)
        return minimumSizeHint();

    return QSize(width, height);
}

QSize ComicVineDialog::minimumSizeHint() const
{
    return QSize(872, 529);
}

void ComicVineDialog::show()
{
    QDialog::show();

    currentIndex = 0;

    seriesQuestionWidget->setYes(true);
    searchSingleComicWidget->clean();
    searchVolumeWidget->clean();

    if (comics.length() == 1) {
        status = AutoSearching;
        mode = SingleComic;

        ComicDB singleComic = comics[0];
        QString title = singleComic.getTitleOrFileName();
        titleHeader->setSubTitle(title);
        showLoading(tr("Looking for volume..."));

        searchVolume(singleComic.getParentFolderName());
        QLOG_TRACE() << singleComic.getParentFolderName();
    } else if (comics.length() > 1) {
        titleHeader->setSubTitle(tr("%1 comics selected").arg(comics.length()));
        showSeriesQuestion();
    }
}

void ComicVineDialog::doLoading()
{
    QWidget *w = new QWidget;
    auto l = new QVBoxLayout;

    auto bw = new YACReaderBusyWidget;
    loadingMessage = new QLabel;

    loadingMessage->setStyleSheet("QLabel {color:white; font-size:12px;font-family:Arial;}");

    l->addStretch();
    l->addWidget(bw, 0, Qt::AlignHCenter);
    l->addStretch();
    l->addWidget(loadingMessage);

    l->setContentsMargins(0, 0, 0, 0);
    w->setLayout(l);
    w->setContentsMargins(0, 0, 0, 0);

    content->addWidget(w);
}

void ComicVineDialog::debugClientResults(const QString &string)
{
    ResponseParser p;
    p.loadJSONResponse(string);
    //QMessageBox::information(0,"Result", QString("Number of results : %1").arg(p.getNumResults()));
    if (p.responseError()) {
        QMessageBox::critical(0, tr("Error connecting to ComicVine"), p.errorDescription());
        goBack();
    } else {
        switch (mode) {
        case SingleComic:
        case SingleComicInList:
            if (p.getNumResults() == 0)
                showSearchSingleComic();
            else if (status == SearchingVolume)
                showSelectVolume(string);
            else
                showSelectComic(string);
            break;
        case Volume:
            if (p.getNumResults() == 0)
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

    if (mode == SingleComicInList)
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

    if (mode == SingleComicInList)
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

void ComicVineDialog::showSelectVolume(const QString &json)
{
    showSelectVolume();
    selectVolumeWidget->load(json, currentVolumeSearchString);
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
    selectComicWidget->load(json, currentVolumeId);

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

    sortVolumeComicsWidget->setData(comics, json, currentVolumeId);

    backButton->setVisible(true);
    nextButton->setVisible(true);
    searchButton->setHidden(true);
    closeButton->setVisible(true);
    toggleSkipButton();
}

void ComicVineDialog::queryTimeOut()
{
    QMessageBox::warning(this, "Comic Vine error", "Time out connecting to Comic Vine");

    switch (status) {
    case AutoSearching:
        if (mode == Volume)
            showSearchVolume();
        else
            showSearchSingleComic();
        break;
    case SearchingVolume:
        if (mode == Volume)
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

void ComicVineDialog::getComicsInfo(QList<QPair<ComicDB, QString>> &matchingInfo, int count, const QString &publisher)
{
    QPair<ComicDB, QString> p;
    QList<ComicDB> comics;
    foreach (p, matchingInfo) {
        auto comicVineClient = new ComicVineClient;
        //connect(comicVineClient,SIGNAL(searchResult(QString)),this,SLOT(debugClientResults(QString)));
        //connect(comicVineClient,SIGNAL(timeOut()),this,SLOT(queryTimeOut()));
        //connect(comicVineClient,SIGNAL(finished()),comicVineClient,SLOT(deleteLater()));
        bool error;
        bool timeout;
        QByteArray result = comicVineClient->getComicDetail(p.second, error, timeout); //TODO check timeOut or Connection error
        if (error || timeout)
            continue; //TODO
        ComicDB comic = parseComicInfo(p.first, result, count, publisher); //TODO check result error
        comic.info.comicVineID = p.second;
        comics.push_back(comic);

        setLoadingMessage(tr("Retrieving tags for : %1").arg(p.first.getFileName()));
    }

    QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
    db.open();
    db.transaction();
    foreach (ComicDB comic, comics) {
        DBHelper::update(&(comic.info), db);
    }
    db.commit();
    db.close();
    QSqlDatabase::removeDatabase(db.connectionName());

    emit accepted();
}

void ComicVineDialog::getComicInfo(const QString &comicId, int count, const QString &publisher)
{

    auto comicVineClient = new ComicVineClient;
    bool error;
    bool timeout;
    QByteArray result = comicVineClient->getComicDetail(comicId, error, timeout); //TODO check timeOut or Connection error
    if (error || timeout) {
        //TODO
        if (mode == SingleComic || currentIndex == (comics.count() - 1)) {
            emit accepted();
        } else {
            goToNextComic();
        }
    }

    ComicDB comic = parseComicInfo(comics[currentIndex], result, count, publisher); //TODO check result error
    comic.info.comicVineID = comicId;
    setLoadingMessage(tr("Retrieving tags for : %1").arg(comics[currentIndex].getFileName()));

    QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
    db.open();
    db.transaction();

    DBHelper::update(&(comic.info), db);

    db.commit();
    db.close();
    QSqlDatabase::removeDatabase(db.connectionName());

    if (mode == SingleComic || currentIndex == (comics.count() - 1)) {
        emit accepted();
    } else {
        goToNextComic();
    }
}

ComicDB ComicVineDialog::parseComicInfo(ComicDB &comic, const QString &json, int count, const QString &publisher)
{
    QScriptEngine engine;
    QScriptValue sc;
    sc = engine.evaluate("(" + json + ")");

    if (!sc.property("error").isValid() && sc.property("error").toString() != "OK") {
        qDebug("Error detected");
    } else {
        int numResults = sc.property("number_of_total_results").toString().toInt(); //fix to weird behaviour using hasNext

        if (numResults > 0) {
            QScriptValue result = sc.property("results");

            if (!result.property("name").isNull()) {
                QString title = result.property("name").toString();

                comic.info.title = title;
            }

            if (!result.property("issue_number").isNull()) {
                QString number = result.property("issue_number").toString();

                comic.info.number = number;
            }

            if (!result.property("volume").property("name").isNull()) {
                QString volume = result.property("volume").property("name").toString();

                comic.info.volume = volume;
            }

            if (!result.property("person_credits").isNull()) {
                QMap<QString, QString> authors = getAuthors(result.property("person_credits"));

                QString writer = QStringList(authors.values("writer")).join("\n");
                QString penciller = QStringList(authors.values("penciller")).join("\n");
                QString inker = QStringList(authors.values("inker")).join("\n");
                QString colorist = QStringList(authors.values("colorist")).join("\n");
                QString letterer = QStringList(authors.values("letterer")).join("\n");
                QString coverArtist = QStringList(authors.values("cover")).join("\n");

                comic.info.writer = writer;
                comic.info.penciller = penciller;
                comic.info.inker = inker;
                comic.info.colorist = colorist;
                comic.info.letterer = letterer;
                comic.info.coverArtist = coverArtist;
            }

            if (!result.property("cover_date").isNull()) {
                QString date = result.property("cover_date").toString();

                QStringList tempList = date.split("-");

                if (tempList.length() == 3) {
                    std::reverse(tempList.begin(), tempList.end());
                    comic.info.date = tempList.join("/");
                }
            }

            if (!result.property("description").isNull()) {
                QString synopsis = result.property("description").toString().remove(QRegExp("<[^>]*>")); //description
                comic.info.synopsis = synopsis;
            }

            if (!result.property("character_credits").isNull()) {
                QString characters = getCharacters(result.property("character_credits"));

                comic.info.characters = characters;
            }
            
            if (!result.property("story_arc_credits").isNull()) {
                QPair<QString, QString> storyArcIdAndName = getFirstStoryArcIdAndName(result.property("story_arc_credits"));
                QString storyArcId = storyArcIdAndName.first;
                QString storyArcName = storyArcIdAndName.second;
                if (!storyArcId.isNull()) {
                    
                    QString comicId = result.property("id").toString();

                    QPair<QString, QString> arcNumberAndArcCount = getArcNumberAndArcCount(storyArcId, comicId);
                    if (!arcNumberAndArcCount.first.isNull()) {
                        QString arcNumber = arcNumberAndArcCount.first;
                        QString arcCount = arcNumberAndArcCount.second;
                        
                        comic.info.storyArc = storyArcName;
                        comic.info.arcNumber = arcNumber;
                        comic.info.arcCount = arcCount;
                    }
                }
            }

            comic.info.count = count;

            comic.info.publisher = publisher;
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
        if (it.flags() & QScriptValue::SkipInEnumeration)
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
        if (it.flags() & QScriptValue::SkipInEnumeration)
            continue;
        resultsValue = it.value();

        QString authorName = resultsValue.property("name").toString();

        QStringList roles = resultsValue.property("role").toString().split(",");
        foreach (QString role, roles) {
            if (role.trimmed() == "writer")
                authors.insertMulti("writer", authorName);
            else if (role.trimmed() == "inker")
                authors.insertMulti("inker", authorName);
            else if (role.trimmed() == "penciler" || role.trimmed() == "penciller")
                authors.insertMulti("penciller", authorName);
            else if (role.trimmed() == "colorist")
                authors.insertMulti("colorist", authorName);
            else if (role.trimmed() == "letterer")
                authors.insertMulti("letterer", authorName);
            else if (role.trimmed() == "cover")
                authors.insertMulti("cover", authorName);
        }
    }

    return authors;
}

QPair<QString, QString> ComicVineDialog::getFirstStoryArcIdAndName(const QScriptValue &json_story_arcs)
{
    QString story_arc_id = QString();
    QString story_arc_name = QString();
    
    QScriptValueIterator it(json_story_arcs);
    QScriptValue resultsValue;
    while (it.hasNext()) {
        it.next();
        if (it.flags() & QScriptValue::SkipInEnumeration)
            continue;
        resultsValue = it.value();
        story_arc_id = resultsValue.property("id").toString();
        story_arc_name = resultsValue.property("name").toString();
        break;
    }
    return qMakePair(story_arc_id, story_arc_name);
}

QPair<QString, QString> ComicVineDialog::getArcNumberAndArcCount(const QString &storyArcId, const QString &comicId) {
    auto comicVineClient = new ComicVineClient;
    bool error;
    bool timeout;
    QByteArray result = comicVineClient->getStoryArcDetail(storyArcId, error, timeout);
    if (error || timeout)
        return qMakePair(QString(), QString());
    QString json = result;

    QScriptEngine engine;
    QScriptValue sc;
    sc = engine.evaluate("(" + json + ")");

    if (!sc.property("error").isValid() && sc.property("error").toString() != "OK") {
        qDebug("Error detected");
        return qMakePair(QString(), QString());
    } else {
        int numResults = sc.property("number_of_total_results").toString().toInt(); //fix to weird behaviour using hasNext

        if (numResults > 0) {
            QScriptValue result = sc.property("results");

            if (!result.property("issues").isNull()) {
                QScriptValue issues = result.property("issues");
                
                int arcNumber = 0;
                int arcCount = 0;
                
                QScriptValueIterator it(issues);
                QScriptValue resultsValue;
                while (it.hasNext()) {
                    it.next();
                    if (it.flags() & QScriptValue::SkipInEnumeration)
                        continue;
                    resultsValue = it.value();
                    if (comicId == resultsValue.property("id").toString()) {
                        arcNumber = arcCount + 1;
                    }
                    arcCount++;
                }
                return qMakePair(QString::number(arcNumber), QString::number(arcCount));
            }
            return qMakePair(QString(), QString());
        }
        return qMakePair(QString(), QString());
    }
    return qMakePair(QString(), QString());
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
    if (mode == SingleComic || currentIndex == (comics.count() - 1)) {
        emit accepted();
        return;
    }

    currentIndex++;

    showSearchSingleComic();

    ComicDB comic = comics[currentIndex];
    QString title = comic.getTitleOrFileName();
    titleHeader->setSubTitle(tr("comic %1 of %2 - %3").arg(currentIndex + 1).arg(comics.length()).arg(title));
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

void ComicVineDialog::searchVolume(const QString &v, int page)
{
    showLoading(tr("Looking for volume..."));

    currentVolumeSearchString = v;

    auto comicVineClient = new ComicVineClient;
    connect(comicVineClient, SIGNAL(searchResult(QString)), this, SLOT(debugClientResults(QString)));
    connect(comicVineClient, SIGNAL(timeOut()), this, SLOT(queryTimeOut()));
    connect(comicVineClient, SIGNAL(finished()), comicVineClient, SLOT(deleteLater()));
    comicVineClient->search(v, page);

    status = SearchingVolume;
}

void ComicVineDialog::getVolumeComicsInfo(const QString &vID, int /* page */)
{
    showLoading(tr("Retrieving volume info..."));

    status = GettingVolumeComics;

    auto comicVineClient = new ComicVineClient;
    if (mode == Volume)
        connect(comicVineClient, SIGNAL(volumeComicsInfo(QString)), this, SLOT(showSortVolumeComics(QString)));
    else
        connect(comicVineClient, SIGNAL(volumeComicsInfo(QString)), this, SLOT(showSelectComic(QString)));
    connect(comicVineClient, SIGNAL(timeOut()), this, SLOT(queryTimeOut()));
    connect(comicVineClient, SIGNAL(finished()), comicVineClient, SLOT(deleteLater()));

    QLOG_TRACE() << vID;

    comicVineClient->getAllVolumeComicsInfo(vID);
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
    //QString comicInfo = searchSingleComicWidget->getComicInfo();
    //int comicNumber = searchSingleComicWidget->getComicNumber();

    //if(comicInfo.isEmpty() && comicNumber == -1)
    searchVolume(volumeInfo);
}
