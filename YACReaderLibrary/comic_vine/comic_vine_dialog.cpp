#include "comic_vine_dialog.h"

#include "QsLog.h"
#include "comic_vine_client.h"
#include "comic_vine_json_parser.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "placeholder_cover.h"
#include "response_parser.h"
#include "scraper_lineedit.h"
#include "search_single_comic.h"
#include "search_volume.h"
#include "select_comic.h"
#include "select_volume.h"
#include "selected_volume_info.h"
#include "series_question.h"
#include "sort_volume_comics.h"
#include "title_header.h"
#include "yacreader_busy_widget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSqlDatabase>
#include <QStackedWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>

ComicVineDialog::ComicVineDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Window);
    setModal(true);

    doLayout();
    doStackedWidgets();
    doConnections();

    initTheme(this);
}

void ComicVineDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);

    clearState();
}

void ComicVineDialog::done(int result)
{
    if (result == QDialog::Rejected)
        run++;
    QDialog::done(result);
}

void ComicVineDialog::doLayout()
{
    skipButton = new QPushButton(tr("skip"));
    backButton = new QPushButton(tr("back"));
    nextButton = new QPushButton(tr("next"));
    searchButton = new QPushButton(tr("search"));
    closeButton = new QPushButton(tr("close"));

    backButton->setShortcut(QKeySequence(Qt::Key_Backspace));

    closeButton->setDefault(false);
    closeButton->setAutoDefault(false);

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

    setWindowTitle("Comic Vine Scraper");
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

    connect(selectVolumeWidget, &SelectVolume::loadPage, this, &ComicVineDialog::searchVolume);
    connect(selectComicWidget, &SelectComic::loadPage, this, &ComicVineDialog::getVolumeComicsInfo);
    connect(sortVolumeComicsWidget, &SortVolumeComics::loadPage, this, &ComicVineDialog::getVolumeComicsInfo);
}

void ComicVineDialog::goNext()
{
    clearState();

    //
    if (content->currentWidget() == seriesQuestionWidget) {
        if (seriesQuestionWidget->getYes()) {
            QString volumeSearchString = parentFolderName(recordPath(comics[0]));
            mode = ScraperMode::Volume;

            showSearchVolume(volumeSearchString);
        } else {
            status = ScraperStatus::AutoSearching;
            mode = ScraperMode::SingleComicInList;
            const auto &comic = comics[currentIndex];
            QString title = titleOrFileName(recordMetadata(comic), recordFileName(comic));
            titleHeader->setSubTitle(tr("comic %1 of %2 - %3").arg(currentIndex + 1).arg(comics.length()).arg(title));

            showLoading(tr("Looking for volume..."));

            searchVolume({ volumeSearchStringFromComic(comic), 1, true });
        }
    } else if (content->currentWidget() == selectVolumeWidget) {
        currentVolumeId = selectVolumeWidget->getSelectedVolumeInfo().id;
        getVolumeComicsInfo(currentVolumeId);

    } else if (content->currentWidget() == sortVolumeComicsWidget) {
        showLoading();

        // comic - Comic Vine id
        QList<QPair<ComicRecord, QString>> matchingInfo = sortVolumeComicsWidget->getMatchingInfo();
        const auto missingIssues = folderId && sortVolumeComicsWidget->addsMissingIssues() ? sortVolumeComicsWidget->getMissingIssues() : QStringList();
        auto volumeInfo = selectVolumeWidget->getSelectedVolumeInfo();

        QtConcurrent::run(&ComicVineDialog::getComicsInfo, this, matchingInfo, missingIssues, volumeInfo, run.load());

    } else if (content->currentWidget() == selectComicWidget) {
        showLoading();
        QString comicId = selectComicWidget->getSelectedComicId();
        auto volumeInfo = selectVolumeWidget->getSelectedVolumeInfo();

        QtConcurrent::run(&ComicVineDialog::getComicInfo, this, comicId, volumeInfo);
    }
}

