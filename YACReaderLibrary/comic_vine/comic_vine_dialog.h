#ifndef COMIC_VINE_DIALOG_H
#define COMIC_VINE_DIALOG_H

#include "comic_record.h"
#include "db_helper.h"
#include "descriptive_comic_metadata.h"
#include "library_ids.h"
#include "themable.h"
#include "volume_search_query.h"

#include <QDialog>

#include <atomic>
#include <optional>

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
class YACReaderBusyWidget;

// TODO this should use a QStateMachine
//----------------------------------------
class ComicVineDialog : public QDialog, protected Themable
{
    Q_OBJECT
public:
    explicit ComicVineDialog(QWidget *parent = nullptr);
    QString databasePath;
    QString basePath;
    // the folder of the comics; only then the other issues of a volume can be added as missing comics
    std::optional<FolderId> folderId;
    void setComics(const QList<ComicRecord> &comics);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void getComicsInfo(QList<QPair<ComicRecord, QString>> matchingInfo, const QStringList &missingIssues, const SelectedVolumeInfo &volumeInfo, int runId);
    void getComicInfo(const QString &comicId, const SelectedVolumeInfo &volumeInfo);
    void closeEvent(QCloseEvent *event) override;
    void done(int result) override;
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
    void finishSuccessfully();
    void toggleSkipButton();
    QString volumeSearchStringFromComic(const ComicRecord &comic);
    // writes each patch to the comic_info of its record when the record still exists, each comic_info once, and the
    // missing issues to the folder, in one transaction
    void saveMetadata(const QList<QPair<ComicRecord, DescriptiveMetadataPatch>> &matches, const QList<MissingIssue> &missingIssues = { });
    MissingIssue missingIssue(const QString &issueJson, const SelectedVolumeInfo &volumeInfo);
    void removeCovers(const QList<MissingIssue> &missingIssues);

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
    YACReaderBusyWidget *busyWidget;

    void doLayout();
    void doStackedWidgets();
    void doLoading();
    void doConnections();

    QList<ComicRecord> comics;

    SeriesQuestion *seriesQuestionWidget;
    SearchSingleComic *searchSingleComicWidget;
    SearchVolume *searchVolumeWidget;
    SelectVolume *selectVolumeWidget;
    SelectComic *selectComicWidget;
    SortVolumeComics *sortVolumeComicsWidget;

    VolumeSearchQuery currentVolumeSearchQuery;
    QString currentVolumeId;
    // a closed or reopened dialog changes it, so the work of the previous run writes nothing
    std::atomic_int run { 0 };

protected:
    void applyTheme(const Theme &theme) override;
};

#endif // COMIC_VINE_DIALOG_H
