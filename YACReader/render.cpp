#include "render.h"
#include <cmath>
#include <QList>
#include <algorithm>
#include <QByteArray>
#include <QPixmap>
#include <QApplication>
#include <QImage>

#include <typeinfo>

#include "comic_db.h"
#include "yacreader_global_gui.h"
#include "configuration.h"

template<class T>
inline const T &kClamp(const T &x, const T &low, const T &high)
{
    if (x < low)
        return low;
    else if (high < x)
        return high;
    else
        return x;
}

inline int changeBrightness(int value, int brightness)
{
    return kClamp(value + brightness * 255 / 100, 0, 255);
}

inline int changeContrast(int value, int contrast)
{
    return kClamp(((value - 127) * contrast / 100) + 127, 0, 255);
}

inline int changeGamma(int value, int gamma)
{
    return kClamp(int(pow(value / 255.0, 100.0 / gamma) * 255), 0, 255);
}

inline int changeUsingTable(int value, const int table[])
{
    return table[value];
}

template<int operation(int, int)>
static QImage changeImage(const QImage &image, int value)
{
    QImage im = image;
    im.detach();
    if (im.colorCount() == 0) /* truecolor */
    {
        if (im.format() != QImage::Format_RGB32) /* just in case */
            im = im.convertToFormat(QImage::Format_RGB32);
        int table[256];
        for (int i = 0;
             i < 256;
             ++i)
            table[i] = operation(i, value);
        if (im.hasAlphaChannel()) {
            for (int y = 0;
                 y < im.height();
                 ++y) {
                QRgb *line = reinterpret_cast<QRgb *>(im.scanLine(y));
                for (int x = 0;
                     x < im.width();
                     ++x)
                    line[x] = qRgba(changeUsingTable(qRed(line[x]), table),
                                    changeUsingTable(qGreen(line[x]), table),
                                    changeUsingTable(qBlue(line[x]), table),
                                    changeUsingTable(qAlpha(line[x]), table));
            }
        } else {
            for (int y = 0;
                 y < im.height();
                 ++y) {
                QRgb *line = reinterpret_cast<QRgb *>(im.scanLine(y));
                for (int x = 0;
                     x < im.width();
                     ++x)
                    line[x] = qRgb(changeUsingTable(qRed(line[x]), table),
                                   changeUsingTable(qGreen(line[x]), table),
                                   changeUsingTable(qBlue(line[x]), table));
            }
        }
    } else {
        QVector<QRgb> colors = im.colorTable();
        for (int i = 0;
             i < im.colorCount();
             ++i)
            colors[i] = qRgb(operation(qRed(colors[i]), value),
                             operation(qGreen(colors[i]), value),
                             operation(qBlue(colors[i]), value));
        im.setColorTable(colors);
    }
    return im;
}

// brightness is multiplied by 100 in order to avoid floating point numbers
QImage changeBrightness(const QImage &image, int brightness)
{
    if (brightness == 0) // no change
        return image;
    return changeImage<changeBrightness>(image, brightness);
}

// contrast is multiplied by 100 in order to avoid floating point numbers
QImage changeContrast(const QImage &image, int contrast)
{
    if (contrast == 100) // no change
        return image;
    return changeImage<changeContrast>(image, contrast);
}

// gamma is multiplied by 100 in order to avoid floating point numbers
QImage changeGamma(const QImage &image, int gamma)
{
    if (gamma == 100) // no change
        return image;
    return changeImage<changeGamma>(image, gamma);
}

//-----------------------------------------------------------------------------
// MeanNoiseReductionFilter
//-----------------------------------------------------------------------------

MeanNoiseReductionFilter::MeanNoiseReductionFilter(enum NeighborghoodSize ns)
    : neighborghoodSize(ns)
{
}

QImage MeanNoiseReductionFilter::setFilter(const QImage &image)
{
    int width = image.width();
    int height = image.height();
    QImage result(width, height, image.format());
    int filterSize = sqrt((float)neighborghoodSize);
    int bound = filterSize / 2;
    QRgb pix;
    int r, g, b;
    for (int j = bound; j < height - bound; j++) {
        for (int i = bound; i < width - bound; i++) {
            r = g = b = 0;
            for (int y = j - bound; y <= j + bound; y++) {
                for (int x = i - bound; x <= i + bound; x++) {
                    pix = image.pixel(x, y);
                    r += qRed(pix);
                    g += qGreen(pix);
                    b += qBlue(pix);
                }
            }
            result.setPixel(i, j, QColor(r / neighborghoodSize, g / neighborghoodSize, b / neighborghoodSize).rgb());
            //qDebug((QString::number(redChannel.at(4))+" "+QString::number(greenChannel.at(4))+" "+QString::number(blueChannel.at(4))).toAscii());
            //qDebug((QString::number(redChannel.size())+" "+QString::number(greenChannel.size())+" "+QString::number(blueChannel.size())).toAscii());
        }
    }
    return result;
}

