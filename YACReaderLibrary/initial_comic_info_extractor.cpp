#include "initial_comic_info_extractor.h"

#include <QsLog.h>
#include "pdf_comic.h"
#include "comic.h"
#include "compressed_archive.h"
#include "qnaturalsorting.h"

using namespace YACReader;

bool InitialComicInfoExtractor::crash = false;

InitialComicInfoExtractor::InitialComicInfoExtractor(QString fileSource, QString target, int coverPage)
    : _fileSource(fileSource), _target(target), _numPages(0), _coverPage(coverPage), _xmlInfoData()
{
}

void InitialComicInfoExtractor::extract()
{
    QFileInfo fi(_fileSource);
    if (!fi.exists()) // TODO: error file not found.
    {
        _cover.load(":/images/notCover.png");
        QLOG_WARN() << "Extracting cover: file not found " << _fileSource;
        return;
    }
#ifndef NO_PDF
    if (fi.suffix().compare("pdf", Qt::CaseInsensitive) == 0) {
#if defined Q_OS_MAC && defined USE_PDFKIT
        MacOSXPDFComic *pdfComic = new MacOSXPDFComic();
        if (!pdfComic->openComic(_fileSource)) {
            delete pdfComic;
            // QImage p;
            // p.load(":/images/notCover.png");
            // p.save(_target);
            return;
        }
#elif defined USE_PDFIUM
        auto pdfComic = new PdfiumComic();
        if (!pdfComic->openComic(_fileSource)) {
            delete pdfComic;
            return;
        }
#else
        Poppler::Document *pdfComic = Poppler::Document::load(_fileSource);
#endif

        if (!pdfComic) {
            QLOG_WARN() << "Extracting cover: unable to open PDF file " << _fileSource;
            // delete pdfComic; //TODO check if the delete is needed
            pdfComic = 0;
            // QImage p;
            // p.load(":/images/notCover.png");
            // p.save(_target);
            return;
        }
#if !defined USE_PDFKIT && !defined USE_PDFIUM
        // poppler only, not mac
        if (pdfComic->isLocked()) {
            QLOG_WARN() << "Extracting cover: unable to open PDF file " << _fileSource;
            delete pdfComic;
            return;
        }
#endif
        _numPages = pdfComic->numPages();
        if (_numPages >= _coverPage) {
#if defined Q_OS_MAC || defined USE_PDFIUM
            QImage p = pdfComic->getPage(_coverPage - 1); // TODO check if the page is valid
#else
            QImage p = pdfComic->page(_coverPage - 1)->renderToImage(72, 72);
#endif //
            _cover = p;
            _coverSize = QPair<int, int>(p.width(), p.height());
            if (_target != "") {
                saveCover(_target, p);
            } else if (_target != "") {
                QLOG_WARN() << "Extracting cover: requested cover index greater than numPages " << _fileSource;
                // QImage p;
                // p.load(":/images/notCover.png");
                // p.save(_target);
            }
            delete pdfComic;
        }
        return;
    }
#endif // NO_PDF

    if (crash) {
        return;
    }

    CompressedArchive archive(_fileSource);
    if (!archive.toolsLoaded()) {
        QLOG_WARN() << "Extracting cover: 7z lib not loaded";
        crash = true;
        return;
    }
    if (!archive.isValid()) {
        QLOG_WARN() << "Extracting cover: file format not supported " << _fileSource;
    }

    QList<QString> order = archive.getFileNames();

    // Try to find embeded XML info (ComicRack or ComicTagger)

    auto infoIndex = 0;
    for (auto &fileName : order) {
        if (fileName.endsWith(".xml", Qt::CaseInsensitive)) {
            _xmlInfoData = archive.getRawDataAtIndex(infoIndex);
            break;
        }

        infoIndex++;
    }

    //--------------------------

    if (_target == "None") {
        return;
    }

    // se filtran para obtener sólo los formatos soportados
    QList<QString> fileNames = FileComic::filter(order);
    _numPages = fileNames.size();
    if (_numPages == 0) {
        QLOG_WARN() << "Extracting cover: empty comic " << _fileSource;
        _cover.load(":/images/notCover.png");
        if (_target != "") {
            _cover.save(_target);
        }
    } else {
        if (_coverPage > _numPages) {
            _coverPage = 1;
        }
        std::sort(fileNames.begin(), fileNames.end(), naturalSortLessThanCI);
        int index = order.indexOf(fileNames.at(_coverPage - 1));

        if (_target == "") {
            if (!_cover.loadFromData(archive.getRawDataAtIndex(index))) {
                QLOG_WARN() << "Extracting cover: unable to load image from extracted cover " << _fileSource;
                _cover.load(":/images/notCover.png");
            }
        } else {
            QImage p;
            if (p.loadFromData(archive.getRawDataAtIndex(index))) {
                _coverSize = QPair<int, int>(p.width(), p.height());
                saveCover(_target, p);
            } else {
                QLOG_WARN() << "Extracting cover: unable to load image from extracted cover " << _fileSource;
                // p.load(":/images/notCover.png");
                // p.save(_target);
            }
        }
    }
}

QByteArray InitialComicInfoExtractor::getXMLInfoRawData()
{
    return _xmlInfoData;
}

void InitialComicInfoExtractor::saveCover(const QString &path, const QImage &cover)
{
    QImage scaled;
    if (cover.width() > cover.height()) {
        scaled = cover.scaledToWidth(640, Qt::SmoothTransformation);
    } else {
        auto aspectRatio = static_cast<double>(cover.width()) / static_cast<double>(cover.height());
        auto maxAllowedAspectRatio = 0.5;
        if (aspectRatio < maxAllowedAspectRatio) { // cover is too tall, e.g. webtoon
            scaled = cover.scaledToHeight(960, Qt::SmoothTransformation);
        } else {
            scaled = cover.scaledToWidth(480, Qt::SmoothTransformation);
        }
    }
    scaled.save(_target, 0, 75);
}
