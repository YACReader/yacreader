#include "goto_flow_gl.h"

#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include <QPushButton>
#include <QSize>
#include <QApplication>

#include "configuration.h"

#include "goto_flow_toolbar.h"


GoToFlowGL::GoToFlowGL(QWidget* parent, FlowType flowType)
	:GoToFlowWidget(parent)
{
	Q_UNUSED(flowType)
	flow = new YACReaderPageFlowGL(this);
	flow->setShowMarks(false);

	imageSize = Configuration::getConfiguration().getGotoSlideSize();

	flow->setSlideSize(imageSize);
	connect(flow,SIGNAL(centerIndexChanged(int)),this,SLOT(setPageNumber(int)));
	connect(flow,SIGNAL(selected(unsigned int)),this,SIGNAL(goToPage(unsigned int)));

	connect(toolBar,SIGNAL(goTo(unsigned int)),this,SIGNAL(goToPage(unsigned int)));
	connect(toolBar,SIGNAL(setCenter(unsigned int)),flow,SLOT(setCenterIndex(unsigned int)));

	mainLayout->addWidget(flow);
	toolBar->raise();

	resize(static_cast<int>(5*imageSize.width()),toolBar->height() + static_cast<int>(imageSize.height()*1.7));

	this->setCursor(QCursor(Qt::ArrowCursor));
}

GoToFlowGL::~GoToFlowGL()
{
	delete flow;
}

void GoToFlowGL::reset()
{
	flow->reset();
}

void GoToFlowGL::centerSlide(int slide)
{
	if(flow->centerIndex()!=slide)
	{
		flow->setCenterIndex(slide);
	}
}

void GoToFlowGL::setFlowType(FlowType flowType)
{
	if(flowType == CoverFlowLike)
		flow->setPreset(presetYACReaderFlowClassicConfig);
	else if(flowType == Strip)
		flow->setPreset(presetYACReaderFlowStripeConfig);
	else if(flowType == StripOverlapped)
		flow->setPreset(presetYACReaderFlowOverlappedStripeConfig);
	else
		flow->setPreset(defaultYACReaderFlowConfig);
}

void GoToFlowGL::setNumSlides(unsigned int slides)
{
	flow->populate(slides);
	toolBar->setTop(slides);
}
void GoToFlowGL::setImageReady(int index,const QByteArray & imageData)
{
	flow->rawImages[index] = imageData;
	flow->imagesReady[index] = true;
}

void GoToFlowGL::updateSize()
{

}

void GoToFlowGL::updateConfig(QSettings * settings)
{
	Performance performance = medium;

	switch (settings->value(PERFORMANCE).toInt())
	{
	case 0:
		performance = low;
		break;
	case 1:
		performance = medium;
		break;
	case 2:
		performance = high;
		break;
	case 3:
		performance = ultraHigh;
		break;
	}

	flow->setPerformance(performance);

	switch (settings->value(FLOW_TYPE_GL).toInt())
	{
	case 0:
		flow->setPreset(presetYACReaderFlowClassicConfig);
		return;
	case 1:
		flow->setPreset(presetYACReaderFlowStripeConfig);
		return;
	case 2:
		flow->setPreset(presetYACReaderFlowOverlappedStripeConfig);
		return;
	case 3:
		flow->setPreset(defaultYACReaderFlowConfig);
		return;
	case 4:
		flow->setPreset(pressetYACReaderFlowDownConfig);
		return;
	}


	//custom config

	flow->setCF_RX(settings->value(X_ROTATION).toInt());
	flow->setCF_Y(settings->value(Y_POSITION).toInt());
	flow->setX_Distance(settings->value(COVER_DISTANCE).toInt());
	flow->setCenter_Distance(settings->value(CENTRAL_DISTANCE).toInt());
	flow->setCF_Z(settings->value(ZOOM_LEVEL).toInt());
	flow->setY_Distance(settings->value(Y_COVER_OFFSET).toInt());
	flow->setZ_Distance(settings->value(Z_COVER_OFFSET).toInt());
	flow->setRotation(settings->value(COVER_ROTATION).toInt());
	flow->setFadeOutDist(settings->value(FADE_OUT_DIST).toInt());
	flow->setLightStrenght(settings->value(LIGHT_STRENGTH).toInt());
	flow->setMaxAngle(settings->value(MAX_ANGLE).toInt());

	/*	flow->setVisibility(settings->value("visibilityDistance").toInt());
	flow->setLightStrenght(settings->value("lightStrength").toInt())*/;

}

void GoToFlowGL::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Left: case Qt::Key_Right: case Qt::Key_Up:
		QApplication::sendEvent(flow,event);
		return;
	default:
		break;
	}

	GoToFlowWidget::keyPressEvent(event);
}

void GoToFlowGL::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	toolBar->move(0, event->size().height() - toolBar->height());
	toolBar->setFixedWidth(width());
}