void ComicVineDialog::goBack()
{
    clearState();

    switch (status) {
    case ScraperStatus::SelectingSeries:
        if (mode == ScraperMode::Volume)
            showSearchVolume(currentVolumeSearchQuery.volume);
        else
            showSearchSingleComic(currentVolumeSearchQuery.volume);
        break;
    case ScraperStatus::SortingComics:
        showSelectVolume();
        break;
    case ScraperStatus::SelectingComic:
        if (mode == ScraperMode::SingleComic)
            showSelectVolume();
        break;
    case ScraperStatus::AutoSearching:
        if (mode == ScraperMode::Volume)
            showSearchVolume(currentVolumeSearchQuery.volume);
        else
            showSearchSingleComic(currentVolumeSearchQuery.volume);
        break;

    case ScraperStatus::AskingForInfo:
    case ScraperStatus::SearchingSingleComic:
    case ScraperStatus::SearchingVolume:
    case ScraperStatus::SearchingExactVolume:
    case ScraperStatus::GettingVolumeComics:
        if (mode == ScraperMode::Volume)
            showSearchVolume(currentVolumeSearchQuery.volume);
        else
            showSearchSingleComic(currentVolumeSearchQuery.volume);
        break;
    }
}

void ComicVineDialog::setComics(const QList<ComicRecord> &comics)
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
    run++;
    failedComics = 0;
    basicMissingIssues = 0;
    missingIssuesWithoutCover = 0;
    limitReached = false;
    QDialog::show();

    currentIndex = 0;

    seriesQuestionWidget->setYes(true);
    searchSingleComicWidget->clean();
    searchVolumeWidget->clean();

    if (comics.length() == 1) {
        status = ScraperStatus::AskingForInfo;
        mode = ScraperMode::SingleComic;

        const auto &singleComic = comics[0];

        QString title = titleOrFileName(recordMetadata(singleComic), recordFileName(singleComic));
        titleHeader->setSubTitle(title);

        showSearchSingleComic(volumeSearchStringFromComic(singleComic));
    } else if (comics.length() > 1) {
        titleHeader->setSubTitle(tr("%1 comics selected").arg(comics.length()));
        showSeriesQuestion();
    }
}

void ComicVineDialog::doLoading()
{
    QWidget *w = new QWidget;
    auto l = new QVBoxLayout;

    busyWidget = new YACReaderBusyWidget;
    loadingMessage = new QLabel;

    l->addStretch();
    l->addWidget(busyWidget, 0, Qt::AlignHCenter);
    l->addStretch();
    l->addWidget(loadingMessage);

    l->setContentsMargins(0, 0, 0, 0);
    w->setLayout(l);
    w->setContentsMargins(0, 0, 0, 0);

    content->addWidget(w);
}

void ComicVineDialog::processClientResults(const QString &string)
{
    ResponseParser p;
    p.loadJSONResponse(string);
    // QMessageBox::information(0,"Result", QString("Number of results : %1").arg(p.getNumResults()));
    if (p.responseError()) {
        QMessageBox::critical(0, tr("Error connecting to Comic Vine"), p.errorDescription());
        goBack();
    } else {

        switch (mode) {
        case ScraperMode::SingleComic:
        case ScraperMode::SingleComicInList:
            if (p.getNumResults() == 0)
                showSearchSingleComic(currentVolumeSearchQuery.volume);
            else if (status == ScraperStatus::SearchingVolume || status == ScraperStatus::SearchingExactVolume)
                showSelectVolume(string);
            else
                showSelectComic(string);
            break;
        case ScraperMode::Volume:
            if (p.getNumResults() == 0)
                showSearchVolume(currentVolumeSearchQuery.volume);
            else
                showSelectVolume(string);
            break;
        }
    }
}

