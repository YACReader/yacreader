#ifndef COMIC_VINE_DIALOG_H
#define COMIC_VINE_DIALOG_H

#include <QDialog>

#include "comic_db.h"

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
class SortVolumeComics;


//----------------------------------------
class ComicVineDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ComicVineDialog(QWidget *parent = 0);
	QString databasePath;
	QString basePath;
	void setComics(const QList<ComicDB> & comics);
signals:
	
public slots:
	void show();
protected slots:
	void goNext();
	void debugClientResults(const QString & string);
	//show widget methods
	void showSeriesQuestion();
	void showSearchSingleComic();
	void showSearchVolume();
	void showLoading();
	void search();
	void searchVolume(const QString & v);
	void launchSearchVolume();
	void launchSearchComic();
	void showSelectVolume(const QString & json);
	void showSelectComic(const QString & json);
	void showSortVolumeComics(const QString & json);
private:

	enum ScrapperMode
	{
		SingleComic,       //the scrapper has been opened for a single comic
		Volume,            //the scrapper is trying to get comics info for a whole volume
		SingleComicInList  //the scrapper has been opened for a list of unrelated comics
	};

	enum ScrapperStatus
	{
		AutoSearching,
		AskingForInfo,
		SelectingComic,
		SelectingSeries,
		SearchingSingleComic,
		SearchingVolume
	};

	ScrapperMode mode;
	ScrapperStatus status;

	int currentIndex;

	TitleHeader * titleHeader;

	QPushButton * skipButton;
	QPushButton * backButton;
	QPushButton * nextButton;
	QPushButton * searchButton;
	QPushButton * closeButton;

	//stacked widgets
	QStackedWidget * content;

	QWidget * infoNotFound;
	QWidget * singleComicBrowser;
	
	void doLayout();
	void doStackedWidgets();
	void doLoading();
	void doConnections();

	QList<ComicDB> comics;

	SeriesQuestion * seriesQuestionWidget;
	SearchSingleComic * searchSingleComicWidget;
	SearchVolume * searchVolumeWidget;
	SelectVolume * selectVolumeWidget;
	SelectComic * selectComicWidget;
	SortVolumeComics * sortVolumeComicsWidget;
};

#endif // COMIC_VINE_DIALOG_H
