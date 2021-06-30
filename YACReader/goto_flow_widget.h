#ifndef __GOTO_FLOW_WIDGET_H
#define __GOTO_FLOW_WIDGET_H

#include <QWidget>
#include <QSettings>
#include "yacreader_global_gui.h"

using namespace YACReader;

class QSettings;
class GoToFlowToolBar;
class QVBoxLayout;

class GoToFlowWidget : public QWidget
{
    Q_OBJECT
protected:
    QVBoxLayout *mainLayout;
    GoToFlowToolBar *toolBar;

public:
    GoToFlowWidget(QWidget *paret = nullptr);
    ~GoToFlowWidget() override = 0;
public slots:
    virtual void reset() = 0;
    virtual void centerSlide(int slide) = 0;
    virtual void setPageNumber(int page);
    virtual void setFlowType(FlowType flowType) = 0;
    virtual void setNumSlides(unsigned int slides) = 0;
    virtual void setImageReady(int index, const QByteArray &image) = 0;
    virtual void updateSize();
    virtual void updateConfig(QSettings *settings);
    virtual void setFlowRightToLeft(bool b) = 0;
signals:
    void goToPage(unsigned int);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    //bool eventFilter(QObject *, QEvent *);
};

#endif