void ComicVineDialog::showSeriesQuestion()
{
    status = ScraperStatus::AskingForInfo;
    content->setCurrentWidget(seriesQuestionWidget);
    backButton->setHidden(true);
    skipButton->setHidden(true);
    nextButton->setVisible(true);
    searchButton->setHidden(true);
    closeButton->setVisible(true);

    nextButton->setDefault(true);

    toggleSkipButton();
}

void ComicVineDialog::showSearchSingleComic(const QString &volume)
{
    searchSingleComicWidget->setVolumeInfo(volume);

    status = ScraperStatus::AskingForInfo;
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

    status = ScraperStatus::AskingForInfo;
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
    selectVolumeWidget->load(json, currentVolumeSearchQuery);
}

void ComicVineDialog::showSelectVolume()
{
    status = ScraperStatus::SelectingSeries;

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
    status = ScraperStatus::SelectingComic;

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
    status = ScraperStatus::SortingComics;

    content->setCurrentWidget(sortVolumeComicsWidget);

    sortVolumeComicsWidget->setData(comics, json, currentVolumeId);
    sortVolumeComicsWidget->setMissingIssuesOption(folderId.has_value());

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
    case ScraperStatus::AutoSearching:
        if (mode == ScraperMode::Volume)
            showSearchVolume(currentVolumeSearchQuery.volume);
        else
            showSearchSingleComic(currentVolumeSearchQuery.volume);
        break;
    case ScraperStatus::SearchingVolume:
    case ScraperStatus::SearchingExactVolume:
        if (mode == ScraperMode::Volume)
            showSearchVolume(currentVolumeSearchQuery.volume);
        else
            showSearchSingleComic(currentVolumeSearchQuery.volume);
        break;
    case ScraperStatus::SearchingSingleComic:
        showSearchSingleComic(currentVolumeSearchQuery.volume);
        break;
    case ScraperStatus::GettingVolumeComics:
        showSelectVolume();
        break;

    case ScraperStatus::AskingForInfo:
    case ScraperStatus::SelectingComic:
    case ScraperStatus::SelectingSeries:
    case ScraperStatus::SortingComics:
        break;
    }
}

void ComicVineDialog::getComicsInfo(QList<QPair<ComicRecord, QString>> matchingInfo, const QStringList &missingIssues, const SelectedVolumeInfo &volumeInfo, int runId)
{
    QList<QPair<ComicRecord, DescriptiveMetadataPatch>> patches;
    const auto &matches = matchingInfo;
    for (auto match : matches) {
        if (run != runId)
            return;
        auto comicVineClient = new ComicVineClient;
        // connect(comicVineClient,SIGNAL(searchResult(QString)),this,SLOT(debugClientResults(QString)));
        // connect(comicVineClient,SIGNAL(timeOut()),this,SLOT(queryTimeOut()));
        // connect(comicVineClient,SIGNAL(finished()),comicVineClient,SLOT(deleteLater()));
        if (limitReached) {
            failedComics++;
            continue;
        }
        bool error;
        bool timeout;
        QByteArray result = comicVineClient->getComicDetail(match.second, error, timeout);
        if (!detailAnswered(error, timeout, result)) {
            failedComics++;
            continue;
        }
        auto patch = YACReader::parseCVJSONComicInfo(result, volumeInfo);
        patch.set(&DescriptiveComicMetadata::comicVineID, match.second);
        patches.append({ match.first, patch });

        setLoadingMessage(tr("Retrieving tags for: %1").arg(recordFileName(match.first)));
    }

    QList<MissingIssue> missing;
    for (int i = 0; i < missingIssues.size(); i++) {
        if (run != runId) {
            removeCovers(missing);
            return;
        }
        setLoadingMessage(tr("Creating missing comic %1 of %2").arg(i + 1).arg(missingIssues.size()));
        missing.append(missingIssue(missingIssues.at(i), volumeInfo));
    }
    if (run != runId) {
        removeCovers(missing);
        return;
    }

    saveMetadata(patches, missing);

    finishSuccessfully();
}

