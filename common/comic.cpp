#include "comic.h"

#include <QPixmap>
#include <QRegExp>
#include <QString>
#include <algorithm>
#include <QDir>
#include <QFileInfoList>
#include <QCoreApplication>

#include "bookmarks.h" //TODO desacoplar la dependencia con bookmarks
#include "qnaturalsorting.h"
#include "compressed_archive.h"
#include "comic_db.h"

#include "QsLog.h"

enum YACReaderPageSortingMode {
    YACReaderNumericalSorting,
    YACReaderHeuristicSorting,
    YACReaderAlphabeticalSorting
};

void comic_pages_sort(QList<QString> &pageNames, YACReaderPageSortingMode sortingMode);

QStringList Comic::getSupportedImageFormats()
{
    QList<QByteArray> supportedImageFormats = QImageReader::supportedImageFormats();
    QStringList supportedImageFormatStrings;
    for (QByteArray item : supportedImageFormats) {
        supportedImageFormatStrings.append(QString::fromLocal8Bit("*." + item));
    }
    return supportedImageFormatStrings;
}

QStringList Comic::getSupportedImageLiteralFormats()
{
    QList<QByteArray> supportedImageFormats = QImageReader::supportedImageFormats();
    QStringList supportedImageFormatStrings;
    for (QByteArray item : supportedImageFormats) {
        supportedImageFormatStrings.append(QString::fromLocal8Bit(item));
    }
    return supportedImageFormatStrings;
}

const QStringList Comic::imageExtensions = QStringList() << "*.jpg"
                                                         << "*.jpeg"
                                                         << "*.png"
                                                         << "*.gif"
                                                         << "*.tiff"
                                                         << "*.tif"
                                                         << "*.bmp"
                                                         << "*.webp";
const QStringList Comic::literalImageExtensions = QStringList() << "jpg"
                                                                << "jpeg"
                                                                << "png"
                                                                << "gif"
                                                                << "tiff"
                                                                << "tif"
                                                                << "bmp"
                                                                << "webp";

#ifndef use_unarr
const QStringList ComicArchiveExtensions = QStringList() << "*.cbr"
                                                         << "*.cbz"
                                                         << "*.rar"
                                                         << "*.zip"
                                                         << "*.tar"
                                                         << "*.7z"
                                                         << "*.cb7"
                                                         << "*.arj"
                                                         << "*.cbt";
const QStringList LiteralComicArchiveExtensions = QStringList() << "cbr"
                                                                << "cbz"
                                                                << "rar"
                                                                << "zip"
                                                                << "tar"
                                                                << "7z"
                                                                << "cb7"
                                                                << "arj"
                                                                << "cbt";
#else
const QStringList ComicArchiveExtensions = QStringList() << "*.cbr"
                                                         << "*.cbz"
                                                         << "*.rar"
                                                         << "*.zip"
                                                         << "*.tar"
                                                         << "*.7z"
                                                         << "*.cb7"
                                                         << "*.cbt";
const QStringList LiteralComicArchiveExtensions = QStringList() << "cbr"
                                                                << "cbz"
                                                                << "rar"
                                                                << "zip"
                                                                << "tar"
                                                                << "7z"
                                                                << "cb7"
                                                                << "cbt";
#endif //use_unarr
#ifndef NO_PDF
const QStringList Comic::comicExtensions = QStringList() << ComicArchiveExtensions << "*.pdf";
const QStringList Comic::literalComicExtensions = QStringList() << LiteralComicArchiveExtensions << "pdf";
#else
const QStringList Comic::comicExtensions = ComicArchiveExtensions;
const QStringList Comic::literalComicExtensions = LiteralComicArchiveExtensions;
#endif //NO_PDF

