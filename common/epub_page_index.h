#ifndef EPUB_PAGE_INDEX_H
#define EPUB_PAGE_INDEX_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>
namespace YACReaderEpub {

struct Page {
    QString fileName;
    int archiveIndex = -1;
};

struct PageIndex {
    QVector<Page> pages;
    bool fixedLayout = false;
    QString coverPath;
    QString error;

    bool isValid() const { return error.isEmpty() && !pages.isEmpty(); }
};

struct ScanInfo {
    int pageCount = 0;
    int coverArchiveIndex = -1;
    QString error;

    bool isValid() const { return error.isEmpty() && pageCount > 0; }
};

using FileReader = std::function<QByteArray(int)>;
using ImageFilter = std::function<bool(const QString &)>;

PageIndex readPageIndex(const QStringList &fileNames, const FileReader &readFile, const ImageFilter &acceptImage = { });
ScanInfo readScanInfo(const QStringList &fileNames, const FileReader &readFile, int coverPage, const ImageFilter &acceptImage);

}

#endif // EPUB_PAGE_INDEX_H
