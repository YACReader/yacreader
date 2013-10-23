#ifndef SORT_VOLUME_COMICS_H
#define SORT_VOLUME_COMICS_H

#include <QWidget>
#include <QModelIndex>

#include "comic_db.h"

class ScraperTableView;
class QPushButton;
class LocalComicListModel;
class VolumeComicsModel;

class SortVolumeComics : public QWidget
{
	Q_OBJECT
public:
	explicit SortVolumeComics(QWidget *parent = 0);

signals:

public slots:
	void setData(QList<ComicDB> & comics, const QString & json);

protected slots:
	void synchronizeScroll(int pos);

private:
	ScraperTableView * tableFiles;
	ScraperTableView * tableVolumeComics;

	LocalComicListModel * localComicsModel;
	VolumeComicsModel * volumeComicsModel;

	QPushButton * moveUpButton;
	QPushButton * moveDownButton;

};

#endif // SORT_VOLUME_COMICS_H
