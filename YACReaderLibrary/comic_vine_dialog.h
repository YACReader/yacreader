#ifndef COMIC_VINE_DIALOG_H
#define COMIC_VINE_DIALOG_H

#include <QDialog>
#include <QLineEdit>

#include "comic_db.h"

class QPushButton;
class QStackedWidget;
class QLabel;
class QRadioButton;
class ComicVineClient;

class ScrapperLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	ScrapperLineEdit(const QString & title, QWidget * widget = 0);
protected:
	void resizeEvent(QResizeEvent *);
private:
	QLabel * titleLabel;
};

//----------------------------------------
class TitleHeader : public QWidget
{
	Q_OBJECT
public:
	TitleHeader(QWidget * parent = 0);
public slots:
	void setTitle(const QString & title);
	void setSubTitle(const QString & title);
	void showButtons(bool show);
private:
	QLabel * mainTitleLabel;
	QLabel * subTitleLabel;
};

//----------------------------------------
class SeriesQuestion : public QWidget
{
	Q_OBJECT
public:
	SeriesQuestion(QWidget * parent = 0);
	inline bool getYes();
private:
	QRadioButton * yes;
	QRadioButton * no;
};

//----------------------------------------
class SearchSingleComic : public QWidget
{
	Q_OBJECT
public:
	SearchSingleComic(QWidget * parent = 0);
private:
	ScrapperLineEdit * titleEdit;
	ScrapperLineEdit * numberEdit;
	ScrapperLineEdit * volumeEdit;
};

//---------------------------------------
class SearchVolume : public QWidget
{
	Q_OBJECT
public:
	SearchVolume(QWidget * parent = 0);
public slots:
	QString getVolumeInfo() {return volumeEdit->text();}
private:
	ScrapperLineEdit * volumeEdit;
};

//---------------------------------------
class SelectComic : public QWidget
{
	Q_OBJECT
public:
	SelectComic(QWidget * parent = 0);
	virtual ~SelectComic();
};

//---------------------------------------
class SelectVolume : public QWidget
{
	Q_OBJECT
public:
	SelectVolume(QWidget * parent = 0);
	virtual ~SelectVolume();
};

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
private:

	enum ScrapperStatus
	{
		SingleComic,
		Volume,
		SingleComicInSeries
	};

	ScrapperStatus status;

	ComicVineClient * comicVineClient;

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

	SeriesQuestion * seriesQuestion;
	SearchSingleComic * searchSingleComic;
	SearchVolume * searchVolume;
};

#endif // COMIC_VINE_DIALOG_H
