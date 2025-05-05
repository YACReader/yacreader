#ifndef COMIC_VINE_DIALOG_H
#define COMIC_VINE_DIALOG_H

#include <QDialog>

#include "comic_db.h"
#include "volume_search_query.h"

class QPushButton;
class QStackedWidget;
class QLabel;
class QRadioButton;
class ComicVineClient;
class QTableView;
class TitleHeader;
class SeriesQuestion;
class SearchSingleComic;
class SearchVolume;
class SelectComic;
class SelectVolume;
struct SelectedVolumeInfo;
class SortVolumeComics;
struct VolumeSearchQuery;

// TODO this should use a QStateMachine
//----------------------------------------
class ComicVineDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ComicVineDialog(QWidget *parent = nullptr);
    QString databasePath;
    QString basePath;
    void setComics(const QList<ComicDB> &comics);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void getComicsInfo(QList<QPair<ComicDB, QString>> matchingInfo, const SelectedVolumeInfo &volumeInfo);
    void getComicInfo(const QString &comicId, const SelectedVolumeInfo &volumeInfo);
    void closeEvent(QCloseEvent *event) override;
signals:

public slots:
    void show();

protected slots:
    void goNext();
    void goBack();
    void processClientResults(const QString &string);
    // show widget methods
    void showSeriesQuestion();
    void showSearchSingleComic(const QString &volume = "");
    void showSearchVolume(const QString &volume = "");
    void showLoading(const QString &message = "");
    void search();
    void searchVolume(const VolumeSearchQuery &query);
    void getVolumeComicsInfo(const QString &vID, int page = 1);
    void launchSearchVolume();
    void launchSearchComic();
    void showSelectVolume(const QString &json);
    void showSelectVolume();
    void showSelectComic(const QString &json);
    void showSortVolumeComics(const QString &json);
    void queryTimeOut();
    void setLoadingMessage(const QString &message);
    void goToNextComic();

private:
    void clearState();
    void toggleSkipButton();
    QString volumeSearchStringFromComic(const ComicDB &comic);

    enum class ScraperMode {
        SingleComic, // the scraper has been opened for a single comic
        Volume, // the scraper is trying to get comics info for a whole volume
        SingleComicInList // the scraper has been opened for a list of unrelated comics
    };

    enum class ScraperStatus {
        AutoSearching, // Searching for volumes maching a single comic
        AskingForInfo, // The dialog is showing some UI to ask the user for some info
        SelectingComic,
        SelectingSeries,
        SearchingSingleComic,
        SearchingVolume,
        SearchingExactVolume,
        SortingComics,
        GettingVolumeComics
    };

    ScraperMode mode;
    ScraperStatus status;

    int currentIndex;

    TitleHeader *titleHeader;

    QPushButton *skipButton;
    QPushButton *backButton;
    QPushButton *nextButton;
    QPushButton *searchButton;
    QPushButton *closeButton;

    // stacked widgets
    QStackedWidget *content;

    QWidget *infoNotFound;
    QWidget *singleComicBrowser;

    QLabel *loadingMessage;

    void doLayout();
    void doStackedWidgets();
    void doLoading();
    void doConnections();

    QList<ComicDB> comics;

    SeriesQuestion *seriesQuestionWidget;
    SearchSingleComic *searchSingleComicWidget;
    SearchVolume *searchVolumeWidget;
    SelectVolume *selectVolumeWidget;
    SelectComic *selectComicWidget;
    SortVolumeComics *sortVolumeComicsWidget;

    VolumeSearchQuery currentVolumeSearchQuery;
    QString currentVolumeId;
};

#endif // COMIC_VINE_DIALOG_H