//-----------------------------------------------------------------------------
// MedianNoiseReductionFilter
//-----------------------------------------------------------------------------

MedianNoiseReductionFilter::MedianNoiseReductionFilter(enum NeighborghoodSize ns)
    : neighborghoodSize(ns)
{
}

QImage MedianNoiseReductionFilter::setFilter(const QImage &image)
{
    int width = image.width();
    int height = image.height();
    QImage result(width, height, image.format());
    int filterSize = sqrt((float)neighborghoodSize);
    int bound = filterSize / 2;
    QRgb pix;
    QList<int> redChannel;
    QList<int> greenChannel;
    QList<int> blueChannel;
    for (int j = bound; j < height - bound; j++) {
        for (int i = bound; i < width - bound; i++) {
            redChannel.clear();
            greenChannel.clear();
            blueChannel.clear();
            for (int y = j - bound; y <= j + bound; y++) {
                for (int x = i - bound; x <= i + bound; x++) {
                    pix = image.pixel(x, y);
                    redChannel.push_back(qRed(pix));
                    greenChannel.push_back(qGreen(pix));
                    blueChannel.push_back(qBlue(pix));
                }
            }

            std::sort(redChannel.begin(), redChannel.end());
            std::sort(greenChannel.begin(), greenChannel.end());
            std::sort(blueChannel.begin(), blueChannel.end());
            result.setPixel(i, j, QColor(redChannel.at(4), greenChannel.at(4), blueChannel.at(4)).rgb());
        }
    }
    return result;
}

//-----------------------------------------------------------------------------
// BrightnessFilter
//-----------------------------------------------------------------------------
BrightnessFilter::BrightnessFilter(int l)
    : ImageFilter()
{
    level = l;
}

QImage BrightnessFilter::setFilter(const QImage &image)
{
    /*int width = image.width();
	int height = image.height();
	QImage result(width,height,image.format());

	for(int j=0;j<height;j++){
		for(int i=0;i<width;i++){
			result.setPixel(i,j,QColor(image.pixel(i,j)).light(level).rgb());
		}
	}
	return result;*/
    if (level == -1) {
        QSettings settings(YACReader::getSettingsPath() + "/YACReader.ini", QSettings::IniFormat);
        return changeBrightness(image, settings.value(BRIGHTNESS, 0).toInt());
    } else {
        return changeBrightness(image, level);
    }
}

//-----------------------------------------------------------------------------
// ContrastFilter
//-----------------------------------------------------------------------------
ContrastFilter::ContrastFilter(int l)
    : ImageFilter()
{
    level = l;
}

QImage ContrastFilter::setFilter(const QImage &image)
{
    /*int width = image.width();
	int height = image.height();
	QImage result(width,height,image.format());

	int min,max,v;
	min = 0;
	max = 255;
	int sum = 0;

	QVector<int> hist(256,0);

	for(int j=0;j<height;j++){
		for(int i=0;i<width;i++){
			hist[QColor(image.pixel(i,j)).lightness()]++;
			sum++;
		}
	}

	long double count = sum;
	long double new_count = 0.0;
	long double percentage,next_percentage;

	for (int i = 0; i < 254; i++)
	{
		new_count += hist[i];
		percentage = new_count/count;
		next_percentage = (new_count+hist[i+1])/count;
		if(fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
		{
			min = i+1;
			break;
		}
	}

	new_count=0.0;
	for (int i = 255; i > 1; i--)
	{
		new_count += hist[i];
		percentage = new_count/count;
		next_percentage = (new_count+hist[i-1])/count;
		if(fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
		{
			max = i-1;
			break;
		}
	}
	QColor c;
	int range = max - min;
	for(int j=0;j<height;j++){
		for(int i=0;i<width;i++){
			c = QColor(image.pixel(i,j));
			result.setPixel(i,j,c.light(((c.lightness()-min)/range*1.0)*255).rgb());
		}
	}

	return result;*/
    if (level == -1) {
        QSettings settings(YACReader::getSettingsPath() + "/YACReader.ini", QSettings::IniFormat);
        return changeContrast(image, settings.value(CONTRAST, 100).toInt());
    } else {
        return changeContrast(image, level);
    }
}
//-----------------------------------------------------------------------------
// ContrastFilter
//-----------------------------------------------------------------------------
GammaFilter::GammaFilter(int l)
    : ImageFilter()
{
    level = l;
}

