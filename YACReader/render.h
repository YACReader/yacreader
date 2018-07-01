 #ifndef RENDER_H
#define RENDER_H

#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QThread>
#include <QByteArray>
#include <QVector>
#include "comic.h"
//-----------------------------------------------------------------------------
// FILTERS
//-----------------------------------------------------------------------------

#include <QThread>

class Comic;
class ComicDB;
class Render;

class ImageFilter {
public:
	ImageFilter(){};
	virtual ~ImageFilter() {};
	virtual QImage setFilter(const QImage & image) = 0;
	inline int getLevel() {return level;};
	inline void setLevel(int l) {level = l;};
protected:
	int level;
};

class MeanNoiseReductionFilter : public ImageFilter {
public:
	enum NeighborghoodSize{SMALL=9, LARGE=25 };
	MeanNoiseReductionFilter(enum NeighborghoodSize ns = SMALL);
	virtual QImage setFilter(const QImage & image);
private:
	enum NeighborghoodSize neighborghoodSize;
};

class MedianNoiseReductionFilter : public ImageFilter {
public:
	enum NeighborghoodSize{SMALL=9, LARGE=25 };
	MedianNoiseReductionFilter(enum NeighborghoodSize ns = SMALL);
	virtual QImage setFilter(const QImage & image);
private:
	enum NeighborghoodSize neighborghoodSize;
};

class BrightnessFilter : public ImageFilter {
public:
	BrightnessFilter(int l=-1);
	virtual QImage setFilter(const QImage & image);
};

class ContrastFilter : public ImageFilter {
public:
	ContrastFilter(int l=-1);
	virtual QImage setFilter(const QImage & image);
};

class GammaFilter : public ImageFilter {
public:
	GammaFilter(int l=-1);
	virtual QImage setFilter(const QImage & image);
};

//-----------------------------------------------------------------------------
// RENDER
//-----------------------------------------------------------------------------

class PageRender : public QThread
{
	Q_OBJECT
public:
	PageRender();
		PageRender(Render * render,int numPage, const QByteArray & rawData, QImage * page,unsigned int degrees=0, QVector<ImageFilter *> filters = QVector<ImageFilter *>());
	int getNumPage(){return numPage;};
	void setData(const QByteArray & rawData){data = rawData;};
		void setPage(QImage * p){page = p;};
	void setRotation(unsigned int d){degrees = d;};
	void setFilters(QVector<ImageFilter *> f){filters = f;};
private:
	int numPage;
	QByteArray data;
		QImage * page;
	unsigned int degrees;
	QVector<ImageFilter *> filters;
	void run();
	Render * render;
signals:
	void pageReady(int);

};
//-----------------------------------------------------------------------------
// RENDER
//-----------------------------------------------------------------------------

/*class DoublePageRender : public PageRender
{
	Q_OBJECT
public:
		DoublePageRender(Render * render, int firstPage, const QByteArray & firstPageData,const QByteArray & secondPageData, QImage * page,unsigned int degrees=0, QVector<ImageFilter *> filters = QVector<ImageFilter *>());
private:
	int numPage;
	QByteArray data;
	QByteArray data2;
		QImage * page;
	unsigned int degrees;
	QVector<ImageFilter *> filters;
	void run();
	Render * render;
signals:
	void pageReady(int);

};
*/

class Render : public QObject {
Q_OBJECT
public:
	Render();
	~Render();

public slots:
	void render();
	QPixmap * getCurrentPage();
	QPixmap * getCurrentDoublePage();
	QPixmap * getCurrentDoubleMangaPage();
	bool currentPageIsDoublePage();
	bool nextPageIsDoublePage();
	bool previousPageIsDoublePage();
	void goTo(int index);
	void doublePageSwitch();
	void doubleMangaPageSwitch();
	void setRotation(int degrees);
	void setComic(Comic * c);
	void prepareAvailablePage(int page);
	void update();
	void setNumPages(unsigned int numPages);
	void pageRawDataReady(int page);
	//--comic interface
	void nextPage();
	void previousPage();
	void nextDoublePage();
	void previousDoublePage();
	void load(const QString & path, const ComicDB & comic);
	void load(const QString & path, int atPage);
	void createComic(const QString & path);
	void loadComic(const QString & path,const ComicDB & comic);
	void loadComic(const QString & path, int atPage);
	void startLoad();
	void rotateRight();
	void rotateLeft();
	unsigned int getIndex();
	unsigned int numPages();
	bool hasLoadedComic();
	void updateBuffer();
	void fillBuffer();
	void invalidate();
	QString getCurrentPagesInformation();
	void setBookmark();
	void removeBookmark();
	void save();
	void reset();
	void reload();
	void updateFilters(int brightness, int contrast, int gamma);
	Bookmarks * getBookmarks();
	//sets the firt page to render
	void renderAt(int page);

signals:
	void currentPageReady();
	void processingPage();
	void imagesLoaded();
	void imageLoaded(int index);
	void imageLoaded(int index,const QByteArray & image);
	void pageChanged(int index);
	void numPages(unsigned int numPages);
	void errorOpening();
	void errorOpening(QString);
	void crcError(QString);
	void currentPageIsBookmark(bool);
	void isLast();
	void isCover();

	void bookmarksUpdated();


private:
	Comic * comic;
	bool doublePage;
	bool doubleMangaPage;
	int previousIndex;
	int currentIndex;
	//QPixmap * currentPage;
	int currentPageBufferedIndex;
	int numLeftPages;
	int numRightPages;
	QList<PageRender *> pageRenders;
		QList<QImage *> buffer;
	void loadAll();
	void updateRightPages();
	void updateLeftPages();
	bool loadedComic;
	QList<int> pagesEmited;
	QVector<bool> pagesReady;
	int imageRotation;
	QVector<ImageFilter *> filters;
	QMutex mutex;

	friend class PageRender;
};


#endif // RENDER_H