//-----------------------------------------------------------------------------
Comic::Comic()
    : _pages(), _loadedPages(), _index(0), _path(), _loaded(false), _isPDF(false), _invalidated(false), _errorOpening(false), bm(new Bookmarks())
{
    setup();
}
//-----------------------------------------------------------------------------
Comic::Comic(const QString &pathFile, int atPage)
    : _pages(), _loadedPages(), _index(0), _path(pathFile), _loaded(false), _firstPage(atPage), _isPDF(false), _errorOpening(false), bm(new Bookmarks())
{
    setup();
}
//-----------------------------------------------------------------------------
Comic::~Comic()
{
    delete bm;
}
//-----------------------------------------------------------------------------
void Comic::setup()
{
    connect(this, SIGNAL(pageChanged(int)), this, SLOT(checkIsBookmark(int)));
    connect(this, SIGNAL(imageLoaded(int)), this, SLOT(updateBookmarkImage(int)));
    connect(this, SIGNAL(imageLoaded(int)), this, SLOT(setPageLoaded(int)));

    auto l = [&]() { _errorOpening = true; };

    void (Comic::*errorOpeningPtr)() = &Comic::errorOpening;
    void (Comic::*errorOpeningWithStringPtr)(QString) = &Comic::errorOpening;

    connect(this, errorOpeningPtr, l);
    connect(this, errorOpeningWithStringPtr, l);

    connect(this, &Comic::crcErrorFound, l);
}
//-----------------------------------------------------------------------------
int Comic::nextPage()
{
    if (_index < _pages.size() - 1) {
        _index++;

        emit pageChanged(_index);
    } else {
        emit isLast();
    }
    return _index;
}
//---------------------------------------------------------------------------
int Comic::previousPage()
{
    if (_index > 0) {
        _index--;

        emit pageChanged(_index);
    } else {
        emit isCover();
    }
    return _index;
}
//-----------------------------------------------------------------------------
void Comic::setIndex(unsigned int index)
{
    int previousIndex = _index;
    if (static_cast<int>(index) < _pages.size() - 1) {
        _index = index;
    } else {
        _index = _pages.size() - 1;
    }

    if (previousIndex != _index) {
        emit pageChanged(_index);
    }
}
//-----------------------------------------------------------------------------
/*QPixmap * Comic::currentPage()
{
	QPixmap * p = new QPixmap();
	p->loadFromData(_pages[_index]);
	return p;
}
//-----------------------------------------------------------------------------
QPixmap * Comic::operator[](unsigned int index)
{
	QPixmap * p = new QPixmap();
	p->loadFromData(_pages[index]);
	return p;
}*/
bool Comic::load(const QString &path, const ComicDB &comic)
{
    Q_UNUSED(path);
    Q_UNUSED(comic);
    return false;
};
//-----------------------------------------------------------------------------
bool Comic::loaded()
{
    return _loaded;
}
//-----------------------------------------------------------------------------
void Comic::loadFinished()
{
    emit imagesLoaded();
}
//-----------------------------------------------------------------------------
void Comic::setBookmark()
{
    QImage p;
    p.loadFromData(_pages[_index]);
    bm->setBookmark(_index, p);
    //emit bookmarksLoaded(*bm);
    emit bookmarksUpdated();
}
//-----------------------------------------------------------------------------
void Comic::removeBookmark()
{
    bm->removeBookmark(_index);
    //emit bookmarksLoaded(*bm);
    emit bookmarksUpdated();
}
//-----------------------------------------------------------------------------
void Comic::saveBookmarks()
{
    QImage p;
    p.loadFromData(_pages[_index]);
    bm->setLastPage(_index, p);
    bm->save();
}
//-----------------------------------------------------------------------------
void Comic::checkIsBookmark(int index)
{
    emit isBookmark(bm->isBookmark(index));
}
//-----------------------------------------------------------------------------
void Comic::updateBookmarkImage(int index)
{
    if (bm == nullptr) {
        return;
    }

    if (bm->isBookmark(index)) {
        QImage p;
        p.loadFromData(_pages[index]);
        bm->setBookmark(index, p);
        emit bookmarksUpdated();
        //emit bookmarksLoaded(*bm);
    }
    if (bm->getLastPage() == index) {
        QImage p;
        p.loadFromData(_pages[index]);
        bm->setLastPage(index, p);
        emit bookmarksUpdated();
        //emit bookmarksLoaded(*bm);
    }
}
//-----------------------------------------------------------------------------
void Comic::setPageLoaded(int page)
{
    _loadedPages[page] = true;
}