QImage GammaFilter::setFilter(const QImage &image)
{
    if (level == -1) {
        QSettings settings(YACReader::getSettingsPath() + "/YACReader.ini", QSettings::IniFormat);
        return changeGamma(image, settings.value(GAMMA, 100).toInt());
    } else {
        return changeGamma(image, level);
    }
}

//-----------------------------------------------------------------------------
// PageRender
//-----------------------------------------------------------------------------
PageRender::PageRender()
    : QThread()
{
}
PageRender::PageRender(Render *r, int np, const QByteArray &rd, QImage *p, unsigned int d, QVector<ImageFilter *> f)
    : QThread(),
      numPage(np),
      data(rd),
      page(p),
      degrees(d),
      filters(f),
      render(r)
{
}

void PageRender::run()
{
    QMutexLocker locker(&(render->mutex));

    QImage img;
    img.loadFromData(data);
    if (degrees > 0) {
        QMatrix m;
        m.rotate(degrees);
        img = img.transformed(m, Qt::SmoothTransformation);
    }
    for (int i = 0; i < filters.size(); i++) {
        img = filters[i]->setFilter(img);
    }

    *page = img;

    emit pageReady(numPage);
}

//-----------------------------------------------------------------------------
// Render
//-----------------------------------------------------------------------------

Render::Render()
    : comic(nullptr), doublePage(false), doubleMangaPage(false), currentIndex(0), numLeftPages(4), numRightPages(4), loadedComic(false), imageRotation(0)
{
    int size = numLeftPages + numRightPages + 1;
    currentPageBufferedIndex = numLeftPages;
    for (int i = 0; i < size; i++) {
        buffer.push_back(new QImage());
        pageRenders.push_back(0);
    }

    filters.push_back(new BrightnessFilter());
    filters.push_back(new ContrastFilter());
    filters.push_back(new GammaFilter());
}

Render::~Render()
{
    if (comic != nullptr) {
        comic->moveToThread(QApplication::instance()->thread());
        comic->deleteLater();
    }

    foreach (PageRender *pr, pageRenders)
        if (pr != nullptr) {
            if (pr->wait())
                delete pr;
        }

    //TODO move to share_ptr
    foreach (ImageFilter *filter, filters)
        delete filter;
}
//Este método se encarga de forzar el renderizado de las páginas.
//Actualiza el buffer según es necesario.
//si la pagina actual no está renderizada, se lanza un hilo que la renderize (double or single page mode) y se emite una señal que indica que se está renderizando.
void Render::render()
{
    updateBuffer();
    if (buffer[currentPageBufferedIndex]->isNull()) {
        if (pagesReady.size() > 0) {
            if (pagesReady[currentIndex]) {
                pageRenders[currentPageBufferedIndex] = new PageRender(this, currentIndex, comic->getRawData()->at(currentIndex), buffer[currentPageBufferedIndex], imageRotation, filters);
            } else
                //las páginas no están listas, y se están cargando en el cómic
                emit processingPage(); //para evitar confusiones esta señal debería llamarse de otra forma

            //si se ha creado un hilo para renderizar la página actual, se arranca
            if (pageRenders[currentPageBufferedIndex] != 0) {
                //se conecta la señal pageReady del hilo, con el SLOT prepareAvailablePage
                connect(pageRenders[currentPageBufferedIndex], SIGNAL(pageReady(int)), this, SLOT(prepareAvailablePage(int)));
                //se emite la señal de procesando, debido a que los hilos se arrancan aquí
                if (filters.size() > 0)
                    emit processingPage();
                pageRenders[currentPageBufferedIndex]->start();
                pageRenders[currentPageBufferedIndex]->setPriority(QThread::TimeCriticalPriority);
            } else
                //en qué caso sería necesario hacer esto??? //TODO: IMPORTANTE, puede que no sea necesario.
                emit processingPage();
        } else
            //no hay ninguna página lista para ser renderizada, es necesario esperar.
            emit processingPage();
    } else
    // la página actual está lista
    {
        //emit currentPageReady();
        //make prepareAvailablePage the only function that emits currentPageReady()
        prepareAvailablePage(currentIndex);
    }
    fillBuffer();
}

QPixmap *Render::getCurrentPage()
{
    auto page = new QPixmap();
    *page = page->fromImage(*buffer[currentPageBufferedIndex]);
    return page;
}

