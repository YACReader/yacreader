#include "comic_flow_widget.h"

ComicFlowWidget::ComicFlowWidget(QWidget * parent)
	:QWidget(parent)
{

}

ComicFlowWidgetSW::ComicFlowWidgetSW(QWidget * parent)
	:ComicFlowWidget(parent)
{
	flow = new ComicFlow(parent);

	connect(flow,SIGNAL(centerIndexChanged(int)),this,SIGNAL(centerIndexChanged(int)));
	connect(flow,SIGNAL(selected(unsigned int)),this,SIGNAL(selected(unsigned int)));

	QVBoxLayout * l = new QVBoxLayout;
	l->addWidget(flow);
	setLayout(l);

	//TODO eleminar "padding"
	QPalette Pal(palette()); 
	// set black background
	Pal.setColor(QPalette::Background, Qt::black);
	setAutoFillBackground(true);
	setPalette(Pal);
}

QSize ComicFlowWidgetSW::minimumSizeHint() const
{
	return flow->minimumSizeHint();
}
QSize ComicFlowWidgetSW::sizeHint() const
{
	return flow->sizeHint();
}

void ComicFlowWidgetSW::setShowMarks(bool value)
{
	flow->setShowMarks(value);
}
void ComicFlowWidgetSW::setMarks(QVector<bool> marks)
{
	flow->setMarks(marks);
}
void ComicFlowWidgetSW::setMarkImage(QImage & image)
{
	flow->setMarkImage(image);
}
void ComicFlowWidgetSW::markSlide(int index)
{
	flow->markSlide(index);
}
void ComicFlowWidgetSW::unmarkSlide(int index)
{
	flow->unmarkSlide(index);
}
void ComicFlowWidgetSW::setSlideSize(QSize size)
{
	flow->setSlideSize(size);
}
void ComicFlowWidgetSW::clear()
{
	flow->clear();
}
void ComicFlowWidgetSW::setImagePaths(QStringList paths)
{
	flow->setImagePaths(paths);
}
void ComicFlowWidgetSW::setCenterIndex(int index)
{
	flow->setCenterIndex(index);
}
void ComicFlowWidgetSW::showSlide(int index)
{
	flow->showSlide(index);
}
int ComicFlowWidgetSW::centerIndex()
{
	return flow->centerIndex();
}
void ComicFlowWidgetSW::updateMarks()
{
	flow->updateMarks();
}
void ComicFlowWidgetSW::setFlowType(PictureFlow::FlowType flowType)
{
	flow->setFlowType(flowType);
}
void ComicFlowWidgetSW::render()
{
	flow->render();
}
void ComicFlowWidgetSW::keyPressEvent(QKeyEvent* event)
{
	flow->keyPressEvent(event);  
}
void ComicFlowWidgetSW::paintEvent(QPaintEvent *event)
{
	flow->paintEvent(event);
}
void ComicFlowWidgetSW::mousePressEvent(QMouseEvent* event)
{
	flow->mousePressEvent(event);
}
void ComicFlowWidgetSW::resizeEvent(QResizeEvent* event)
{
	flow->resizeEvent(event);
}
void ComicFlowWidgetSW::mouseDoubleClickEvent(QMouseEvent* event)
{
	flow->mouseDoubleClickEvent(event);
}




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///OpenGL ComicFlow
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ComicFlowWidgetGL::ComicFlowWidgetGL(QWidget * parent)
	:ComicFlowWidget(parent)
{
	flow = new YACReaderComicFlowGL(parent);

	connect(flow,SIGNAL(centerIndexChanged(int)),this,SIGNAL(centerIndexChanged(int)));
	connect(flow,SIGNAL(selected(unsigned int)),this,SIGNAL(selected(unsigned int)));

	QVBoxLayout * l = new QVBoxLayout;
	l->addWidget(flow);
	setLayout(l);

	//TODO eleminar "padding"
	QPalette Pal(palette()); 
	// set black background
	Pal.setColor(QPalette::Background, Qt::black);
	setAutoFillBackground(true);
	setPalette(Pal);
}

QSize ComicFlowWidgetGL::minimumSizeHint() const
{
	return flow->minimumSizeHint();
}
QSize ComicFlowWidgetGL::sizeHint() const
{
	return flow->sizeHint();
}

void ComicFlowWidgetGL::setShowMarks(bool value)
{
	flow->setShowMarks(value);
}
void ComicFlowWidgetGL::setMarks(QVector<bool> marks)
{
	flow->setMarks(marks);
}
void ComicFlowWidgetGL::setMarkImage(QImage & image)
{
	flow->setMarkImage(image);
}
void ComicFlowWidgetGL::markSlide(int index)
{
	flow->markSlide(index);
}
void ComicFlowWidgetGL::unmarkSlide(int index)
{
	flow->unmarkSlide(index);
}
void ComicFlowWidgetGL::setSlideSize(QSize size)
{
	flow->setSlideSize(size);
}
void ComicFlowWidgetGL::clear()
{
	flow->clear();
}
void ComicFlowWidgetGL::setImagePaths(QStringList paths)
{
	flow->setImagePaths(paths);
}
void ComicFlowWidgetGL::setCenterIndex(int index)
{
	flow->setCenterIndex(index);
}
void ComicFlowWidgetGL::showSlide(int index)
{
	flow->showSlide(index);
}
int ComicFlowWidgetGL::centerIndex()
{
	return flow->centerIndex();
}
void ComicFlowWidgetGL::updateMarks()
{
	flow->updateMarks();
}
void ComicFlowWidgetGL::setFlowType(PictureFlow::FlowType flowType)
{
		if(flowType == PictureFlow::CoverFlowLike)
			flow->setPreset(presetYACReaderFlowClassicConfig);
		else if(flowType == PictureFlow::Strip)
			flow->setPreset(presetYACReaderFlowStripeConfig);
		else if(flowType == PictureFlow::StripOverlapped)
			flow->setPreset(presetYACReaderFlowOverlappedStripeConfig);
		else
			flow->setPreset(defaultYACReaderFlowConfig);
}
void ComicFlowWidgetGL::render()
{
	flow->render();
}
void ComicFlowWidgetGL::keyPressEvent(QKeyEvent* event)
{
	flow->keyPressEvent(event);  
}
void ComicFlowWidgetGL::paintEvent(QPaintEvent *event)
{
	//flow->paintEvent(event);
}
void ComicFlowWidgetGL::mousePressEvent(QMouseEvent* event)
{
	flow->mousePressEvent(event);
}
void ComicFlowWidgetGL::resizeEvent(QResizeEvent* event)
{
	flow->resizeGL(event->size().width(),event->size().height());
}
void ComicFlowWidgetGL::mouseDoubleClickEvent(QMouseEvent* event)
{
	flow->mouseDoubleClickEvent(event);
}