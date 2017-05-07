#include "comic.h"
#include "pdf_comic.h"
#if defined USE_PDFIUM && !defined NO_PDF
PdfiumComic::PdfiumComic()
{
	FPDF_InitLibrary();
}

PdfiumComic::~PdfiumComic()
{
	if (doc)
	{
		FPDF_CloseDocument(doc);
	}
	FPDF_DestroyLibrary();
}

bool PdfiumComic::openComic(const QString & path)
{
	doc = FPDF_LoadDocument(path.toStdString().c_str(), NULL);
	if (doc)
	{
		return true;
	}
	else
	{
		qDebug() << FPDF_GetLastError();
		return false;
	}
}

void PdfiumComic::closeComic()
{
	FPDF_CloseDocument(doc);
}

unsigned int PdfiumComic::numPages()
{
	if (doc)
	{
		return FPDF_GetPageCount(doc);
	}
	else
	{
		return 0; //-1?
	}
}

QImage PdfiumComic::getPage(const int page)
{
	QImage image;
	FPDF_PAGE pdfpage;
	FPDF_BITMAP bitmap;
	
	pdfpage = FPDF_LoadPage(doc, page);
	
	if (!pdfpage)
	{
		qDebug() << FPDF_GetLastError();
		return QImage();
	}
	
	//TODO: make target DPI configurable
	double width = (FPDF_GetPageWidth(pdfpage)/72)*150;
	double height = (FPDF_GetPageHeight(pdfpage)/72)*150;
	
	image = QImage(width, height, QImage::Format_ARGB32);// QImage::Format_RGBX8888);
	image.fill(0xFFFFFFFF);

	bitmap = FPDFBitmap_CreateEx(image.width(), image.height(), FPDFBitmap_BGRA, image.scanLine(0), image.bytesPerLine());
	//TODO: make render flags costumizable
	FPDF_RenderPageBitmap(bitmap, pdfpage, 0,0, image.width(), image.height(), 0, (FPDF_LCD_TEXT));
	FPDFBitmap_Destroy(bitmap);
	FPDF_ClosePage(pdfpage);
	return image;
}
#endif //USE_PDFIUM
