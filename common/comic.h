#ifndef __COMIC_H
#define __COMIC_H
#include <QtCore>
#include <QImage>
#include <QtGui>
#include <QByteArray>
#include <QMap>

#include "extract_delegate.h"
#include "bookmarks.h"
#include "pdf_comic.h"

class ComicDB;
//#define EXTENSIONS << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.tiff" << "*.tif" << "*.bmp" Comic::getSupportedImageFormats()
//#define EXTENSIONS_LITERAL << ".jpg" << ".jpeg" << ".png" << ".gif" << ".tiff" << ".tif" << ".bmp" //Comic::getSupportedImageLiteralFormats()
class Comic : public QObject
{
	Q_OBJECT
	
	protected:
		
		//Comic pages, one QPixmap for each file.
		QVector<QByteArray> _pages;
		QVector<bool> _loadedPages;
		//QVector<uint> _sizes;
		QStringList _fileNames;
		QMap<QString,int> _newOrder;
		QList<QString> _order;
		int _index;
		QString _path;
		bool _loaded;

		int _cfi;

		//open the comic at this point
		int _firstPage;

		bool _isPDF;

	public:
		
		static const QStringList imageExtensions;
		static const QStringList literalImageExtensions;
		static const QStringList comicExtensions;
		static const QStringList literalComicExtensions;

		Bookmarks * bm;

		//Constructors
		Comic();
		Comic(const QString & pathFile, int atPage = -1);
		~Comic();
		void setup();
		//Load pages from file
		virtual bool load(const QString & path, int atPage = -1) = 0;
		virtual bool load(const QString & path, const ComicDB & comic);
		
		/*void loadFromFile(const QString & pathFile);
		void loadFromDir(const QString & pathDir);
		void loadFromPDF(const QString & pathPDF);*/
		int nextPage();
		int previousPage();
		void setIndex(unsigned int index);
		unsigned int getIndex(){return _index;};
		unsigned int numPages(){return _pages.size();}
		//QPixmap * currentPage();
		bool loaded();
		//QPixmap * operator[](unsigned int index);
		QVector<QByteArray> * getRawData(){return &_pages;}
		QByteArray getRawPage(int page);
		bool pageIsLoaded(int page);

		inline static QStringList getSupportedImageFormats() { return imageExtensions;}
		inline static QStringList getSupportedImageLiteralFormats() { return literalImageExtensions;}

		static bool fileIsComic(const QString &path);
		static QList<QString> findValidComicFiles(const QList<QUrl> & list);
		static QList<QString> findValidComicFilesInFolder(const QString &path);
	
	public slots:
	
		void loadFinished();
		void setBookmark();
		void removeBookmark();
		void saveBookmarks();
		void checkIsBookmark(int index);
		void updateBookmarkImage(int);
		void setPageLoaded(int page);
		
	signals:
	
		void imagesLoaded();
		void imageLoaded(int index);
		void imageLoaded(int index,const QByteArray & image);
		void pageChanged(int index);
		void openAt(int index);
		void numPages(unsigned int numPages);
		void errorOpening();
		void errorOpening(QString);
		void crcErrorFound(QString);
		void isBookmark(bool);
		void bookmarksUpdated();
		void isCover();
		void isLast();
};

class FileComic : public Comic, public ExtractDelegate
{
	Q_OBJECT
	
	private:
		
		QList<QVector<quint32> > getSections(int & sectionIndex);
	
	public:
	
		FileComic();
		FileComic(const QString & path, int atPage = -1);
		~FileComic();
		void fileExtracted(int index, const QByteArray & rawData);
		virtual bool load(const QString & path, int atPage = -1);
		virtual bool load(const QString & path, const ComicDB & comic);
		void crcError(int index);
		void unknownError(int index);
		static QList<QString> filter(const QList<QString> & src);
	
	public slots:
		
		void process();
};

class FolderComic : public Comic 
{
	Q_OBJECT
	
	private:
		//void run();
	
	public:
		
		FolderComic();
		FolderComic(const QString & path, int atPage = -1);
		~FolderComic();

		virtual bool load(const QString & path, int atPage = -1);
	
	public slots:
	
		void process();
};

class PDFComic : public Comic 
{
	Q_OBJECT
	
	private:
		//pdf
#ifdef Q_OS_MAC
			MacOSXPDFComic * pdfComic;
#else
		Poppler::Document * pdfComic;
#endif
		void renderPage(int page);

		//void run();
	
	public:
	
		PDFComic();
		PDFComic(const QString & path, int atPage = -1);
		~PDFComic();

		virtual bool load(const QString & path, int atPage = -1);
		virtual bool load(const QString & path, const ComicDB & comic);
	
	public slots:
	
		void process();
};

class FactoryComic
{
	public:
		static Comic * newComic(const QString & path);
};
#endif