QPixmap *Render::getCurrentDoublePage()
{
    if (currentPageIsDoublePage()) {
        QPoint leftpage(0, 0);
        QPoint rightpage(0, 0);
        QSize leftsize = buffer[currentPageBufferedIndex]->size();
        QSize rightsize = buffer[currentPageBufferedIndex + 1]->size();
        int totalWidth, totalHeight;
        switch (imageRotation) {
        case 0:
            totalHeight = qMax(leftsize.rheight(), rightsize.rheight());
            leftsize.scale(leftsize.rwidth(), totalHeight, Qt::KeepAspectRatioByExpanding);
            rightsize.scale(rightsize.rwidth(), totalHeight, Qt::KeepAspectRatioByExpanding);
            totalWidth = leftsize.rwidth() + rightsize.rwidth();
            rightpage.setX(leftsize.rwidth());
            break;
        case 90:
            totalWidth = qMax(leftsize.rwidth(), rightsize.rwidth());
            leftsize.scale(totalWidth, leftsize.rheight(), Qt::KeepAspectRatioByExpanding);
            rightsize.scale(totalWidth, rightsize.rheight(), Qt::KeepAspectRatioByExpanding);
            totalHeight = leftsize.rheight() + rightsize.rheight();
            rightpage.setY(leftsize.rheight());
            break;
        case 180:
            totalHeight = qMax(leftsize.rheight(), rightsize.rheight());
            leftsize.scale(leftsize.rwidth(), totalHeight, Qt::KeepAspectRatioByExpanding);
            rightsize.scale(rightsize.rwidth(), totalHeight, Qt::KeepAspectRatioByExpanding);
            totalWidth = leftsize.rwidth() + rightsize.rwidth();
            leftpage.setX(rightsize.rwidth());
            break;
        case 270:
            totalWidth = qMax(leftsize.rwidth(), rightsize.rwidth());
            leftsize.scale(totalWidth, leftsize.rheight(), Qt::KeepAspectRatioByExpanding);
            rightsize.scale(totalWidth, rightsize.rheight(), Qt::KeepAspectRatioByExpanding);
            totalHeight = leftsize.rheight() + rightsize.rheight();
            leftpage.setY(rightsize.rheight());
            break;
        default:
            return nullptr;
        }
        auto page = new QPixmap(totalWidth, totalHeight);
        QPainter painter(page);
        painter.drawImage(QRect(leftpage, leftsize), *buffer[currentPageBufferedIndex]);
        painter.drawImage(QRect(rightpage, rightsize), *buffer[currentPageBufferedIndex + 1]);
        return page;
    } else {
        return nullptr;
    }
}

QPixmap *Render::getCurrentDoubleMangaPage()
{
    if (currentPageIsDoublePage()) {
        QPoint leftpage(0, 0);
        QPoint rightpage(0, 0);
        QSize leftsize = buffer[currentPageBufferedIndex + 1]->size();
        QSize rightsize = buffer[currentPageBufferedIndex]->size();
        int totalWidth, totalHeight;
        switch (imageRotation) {
        case 0:
            totalHeight = qMax(leftsize.rheight(), rightsize.rheight());
            leftsize.scale(leftsize.rwidth(), totalHeight, Qt::KeepAspectRatioByExpanding);
            rightsize.scale(rightsize.rwidth(), totalHeight, Qt::KeepAspectRatioByExpanding);
            totalWidth = leftsize.rwidth() + rightsize.rwidth();
            rightpage.setX(leftsize.rwidth());
            break;
        case 90:
            totalWidth = qMax(leftsize.rwidth(), rightsize.rwidth());
            leftsize.scale(totalWidth, leftsize.rheight(), Qt::KeepAspectRatioByExpanding);
            rightsize.scale(totalWidth, rightsize.rheight(), Qt::KeepAspectRatioByExpanding);
            totalHeight = leftsize.rheight() + rightsize.rheight();
            rightpage.setY(leftsize.rheight());
            break;
        case 180:
            totalHeight = qMax(leftsize.rheight(), rightsize.rheight());
            leftsize.scale(leftsize.rwidth(), totalHeight, Qt::KeepAspectRatioByExpanding);
            rightsize.scale(rightsize.rwidth(), totalHeight, Qt::KeepAspectRatioByExpanding);
            totalWidth = leftsize.rwidth() + rightsize.rwidth();
            leftpage.setX(rightsize.rwidth());
            break;
        case 270:
            totalWidth = qMax(leftsize.rwidth(), rightsize.rwidth());
            leftsize.scale(totalWidth, leftsize.rheight(), Qt::KeepAspectRatioByExpanding);
            rightsize.scale(totalWidth, rightsize.rheight(), Qt::KeepAspectRatioByExpanding);
            totalHeight = leftsize.rheight() + rightsize.rheight();
            leftpage.setY(rightsize.rheight());
            break;
        default:
            return nullptr;
        }
        auto page = new QPixmap(totalWidth, totalHeight);
        QPainter painter(page);
        painter.drawImage(QRect(rightpage, rightsize), *buffer[currentPageBufferedIndex]);
        painter.drawImage(QRect(leftpage, leftsize), *buffer[currentPageBufferedIndex + 1]);
        return page;
    } else {
        return nullptr;
    }
}

