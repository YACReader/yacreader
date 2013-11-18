#ifndef SORT_VOLUME_COMICS_H
#define SORT_VOLUME_COMICS_H

#include "scraper_selector.h"

#include <QModelIndex>
#include <QPushButton>
#include <QPainter>

#include "comic_db.h"

class ScraperTableView;
class LocalComicListModel;
class VolumeComicsModel;

class ScrapperToolButton : public QPushButton
{
	Q_OBJECT
public:
	enum Appearance {
		DEFAULT,
		LEFT,
		RIGHT
	};

	ScrapperToolButton(ScrapperToolButton::Appearance appearance = DEFAULT, QWidget * parent=0):QPushButton(parent),appearance(appearance) {
		setStyleSheet("QPushButton {border: none; background: #2e2e2e; color:white; border-radius:2px;}"
					  "QPushButton::pressed {border: none; background: #282828; color:white; border-radius:2px;}");
		setFixedSize(18,17);
	}
	static QWidget * getSeparator(){QWidget * w = new QWidget; w->setFixedWidth(1); w->setStyleSheet("QWidget {background:#282828;}"); return w;}
	void setAppearance(ScrapperToolButton::Appearance appearance){this->appearance = appearance;}
	virtual ~ScrapperToolButton() {}



protected:
	void paintEvent(QPaintEvent * e)
	{
		QPainter p(this);

		switch (appearance) {
		case LEFT:
			p.fillRect(16,0,2,18,QColor("#2E2E2E"));
			break;
		case RIGHT:
			p.fillRect(0,0,2,18,QColor("#2E2E2E"));
			break;
		default:
			break;
		}

		QPushButton::paintEvent(e);
	}

private:
	Appearance appearance;
};


class SortVolumeComics : public ScraperSelector
{
	Q_OBJECT
public:
	explicit SortVolumeComics(QWidget *parent = 0);

signals:

public slots:
	void setData(QList<ComicDB> & comics, const QString & json, const QString & vID);
	QList<QPair<ComicDB,QString> > getMatchingInfo();

protected slots:
	void synchronizeScroll(int pos);
	void moveUpCL();
	void moveDownCL();
	void moveUpIL();
	void moveDownIL();

    void removeSelectedComics();
    void restoreAllComics();
    void showRemovedComicsSelector();


private:
	ScraperTableView * tableFiles;
	ScraperTableView * tableVolumeComics;

	LocalComicListModel * localComicsModel;
	VolumeComicsModel * volumeComicsModel;

	ScrapperToolButton * moveUpButtonCL;
	ScrapperToolButton * moveDownButtonCL;
	ScrapperToolButton * moveUpButtonIL;
	ScrapperToolButton * moveDownButtonIL;

};

#endif // SORT_VOLUME_COMICS_H
