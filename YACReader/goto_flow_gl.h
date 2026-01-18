#ifndef __GOTO_FLOW_GL_H
#define __GOTO_FLOW_GL_H

#include "yacreader_global.h"
#include "goto_flow_widget.h"

// Conditional include based on Qt version and RHI availability
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0) && defined(YACREADER_USE_RHI)
#include "yacreader_page_flow_rhi.h"
using YACReaderPageFlowImpl = YACReaderPageFlow3D;
#else
#include "yacreader_flow_gl.h"
using YACReaderPageFlowImpl = YACReaderPageFlowGL;
#endif

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

    void updateConfig(QSettings *settings) override;
    void setFlowRightToLeft(bool b) override;

private:
    YACReaderPageFlowImpl *flow;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    // Comic * comic;
    QSize imageSize;
};

#endif
