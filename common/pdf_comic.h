#ifndef PDF_COMIC_H
#define PDF_COMIC_H

#include <QObject>
#include <QImage>

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

#endif // PDF_COMIC_H
