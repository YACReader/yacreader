#include "goto_flow.h"
#include "configuration.h"
#include "comic.h"

#include <QVBoxLayout>
#include <QSize>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QMutex>
#include <QApplication>

#include <QLineEdit>
#include <QPushButton>
#include <QPixmap>
#include <QThread>
#include <QSize>
#include <QIntValidator>
#include <QWaitCondition>
#include <QObject>
#include <QEvent>
#include <QLabel>

#include "yacreader_flow.h"

#include "goto_flow_toolbar.h"

GoToFlow::GoToFlow(QWidget *parent, FlowType flowType)
    : GoToFlowWidget(parent), ready(false)
{
    updateTimer = new QTimer;
    connect(updateTimer, &QTimer::timeout, this, &GoToFlow::updateImageData);

    worker = new PageLoader(&mutexGoToFlow);

    flow = new YACReaderFlow(this, flowType);
    flow->setReflectionEffect(PictureFlow::PlainReflection);
    imageSize = Configuration::getConfiguration().getGotoSlideSize();

    flow->setSlideSize(imageSize);
    connect(flow, &PictureFlow::centerIndexChanged, this, &GoToFlowWidget::setPageNumber);
    connect(flow, &YACReaderFlow::selected, this, &GoToFlow::goToPage);
    connect(flow, &PictureFlow::centerIndexChanged, this, &GoToFlow::preload);
    connect(flow, &PictureFlow::centerIndexChangedSilent, this, &GoToFlow::preload);

    connect(toolBar, SIGNAL(goTo(unsigned int)), this, SIGNAL(goToPage(unsigned int)));
    connect(toolBar, &GoToFlowToolBar::setCenter, flow, &PictureFlow::showSlide);

    mainLayout->addWidget(flow);
    toolBar->raise();

    resize(static_cast<int>(5 * imageSize.width()), toolBar->height() + static_cast<int>(imageSize.height() * 1.7));

    this->setCursor(QCursor(Qt::ArrowCursor));
}

GoToFlow::~GoToFlow()
{
    delete flow;
    delete updateTimer;
    worker->deleteLater();
}

void GoToFlow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
        QApplication::sendEvent(flow, event);
        return;
    default:
        break;
    }

    GoToFlowWidget::keyPressEvent(event);
}

void GoToFlow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    toolBar->move(0, event->size().height() - toolBar->height());
    toolBar->setFixedWidth(width());
}

void GoToFlow::centerSlide(int slide)
{
    if (flow->centerIndex() != slide) {
        flow->setCenterIndex(slide);
        if (ready) // load images if pages are loaded.
        {
            //worker->reset(); //BUG FIXED : image didn't load if worker was working
            preload();
        }
    }
}

void GoToFlow::setNumSlides(unsigned int slides)
{
    //	numPagesLabel->setText(tr("Total pages : ")+QString::number(slides));
    //	numPagesLabel->adjustSize();
    imagesReady.clear();
    imagesReady.fill(false, slides);

    rawImages.clear();
    rawImages.resize(slides);

    toolBar->setTop(slides);

    imagesLoaded.clear();
    imagesLoaded.fill(false, slides);

    imagesSetted.clear();
    imagesSetted.fill(false, slides);

    numImagesLoaded = 0;

    ready = true;
    worker->reset();

    flow->clear();
    for (unsigned int i = 0; i < slides; i++)
        flow->addSlide(QImage());
    flow->setCenterIndex(0);
}

void GoToFlow::reset()
{
    updateTimer->stop();
    /*imagesLoaded.clear();
	numImagesLoaded = 0;
	imagesReady.clear();
	rawImages.clear();*/
    ready = false;
}

void GoToFlow::setImageReady(int index, const QByteArray &image)
{
    rawImages[index] = image;
    imagesReady[index] = true;
    preload();
}

void GoToFlow::preload()
{
    if (numImagesLoaded < imagesLoaded.size())
        updateTimer->start(30); //TODO comprobar rendimiento, antes era 70
}

void GoToFlow::updateImageData()
{
    // can't do anything, wait for the next possibility
    if (worker->busy())
        return;

    // set image of last one
    int idx = worker->index();
    if (idx >= 0 && !worker->result().isNull()) {
        if (!imagesSetted[idx]) {
            flow->setSlide(idx, worker->result());
            imagesSetted[idx] = true;
            numImagesLoaded++;
            rawImages[idx].clear();
            ; //release memory
            imagesLoaded[idx] = true;
        }
    }

    // try to load only few images on the left and right side
    // i.e. all visible ones plus some extra
#define COUNT 8
    int indexes[2 * COUNT + 1];
    int center = flow->centerIndex();
    indexes[0] = center;
    for (int j = 0; j < COUNT; j++) {
        indexes[j * 2 + 1] = center + j + 1;
        indexes[j * 2 + 2] = center - j - 1;
    }
    for (int c = 0; c < 2 * COUNT + 1; c++) {
        int i = indexes[c];
        if ((i >= 0) && (i < flow->slideCount()))
            if (!imagesLoaded[i] && imagesReady[i]) //slide(i).isNull())
            {
                // schedule thumbnail generation

                worker->generate(i, flow->slideSize(), rawImages[i]);
                return;
            }
    }

    // no need to generate anything? stop polling...
    updateTimer->stop();
}

void GoToFlow::wheelEvent(QWheelEvent *event)
{
    if (event->delta() < 0)
        flow->showNext();
    else
        flow->showPrevious();
    event->accept();
}

void GoToFlow::setFlowType(FlowType flowType)
{
    flow->setFlowType(flowType);
}

void GoToFlow::updateConfig(QSettings *settings)
{
    GoToFlowWidget::updateConfig(settings);

    imageSize = Configuration::getConfiguration().getGotoSlideSize();
    flow->setFlowType(Configuration::getConfiguration().getFlowType());
    resize(5 * imageSize.width(), toolBar->height() + imageSize.height() * 1.7);
    updateSize();
}

void GoToFlow::setFlowRightToLeft(bool b)
{
    flow->setFlowRightToLeft(b);
}

//-----------------------------------------------------------------------------
//PageLoader
//-----------------------------------------------------------------------------

PageLoader::PageLoader(QMutex *m)
    : QThread(), mutex(m), restart(false), working(false), idx(-1)
{
}

PageLoader::~PageLoader()
{
    //TODO this destructor never runs. If it is ever called, it will hang, because
    //the implementation is broken due to the absolutely endless loop in run().
    mutex->lock();
    condition.wakeOne();
    mutex->unlock();
    wait();
}

bool PageLoader::busy() const
{
    return isRunning() ? working.load() : false;
}

void PageLoader::generate(int index, QSize size, const QByteArray &rImage)
{
    mutex->lock();
    this->idx = index;
    //this->img = QImage();
    this->size = size;
    this->rawImage = rImage;
    mutex->unlock();

    if (!isRunning())
        start();
    else {
        mutex->lock();
        // already running, wake up whenever ready
        restart = true;
        condition.wakeOne();
        mutex->unlock();
    }
}

void PageLoader::run()
{
    for (;;) {
        // copy necessary data
        mutex->lock();
        this->working = true;
        //int idx = this->idx;

        QImage image;
        image.loadFromData(this->rawImage);
        // let everyone knows it is ready
        image = image.scaled(this->size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        mutex->unlock();

        mutex->lock();
        this->img = image;
        this->working = false;
        mutex->unlock();

        // put to sleep
        mutex->lock();
        while (!this->restart)
            condition.wait(mutex);
        restart = false;
        mutex->unlock();
    }
}
