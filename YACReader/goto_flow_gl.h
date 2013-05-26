#ifndef __GOTO_FLOW_GL_H
#define __GOTO_FLOW_GL_H

#include "yacreader_global.h"
#include "goto_flow_widget.h"
#include "yacreader_flow_gl.h"

class QLineEdit;
class QIntValidator;
class QPushButton;
class QPushButton;
class QSize;

class GoToFlowGL : public GoToFlowWidget
{
	Q_OBJECT
public:
	GoToFlowGL(QWidget* parent = 0,FlowType flowType = CoverFlowLike);
	void reset();
	void centerSlide(int slide);
	void setFlowType(FlowType flowType);
	void setNumSlides(unsigned int slides);
	void setImageReady(int index,const QByteArray & image);
	void updateSize();
	bool eventFilter(QObject *target, QEvent *event);

	void updateConfig(QSettings * settings);

signals:
	void goToPage(unsigned int page);
private:
	YACReaderPageFlowGL * flow;
	
	
	//Comic * comic;
	QSize imageSize;
};

#endif