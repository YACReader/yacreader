#include "yacreader_comic_flow_rhi.h"

#include <QImage>

// YACReaderComicFlow3D implementation
YACReaderComicFlow3D::YACReaderComicFlow3D(QWidget *parent, struct Preset p)
    : YACReaderFlow3D(parent, p)
{
    worker = new ImageLoader3D(this);
    worker->flow = this;
}

void YACReaderComicFlow3D::setImagePaths(QStringList paths)
{
    worker->reset();
    reset();
    numObjects = 0;

    if (hasBeenInitialized) {
        YACReaderFlow3D::populate(paths.size());
    } else {
        lazyPopulateObjects = paths.size();
    }

    this->paths = paths;
}

void YACReaderComicFlow3D::updateImageData()
{
    if (worker->busy())
        return;

    int idx = worker->index();
    if (idx >= 0 && !worker->result().isNull()) {
        if (!loaded[idx]) {
            float x = 1;
            QImage img = worker->result();

            // Create QRhiTexture from the loaded image
            if (m_rhi) {
                QRhiTexture *texture = m_rhi->newTexture(QRhiTexture::RGBA8, img.size(), 1, QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips);

                if (texture->create()) {
                    PendingTextureUpload upload;
                    upload.index = idx;
                    upload.image = img;
                    upload.x = x;
                    upload.y = 1 * (float(img.height()) / img.width());
                    pendingTextureUploads.append(upload);

                    replace(texture, upload.x, upload.y, idx);
                }
            }
        }
    }

    int count = 8;
    switch (performance) {
    case low:
        count = 8;
        break;
    case medium:
        count = 10;
        break;
    case high:
        count = 12;
        break;
    case ultraHigh:
        count = 16;
        break;
    }

    int *indexes = new int[2 * count + 1];
    int center = currentSelected;
    indexes[0] = center;
    for (int j = 0; j < count; j++) {
        indexes[j * 2 + 1] = center + j + 1;
        indexes[j * 2 + 2] = center - j - 1;
    }

    for (int c = 0; c < 2 * count + 1; c++) {
        int i = indexes[c];
        if ((i >= 0) && (i < numObjects))
            if (!loaded[i]) {
                if (paths.size() > 0) {
                    QString fname = paths.at(i);
                    worker->generate(i, fname);
                }
                delete[] indexes;
                return;
            }
    }

    delete[] indexes;
}

void YACReaderComicFlow3D::remove(int item)
{
    worker->lock();
    worker->reset();
    YACReaderFlow3D::remove(item);
    if (item >= 0 && item < paths.size()) {
        paths.removeAt(item);
    }
    worker->unlock();
}

void YACReaderComicFlow3D::add(const QString &path, int index)
{
    worker->lock();
    worker->reset();
    paths.insert(index, path);
    YACReaderFlow3D::add(index);
    worker->unlock();
}

void YACReaderComicFlow3D::resortCovers(QList<int> newOrder)
{
    worker->lock();
    worker->reset();
    startAnimationTimer();

    QList<QString> pathsNew;
    QVector<bool> loadedNew;
    QVector<YACReader::YACReaderComicReadStatus> marksNew;
    QVector<YACReader3DImageRHI> imagesNew;

    int index = 0;
    foreach (int i, newOrder) {
        if (i < 0 || i >= images.size()) {
            continue;
        }

        pathsNew << paths.at(i);
        loadedNew << loaded.at(i);
        marksNew << marks.at(i);
        imagesNew << images.at(i);
        imagesNew.last().index = index++;
    }

    paths = pathsNew;
    loaded = loadedNew;
    marks = marksNew;
    images = imagesNew;

    worker->unlock();
}

// ImageLoader3D implementation
QImage ImageLoader3D::loadImage(const QString &fileName)
{
    QImage image;

    if (!image.load(fileName)) {
        return QImage();
    }

    switch (flow->performance) {
    case low:
        image = image.scaledToWidth(200, Qt::SmoothTransformation);
        break;
    case medium:
        image = image.scaledToWidth(256, Qt::SmoothTransformation);
        break;
    case high:
        image = image.scaledToWidth(320, Qt::SmoothTransformation);
        break;
    case ultraHigh:
        break;
    }

    return image.convertToFormat(QImage::Format_RGBA8888);
}

ImageLoader3D::ImageLoader3D(YACReaderFlow3D *flow)
    : QThread(), flow(flow), restart(false), working(false), idx(-1)
{
}

ImageLoader3D::~ImageLoader3D()
{
    mutex.lock();
    condition.wakeOne();
    mutex.unlock();
    wait();
}

bool ImageLoader3D::busy() const
{
    return isRunning() ? working : false;
}

void ImageLoader3D::generate(int index, const QString &fileName)
{
    mutex.lock();
    this->idx = index;
    this->fileName = fileName;
    this->img = QImage();
    mutex.unlock();

    if (!isRunning())
        start();
    else {
        restart = true;
        condition.wakeOne();
    }
}

void ImageLoader3D::lock()
{
    mutex.lock();
}

void ImageLoader3D::unlock()
{
    mutex.unlock();
}

void ImageLoader3D::run()
{
    for (;;) {
        mutex.lock();
        this->working = true;
        QString fileName = this->fileName;
        mutex.unlock();

        QImage image = loadImage(fileName);

        mutex.lock();
        this->working = false;
        this->img = image;
        mutex.unlock();

        mutex.lock();
        if (!this->restart)
            condition.wait(&mutex);
        restart = false;
        mutex.unlock();
    }
}

QImage ImageLoader3D::result()
{
    return img;
}
