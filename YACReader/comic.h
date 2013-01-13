#ifndef __COMIC_H
#define __COMIC_H
#include <QtCore>
#include <QImage>
#include <QtGui>
#include <QByteArray>
#include <QMap>

#include "bookmarks.h"

#include "poppler-qt4.h"

	class Comic : public QThread
	{
	Q_OBJECT
	private:
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
		QProcess * _7z;
		QProcess * _7ze;
		int _cfi;
		QString _pathDir;
		Bookmarks * bm;
		void run();
		//pdf
		Poppler::Document * pdfComic;
		bool _isPDF;
	public:
		//Constructors
		Comic();
		Comic(const QString pathFile);
		~Comic();
		void setup();
		//Load pages from file
		bool load(const QString & path);
		void loadFromFile(const QString & pathFile);
		void loadFromDir(const QString & pathDir);
		void loadFromPDF(const QString & pathPDF);
		int nextPage();
		int previousPage();
		void setIndex(unsigned int index);
		unsigned int getIndex(){return _index;};
		unsigned int numPages(){return _pages.size();}
		QPixmap * currentPage();
		bool loaded();
		QPixmap * operator[](unsigned int index);
		QVector<QByteArray> * getRawData(){return &_pages;};
		QByteArray getRawPage(int page);
		bool pageIsLoaded(int page);
		
	public slots:
		void loadImages();
		void loadSizes();
		void openingError(QProcess::ProcessError error);
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
		void bookmarksLoaded(const Bookmarks &);
		
	};



	class Comic2 : public QObject
	{
		Q_OBJECT
	private:
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
		QProcess * _7z;
		QProcess * _7ze;
		int _cfi;
		QString _pathDir;

	public:
		//Constructors
		Comic2();
		Comic2(const QString pathFile);
		~Comic2();
		void setup();
		//Load pages from file
		bool load(const QString & path);
		void loadFromFile(const QString & pathFile);
		int nextPage();
		int previousPage();
		void setIndex(unsigned int index);
		unsigned int getIndex(){return _index;};
		unsigned int numPages(){return _pages.size();}
		QPixmap * currentPage();
		bool loaded();
		QPixmap * operator[](unsigned int index);
		QVector<QByteArray> * getRawData(){return &_pages;};
		QByteArray getRawPage(int page);
		bool pageIsLoaded(int page);

		public slots:
			void loadImages();
			void loadSizes();
			void openingError(QProcess::ProcessError error);
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
			void bookmarksLoaded(const Bookmarks &);

	};

#endif
