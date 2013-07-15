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

class ImageFilter {
public:
    ImageFilter(){};
    virtual QImage setFilter(const QImage & image) = 0;
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
    BrightnessFilter(int l=150);
    virtual QImage setFilter(const QImage & image);
private:
    int level;
};

class ContrastFilter : public ImageFilter {
public:
    ContrastFilter(int l=150);
    virtual QImage setFilter(const QImage & image);
private:
    int level;
};

class GammaFilter : public ImageFilter {
public:
    GammaFilter(int l=150);
    virtual QImage setFilter(const QImage & image);
private:
    int level;
};

//-----------------------------------------------------------------------------
// RENDER
//-----------------------------------------------------------------------------

class PageRender : public QThread
{
	Q_OBJECT
public:
	PageRender();
        PageRender(int numPage, const QByteArray & rawData, QImage * page,unsigned int degrees=0, QVector<ImageFilter *> filters = QVector<ImageFilter *>());
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
signals:
	void pageReady(int);

};
//-----------------------------------------------------------------------------
// RENDER
//-----------------------------------------------------------------------------

class DoublePageRender : public PageRender
{
	Q_OBJECT
public:
        DoublePageRender(int firstPage, const QByteArray & firstPageData,const QByteArray & secondPageData, QImage * page,unsigned int degrees=0, QVector<ImageFilter *> filters = QVector<ImageFilter *>());
private:
	int numPage;
	QByteArray data;
	QByteArray data2;
        QImage * page;
	unsigned int degrees;
	QVector<ImageFilter *> filters;
	void run();
signals:
	void pageReady(int);

};


class Render : public QObject {
Q_OBJECT
public:
	Render();

public slots:
	void render();
	QPixmap * getCurrentPage();
	void goTo(int index);
	void doublePageSwitch();
	void setRotation(int degrees);
	void setComic(Comic * c);
	void prepareAvailablePage(int page);
	void update();
	void setNumPages(unsigned int numPages);
	void pageRawDataReady(int page);
	//--comic interface
	void nextPage();
	void previousPage();
	void load(const QString & path);
	void rotateRight();
	void rotateLeft();
	unsigned int getIndex();
	unsigned int numPages();
	bool hasLoadedComic();
	void updateBuffer();
	void fillBuffer();
	void fillBufferDoublePage();
	void invalidate();
	QString getCurrentPagesInformation();
	void setBookmark();
	void removeBookmark();
	void save();
	void reset();
	void reload();
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
	void currentPageIsBookmark(bool);
	void isLast();
	void isCover();

	void bookmarksUpdated();


private:
	Comic * comic;
	bool doublePage;
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

};


#endif // RENDER_H
