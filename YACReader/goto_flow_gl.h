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
class QKeyEvent;

class GoToFlowGL : public GoToFlowWidget
{
    Q_OBJECT
public:
    GoToFlowGL(QWidget *parent = nullptr, FlowType flowType = CoverFlowLike);
    ~GoToFlowGL();
    void reset();
    void centerSlide(int slide);
    void setFlowType(FlowType flowType);
    void setNumSlides(unsigned int slides);
    void setImageReady(int index, const QByteArray &image);

    void updateConfig(QSettings *settings);
    void setFlowRightToLeft(bool b);

signals:
    void goToPage(unsigned int page);

private:
    YACReaderPageFlowGL *flow;
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
    //Comic * comic;
    QSize imageSize;
};

#endif