// the full tags of the issue, or the values of the volume's list when they cannot be read; and its cover
MissingIssue ComicVineDialog::missingIssue(const QString &issueJson, const SelectedVolumeInfo &volumeInfo)
{
    const auto issue = QJsonDocument::fromJson(issueJson.toUtf8()).object();
    MissingIssue result;
    result.comicVineId = QString::number(issue.value("id").toInteger());

    ComicVineClient comicVineClient;
    bool error;
    bool timeout;
    auto image = issue.value("image").toObject();
    if (!limitReached) {
        const auto detail = comicVineClient.getComicDetail(result.comicVineId, error, timeout);
        if (detailAnswered(error, timeout, detail)) {
            result.patch = YACReader::parseCVJSONComicInfo(detail, volumeInfo);
            const auto detailImage = QJsonDocument::fromJson(detail).object().value("results").toObject().value("image").toObject();
            if (!detailImage.isEmpty())
                image = detailImage;
        }
    }
    if (result.patch.isEmpty()) {
        basicMissingIssues++;
        const QJsonObject list { { "number_of_total_results", 1 }, { "results", issue } };
        result.patch = YACReader::parseCVJSONComicInfo(QString::fromUtf8(QJsonDocument(list).toJson()), volumeInfo);
    }
    result.patch.set(&DescriptiveComicMetadata::comicVineID, result.comicVineId);

    auto coverUrl = image.value("super_url").toString();
    if (coverUrl.isEmpty())
        coverUrl = image.value("medium_url").toString();
    if (!coverUrl.isEmpty()) {
        const auto cover = comicVineClient.getImage(coverUrl, error, timeout);
        if (!error && !timeout)
            result.coverKey = savePlaceholderCover(databasePath, QImage::fromData(cover));
        if (!result.coverKey)
            missingIssuesWithoutCover++;
    }
    return result;
}

bool ComicVineDialog::detailAnswered(bool error, bool timeout, const QByteArray &detail)
{
    const auto answer = YACReader::comicVineAnswer(error, timeout, detail);
    if (answer == YACReader::ComicVineAnswer::LimitReached)
        limitReached = true;
    return answer == YACReader::ComicVineAnswer::Answered;
}

// only when something failed
void ComicVineDialog::showFailures()
{
    QStringList lines;
    if (failedComics > 0)
        lines.append(tr("Comic Vine did not answer for %n comic(s).", "", failedComics));
    if (basicMissingIssues > 0)
        lines.append(tr("%n missing comic(s) got only the basic data of the volume.", "", basicMissingIssues));
    if (missingIssuesWithoutCover > 0)
        lines.append(tr("%n missing comic(s) have no cover.", "", missingIssuesWithoutCover));
    if (lines.isEmpty())
        return;

    lines.append(limitReached ? tr("Comic Vine allows about 200 requests per hour; try again later for the comics that were not updated.")
                              : tr("Try again later for the comics that were not updated."));
    QMessageBox::warning(parentWidget(), tr("Comic Vine"), lines.join(QLatin1Char('\n')));
}

void ComicVineDialog::removeCovers(const QList<MissingIssue> &missingIssues)
{
    QStringList coverKeys;
    for (const auto &issue : missingIssues) {
        if (issue.coverKey)
            coverKeys.append(*issue.coverKey);
    }
    DBHelper::removePlaceholderCovers(databasePath, coverKeys);
}

void ComicVineDialog::getComicInfo(const QString &comicId, const SelectedVolumeInfo &volumeInfo)
{

    auto comicVineClient = new ComicVineClient;
    bool error;
    bool timeout;
    QByteArray result = comicVineClient->getComicDetail(comicId, error, timeout);
    if (!detailAnswered(error, timeout, result)) {
        failedComics++;
        if (mode == ScraperMode::SingleComic || currentIndex == (comics.count() - 1))
            finishSuccessfully();
        else
            goToNextComic();
        return;
    }

    auto patch = YACReader::parseCVJSONComicInfo(result, volumeInfo);
    patch.set(&DescriptiveComicMetadata::comicVineID, comicId);
    setLoadingMessage(tr("Retrieving tags for: %1").arg(recordFileName(comics[currentIndex])));
    saveMetadata({ { comics[currentIndex], patch } });

    if (mode == ScraperMode::SingleComic || currentIndex == (comics.count() - 1)) {
        finishSuccessfully();
    } else {
        goToNextComic();
    }
}

