#include "comic.h"
#include "pdf_comic.h"

#if defined USE_PDFIUM && !defined NO_PDF

int pdfRead(void* param,
            unsigned long position,
            unsigned char* pBuf,
            unsigned long size) {

    QFile *file = static_cast<QFile *>(param);

    file->seek(position);

    qint64 numBytesRead = file->read(reinterpret_cast<char *>(pBuf), size);

    if(numBytesRead > 0)
    {
        return numBytesRead;
    }

    return 0;
}

// pdfium is not threadsafe
// We need to use mutex locking & refcounting to avoid crashes

int PdfiumComic::refcount = 0;
QMutex PdfiumComic::pdfmutex;

PdfiumComic::PdfiumComic()
{
  QMutexLocker locker(&pdfmutex);
  if (++refcount == 1) {
	   FPDF_InitLibrary();
   }
}

PdfiumComic::~PdfiumComic()
{
  QMutexLocker locker(&pdfmutex);
	if (doc)
	{
		FPDF_CloseDocument(doc);
	}
  if (--refcount == 0) {
    FPDF_DestroyLibrary();
  }
}

bool PdfiumComic::openComic(const QString & path)
{
    pdfFile.setFileName(path);

    if(pdfFile.open(QIODevice::ReadOnly) == false)
    {
        qDebug() << "unable to open file : " << path;
        return false;
    }

    fileAccess.m_FileLen = pdfFile.size();
    fileAccess.m_GetBlock = pdfRead;
    fileAccess.m_Param = &pdfFile;

    QMutexLocker lock(&pdfmutex);
    doc = FPDF_LoadCustomDocument(&fileAccess, NULL);
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
    QMutexLocker locker(&pdfmutex);
    FPDF_CloseDocument(doc);
}

unsigned int PdfiumComic::numPages()
{
	if (doc)
	{
    QMutexLocker locker(&pdfmutex);
		return FPDF_GetPageCount(doc);
	}
	else
	{
		return 0; //-1?
	}
}

QImage PdfiumComic::getPage(const int page)
{
	if (!doc)
	{
		return QImage();
	}

	QImage image;
	FPDF_PAGE pdfpage;
	FPDF_BITMAP bitmap;

  QMutexLocker locker(&pdfmutex);
	pdfpage = FPDF_LoadPage(doc, page);

	if (!pdfpage)
	{
    // TODO report error
    qDebug() << FPDF_GetLastError();
		return QImage();
	}

	// TODO: make target DPI configurable
	double width = (FPDF_GetPageWidth(pdfpage)/72)*150;
	double height = (FPDF_GetPageHeight(pdfpage)/72)*150;

	image = QImage(width, height, QImage::Format_ARGB32);// QImage::Format_RGBX8888);
	if (image.isNull())
	{
    // TODO report OOM error
    qDebug() << "Image too large, OOM";
		return image;
	}
	image.fill(0xFFFFFFFF);

	bitmap = FPDFBitmap_CreateEx(image.width(), image.height(), FPDFBitmap_BGRA, image.scanLine(0), image.bytesPerLine());
	// TODO: make render flags costumizable
	FPDF_RenderPageBitmap(bitmap, pdfpage, 0,0, image.width(), image.height(), 0, (FPDF_LCD_TEXT));
	FPDFBitmap_Destroy(bitmap);
	FPDF_ClosePage(pdfpage);
	return image;
}
#endif //USE_PDFIUM
