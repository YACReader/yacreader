#ifndef SEARCH_SINGLE_COMIC_H
#define SEARCH_SINGLE_COMIC_H

#include <QWidget>

class ScraperLineEdit;

class SearchSingleComic : public QWidget
{
	Q_OBJECT
public:
	SearchSingleComic(QWidget * parent = 0);
	QString getVolumeInfo();
	QString getComicInfo();
	int getComicNumber();
	void clean();
private:
	ScraperLineEdit * titleEdit;
	ScraperLineEdit * numberEdit;
	ScraperLineEdit * volumeEdit;
};
#endif // SEARCH_SINGLE_COMIC_H
