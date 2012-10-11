#ifndef __GOTO_FLOW_WIDGET_H
#define __GOTO_FLOW_WIDGET_H

#include <QWidget>
#include <QSettings>
#include "yacreader_global.h"

class GoToFlowWidget : public QWidget
{	
	Q_OBJECT
public:
	GoToFlowWidget(QWidget * paret = 0);
public slots:
	virtual void reset() = 0;
	virtual void centerSlide() = 0;
	virtual void centerSlide(int slide) = 0;
	virtual void setPageNumber(int page) = 0;
	virtual void setFlowType(FlowType flowType) = 0;
	virtual void setNumSlides(unsigned int slides) = 0;
	virtual void setImageReady(int index,const QByteArray & image) = 0;
	virtual void updateSize() = 0;
	virtual void goTo() = 0;
	virtual void updateConfig(QSettings * settings) = 0;

};

#endif
