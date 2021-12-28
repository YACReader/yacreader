#include "comic_vine_dialog.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QtWidgets>
#include <QtConcurrent/QtConcurrentRun>
#include "data_base_management.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSqlDatabase>

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

    backButton->setShortcut(QKeySequence(Qt::Key_Backspace));

    closeButton->setDefault(false);
    closeButton->setAutoDefault(false);

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
    connect(closeButton, &QAbstractButton::clicked, this, &QWidget::close);
    connect(nextButton, &QAbstractButton::clicked, this, &ComicVineDialog::goNext);
    connect(backButton, &QAbstractButton::clicked, this, &ComicVineDialog::goBack);
    connect(searchButton, &QAbstractButton::clicked, this, &ComicVineDialog::search);
    connect(skipButton, &QAbstractButton::clicked, this, &ComicVineDialog::goToNextComic);

    connect(selectVolumeWidget, &ScraperSelector::loadPage, this, &ComicVineDialog::searchVolume);
    connect(selectComicWidget, &ScraperSelector::loadPage, this, &ComicVineDialog::getVolumeComicsInfo);
    connect(sortVolumeComicsWidget, &ScraperSelector::loadPage, this, &ComicVineDialog::getVolumeComicsInfo);

    connect(this, &QDialog::accepted, this, &QWidget::close, Qt::QueuedConnection);
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

        // ComicDB-ComicVineID
        QList<QPair<ComicDB, QString>> matchingInfo = sortVolumeComicsWidget->getMatchingInfo();
        int count = selectVolumeWidget->getSelectedVolumeNumIssues();
        QString publisher = selectVolumeWidget->getSelectedVolumePublisher();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QtConcurrent::run(&ComicVineDialog::getComicsInfo, this, matchingInfo, count, publisher);
#else
        QtConcurrent::run(this, &ComicVineDialog::getComicsInfo, matchingInfo, count, publisher);
#endif

    } else if (content->currentWidget() == selectComicWidget) {
        showLoading();
        QString comicId = selectComicWidget->getSelectedComicId();
        int count = selectVolumeWidget->getSelectedVolumeNumIssues();
        QString publisher = selectVolumeWidget->getSelectedVolumePublisher();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QtConcurrent::run(&ComicVineDialog::getComicInfo, this, comicId, count, publisher);
#else
        QtConcurrent::run(this, &ComicVineDialog::getComicInfo, comicId, count, publisher);
#endif
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
    QScreen *screen = window()->screen();
    if (screen == nullptr) {
        screen = QApplication::screens().constFirst();
    }

    int heightDesktopResolution = screen->geometry().height();
    int widthDesktopResolution = screen->geometry().width();

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
    // QMessageBox::information(0,"Result", QString("Number of results : %1").arg(p.getNumResults()));
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

    nextButton->setDefault(true);

    toggleSkipButton();
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

    searchButton->setDefault(true);

    toggleSkipButton();
}