void ComicVineDialog::toggleSkipButton()
{
    if (mode == ScraperMode::SingleComicInList)
        skipButton->setVisible(true);
    else
        skipButton->setHidden(true);
}

QString ComicVineDialog::volumeSearchStringFromComic(const ComicRecord &comic)
{
    const auto &info = recordMetadata(comic);
    auto volume = info.volume.toString().trimmed();
    if (!volume.isEmpty())
        return volume;

    auto series = info.series.toString().trimmed();
    if (!series.isEmpty())
        return series;

    auto alternateSeries = info.alternateSeries.toString().trimmed();
    if (!alternateSeries.isEmpty())
        return alternateSeries;

    // the folder that holds the file, or held it for a deleted comic
    return parentFolderName(recordPath(comic));
}

void ComicVineDialog::saveMetadata(const QList<QPair<ComicRecord, DescriptiveMetadataPatch>> &matches, const QList<MissingIssue> &missingIssues)
{
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(databasePath);
        db.transaction();

        // each comic_info or placeholder is written once
        QSet<ComicInfoId> written;
        QSet<PlaceholderId> writtenPlaceholders;
        for (const auto &[comic, patch] : matches) {
            if (const auto *placeholder = recordPlaceholder(comic)) {
                if (writtenPlaceholders.contains(placeholder->id) || !DBHelper::loadRecord(comic, db))
                    continue;
                writtenPlaceholders.insert(placeholder->id);
                DBHelper::updatePlaceholderMetadata(placeholder->id, patch, db);
                continue;
            }
            const auto id = recordInfo(comic)->id;
            if (written.contains(id) || !DBHelper::loadRecord(comic, db))
                continue;
            written.insert(id);
            DBHelper::updateComicInfoMetadata(id, patch, db);
        }

        std::optional<QStringList> unusedCovers = QStringList();
        if (!missingIssues.isEmpty())
            unusedCovers = folderId ? DBHelper::placeIssuesInFolder(missingIssues, *folderId, db) : std::nullopt;

        if (unusedCovers && db.commit()) {
            DBHelper::removePlaceholderCovers(databasePath, *unusedCovers);
        } else {
            db.rollback();
            removeCovers(missingIssues);
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void ComicVineDialog::goToNextComic()
{
    if (mode == ScraperMode::SingleComic || currentIndex == (comics.count() - 1)) {
        finishSuccessfully();
        return;
    }

    currentIndex++;

    const auto &comic = comics[currentIndex];

    QString title = titleOrFileName(recordMetadata(comic), recordFileName(comic));
    titleHeader->setSubTitle(tr("comic %1 of %2 - %3").arg(currentIndex + 1).arg(comics.length()).arg(title));

    showSearchSingleComic(volumeSearchStringFromComic(comic));
}

void ComicVineDialog::clearState()
{
    selectVolumeWidget->clearFilter();
}

void ComicVineDialog::finishSuccessfully()
{
    // Scraping completion may be reported from a worker thread. Complete the
    // dialog through QDialog's canonical success path on the GUI thread so it
    // closes and emits accepted exactly once.
    QMetaObject::invokeMethod(this, [this]() {
        clearState();
        accept();
        showFailures(); }, Qt::QueuedConnection);
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

// the work threads also report their progress
void ComicVineDialog::setLoadingMessage(const QString &message)
{
    QMetaObject::invokeMethod(loadingMessage, [this, message] { loadingMessage->setText(message); });
}

void ComicVineDialog::search()
{
    switch (mode) {
    case ScraperMode::Volume:
        launchSearchVolume();
        break;

    case ScraperMode::SingleComic:
    case ScraperMode::SingleComicInList:
        launchSearchComic();
        break;
    }
}

void ComicVineDialog::searchVolume(const VolumeSearchQuery &query)
{
    showLoading(tr("Looking for volume..."));

    currentVolumeSearchQuery = query;

    auto comicVineClient = new ComicVineClient;
    connect(comicVineClient, &ComicVineClient::searchResult, this, &ComicVineDialog::processClientResults);
    connect(comicVineClient, &ComicVineClient::timeOut, this, &ComicVineDialog::queryTimeOut);
    connect(comicVineClient, &ComicVineClient::finished, comicVineClient, &QObject::deleteLater);
    if (query.exactMatch) {
        status = ScraperStatus::SearchingExactVolume;
        comicVineClient->searchExactVolume(query.volume, query.page);
    } else {
        status = ScraperStatus::SearchingVolume;
        comicVineClient->search(query.volume, query.page);
    }
}

void ComicVineDialog::getVolumeComicsInfo(const QString &vID, int /* page */)
{
    showLoading(tr("Retrieving volume info..."));

    status = ScraperStatus::GettingVolumeComics;

    auto comicVineClient = new ComicVineClient;
    if (mode == ScraperMode::Volume)
        connect(comicVineClient, &ComicVineClient::volumeComicsInfo, this, &ComicVineDialog::showSortVolumeComics);
    else
        connect(comicVineClient, &ComicVineClient::volumeComicsInfo, this, &ComicVineDialog::showSelectComic);
    connect(comicVineClient, &ComicVineClient::timeOut, this, &ComicVineDialog::queryTimeOut);
    connect(comicVineClient, &ComicVineClient::finished, comicVineClient, &QObject::deleteLater);

    QLOG_TRACE() << vID;

    comicVineClient->getAllVolumeComicsInfo(vID);
}

// TODO: get the search configuration for exact match or not
void ComicVineDialog::launchSearchVolume()
{
    showLoading(tr("Looking for volume..."));
    // TODO: check if volume info is empty.

    QString volumeInfo = searchVolumeWidget->getVolumeInfo();
    bool exactMatch = searchVolumeWidget->getExactMatch();

    searchVolume({ volumeInfo, 1, exactMatch });
}

// TODO: get the search configuration for exact match or not
void ComicVineDialog::launchSearchComic()
{
    showLoading(tr("Looking for comic..."));

    QString volumeInfo = searchSingleComicWidget->getVolumeInfo();
    bool exactMatch = searchSingleComicWidget->getExactMatch();
    // QString comicInfo = searchSingleComicWidget->getComicInfo();
    // int comicNumber = searchSingleComicWidget->getComicNumber();

    // if(comicInfo.isEmpty() && comicNumber == -1)
    searchVolume({ volumeInfo, 1, exactMatch });
}

void ComicVineDialog::applyTheme(const Theme &theme)
{
    auto metadataScraperDialogTheme = theme.metadataScraperDialog;

    setStyleSheet(metadataScraperDialogTheme.dialogQSS);

    skipButton->setStyleSheet(metadataScraperDialogTheme.dialogButtonsQSS);
    backButton->setStyleSheet(metadataScraperDialogTheme.dialogButtonsQSS);
    nextButton->setStyleSheet(metadataScraperDialogTheme.dialogButtonsQSS);
    searchButton->setStyleSheet(metadataScraperDialogTheme.dialogButtonsQSS);
    closeButton->setStyleSheet(metadataScraperDialogTheme.dialogButtonsQSS);

    loadingMessage->setStyleSheet(metadataScraperDialogTheme.defaultLabelQSS);
    busyWidget->setColor(metadataScraperDialogTheme.busyIndicatorColor);
}
