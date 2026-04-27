#ifndef __GOTO_FLOW_WIDGET_H
#define __GOTO_FLOW_WIDGET_H

#include "themable.h"
#include "yacreader_global_gui.h"
#include "yacreader_page_flow_rhi.h"

#include <QSettings>
#include <QWidget>

using namespace YACReader;

class QSettings;
class GoToFlowToolBar;
class QVBoxLayout;
class QKeyEvent;

class GoToFlowWidget : public QWidget, protected Themable
{
    Q_OBJECT

public:
    GoToFlowWidget(QWidget *parent = nullptr, FlowType flowType = CoverFlowLike);
    ~GoToFlowWidget() override;

public slots:
    void reset();
    void centerSlide(int slide);
    void setPageNumber(int page);
    void setFlowType(FlowType flowType);
    void setNumSlides(unsigned int slides);
    void setImageReady(int index, const QByteArray &image);
    void updateSize();
    void updateConfig(QSettings *settings);
    void setFlowRightToLeft(bool b);

signals:
    void goToPage(unsigned int);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void applyTheme(const Theme &theme) override;

private:
    QVBoxLayout *mainLayout;
    GoToFlowToolBar *toolBar;
    YACReaderPageFlow3D *flow;
    QSize imageSize;
};

#endif