void ComicVineDialog::showSearchVolume()
{
    status = AskingForInfo;
    content->setCurrentWidget(searchVolumeWidget);
    backButton->setHidden(true);
    nextButton->setHidden(true);
    searchButton->setVisible(true);
    closeButton->setVisible(true);

    searchButton->setDefault(true);

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

    nextButton->setDefault(true);

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

    nextButton->setDefault(true);

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

    nextButton->setDefault(true);

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

void ComicVineDialog::getComicsInfo(QList<QPair<ComicDB, QString>> matchingInfo, int count, const QString &publisher)
{
    QPair<ComicDB, QString> p;
    QList<ComicDB> comics;
    foreach (p, matchingInfo) {
        auto comicVineClient = new ComicVineClient;
        // connect(comicVineClient,SIGNAL(searchResult(QString)),this,SLOT(debugClientResults(QString)));
        // connect(comicVineClient,SIGNAL(timeOut()),this,SLOT(queryTimeOut()));
        // connect(comicVineClient,SIGNAL(finished()),comicVineClient,SLOT(deleteLater()));
        bool error;
        bool timeout;
        QByteArray result = comicVineClient->getComicDetail(p.second, error, timeout); // TODO check timeOut or Connection error
        if (error || timeout)
            continue; // TODO
        ComicDB comic = parseComicInfo(p.first, result, count, publisher); // TODO check result error
        comic.info.comicVineID = p.second;
        comics.push_back(comic);

        setLoadingMessage(tr("Retrieving tags for : %1").arg(p.first.getFileName()));
    }
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        db.open();
        db.transaction();
        foreach (ComicDB comic, comics) {
            DBHelper::update(&(comic.info), db);
        }
        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    emit accepted();
}

void ComicVineDialog::getComicInfo(const QString &comicId, int count, const QString &publisher)
{

    auto comicVineClient = new ComicVineClient;
    bool error;
    bool timeout;
    QByteArray result = comicVineClient->getComicDetail(comicId, error, timeout); // TODO check timeOut or Connection error
    if (error || timeout) {
        // TODO
        if (mode == SingleComic || currentIndex == (comics.count() - 1)) {
            emit accepted();
        } else {
            goToNextComic();
        }
    }

    ComicDB comic = parseComicInfo(comics[currentIndex], result, count, publisher); // TODO check result error
    comic.info.comicVineID = comicId;
    setLoadingMessage(tr("Retrieving tags for : %1").arg(comics[currentIndex].getFileName()));
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        db.open();
        db.transaction();

        DBHelper::update(&(comic.info), db);

        db.commit();
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (mode == SingleComic || currentIndex == (comics.count() - 1)) {
        emit accepted();
    } else {
        goToNextComic();
    }
}

ComicDB ComicVineDialog::parseComicInfo(ComicDB &comic, const QString &json, int count, const QString &publisher)
{
    QJsonParseError Err;

    QVariantMap sc = QJsonDocument::fromJson(json.toUtf8(), &Err).toVariant().toMap();
    if (Err.error != QJsonParseError::NoError) {
        qDebug("Error detected");
        return comic;
    }

    int numResults = sc.value("number_of_total_results").toInt(); // fix to weird behaviour using hasNext

    if (numResults > 0) {
        QVariantMap result = sc.value("results").toMap();
        comic.info.title = result.value("name");
        comic.info.number = result.value("issue_number");
        comic.info.volume = result.value("volume").toMap().value("name");

        if (result.contains("person_credits") && !result.value("person_credits").isNull()) {
            auto authors = getAuthors(result.value("person_credits"));

            QString writer = authors.values("writer").join("\n");
            QString penciller = authors.values("penciller").join("\n");
            QString inker = authors.values("inker").join("\n");
            QString colorist = authors.values("colorist").join("\n");
            QString letterer = authors.values("letterer").join("\n");
            QString coverArtist = authors.values("cover").join("\n");

            comic.info.writer = writer;
            comic.info.penciller = penciller;
            comic.info.inker = inker;
            comic.info.colorist = colorist;
            comic.info.letterer = letterer;
            comic.info.coverArtist = coverArtist;
        }

        if (result.contains("cover_date") && !result.value("cover_date").isNull()) {
            QString date = result.value("cover_date").toString();

            QStringList tempList = date.split("-");

            if (tempList.length() == 3) {
                std::reverse(tempList.begin(), tempList.end());
                comic.info.date = tempList.join("/");
            }
        }

        if (result.contains("description") && !result.value("description").isNull()) {
            comic.info.synopsis = result.value("description");
        }

        if (result.contains("character_credits") && !result.value("character_credits").isNull()) {
            comic.info.characters = getCharacters(result.value("character_credits"));
        }

        if (result.contains("story_arc_credits") && !result.value("story_arc_credits").isNull()) {
            QPair<QString, QString> storyArcIdAndName = getFirstStoryArcIdAndName(result.value("story_arc_credits"));
            QString storyArcId = storyArcIdAndName.first;
            QString storyArcName = storyArcIdAndName.second;
            if (!storyArcId.isNull()) {

                QString comicId = result.value("id").toString();

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

    return comic;
}

QString ComicVineDialog::getCharacters(const QVariant &json_characters)
{
    QStringList characters;

    QListIterator<QVariant> it(json_characters.toList());
    QVariantMap resultsValue;
    while (it.hasNext()) {
        resultsValue = it.next().toMap();

        characters << resultsValue.value("name").toString();
    }

    return (characters.isEmpty()) ? "" : (characters.join("\n") + "\n");
}

QMultiMap<QString, QString> ComicVineDialog::getAuthors(const QVariant &json_authors)
{
    QMultiMap<QString, QString> authors;

    QListIterator<QVariant> it(json_authors.toList());
    QVariantMap resultsValue;
    while (it.hasNext()) {
        resultsValue = it.next().toMap();

        QString authorName = resultsValue.value("name").toString();

        QStringList roles = resultsValue.value("role").toString().split(",");
        foreach (QString role, roles) {
            if (role.trimmed() == "writer")
                authors.insert("writer", authorName);
            else if (role.trimmed() == "inker")
                authors.insert("inker", authorName);
            else if (role.trimmed() == "penciler" || role.trimmed() == "penciller")
                authors.insert("penciller", authorName);
            else if (role.trimmed() == "colorist")
                authors.insert("colorist", authorName);
            else if (role.trimmed() == "letterer")
                authors.insert("letterer", authorName);
            else if (role.trimmed() == "cover")
                authors.insert("cover", authorName);
        }
    }

    return authors;
}

QPair<QString, QString> ComicVineDialog::getFirstStoryArcIdAndName(const QVariant &json_story_arcs)
{
    QString story_arc_id = QString();
    QString story_arc_name = QString();

    QListIterator<QVariant> it(json_story_arcs.toList());
    QVariantMap resultsValue;
    while (it.hasNext()) {
        resultsValue = it.next().toMap();
        story_arc_id = resultsValue.value("id").toString();
        story_arc_name = resultsValue.value("name").toString();
        break;
    }
    return qMakePair(story_arc_id, story_arc_name);
}

QPair<QString, QString> ComicVineDialog::getArcNumberAndArcCount(const QString &storyArcId, const QString &comicId)
{
    auto comicVineClient = new ComicVineClient;
    bool error;
    bool timeout;
    QByteArray result = comicVineClient->getStoryArcDetail(storyArcId, error, timeout);
    if (error || timeout)
        return qMakePair(QString(), QString());
    QString json = result;

    QJsonParseError Err;
    QVariantMap sc = QJsonDocument::fromJson(json.toUtf8(), &Err).toVariant().toMap();

    if (Err.error != QJsonParseError::NoError) {
        qDebug("Error detected");
        return qMakePair(QString(), QString());
    }

    int numResults = sc.value("number_of_total_results").toInt(); // fix to weird behaviour using hasNext

    if (numResults > 0) {
        QVariantMap result = sc.value("results").toMap();

        if (result.contains("issues")) {
            QListIterator<QVariant> it(result.value("issues").toList());
            int arcNumber = 0;
            int arcCount = 0;

            QVariantMap resultsValue;
            while (it.hasNext()) {
                resultsValue = it.next().toMap();
                if (comicId == resultsValue.value("id").toString()) {
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
    connect(comicVineClient, &ComicVineClient::searchResult, this, &ComicVineDialog::debugClientResults);
    connect(comicVineClient, &ComicVineClient::timeOut, this, &ComicVineDialog::queryTimeOut);
    connect(comicVineClient, &ComicVineClient::finished, comicVineClient, &QObject::deleteLater);
    comicVineClient->search(v, page);

    status = SearchingVolume;
}

void ComicVineDialog::getVolumeComicsInfo(const QString &vID, int /* page */)
{
    showLoading(tr("Retrieving volume info..."));

    status = GettingVolumeComics;

    auto comicVineClient = new ComicVineClient;
    if (mode == Volume)
        connect(comicVineClient, &ComicVineClient::volumeComicsInfo, this, &ComicVineDialog::showSortVolumeComics);
    else
        connect(comicVineClient, &ComicVineClient::volumeComicsInfo, this, &ComicVineDialog::showSelectComic);
    connect(comicVineClient, &ComicVineClient::timeOut, this, &ComicVineDialog::queryTimeOut);
    connect(comicVineClient, &ComicVineClient::finished, comicVineClient, &QObject::deleteLater);

    QLOG_TRACE() << vID;

    comicVineClient->getAllVolumeComicsInfo(vID);
}

void ComicVineDialog::launchSearchVolume()
{
    showLoading(tr("Looking for volume..."));
    // TODO: check if volume info is empty.
    searchVolume(searchVolumeWidget->getVolumeInfo());
}

void ComicVineDialog::launchSearchComic()
{
    showLoading(tr("Looking for comic..."));

    QString volumeInfo = searchSingleComicWidget->getVolumeInfo();
    // QString comicInfo = searchSingleComicWidget->getComicInfo();
    // int comicNumber = searchSingleComicWidget->getComicNumber();

    // if(comicInfo.isEmpty() && comicNumber == -1)
    searchVolume(volumeInfo);
}