void Comic::invalidate()
{
    _invalidated = true;
    emit invalidated();
}
//-----------------------------------------------------------------------------
QByteArray Comic::getRawPage(int page)
{
    if (page < 0 || page >= _pages.size()) {
        return QByteArray();
    }
    return _pages[page];
}
//-----------------------------------------------------------------------------
bool Comic::pageIsLoaded(int page)
{
    if (page < 0 || page >= _pages.size()) {
        return false;
    }
    return _loadedPages[page];
}

bool Comic::hasBeenAnErrorOpening()
{
    return _errorOpening;
}

bool Comic::fileIsComic(const QString &path)
{
    QFileInfo info(path);
    return literalComicExtensions.contains(info.suffix());
}

QList<QString> Comic::findValidComicFiles(const QList<QUrl> &list)
{
    QLOG_DEBUG() << "-findValidComicFiles-";
    QList<QString> validComicFiles;
    QString currentPath;
    foreach (QUrl url, list) {
        currentPath = url.toLocalFile();
        if (Comic::fileIsComic(currentPath)) {
            validComicFiles << currentPath;
        } else if (QFileInfo(currentPath).isDir()) {
            validComicFiles << findValidComicFilesInFolder(currentPath);
        }
    }
    QLOG_DEBUG() << "-" << validComicFiles << "-";
    return validComicFiles;
}

