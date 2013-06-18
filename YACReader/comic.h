#ifndef __COMIC_H
#define __COMIC_H
#include <QtCore>
#include <QImage>
#include <QtGui>
#include <QByteArray>
#include <QMap>

#include "bookmarks.h"

#include "poppler-qt4.h"

	class Comic : public QObject
	{
	Q_OBJECT
	protected:
		//Comic pages, one QPixmap for each file.
		QVector<QByteArray> _pages;
		QVector<bool> _loadedPages;
		QVector<uint> _sizes;
		QStringList _fileNames;
		QMap<QString,int> _newOrder;
		QVector<QString> _order;
		int _index;
		QString _path;
		bool _loaded;

		int _cfi;

		
		

		bool _isPDF;
	public:
		Bookmarks * bm;

		//Constructors
		Comic();
		Comic(const QString & pathFile);
		~Comic();
		void setup();
		//Load pages from file
		virtual bool load(const QString & path) = 0;
		
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
		QVector<QByteArray> * getRawData(){return &_pages;};
		QByteArray getRawPage(int page);
		bool pageIsLoaded(int page);
		
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
		void numPages(unsigned int numPages);
		void errorOpening();
		void isBookmark(bool);
		void bookmarksUpdated();
		void isCover();
		void isLast();
		
	};

	class FileComic : public Comic
	{
		Q_OBJECT
	private:
		QProcess * _7z;
		QProcess * _7ze;
	public:
		FileComic();
		FileComic(const QString & path);
		~FileComic();

		virtual bool load(const QString & path);
	
	public slots:
		void loadImages();
		void loadSizes();
		void openingError(QProcess::ProcessError error);
	};

	class FolderComic : public Comic 
	{
		Q_OBJECT
	private:
		//void run();
	public:
		FolderComic();
		FolderComic(const QString & path);
		~FolderComic();

		virtual bool load(const QString & path);
	public slots:
		void process();

	};

	class PDFComic : public Comic 
	{
		Q_OBJECT
	private:
		//pdf
		Poppler::Document * pdfComic;

		//void run();
	public:
		PDFComic();
		PDFComic(const QString & path);
		~PDFComic();

		virtual bool load(const QString & path);
	public slots:
		void process();
	};

	class FactoryComic
	{
	public:
		static Comic * newComic(const QString & path);
	};
	

#endif
