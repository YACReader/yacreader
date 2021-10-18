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
    ~GoToFlowGL() override;
    void reset() override;
    void centerSlide(int slide) override;
    void setFlowType(FlowType flowType) override;
    void setNumSlides(unsigned int slides) override;
    void setImageReady(int index, const QByteArray &image) override;

    void updateConfig(QSettings *settings);
    void setFlowRightToLeft(bool b) override;

private:
    YACReaderPageFlowGL *flow;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    // Comic * comic;
    QSize imageSize;
};

#endif