QList<QString> Comic::findValidComicFilesInFolder(const QString &path)
{
    QLOG_DEBUG() << "-findValidComicFilesInFolder-" << path;

    if (!QFileInfo(path).isDir())
        return QList<QString>();

    QList<QString> validComicFiles;
    QDir folder(path);
    folder.setNameFilters(Comic::comicExtensions);
    folder.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    QFileInfoList folderContent = folder.entryInfoList();

    QString currentPath;
    foreach (QFileInfo info, folderContent) {
        currentPath = info.absoluteFilePath();
        if (info.isDir()) {
            validComicFiles << findValidComicFilesInFolder(currentPath); //find comics recursively
        } else if (Comic::fileIsComic(currentPath)) {
            validComicFiles << currentPath;
        }
    }
    return validComicFiles;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FileComic::FileComic()
    : Comic()
{
}

FileComic::FileComic(const QString &path, int atPage)
    : Comic(path, atPage)
{
    load(path, atPage);
}

FileComic::~FileComic()
{
    _pages.clear();
    _loadedPages.clear();
    _fileNames.clear();
    _newOrder.clear();
    _order.clear();
}

bool FileComic::load(const QString &path, int atPage)
{
    QFileInfo fi(path);

    if (fi.exists()) {
        if (atPage == -1) {
            bm->newComic(path);
            emit bookmarksUpdated();
        }
        _firstPage = atPage;
        //emit bookmarksLoaded(*bm);

        _path = QDir::cleanPath(path);
        //load files size

        return true;
    } else {
        //QMessageBox::critical(NULL,tr("Not found"),tr("Comic not found")+" : " + path);
        emit errorOpening();
        return false;
    }
}

bool FileComic::load(const QString &path, const ComicDB &comic)
{
    QFileInfo fi(path);

    if (fi.exists()) {
        QList<int> bookmarkIndexes;
        bookmarkIndexes << comic.info.bookmark1 << comic.info.bookmark2 << comic.info.bookmark3;
        if (bm->load(bookmarkIndexes, comic.info.currentPage - 1)) {
            emit bookmarksUpdated();
        }
        _firstPage = comic.info.currentPage - 1;
        _path = QDir::cleanPath(path);
        return true;
    } else {
        //QMessageBox::critical(NULL,tr("Not found"),tr("Comic not found")+" : " + path);
        moveToThread(QCoreApplication::instance()->thread());
        emit errorOpening();
        return false;
    }
}

QList<QString> FileComic::filter(const QList<QString> &src)
{
    QList<QString> extensions = getSupportedImageLiteralFormats();
    QList<QString> filtered;
    bool fileAccepted = false;

    foreach (QString fileName, src) {
        fileAccepted = false;
        if (!fileName.contains("__MACOSX")) {
            foreach (QString extension, extensions) {
                if (fileName.endsWith(extension, Qt::CaseInsensitive)) {
                    fileAccepted = true;
                    break;
                }
            }
        }
        if (fileAccepted) {
            filtered.append(fileName);
        }
    }

    return filtered;
}

//DELEGATE methods
void FileComic::fileExtracted(int index, const QByteArray &rawData)
{
    /*QFile f("c:/temp/out2.txt");
	f.open(QIODevice::Append);
	QTextStream out(&f);*/
    int sortedIndex = _fileNames.indexOf(_order.at(index));
    //out << sortedIndex << " , ";
    //f.close();
    if (sortedIndex == -1) {
        return;
    }
    _pages[sortedIndex] = rawData;
    emit imageLoaded(sortedIndex);
    emit imageLoaded(sortedIndex, _pages[sortedIndex]);
}

void FileComic::crcError(int index)
{
    emit crcErrorFound(tr("CRC error on page (%1): some of the pages will not be displayed correctly").arg(index + 1));
}

//TODO: comprobar que si se produce uno de estos errores, la carga del c�mic es irrecuperable
void FileComic::unknownError(int index)
{
    Q_UNUSED(index)
    emit errorOpening(tr("Unknown error opening the file"));
    //emit errorOpening();
}

bool FileComic::isCancelled()
{
    return _invalidated;
}

//--------------------------------------

QList<QVector<quint32>> FileComic::getSections(int &sectionIndex)
{
    QVector<quint32> sortedIndexes;
    foreach (QString name, _fileNames) {
        sortedIndexes.append(_order.indexOf(name));
    }
    QList<QVector<quint32>> sections;
    quint32 previous = 0;
    sectionIndex = -1;
    int sectionCount = 0;
    QVector<quint32> section;
    int idx = 0;
    unsigned int realIdx;
    foreach (quint32 i, sortedIndexes) {

        if (_firstPage == idx) {
            sectionIndex = sectionCount;
            realIdx = i;
        }
        if (previous <= i) {
            //out << "idx : " << i << endl;
            section.append(i);
            previous = i;
        } else {
            if (sectionIndex == sectionCount) //found
            {
                if (section.indexOf(realIdx) != 0) {
                    QVector<quint32> section1;
                    QVector<quint32> section2;
                    foreach (quint32 si, section) {
                        if (si < realIdx) {
                            section1.append(si);
                        } else {
                            section2.append(si);
                        }
                    }
                    sectionIndex++;
                    sections.append(section1);
                    sections.append(section2);
                    //out << "SPLIT" << endl;
                } else {
                    sections.append(section);
                }
            } else {
                sections.append(section);
            }
            section = QVector<quint32>();
            //out << "---------------" << endl;
            section.append(i);
            //out << "idx : " << i << endl;
            previous = i;
            sectionCount++;
        }

        idx++;
    }

    if (sectionIndex == sectionCount) //found
    {
        if (section.indexOf(realIdx) != 0) {
            QVector<quint32> section1;
            QVector<quint32> section2;
            foreach (quint32 si, section) {
                if (si < realIdx) {
                    section1.append(si);
                } else {
                    section2.append(si);
                }
            }
            sectionIndex++;
            sections.append(section1);
            sections.append(section2);
            //out << "SPLIT" << endl;

        } else {
            sections.append(section);
        }
    } else {
        sections.append(section);
    }

    //out << "se han encontrado : " << sections.count() << " sectionIndex : " << sectionIndex << endl;
    return sections;
}

void FileComic::process()
{
    CompressedArchive archive(_path);
    if (!archive.toolsLoaded()) {
        moveToThread(QCoreApplication::instance()->thread());
        emit errorOpening(tr("7z not found"));
        return;
    }

    if (!archive.isValid()) {
        moveToThread(QCoreApplication::instance()->thread());
        emit errorOpening(tr("Format not supported"));
        return;
    }

    //se filtran para obtener s�lo los formatos soportados
    _order = archive.getFileNames();
    _fileNames = filter(_order);

    if (_fileNames.size() == 0) {
        //QMessageBox::critical(NULL,tr("File error"),tr("File not found or not images in file"));
        moveToThread(QCoreApplication::instance()->thread());
        emit errorOpening();
        return;
    }

    //TODO, cambiar por listas
    //_order = _fileNames;

    _pages.resize(_fileNames.size());
    _loadedPages = QVector<bool>(_fileNames.size(), false);

    emit pageChanged(0); // this indicates new comic, index=0
    emit numPages(_pages.size());
    _loaded = true;

    _cfi = 0;

    //TODO, add a setting for choosing the type of page sorting used.
    comic_pages_sort(_fileNames, YACReaderHeuristicSorting);

    if (_firstPage == -1) {
        _firstPage = bm->getLastPage();
    }

    if (_firstPage >= _pages.length()) {
        _firstPage = 0;
    }

    _index = _firstPage;
    emit(openAt(_index));

    int sectionIndex;
    QList<QVector<quint32>> sections = getSections(sectionIndex);

    for (int i = sectionIndex; i < sections.count(); i++) {
        if (_invalidated) {
            moveToThread(QCoreApplication::instance()->thread());
            return;
        }
        archive.getAllData(sections.at(i), this);
    }
    for (int i = 0; i < sectionIndex; i++) {
        if (_invalidated) {
            moveToThread(QCoreApplication::instance()->thread());
            return;
        }
        archive.getAllData(sections.at(i), this);
    }
    //archive.getAllData(QVector<quint32>(),this);
    /*
	foreach(QString name,_fileNames)
	{
		index = _order.indexOf(name);
		sortedIndex = _fileNames.indexOf(name);
		_pages[sortedIndex] = allData.at(index);
		emit imageLoaded(sortedIndex);
		emit imageLoaded(sortedIndex,_pages[sortedIndex]);
	}*/
    moveToThread(QCoreApplication::instance()->thread());
    emit imagesLoaded();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FolderComic::FolderComic()
    : Comic()
{
}

FolderComic::FolderComic(const QString &path, int atPage)
    : Comic(path, atPage)
{
    load(path, atPage);
}

FolderComic::~FolderComic()
{
}

bool FolderComic::load(const QString &path, int atPage)
{
    _path = path;
    if (atPage == -1) {
        bm->newComic(_path);
        emit bookmarksUpdated();
    }
    _firstPage = atPage;
    //emit bookmarksLoaded(*bm);
    return true;
}

void FolderComic::process()
{
    QDir d(_path);

    d.setNameFilters(getSupportedImageFormats());
    d.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    //d.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
    QFileInfoList list = d.entryInfoList();

    //don't fix double page files sorting, because the user can see how the SO sorts the files in the folder.
    std::sort(list.begin(), list.end(), naturalSortLessThanCIFileInfo);

    int nPages = list.size();
    _pages.clear();
    _pages.resize(nPages);
    _loadedPages = QVector<bool>(nPages, false);

    if (nPages == 0) {
        //TODO emitir este mensaje en otro sitio
        //QMessageBox::critical(NULL,QObject::tr("No images found"),QObject::tr("There are not images on the selected folder"));
        moveToThread(QCoreApplication::instance()->thread());
        emit errorOpening();
    } else {
        if (_firstPage == -1) {
            _firstPage = bm->getLastPage();
        }

        if (_firstPage >= _pages.length()) {
            _firstPage = 0;
        }

        _index = _firstPage;

        emit(openAt(_index));

        emit pageChanged(0); // this indicates new comic, index=0
        emit numPages(_pages.size());
        _loaded = true;

        int count = 0;
        int i = _firstPage;
        while (count < nPages) {
            if (_invalidated) {
                moveToThread(QCoreApplication::instance()->thread());
                return;
            }

            QFile f(list.at(i).absoluteFilePath());
            f.open(QIODevice::ReadOnly);
            _pages[i] = f.readAll();
            emit imageLoaded(i);
            emit imageLoaded(i, _pages[i]);
            i++;
            if (i == nPages) {
                i = 0;
            }
            count++;
        }
    }
    moveToThread(QCoreApplication::instance()->thread());
    emit imagesLoaded();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef NO_PDF

PDFComic::PDFComic()
    : Comic()
{
}

PDFComic::PDFComic(const QString &path, int atPage)
    : Comic(path, atPage)
{
    load(path, atPage);
}

PDFComic::~PDFComic()
{
}

bool PDFComic::load(const QString &path, int atPage)
{
    QFileInfo fi(path);

    if (fi.exists()) {
        _path = path;
        if (atPage == -1) {
            bm->newComic(_path);
            emit bookmarksUpdated();
        }
        _firstPage = atPage;
        //emit bookmarksLoaded(*bm);
        return true;
    } else {
        moveToThread(QCoreApplication::instance()->thread());
        emit errorOpening();
        return false;
    }
}

bool PDFComic::load(const QString &path, const ComicDB &comic)
{
    QFileInfo fi(path);

    if (fi.exists()) {
        QList<int> bookmarkIndexes;
        bookmarkIndexes << comic.info.bookmark1 << comic.info.bookmark2 << comic.info.bookmark3;
        if (bm->load(bookmarkIndexes, comic.info.currentPage - 1)) {
            emit bookmarksUpdated();
        }
        _firstPage = comic.info.currentPage - 1;
        _path = QDir::cleanPath(path);
        return true;
    } else {
        //QMessageBox::critical(NULL,tr("Not found"),tr("Comic not found")+" : " + path);
        moveToThread(QCoreApplication::instance()->thread());
        emit errorOpening();
        return false;
    }
}

void PDFComic::process()
{
#if defined Q_OS_MAC && defined USE_PDFKIT
    pdfComic = new MacOSXPDFComic();
    if (!pdfComic->openComic(_path)) {
        delete pdfComic;
        emit errorOpening();
        return;
    }
#elif defined USE_PDFIUM
    pdfComic = new PdfiumComic();
    if (!pdfComic->openComic(_path)) {
        delete pdfComic;
        emit errorOpening();
        return;
    }
#else
    pdfComic = Poppler::Document::load(_path);
    if (!pdfComic) {
        //delete pdfComic;
        //pdfComic = 0;
        moveToThread(QCoreApplication::instance()->thread());
        emit errorOpening();
        return;
    }
    if (pdfComic->isLocked()) {
        moveToThread(QCoreApplication::instance()->thread());
        emit errorOpening();
        return;
    }

    //pdfComic->setRenderHint(Poppler::Document::Antialiasing, true);
    pdfComic->setRenderHint(Poppler::Document::TextAntialiasing, true);
#endif

    int nPages = pdfComic->numPages();
    emit pageChanged(0); // this indicates new comic, index=0
    emit numPages(nPages);
    _loaded = true;
    //QMessageBox::critical(NULL,QString("%1").arg(nPages),tr("Invalid PDF file"));

    _pages.clear();
    _pages.resize(nPages);
    _loadedPages = QVector<bool>(nPages, false);

    if (_firstPage == -1) {
        _firstPage = bm->getLastPage();
    }

    if (_firstPage >= _pages.length()) {
        _firstPage = 0;
    }

    _index = _firstPage;
    emit(openAt(_index));

    //buffer index to avoid race conditions
    int buffered_index = _index;
    for (int i = buffered_index; i < nPages; i++) {
        if (_invalidated) {
            delete pdfComic;
            moveToThread(QCoreApplication::instance()->thread());
            return;
        }

        renderPage(i);
    }
    for (int i = 0; i < buffered_index; i++) {
        if (_invalidated) {
            delete pdfComic;
            moveToThread(QCoreApplication::instance()->thread());
            return;
        }
        renderPage(i);
    }

    delete pdfComic;
    moveToThread(QCoreApplication::instance()->thread());
    emit imagesLoaded();
}

void PDFComic::renderPage(int page)
{
#if defined Q_OS_MAC && defined USE_PDFKIT
    QImage img = pdfComic->getPage(page);
    if (!img.isNull()) {
#elif defined USE_PDFIUM
    QImage img = pdfComic->getPage(page);
    if (!img.isNull()) {
#else
    Poppler::Page *pdfpage = pdfComic->page(page);
    if (pdfpage) {
        QImage img = pdfpage->renderToImage(150, 150);
        delete pdfpage;
#endif
        QByteArray ba;
        QBuffer buf(&ba);
        img.save(&buf, "jpg", 96);
        _pages[page] = ba;
        emit imageLoaded(page);
        emit imageLoaded(page, _pages[page]);
    }
}

#endif //NO_PDF

Comic *FactoryComic::newComic(const QString &path)
{

    QFileInfo fi(path);
    if (fi.exists()) {
        if (fi.isFile()) {
#ifndef NO_PDF
            if (fi.suffix().compare("pdf", Qt::CaseInsensitive) == 0) {
                return new PDFComic();
            } else {
                return new FileComic();
            }
#else
            return new FileComic();
#endif
        } else {
            if (fi.isDir()) {
                return new FolderComic();
            } else {
                return NULL;
            }
        }
    } else
        return NULL;
}

bool is_double_page(const QString &pageName, const QString &commonPrefix, const int maxExpectedDoublePagesNumberLenght)
{
    if (pageName.startsWith(commonPrefix)) {
        QString substringContainingPageNumbers = pageName.mid(commonPrefix.length());
        QString pageNumbersSubString;
        for (int i = 0; i < substringContainingPageNumbers.length() && substringContainingPageNumbers.at(i).isDigit(); i++) {
            pageNumbersSubString.append(substringContainingPageNumbers.at(i));
        }
        if (pageNumbersSubString.length() < 3 || pageNumbersSubString.length() > maxExpectedDoublePagesNumberLenght || pageNumbersSubString.length() % 2 == 1) {
            return false;
        }

        int leftPageNumber = pageNumbersSubString.left(pageNumbersSubString.length() / 2).toInt();
        int rightPageNumber = pageNumbersSubString.mid(pageNumbersSubString.length() / 2).toInt();

        if (leftPageNumber == 0 || rightPageNumber == 0) {
            return false;
        }
        if ((rightPageNumber - leftPageNumber) == 1) {
            return true;
        }
    }
    return false;
}

QString get_most_common_prefix(const QList<QString> &pageNames)
{
    if (pageNames.isEmpty()) {
        return "";
    }
    QMap<QString, uint> frequency;
    int currentPrefixLenght = pageNames.at(0).split('/').last().length();
    int currentPrefixCount = 1;

    int i;
    QString previous;
    QString current;
    for (i = 1; i < pageNames.length(); i++) {
        int pos = 0;
        previous = pageNames.at(i - 1).split('/').last();
        current = pageNames.at(i).split('/').last();
        for (; pos < current.length() && previous[pos] == current[pos]; pos++)
            ;

        if (pos < currentPrefixLenght && pos > 0) {
            frequency.insert(previous.left(currentPrefixLenght), currentPrefixCount);
            currentPrefixLenght = pos;
            currentPrefixCount++;
        }
        /*
        else if(pos > currentPrefixLenght)
        {
		frequency.insert(pageNames.at(i-1).left(currentPrefixLenght), currentPrefixCount - 1);
		currentPrefixLenght = pos;
		currentPrefixCount = 2;
        }*/
        else if (pos == 0) {
            frequency.insert(previous.left(currentPrefixLenght), currentPrefixCount);
            currentPrefixLenght = current.length();
            currentPrefixCount = 1;
        } else {
            currentPrefixCount++;
        }
    }

    frequency.insert(previous.left(currentPrefixLenght), currentPrefixCount);

    uint maxFrequency = 0;
    QString common_prefix = "";
    foreach (QString key, frequency.keys()) {
        if (maxFrequency < frequency.value(key)) {
            maxFrequency = frequency.value(key);
            common_prefix = key;
        }
    }

    QRegExp allNumberRegExp("\\d+");
    if (allNumberRegExp.exactMatch(common_prefix)) {
        return "";
    }

    if (maxFrequency < pageNames.length() * 0.60) //the most common tipe of image file should a proper page, so we can asume that the common_prefix should be in, at least, the 60% of the pages
    {
        return "";
    }

    return common_prefix;
}

void get_double_pages(const QList<QString> &pageNames, QList<QString> &singlePageNames /*out*/, QList<QString> &doublePageNames /*out*/)
{
    uint maxExpectedDoublePagesNumberLenght = (int)(log10(pageNames.length()) + 1) * 2;

    QString mostCommonPrefix = get_most_common_prefix(pageNames);

    foreach (const QString &pageName, pageNames) {
        if (is_double_page(pageName.split('/').last(), mostCommonPrefix, maxExpectedDoublePagesNumberLenght)) {
            doublePageNames.append(pageName);
        } else {
            singlePageNames.append(pageName);
        }
    }
}

QList<QString> merge_pages(QList<QString> &singlePageNames, QList<QString> &doublePageNames)
{
    //NOTE: this implementation doesn't differ from std::merge using a custom comparator, but it can be easily tweaked if merging requeries an additional heuristic behaviour
    QList<QString> pageNames;

    int i = 0;
    int j = 0;

    while (i < singlePageNames.length() && j < doublePageNames.length()) {
        if (singlePageNames.at(i).compare(doublePageNames.at(j), Qt::CaseInsensitive) < 0) {
            pageNames.append(singlePageNames.at(i++));
        } else {
            pageNames.append(doublePageNames.at(j++));
        }
    }

    while (i < singlePageNames.length()) {
        pageNames.append(singlePageNames.at(i++));
    }

    while (j < doublePageNames.length()) {
        pageNames.append(doublePageNames.at(j++));
    }

    return pageNames;
}

void comic_pages_sort(QList<QString> &pageNames, YACReaderPageSortingMode sortingMode)
{
    switch (sortingMode) {
    case YACReaderNumericalSorting:
        std::sort(pageNames.begin(), pageNames.end(), naturalSortLessThanCI);
        break;

    case YACReaderHeuristicSorting: {
        std::sort(pageNames.begin(), pageNames.end(), naturalSortLessThanCI);

        QList<QString> singlePageNames;
        QList<QString> doublePageNames;

        get_double_pages(pageNames, singlePageNames, doublePageNames);

        if (doublePageNames.length() > 0) {
            pageNames = merge_pages(singlePageNames, doublePageNames);
        }
    } break;

    case YACReaderAlphabeticalSorting:
        std::sort(pageNames.begin(), pageNames.end());
        break;
    }
}