bool Render::currentPageIsDoublePage()
{
    if (currentIndex == 0 && Configuration::getConfiguration().getSettings()->value(COVER_IS_SP, true).toBool()) {
        return false;
    }
    if (buffer[currentPageBufferedIndex]->isNull() || buffer[currentPageBufferedIndex + 1]->isNull()) {
        return false;
    }
    if (imageRotation == 0 || imageRotation == 180) {
        if (buffer[currentPageBufferedIndex]->height() > buffer[currentPageBufferedIndex]->width() &&
            buffer[currentPageBufferedIndex + 1]->height() > buffer[currentPageBufferedIndex + 1]->width()) {
            return true;
        }
    } else if (imageRotation == 90 || imageRotation == 270) {
        if (buffer[currentPageBufferedIndex]->width() > buffer[currentPageBufferedIndex]->height() &&
            buffer[currentPageBufferedIndex + 1]->width() > buffer[currentPageBufferedIndex + 1]->height()) {
            return true;
        }
    }
    return false;
}

bool Render::nextPageIsDoublePage()
{
    //this function is not used right now
    if (buffer[currentPageBufferedIndex + 2]->isNull() || buffer[currentPageBufferedIndex + 3]->isNull()) {
        return false;
    }
    if (imageRotation == 0 || imageRotation == 180) {
        if (buffer[currentPageBufferedIndex + 2]->height() > buffer[currentPageBufferedIndex + 2]->width() &&
            buffer[currentPageBufferedIndex + 3]->height() > buffer[currentPageBufferedIndex + 3]->width()) {
            return true;
        }
    } else if (imageRotation == 90 || imageRotation == 270) {
        if (buffer[currentPageBufferedIndex]->width() > buffer[currentPageBufferedIndex]->height() &&
            buffer[currentPageBufferedIndex + 1]->width() > buffer[currentPageBufferedIndex + 1]->height()) {
            return true;
        }
    }
    return false;
}

bool Render::previousPageIsDoublePage()
{
    qWarning("Previous page is doublepage!");
    qWarning("%d", currentIndex);
    qWarning("%d", currentPageBufferedIndex);
    if (currentIndex == 2 && Configuration::getConfiguration().getSettings()->value(COVER_IS_SP, true).toBool()) {
        return false;
    }
    if (buffer[currentPageBufferedIndex - 1]->isNull() || buffer[currentPageBufferedIndex - 2]->isNull()) {
        return false;
    }
    if (imageRotation == 0 || imageRotation == 180) {
        if (buffer[currentPageBufferedIndex - 1]->height() > buffer[currentPageBufferedIndex - 1]->width() &&
            buffer[currentPageBufferedIndex - 2]->height() > buffer[currentPageBufferedIndex - 2]->width()) {
            return true;
        }
    } else if (imageRotation == 90 || imageRotation == 270) {
        if (buffer[currentPageBufferedIndex - 1]->width() > buffer[currentPageBufferedIndex - 1]->height() &&
            buffer[currentPageBufferedIndex - 2]->width() > buffer[currentPageBufferedIndex - 2]->height()) {
            return true;
        }
    }
    return false;
}

void Render::setRotation(int degrees)
{
    Q_UNUSED(degrees)
}

void Render::setComic(Comic *c)
{
    if (comic != nullptr) {
        comic->moveToThread(QApplication::instance()->thread());
        comic->disconnect();
        comic->deleteLater();
    }
    comic = c;
}

void Render::prepareAvailablePage(int page)
{
    if (!doublePage) {
        if (currentIndex == page) {
            emit currentPageReady();
        }
    } else {
        //check for last page in double page mode
        if ((currentIndex == page) && (currentIndex + 1) >= (int)comic->numPages()) {
            emit currentPageReady();
        } else if ((currentIndex == page && !buffer[currentPageBufferedIndex + 1]->isNull()) ||
                   (currentIndex + 1 == page && !buffer[currentPageBufferedIndex]->isNull())) {
            emit currentPageReady();
        }
    }
}

void Render::update()
{
    render();
}
//-----------------------------------------------------------------------------
// Comic interface
//-----------------------------------------------------------------------------
void Render::load(const QString &path, int atPage)
{
    createComic(path);
    if (comic != nullptr) {
        loadComic(path, atPage);
        startLoad();
    }
}

