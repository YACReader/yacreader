#ifndef YACREADER_FLOW_H
#define YACREADER_FLOW_H

#include "pictureflow.h"

class QMouseEvent;

class YACReaderFlow : public PictureFlow
{
Q_OBJECT
public:
	YACReaderFlow(QWidget * parent,FlowType flowType = CoverFlowLike);

	void mousePressEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);

signals:
	void selected(unsigned int centerIndex);
};

#endif // YACREADER_FLOW_H