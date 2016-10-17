#ifndef PDF_COMIC_H
#define PDF_COMIC_H

#include <QObject>
#include <QImage>

#ifdef Q_OS_MAC
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

#else
#include "poppler-qt5.h"
#endif // Q_OS_MAC

#endif // PDF_COMIC_H
