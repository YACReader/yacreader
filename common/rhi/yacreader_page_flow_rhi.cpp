#include "yacreader_page_flow_rhi.h"
#include <QImage>

// YACReaderPageFlow3D implementation
YACReaderPageFlow3D::YACReaderPageFlow3D(QWidget *parent, struct Preset p)
    : YACReaderFlow3D(parent, p)
{
    worker = new ImageLoaderByteArray3D(this);
    worker->flow = this;
}

YACReaderPageFlow3D::~YACReaderPageFlow3D()
{
    if (timerId != -1) {
        this->killTimer(timerId);
        timerId = -1;
    }
    rawImages.clear();

    // Clean up textures and clear images to prevent double-delete in base destructor
    for (auto &image : images) {
        if (image.texture != scene.defaultTexture.get()) {
            delete image.texture;
        }
    }
    images.clear();
    numObjects = 0;
}

void YACReaderPageFlow3D::updateImageData()
{
    if (worker->busy())
        return;

    int idx = worker->index();
    if (idx >= 0 && !worker->result().isNull()) {
        if (!loaded[idx]) {
            float x = 1;
            QImage img = worker->result();

            // Create QRhiTexture from the loaded image and queue the pixel upload
            if (m_rhi) {
                QRhiTexture *texture = m_rhi->newTexture(QRhiTexture::RGBA8, img.size(), 1, QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips);

                if (texture->create()) {
                    // Queue the image upload so it happens together with other resource updates
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
        count = 14;
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
            if (rawImages.size() > 0)
                if (!loaded[i] && imagesReady[i]) {
                    worker->generate(i, rawImages.at(i));
                    delete[] indexes;
                    return;
                }
    }

    delete[] indexes;
}

void YACReaderPageFlow3D::populate(int n)
{
    worker->reset();
    reset();
    numObjects = 0;

    if (hasBeenInitialized) {
        YACReaderFlow3D::populate(n);
    } else {
        lazyPopulateObjects = n;
    }

    imagesReady = QVector<bool>(n, false);
    rawImages = QVector<QByteArray>(n);
    imagesSetted = QVector<bool>(n, false);
}

// ImageLoaderByteArray3D implementation
QImage ImageLoaderByteArray3D::loadImage(const QByteArray &raw)
{
    QImage image;

    if (!image.loadFromData(raw)) {
        return QImage();
    }

    switch (flow->performance) {
    case low:
        image = image.scaledToWidth(128, Qt::SmoothTransformation);
        break;
    case medium:
        image = image.scaledToWidth(196, Qt::SmoothTransformation);
        break;
    case high:
        image = image.scaledToWidth(256, Qt::SmoothTransformation);
        break;
    case ultraHigh:
        image = image.scaledToWidth(320, Qt::SmoothTransformation);
        break;
    }

    return image.convertToFormat(QImage::Format_RGBA8888);
}

ImageLoaderByteArray3D::ImageLoaderByteArray3D(YACReaderFlow3D *flow)
    : QThread(), flow(flow), restart(false), working(false), idx(-1)
{
}

ImageLoaderByteArray3D::~ImageLoaderByteArray3D()
{
    mutex.lock();
    condition.wakeOne();
    mutex.unlock();
    wait();
}

bool ImageLoaderByteArray3D::busy() const
{
    return isRunning() ? working : false;
}

void ImageLoaderByteArray3D::generate(int index, const QByteArray &raw)
{
    mutex.lock();
    this->idx = index;
    this->rawData = raw;
    this->img = QImage();
    mutex.unlock();

    if (!isRunning())
        start();
    else {
        restart = true;
        condition.wakeOne();
    }
}

void ImageLoaderByteArray3D::run()
{
    for (;;) {
        mutex.lock();
        this->working = true;
        QByteArray raw = this->rawData;
        mutex.unlock();

        QImage image = loadImage(raw);

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

QImage ImageLoaderByteArray3D::result()
{
    return img;
}
