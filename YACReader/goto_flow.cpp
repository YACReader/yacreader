#include "goto_flow.h"
#include "configuration.h"
#include "comic.h"
#include "custom_widgets.h"

#include <QVBoxLayout>
#include <QSize>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QMutex>
#include <QCoreApplication>

/*#define WIDTH 126
#define HEIGHT 200*/

QMutex mutexGoToFlow;



GoToFlow::GoToFlow(QWidget *parent,FlowType flowType)
:GoToFlowWidget(parent),ready(false)
{
	updateTimer = new QTimer;
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateImageData()));

	worker = new PageLoader;

	QVBoxLayout * layout = new QVBoxLayout(this);
	flow = new YACReaderFlow(this,flowType);
	flow->setReflectionEffect(PictureFlow::PlainReflection);
	imageSize = Configuration::getConfiguration().getGotoSlideSize();

	flow->setSlideSize(imageSize);
	connect(flow,SIGNAL(centerIndexChanged(int)),this,SLOT(setPageNumber(int)));
	connect(flow,SIGNAL(selected(unsigned int)),this,SLOT(goTo()));

	QHBoxLayout * bottom = new QHBoxLayout(this);
	bottom->addStretch();
	bottom->addWidget(new QLabel(tr("Page : "),this));
	bottom->addWidget(edit = new QLineEdit(this));
	v = new QIntValidator(this);
	v->setBottom(1);
	edit->setValidator(v);
	edit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	edit->setFixedWidth(40);
	edit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum));

	centerButton = new QPushButton(this);
	centerButton->setIcon(QIcon(":/images/center.png"));
	connect(centerButton,SIGNAL(clicked()),this,SLOT(centerSlide()));
	bottom->addWidget(centerButton);

	goToButton = new QPushButton(this);
	goToButton->setIcon(QIcon(":/images/goto.png"));
	connect(goToButton,SIGNAL(clicked()),this,SLOT(goTo()));
	bottom->addWidget(goToButton);

	bottom->addStretch();

	layout->addWidget(flow);
	layout->addLayout(bottom);
	layout->setStretchFactor(flow,1);
	layout->setStretchFactor(bottom,0);
	layout->setMargin(0);
	layout->setSpacing(0);
	setLayout(layout);
	this->setAutoFillBackground(true);
	resize(static_cast<int>(5*imageSize.width()),static_cast<int>(imageSize.height()*1.7));

	//install eventFilter
	//flow->installEventFilter(this);
	edit->installEventFilter(this);
	centerButton->installEventFilter(this);
	goToButton->installEventFilter(this);

	connect(edit,SIGNAL(returnPressed()),goToButton,SIGNAL(clicked()));

	this->setCursor(QCursor(Qt::ArrowCursor));
}

void GoToFlow::goTo()
{
	//emit(goToPage(flow->centerIndex()+1));
	emit(goToPage(edit->text().toInt()));
}

void GoToFlow::setPageNumber(int page)
{
	edit->setText(QString::number(page+1));
}

void GoToFlow::centerSlide()
{
	int page = edit->text().toInt()-1;
	int distance = flow->centerIndex()-page;

	if(abs(distance)>10)
	{
		if(distance<0)
			flow->setCenterIndex(flow->centerIndex()+(-distance)-10);
		else
			flow->setCenterIndex(flow->centerIndex()-distance+10);
	}

	flow->showSlide(page);
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

	v->setTop(slides);

	SlideInitializer * si = new SlideInitializer(flow,slides);

	imagesLoaded.clear();
	imagesLoaded.fill(false,slides);

	imagesSetted.clear();
	imagesSetted.fill(false,slides);

	numImagesLoaded = 0;

	connect(flow, SIGNAL(centerIndexChanged(int)), this, SLOT(preload()));

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
				imagesLoaded[i]=true;
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


void GoToFlow::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Return: case Qt::Key_Enter:
		goTo();
		centerSlide();
		break;
	case Qt::Key_Space:
		centerSlide();
		break;
	case Qt::Key_S:
		QCoreApplication::sendEvent(this->parent(),event);
		break;
	case Qt::Key_Left: case Qt::Key_Right:
		QCoreApplication::sendEvent(flow,event);
	}

	event->accept();
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
SlideInitializer::SlideInitializer(PictureFlow * flow,int slides)
:QThread(),_flow(flow),_slides(slides)		
{

}
void SlideInitializer::run()
{
	mutexGoToFlow.lock();

	_flow->clear();
	for(int i=0;i<_slides;i++)
		_flow->addSlide(QImage());
	_flow->setCenterIndex(0);

	mutexGoToFlow.unlock();
}
//-----------------------------------------------------------------------------
//PageLoader
//-----------------------------------------------------------------------------


PageLoader::PageLoader(): 
QThread(), restart(false), working(false), idx(-1)
{
}

PageLoader::~PageLoader()
{
	mutexGoToFlow.lock();
	condition.wakeOne();
	mutexGoToFlow.unlock();
	wait();
}

bool PageLoader::busy() const
{
	return isRunning() ? working : false;
}  

void PageLoader::generate(int index, QSize size,const QByteArray & rImage)
{
	mutexGoToFlow.lock();
	this->idx = index;
	//this->img = QImage();
	this->size = size;
	this->rawImage = rImage;
	mutexGoToFlow.unlock();

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
		mutexGoToFlow.lock();
		this->working = true;
		//int idx = this->idx;


		QImage image;
		image.loadFromData(this->rawImage);
		// let everyone knows it is ready
		image = image.scaled(this->size,Qt::KeepAspectRatio,Qt::SmoothTransformation);

		mutexGoToFlow.unlock();

		mutexGoToFlow.lock();
		this->working = false;
		this->img = image;
		mutexGoToFlow.unlock();

		// put to sleep
		mutexGoToFlow.lock();
		if (!this->restart)
			condition.wait(&mutexGoToFlow);
		restart = false;
		mutexGoToFlow.unlock();
	}
}