//-----------------------------------------------------------------------------
void Render::load(const QString &path, const ComicDB &comicDB)
{
    //TODO prepare filters
    for (int i = 0; i < filters.count(); i++) {
        if (typeid(*filters[i]) == typeid(BrightnessFilter)) {
            if (comicDB.info.brightness == -1)
                filters[i]->setLevel(0);
            else
                filters[i]->setLevel(comicDB.info.brightness);
        }
        if (typeid(*filters[i]) == typeid(ContrastFilter)) {
            if (comicDB.info.contrast == -1)
                filters[i]->setLevel(100);
            else
                filters[i]->setLevel(comicDB.info.contrast);
        }
        if (typeid(*filters[i]) == typeid(GammaFilter)) {
            if (comicDB.info.gamma == -1)
                filters[i]->setLevel(100);
            else
                filters[i]->setLevel(comicDB.info.gamma);
        }
    }
    createComic(path);
    if (comic != nullptr) {
        loadComic(path, comicDB);
        startLoad();
    }
}

void Render::createComic(const QString &path)
{
    previousIndex = currentIndex = 0;
    pagesEmited.clear();

    if (comic != nullptr) {
        //comic->moveToThread(QApplication::instance()->thread());
        comic->invalidate();

        comic->disconnect();
        comic->deleteLater();
    }
    //comic->moveToThread(QApplication::instance()->thread());
    comic = FactoryComic::newComic(path);

    if (comic == nullptr) //archivo no encontrado o no válido
    {
        emit errorOpening();
        reset();
        return;
    }

    connect(comic, SIGNAL(errorOpening()), this, SIGNAL(errorOpening()), Qt::QueuedConnection);
    connect(comic, SIGNAL(errorOpening(QString)), this, SIGNAL(errorOpening(QString)), Qt::QueuedConnection);
    connect(comic, SIGNAL(crcErrorFound(QString)), this, SIGNAL(crcError(QString)), Qt::QueuedConnection);
    connect(comic, SIGNAL(errorOpening()), this, SLOT(reset()), Qt::QueuedConnection);
    connect(comic, SIGNAL(imageLoaded(int)), this, SLOT(pageRawDataReady(int)), Qt::QueuedConnection);
    connect(comic, SIGNAL(imageLoaded(int)), this, SIGNAL(imageLoaded(int)), Qt::QueuedConnection);
    connect(comic, SIGNAL(openAt(int)), this, SLOT(renderAt(int)), Qt::QueuedConnection);
    connect(comic, SIGNAL(numPages(unsigned int)), this, SIGNAL(numPages(unsigned int)), Qt::QueuedConnection);
    connect(comic, SIGNAL(numPages(unsigned int)), this, SLOT(setNumPages(unsigned int)), Qt::QueuedConnection);
    connect(comic, SIGNAL(imageLoaded(int, QByteArray)), this, SIGNAL(imageLoaded(int, QByteArray)), Qt::QueuedConnection);
    connect(comic, SIGNAL(isBookmark(bool)), this, SIGNAL(currentPageIsBookmark(bool)), Qt::QueuedConnection);

    connect(comic, SIGNAL(bookmarksUpdated()), this, SIGNAL(bookmarksUpdated()), Qt::QueuedConnection);

    //connect(comic,SIGNAL(isLast()),this,SIGNAL(isLast()));
    //connect(comic,SIGNAL(isCover()),this,SIGNAL(isCover()));

    pagesReady.clear();
}
void Render::loadComic(const QString &path, const ComicDB &comicDB)
{
    comic->load(path, comicDB);
}
void Render::loadComic(const QString &path, int atPage)
{
    comic->load(path, atPage);
}

