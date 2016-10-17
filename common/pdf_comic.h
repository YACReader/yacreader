#ifndef PDF_COMIC_H
#define PDF_COMIC_H

#include <QObject>
#include <QImage>

#if defined Q_OS_MAC && defined USE_PDFKIT
class MacOSXPDFComic
{
	public:
		MacOSXPDFComic();
		~MacOSXPDFComic();
		bool openComic(const QString & path);
		void closeComic();
		unsigned int numPages();
		QImage getPage(const int page);
		void releaseLastPageData();
	
	private:
		void * document;
		void * lastPageData;
	};

#elif defined USE_PDFIUM
#include <fpdfview.h>

class PdfiumComic
{
	public:
		PdfiumComic();
		~PdfiumComic();
		bool openComic(const QString & path);
		void closeComic();
		unsigned int numPages();
		QImage getPage(const int page);
	
	private:
		FPDF_LIBRARY_CONFIG config;
		FPDF_DOCUMENT doc;
};
#else
#include "poppler-qt5.h"
#endif // Q_OS_MAC
#endif // PDF_COMIC_H
