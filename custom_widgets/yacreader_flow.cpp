#include "yacreader_flow.h"

#include <QMouseEvent>


YACReaderFlow::YACReaderFlow(QWidget * parent,FlowType flowType) : PictureFlow(parent,flowType) {}

void YACReaderFlow::mousePressEvent(QMouseEvent* event)
{
	if(event->x() > (width()+slideSize().width())/2)
		showNext();
	else
		if(event->x() < (width()-slideSize().width())/2)
			showPrevious();
	//else (centered cover space)
}

void YACReaderFlow::mouseDoubleClickEvent(QMouseEvent* event)
{
	if((event->x() > (width()-slideSize().width())/2)&&(event->x() < (width()+slideSize().width())/2))
		emit selected(centerIndex());
}

