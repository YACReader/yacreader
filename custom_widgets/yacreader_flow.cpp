#include "yacreader_flow.h"

#include <QMouseEvent>


YACReaderFlow::YACReaderFlow(QWidget * parent,FlowType flowType) : PictureFlow(parent,flowType) {}

void YACReaderFlow::mousePressEvent(QMouseEvent* event)
{
    PictureFlow::mousePressEvent(event, slideSize().width());
}

void YACReaderFlow::mouseDoubleClickEvent(QMouseEvent* event)
{
	if((event->x() > (width()-slideSize().width())/2)&&(event->x() < (width()+slideSize().width())/2))
		emit selected(centerIndex());
}

