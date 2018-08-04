#ifndef SELECT_COMIC_H
#define SELECT_COMIC_H

#include "scraper_selector.h"

class QLabel;
class VolumeComicsModel;
class QModelIndex;

class ScraperScrollLabel;
class ScraperTableView;

class SelectComic : public ScraperSelector
{
	Q_OBJECT
public:
	SelectComic(QWidget * parent = 0);
	void load(const QString & json, const QString & searchString);
	virtual ~SelectComic();

public slots:
	void loadComicInfo(const QModelIndex & mi);
	void setCover(const QByteArray &);
	void setDescription(const QString & jsonDetail);
	QString getSelectedComicId();

private:
	QLabel * cover;
	ScraperScrollLabel * detailLabel;
	ScraperTableView * tableComics;
	VolumeComicsModel * model;
};

#endif // SELECT_COMIC_H
