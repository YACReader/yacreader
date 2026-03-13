#if !defined PDF_COMIC_H && !defined NO_PDF
#define PDF_COMIC_H

#include <QFile>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QtGlobal>

#if defined Q_OS_MACOS && defined USE_PDFKIT
class MacOSXPDFComic
{
public:
    MacOSXPDFComic();
    ~MacOSXPDFComic();
    bool openComic(const QString &path);
    void closeComic();
    unsigned int numPages();
    QImage getPage(const int page);
    // void releaseLastPageData();

private:
    void *document;
    void *lastPageData;
};

#elif defined USE_PDFIUM
#include <fpdfview.h>

class PdfiumComic
{
public:
    PdfiumComic();
    ~PdfiumComic();
    bool openComic(const QString &path);
    void closeComic();
    unsigned int numPages();
    QImage getPage(const int page);

private:
    static int refcount;
    static QMutex pdfmutex;
    FPDF_LIBRARY_CONFIG config;
    FPDF_DOCUMENT doc;
    FPDF_FILEACCESS fileAccess;
    QFile pdfFile;
};
#else
#include <poppler-qt6.h>
#endif // Q_OS_MACOS
#endif // PDF_COMIC_H