void Render::startLoad()
{
    QThread *thread = nullptr;

    thread = new QThread();

    comic->moveToThread(thread);

    connect(comic, SIGNAL(errorOpening()), thread, SLOT(quit()), Qt::QueuedConnection);
    connect(comic, SIGNAL(errorOpening(QString)), thread, SLOT(quit()), Qt::QueuedConnection);
    connect(comic, SIGNAL(imagesLoaded()), thread, SLOT(quit()), Qt::QueuedConnection);
    connect(comic, SIGNAL(destroyed()), thread, SLOT(quit()), Qt::QueuedConnection);
    connect(comic, SIGNAL(invalidated()), thread, SLOT(quit()), Qt::QueuedConnection);
    connect(thread, SIGNAL(started()), comic, SLOT(process()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    if (thread != nullptr)
        thread->start();

    invalidate();
    loadedComic = true;
    update();
}

void Render::renderAt(int page)
{
    previousIndex = currentIndex = page;
    emit pageChanged(page);
}

void Render::reset()
{
    loadedComic = false;
    invalidate();
}
//si se solicita la siguiente página, se calcula cuál debe ser en función de si se lee en modo a doble página o no.
//la página sólo se renderiza, si realmente ha cambiado.
void Render::nextPage()
{
    int nextPage; //indica cuál será la próxima página
    nextPage = comic->nextPage();
    //se fuerza renderizado si la página ha cambiado
    if (currentIndex != nextPage) {
        previousIndex = currentIndex;
        currentIndex = nextPage;
        update();
        emit pageChanged(currentIndex);
    } else if (hasLoadedComic() && ((unsigned int)currentIndex == numPages() - 1)) {
        emit isLast();
    }
}
void Render::nextDoublePage()
{
    int nextPage;
    if (currentIndex + 2 < (int)comic->numPages()) {
        nextPage = currentIndex + 2;
    } else {
        nextPage = currentIndex;
    }
    if (currentIndex != nextPage) {
        comic->setIndex(nextPage);
        previousIndex = currentIndex;
        currentIndex = nextPage;
        update();
        emit pageChanged(currentIndex);
    } else if (hasLoadedComic() && ((unsigned int)currentIndex >= numPages() - 2)) {
        emit isLast();
    }
}

//si se solicita la página anterior, se calcula cuál debe ser en función de si se lee en modo a doble página o no.
//la página sólo se renderiza, si realmente ha cambiado.
void Render::previousPage()
{
    int previousPage; //indica cuál será la próxima página
    previousPage = comic->previousPage();

    //se fuerza renderizado si la página ha cambiado
    if (currentIndex != previousPage) {
        previousIndex = currentIndex;
        currentIndex = previousPage;
        update();
        emit pageChanged(currentIndex);
    } else if (hasLoadedComic() && (currentIndex == 0)) {
        emit isCover();
    }
}

void Render::previousDoublePage()
{
    int previousPage; //indica cuál será la próxima página
    previousPage = qMax(currentIndex - 2, 0);
    if (currentIndex != previousPage) {
        comic->setIndex(previousPage);
        previousIndex = currentIndex;
        currentIndex = previousPage;
        update();
        emit pageChanged(currentIndex);
    }
}

unsigned int Render::getIndex()
{
    return comic->getIndex();
}
unsigned int Render::numPages()
{
    return comic->numPages();
}

bool Render::hasLoadedComic()
{
    if (comic != nullptr)
        return comic->loaded();
    return false;
}

void Render::setNumPages(unsigned int numPages)
{
    pagesReady.fill(false, numPages);
}

void Render::pageRawDataReady(int page)
{
    if (!hasLoadedComic())
        return;

    pagesEmited.push_back(page);
    if (pageRenders.size() > 0) {
        for (int i = 0; i < pagesEmited.size(); i++) {
            if (pagesEmited.at(i) >= pagesReady.size()) {
                pagesEmited.clear();
                return; //Oooops, something went wrong
            }

            pagesReady[pagesEmited.at(i)] = true;
            if (pagesEmited.at(i) == currentIndex)
                update();
            else {
                if (((pagesEmited.at(i) < currentIndex) && (pagesEmited.at(i) > currentIndex - numLeftPages)) ||
                    ((pagesEmited.at(i) > currentIndex) && (pagesEmited.at(i) < currentIndex + numRightPages))) {
                    fillBuffer();
                }
            }
        }
        pagesEmited.clear();
    }
}

//sólo se renderiza la página, si ha habido un cambio de página
void Render::goTo(int index)
{

    if (currentIndex != index) {
        comic->setIndex(index);
        previousIndex = currentIndex;
        currentIndex = index;
        update();
        emit pageChanged(currentIndex);
    }
}

void Render::rotateRight()
{
    imageRotation = (imageRotation + 90) % 360;
    reload();
}
void Render::rotateLeft()
{
    if (imageRotation == 0)
        imageRotation = 270;
    else
        imageRotation = imageRotation - 90;
    reload();
}

//Actualiza el buffer, añadiendo las imágenes (vacías) necesarias para su posterior renderizado y
//eliminado aquellas que ya no sean necesarias. También libera los hilos (no estoy seguro de que sea responsabilidad suya)
//Calcula el número de nuevas páginas que hay que buferear y si debe hacerlo por la izquierda o la derecha (según sea el sentido de la lectura)
void Render::updateBuffer()
{
    QMutexLocker locker(&mutex);
    int windowSize = currentIndex - previousIndex;

    if (windowSize > 0) //add pages to right pages and remove on the left
    {
        windowSize = qMin(windowSize, buffer.size());
        for (int i = 0; i < windowSize; i++) {
            //renders
            PageRender *pr = pageRenders.front();
            pageRenders.pop_front();
            if (pr != nullptr) {
                if (pr->wait())
                    delete pr;
            }
            pageRenders.push_back(0);

            //images

            if (buffer.front() != 0)
                delete buffer.front();
            buffer.pop_front();
            buffer.push_back(new QImage());
        }
    } else //add pages to left pages and remove on the right
    {
        if (windowSize < 0) {
            windowSize = -windowSize;
            windowSize = qMin(windowSize, buffer.size());
            for (int i = 0; i < windowSize; i++) {
                //renders
                PageRender *pr = pageRenders.back();
                pageRenders.pop_back();
                if (pr != nullptr) {
                    if (pr->wait())
                        delete pr;
                }
                pageRenders.push_front(0);

                //images
                buffer.push_front(new QImage());
                QImage *p = buffer.back();
                if (p != nullptr)
                    delete p;
                buffer.pop_back();
            }
        }
    }
    previousIndex = currentIndex;
}

void Render::fillBuffer()
{
    if (pagesReady.size() < 1) {
        return;
    }

    for (int i = 1; i <= qMax(numLeftPages, numRightPages); i++) {
        if ((currentIndex + i < (int)comic->numPages()) &&
            buffer[currentPageBufferedIndex + i]->isNull() &&
            i <= numRightPages &&
            pageRenders[currentPageBufferedIndex + i] == 0 &&
            pagesReady[currentIndex + i]) //preload next pages
        {
            pageRenders[currentPageBufferedIndex + i] = new PageRender(this, currentIndex + i, comic->getRawData()->at(currentIndex + i), buffer[currentPageBufferedIndex + i], imageRotation, filters);
            connect(pageRenders[currentPageBufferedIndex + i], SIGNAL(pageReady(int)), this, SLOT(prepareAvailablePage(int)));
            pageRenders[currentPageBufferedIndex + i]->start();
        }

        if ((currentIndex - i > 0) &&
            buffer[currentPageBufferedIndex - i]->isNull() &&
            i <= numLeftPages &&
            pageRenders[currentPageBufferedIndex - i] == 0 &&
            pagesReady[currentIndex - i]) //preload previous pages
        {
            pageRenders[currentPageBufferedIndex - i] = new PageRender(this, currentIndex - i, comic->getRawData()->at(currentIndex - i), buffer[currentPageBufferedIndex - i], imageRotation, filters);
            connect(pageRenders[currentPageBufferedIndex - i], SIGNAL(pageReady(int)), this, SLOT(prepareAvailablePage(int)));
            pageRenders[currentPageBufferedIndex - i]->start();
        }
    }
}

//Método que debe ser llamado cada vez que la estructura del buffer se vuelve inconsistente con el modo de lectura actual.
//se terminan todos los hilos en ejecución y se libera la memoria (de hilos e imágenes)
void Render::invalidate()
{
    for (int i = 0; i < pageRenders.size(); i++) {
        if (pageRenders[i] != 0) {
            pageRenders[i]->wait();
            delete pageRenders[i];
            pageRenders[i] = 0;
        }
    }

    for (int i = 0; i < buffer.size(); i++) {
        delete buffer[i];
        buffer[i] = new QImage();
    }
}

void Render::doublePageSwitch()
{
    doublePage = !doublePage;
    if (comic) {
        //invalidate();
        update();
    }
}

void Render::doubleMangaPageSwitch()
{
    doubleMangaPage = !doubleMangaPage;
    if (comic && doublePage) {
        //invalidate();
        update();
    }
}

QString Render::getCurrentPagesInformation()
{
    QString s = QString::number(currentIndex + 1);
    if (doublePage && (currentIndex + 1 < (int)comic->numPages())) {
        if (currentPageIsDoublePage()) {
            if (doubleMangaPage)
                s = QString::number(currentIndex + 2) + "-" + s;
            else
                s += "-" + QString::number(currentIndex + 2);
        }
    }
    s += "/" + QString::number(comic->numPages());
    return s;
}

void Render::setBookmark()
{
    comic->setBookmark();
}

void Render::removeBookmark()
{
    comic->removeBookmark();
}

void Render::save()
{
    comic->saveBookmarks();
}

Bookmarks *Render::getBookmarks()
{
    return comic->bm;
}

void Render::reload()
{
    if (comic) {
        invalidate();
        update();
    }
}

void Render::updateFilters(int brightness, int contrast, int gamma)
{
    for (int i = 0; i < filters.count(); i++) {
        if (typeid(*filters[i]) == typeid(BrightnessFilter))
            filters[i]->setLevel(brightness);
        if (typeid(*filters[i]) == typeid(ContrastFilter))
            filters[i]->setLevel(contrast);
        if (typeid(*filters[i]) == typeid(GammaFilter))
            filters[i]->setLevel(gamma);
    }

    reload();
}
