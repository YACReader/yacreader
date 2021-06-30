#include "comic_flow.h"
#include "worker_thread.h"

#include "yacreader_global.h"

ComicFlow::ComicFlow(QWidget *parent, FlowType flowType)
    : YACReaderFlow(parent, flowType), worker(new WorkerThread<QImage>)
{
    resetWorkerIndex();
    connect(&updateTimer, &QTimer::timeout, this, &ComicFlow::updateImageData);

    connect(this, &PictureFlow::centerIndexChanged, this, &ComicFlow::preload);
    connect(this, &PictureFlow::centerIndexChangedSilent, this, &ComicFlow::preload);

    setReflectionEffect(PlainReflection);
}

ComicFlow::~ComicFlow() = default;

void ComicFlow::setImagePaths(const QStringList &paths)
{
    clear();

    imageFiles = paths;
    imagesLoaded.clear();
    imagesLoaded.fill(false, imageFiles.size());
    numImagesLoaded = 0;

    imagesSetted.clear();
    imagesSetted.fill(false, imageFiles.size());

    // populate with empty images
    QImage img; //TODO remove
    QString s;
    for (int i = 0; i < (int)imageFiles.size(); i++) {
        addSlide(img);
        s = imageFiles.at(i);
        s.remove(s.size() - 4, 4);
        if (QFileInfo(s + ".r").exists())
            markSlide(i);
    }

    setCenterIndex(0);

    resetWorkerIndex();
    preload();
}

void ComicFlow::preload()
{
    if (numImagesLoaded < imagesLoaded.size())
        updateTimer.start(30); //TODO comprobar rendimiento, originalmente era 70
}

void ComicFlow::updateImageData()
{
    // can't do anything, wait for the next possibility
    if (worker->busy())
        return;

    // set image of last one
    const int idx = workerIndex;
    if (idx >= 0) {
        const QImage result = worker->extractResult();
        if (!result.isNull() && !imagesSetted[idx]) {
            setSlide(idx, result);
            imagesSetted[idx] = true;
            numImagesLoaded++;
            imagesLoaded[idx] = true;
        }
    }

    // try to load only few images on the left and right side
    // i.e. all visible ones plus some extra
#define COUNT 8
    int indexes[2 * COUNT + 1];
    int center = centerIndex();
    indexes[0] = center;
    for (int j = 0; j < COUNT; j++) {
        indexes[j * 2 + 1] = center + j + 1;
        indexes[j * 2 + 2] = center - j - 1;
    }
    for (int c = 0; c < 2 * COUNT + 1; c++) {
        int i = indexes[c];
        if ((i >= 0) && (i < slideCount()))
            if (!imagesLoaded[i]) //slide(i).isNull())
            {
                // schedule thumbnail generation
                QString fname = imageFiles[i];

                workerIndex = i;
                worker->performTask([fname] { return QImage { fname }; });
                return;
            }
    }

    // no need to generate anything? stop polling...
    updateTimer.stop();
}

void ComicFlow::keyPressEvent(QKeyEvent *event)
{
    PictureFlow::keyPressEvent(event);
}

void ComicFlow::wheelEvent(QWheelEvent *event)
{
    if (event->delta() < 0)
        showNext();
    else
        showPrevious();
    event->accept();
}

void ComicFlow::removeSlide(int cover)
{
    imageFiles.removeAt(cover);
    if (imagesLoaded[cover])
        numImagesLoaded--;
    imagesLoaded.remove(cover);
    imagesSetted.remove(cover);

    YACReaderFlow::removeSlide(cover);

    resetWorkerIndex();
    preload();
}

void ComicFlow::resortCovers(QList<int> newOrder)
{
    YACReaderFlow::resortCovers(newOrder);

    QStringList imageFilesNew;
    QVector<bool> imagesLoadedNew;
    QVector<bool> imagesSettedNew;
    foreach (int index, newOrder) {
        imageFilesNew << imageFiles.at(index);
        imagesLoadedNew << imagesLoaded.at(index);
        imagesSettedNew << imagesSetted.at(index);
    }

    imageFiles = imageFilesNew;
    imagesLoaded = imagesLoadedNew;
    imagesSetted = imagesSettedNew;

    resetWorkerIndex();
}
