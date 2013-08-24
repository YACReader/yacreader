#include "goto_flow.h"
#include "configuration.h"
#include "comic.h"

#include <QVBoxLayout>
#include <QSize>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QMutex>
#include <QCoreApplication>

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
#include "goto_flow_decorationbar.h"


GoToFlow::GoToFlow(QWidget *parent,FlowType flowType)
:GoToFlowWidget(parent),ready(false)
{
	updateTimer = new QTimer;
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateImageData()));

	worker = new PageLoader(&mutexGoToFlow);


	flow = new YACReaderFlow(this,flowType);
	flow->setReflectionEffect(PictureFlow::PlainReflection);
	imageSize = Configuration::getConfiguration().getGotoSlideSize();

	flow->setSlideSize(imageSize);
	connect(flow,SIGNAL(centerIndexChanged(int)),this,SLOT(setPageNumber(int)));
	connect(flow,SIGNAL(selected(unsigned int)),this,SIGNAL(goToPage(unsigned int)));

	connect(toolBar,SIGNAL(goTo(unsigned int)),this,SIGNAL(goToPage(unsigned int)));
	connect(toolBar,SIGNAL(setCenter(unsigned int)),flow,SLOT(showSlide(unsigned int))); 

	mainLayout->insertWidget(1,flow);
	mainLayout->setStretchFactor(flow,1);

	resize(static_cast<int>(5*imageSize.width()),static_cast<int>(imageSize.height()*1.7));

	//install eventFilter
	//flow->installEventFilter(this);
	/*edit->installEventFilter(this);
	centerButton->installEventFilter(this);
	goToButton->installEventFilter(this);

	connect(edit,SIGNAL(returnPressed()),goToButton,SIGNAL(clicked()));*/

	this->setCursor(QCursor(Qt::ArrowCursor));
	

}
GoToFlow::~GoToFlow()
{
	delete flow;
	delete updateTimer;
	worker->deleteLater();
}

void GoToFlow::centerSlide(int slide)
{
	if(flow->centerIndex()!=slide)
	{
		flow->setCenterIndex(slide);
		if(ready)// load images if pages are loaded.
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
	imagesReady.fill(false,slides);

	rawImages.clear();
	rawImages.resize(slides);

	toolBar->setTop(slides);

	SlideInitializer * si = new SlideInitializer(&mutexGoToFlow,flow,slides);

	imagesLoaded.clear();
	imagesLoaded.fill(false,slides);

	imagesSetted.clear();
	imagesSetted.fill(false,slides);

	numImagesLoaded = 0;

	connect(flow, SIGNAL(centerIndexChanged(int)), this, SLOT(preload()));
	connect(flow, SIGNAL(centerIndexChangedSilent(int)), this, SLOT(preload()));

	ready = true;
	worker->reset();

	si->start();
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

void GoToFlow::setImageReady(int index,const QByteArray & image)
{
	rawImages[index]=image;
	imagesReady[index]=true;
	preload();
}

void GoToFlow::preload()
{
	if(numImagesLoaded < imagesLoaded.size())
		updateTimer->start(30); //TODO comprobar rendimiento, antes era 70
}

void GoToFlow::updateImageData()
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
			flow->setSlide(idx, worker->result());
			imagesSetted[idx] = true;
			numImagesLoaded++;
			rawImages[idx].clear();; //release memory
			imagesLoaded[idx]=true;
		}

	} 

	// try to load only few images on the left and right side 
	// i.e. all visible ones plus some extra
#define COUNT 8  
	int indexes[2*COUNT+1];
	int center = flow->centerIndex();
	indexes[0] = center;
	for(int j = 0; j < COUNT; j++)
	{
		indexes[j*2+1] = center+j+1;
		indexes[j*2+2] = center-j-1;
	}  
	for(int c = 0; c < 2*COUNT+1; c++)
	{
		int i = indexes[c];
		if((i >= 0) && (i < flow->slideCount()))
			if(!imagesLoaded[i]&&imagesReady[i])//slide(i).isNull())
			{
				// schedule thumbnail generation
				
				worker->generate(i, flow->slideSize(),rawImages[i]);
				return;
			}

	}

	// no need to generate anything? stop polling...
	updateTimer->stop();
}

bool GoToFlow::eventFilter(QObject *target, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) 
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		int key = keyEvent->key();
		if((key==Qt::Key_Return)||
			(key==Qt::Key_Enter)||
			(key==Qt::Key_Space)||
			(key==Qt::Key_Left)||
			(key==Qt::Key_Right)||
			(key==Qt::Key_S))
			this->keyPressEvent(keyEvent);
	}
	return QWidget::eventFilter(target, event);
}

void GoToFlow::wheelEvent(QWheelEvent * event)
{
	if(event->delta()<0)
		flow->showNext();
	else
		flow->showPrevious();
	event->accept();
}

void GoToFlow::setFlowType(FlowType flowType)
{
	flow->setFlowType(flowType);
}

void GoToFlow::updateSize() //TODO : fix. it doesn't work.
{
	imageSize = Configuration::getConfiguration().getGotoSlideSize();
	flow->setSlideSize(imageSize);
	resize(static_cast<int>(5*imageSize.width()),static_cast<int>(imageSize.height()*1.7));
}

void GoToFlow::updateConfig(QSettings * settings)
{


}
//-----------------------------------------------------------------------------
//SlideInitializer
//-----------------------------------------------------------------------------
SlideInitializer::SlideInitializer(QMutex * m,PictureFlow * flow,int slides)
:QThread(),mutex(m),_flow(flow),_slides(slides)		
{

}
void SlideInitializer::run()
{
	mutex->lock();

	_flow->clear();
	for(int i=0;i<_slides;i++)
		_flow->addSlide(QImage());
	_flow->setCenterIndex(0);

	mutex->unlock();
}
//-----------------------------------------------------------------------------
//PageLoader
//-----------------------------------------------------------------------------


PageLoader::PageLoader(QMutex * m): 
QThread(),mutex(m), restart(false), working(false), idx(-1)
{
}

PageLoader::~PageLoader()
{
	mutex->lock();
	condition.wakeOne();
	mutex->unlock();
	wait();
}

bool PageLoader::busy() const
{
	return isRunning() ? working : false;
}  

void PageLoader::generate(int index, QSize size,const QByteArray & rImage)
{
	mutex->lock();
	this->idx = index;
	//this->img = QImage();
	this->size = size;
	this->rawImage = rImage;
	mutex->unlock();

	if (!isRunning())
		start();
	else
	{
		// already running, wake up whenever ready
		restart = true;
		condition.wakeOne();
	}
}

void PageLoader::run()
{
	for(;;)
	{
		// copy necessary data
		mutex->lock();
		this->working = true;
		//int idx = this->idx;


		QImage image;
		image.loadFromData(this->rawImage);
		// let everyone knows it is ready
		image = image.scaled(this->size,Qt::KeepAspectRatio,Qt::SmoothTransformation);

		mutex->unlock();

		mutex->lock();
		this->working = false;
		this->img = image;
		mutex->unlock();

		// put to sleep
		mutex->lock();
		if (!this->restart)
			condition.wait(mutex);
		restart = false;
		mutex->unlock();
	}
}
