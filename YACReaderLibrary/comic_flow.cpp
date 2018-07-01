#include "comic_flow.h"
#include "qnaturalsorting.h"

#include "yacreader_global.h"

#include <algorithm>

#include <QMutex>
#include <QImageReader>
#include <QTimer>

ComicFlow::ComicFlow(QWidget* parent,FlowType flowType)
:YACReaderFlow(parent,flowType)
{
	updateTimer = new QTimer;
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateImageData()));

	worker = new ImageLoader;
	connect(this, SIGNAL(centerIndexChanged(int)), this, SLOT(preload()));
	connect(this, SIGNAL(centerIndexChangedSilent(int)), this, SLOT(preload()));

	setReflectionEffect(PlainReflection);
}

ComicFlow::~ComicFlow()
{
    worker->terminate();
	delete worker;
	delete updateTimer;
}

void ComicFlow::setImagePaths(const QStringList& paths)
{
	clear();

	//imagePath = path;
	imageFiles = paths;
	imagesLoaded.clear();
	imagesLoaded.fill(false,imageFiles.size());
	numImagesLoaded = 0;

	imagesSetted.clear();
	imagesSetted.fill(false,imageFiles.size());

	// populate with empty images
	QImage img; //TODO remove
	QString s;
	for(int i = 0; i < (int)imageFiles.size(); i++)
	{
		addSlide(img);
		s = imageFiles.at(i);
		s.remove(s.size()-4,4);
		if(QFileInfo(s+".r").exists())
			markSlide(i);
	}

	setCenterIndex(0);
	worker->reset();
	preload();
}

void ComicFlow::preload()
{
	if(numImagesLoaded < imagesLoaded.size())
		updateTimer->start(30); //TODO comprobar rendimiento, originalmente era 70
}

void ComicFlow::updateImageData()
{
	// can't do anything, wait for the next possibility
	if(worker->busy())
		return;

	// set image of last one
	int idx = worker->index();
	if( idx >= 0 && !worker->result().isNull())
	{
		if(!imagesSetted[idx])
		{
			setSlide(idx, worker->result());
			imagesSetted[idx] = true;
			numImagesLoaded++;
			imagesLoaded[idx]=true;
		}
	}

	// try to load only few images on the left and right side 
	// i.e. all visible ones plus some extra
#define COUNT 8  
	int indexes[2*COUNT+1];
	int center = centerIndex();
	indexes[0] = center;
	for(int j = 0; j < COUNT; j++)
	{
		indexes[j*2+1] = center+j+1;
		indexes[j*2+2] = center-j-1;
	}  
	for(int c = 0; c < 2*COUNT+1; c++)
	{
		int i = indexes[c];
		if((i >= 0) && (i < slideCount()))
			if(!imagesLoaded[i])//slide(i).isNull())
			{
				// schedule thumbnail generation
				QString fname = imageFiles[i];
				
				
				worker->generate(i, fname, slideSize());
				return;
			}
	}

	// no need to generate anything? stop polling...
	updateTimer->stop();
}

void ComicFlow::keyPressEvent(QKeyEvent* event)
{
	PictureFlow::keyPressEvent(event);  
}

void ComicFlow::wheelEvent(QWheelEvent * event)
{
	if(event->delta()<0)
		showNext();
	else
		showPrevious();
	event->accept();
}

void ComicFlow::removeSlide(int cover)
{
	worker->lock();

	worker->reset();

	imageFiles.removeAt(cover);
	if(imagesLoaded[cover])
		numImagesLoaded--;
	imagesLoaded.remove(cover);
	imagesSetted.remove(cover);

	YACReaderFlow::removeSlide(cover);
	worker->unlock();

    preload();
}

void ComicFlow::resortCovers(QList<int> newOrder)
{
    worker->lock();
    worker->reset();

    YACReaderFlow::resortCovers(newOrder);

    QStringList imageFilesNew;
    QVector<bool> imagesLoadedNew;
    QVector<bool> imagesSettedNew;
    foreach(int index, newOrder)
    {
        imageFilesNew << imageFiles.at(index);
        imagesLoadedNew << imagesLoaded.at(index);
        imagesSettedNew << imagesSetted.at(index);
    }



    imageFiles = imageFilesNew;
    imagesLoaded = imagesLoadedNew;
    imagesSetted = imagesSettedNew;

    worker->unlock();
}
//-----------------------------------------------------------------------------
//ImageLoader
//-----------------------------------------------------------------------------
static QImage loadImage(const QString& fileName)
{
	QImage image;
	bool result = image.load(fileName);

	if(!result)
		return QImage();

	return image;
}

ImageLoader::ImageLoader(): 
QThread(), restart(false), working(false), idx(-1)
{
}

ImageLoader::~ImageLoader()
{
	mutex.lock();
	condition.wakeOne();
	mutex.unlock();
	wait();
}

bool ImageLoader::busy() const
{
	return isRunning() ? working : false;
}  

void ImageLoader::generate(int index, const QString& fileName, QSize size)
{
	mutex.lock();
	this->idx = index;
	this->fileName = fileName;
	this->size = size;
	this->img = QImage();
	mutex.unlock();

	if (!isRunning())
		start();
	else
	{
		// already running, wake up whenever ready
		restart = true;
		condition.wakeOne();
	}
}

void ImageLoader::lock()
{
	mutex.lock();
}

void ImageLoader::unlock()
{
	mutex.unlock();
}

void ImageLoader::run()
{
	for(;;)
	{
		// copy necessary data
		mutex.lock();
		this->working = true;
		QString fileName = this->fileName;
		mutex.unlock();

		QImage image = loadImage(fileName);

		// let everyone knows it is ready
		mutex.lock();
		this->working = false;
		this->img = image;
		mutex.unlock();

		// put to sleep
		mutex.lock();
		if (!this->restart)
			condition.wait(&mutex);
		restart = false;
		mutex.unlock();
	}
}

QImage ImageLoader::result() 
{ 
	return img; 
}
