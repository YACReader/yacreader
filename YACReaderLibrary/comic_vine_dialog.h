#ifndef COMIC_VINE_DIALOG_H
#define COMIC_VINE_DIALOG_H

#include <QDialog>

class QPushButton;
class QStackedWidget;

class ComicVineDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ComicVineDialog(QWidget *parent = 0);
		
signals:
	
public slots:

private:
	QPushButton * nextButton; 
	QPushButton * closeButton;

	//stacked widgets
	QStackedWidget * content;

	QWidget * infoNotFound;
	QWidget * singleComicBrowser;
	
	void doLayout();
	void doStackedWidgets();
	void doSeriesQuestion();
	void doConnections();
};

#endif // COMIC_VINE_DIALOG_H
