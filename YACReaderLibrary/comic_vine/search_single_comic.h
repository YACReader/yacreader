#ifndef SEARCH_SINGLE_COMIC_H
#define SEARCH_SINGLE_COMIC_H

#include <QWidget>

class ScrapperLineEdit;

class SearchSingleComic : public QWidget
{
	Q_OBJECT
public:
	SearchSingleComic(QWidget * parent = 0);
	QString getVolumeInfo();
	QString getComicInfo();
	int getComicNumber();
private:
	ScrapperLineEdit * titleEdit;
	ScrapperLineEdit * numberEdit;
	ScrapperLineEdit * volumeEdit;
};
#endif // SEARCH_SINGLE_COMIC_H
