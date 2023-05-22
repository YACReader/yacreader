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
#include <QSqlDatabase>

#include "yacreader_busy_widget.h"
#include "comic_vine_client.h"
#include "comic_vine_json_parser.h"
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
    setModal(true);

    doLayout();
    doStackedWidgets();
    doConnections();
}

void ComicVineDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);

    clearState();
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
    clearState();

    //
    if (content->currentWidget() == seriesQuestionWidget) {
        if (seriesQuestionWidget->getYes()) {
            QString volumeSearchString = comics[0].getParentFolderName();
            mode = Volume;

            showSearchVolume(volumeSearchString);
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
    clearState();

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

void ComicVineDialog::showSearchVolume(const QString &volume)
{
    searchVolumeWidget->setVolumeInfo(volume);

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
        ComicDB comic = YACReader::parseCVJSONComicInfo(p.first, result, count, publisher); // TODO check result error
        comic.info.comicVineID = p.second;
        comics.push_back(comic);

        setLoadingMessage(tr("Retrieving tags for : %1").arg(p.first.getFileName()));
    }

    DBHelper::updateComicsInfo(comics, databasePath);

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

    ComicDB comic = YACReader::parseCVJSONComicInfo(comics[currentIndex], result, count, publisher); // TODO check result error
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

void ComicVineDialog::clearState()
{
    selectVolumeWidget->clearFilter();
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
