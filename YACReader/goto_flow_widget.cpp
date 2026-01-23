#include "goto_flow_widget.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QCoreApplication>
#include <QApplication>
#include <QSettings>

#include "goto_flow_toolbar.h"
#include "configuration.h"

GoToFlowWidget::GoToFlowWidget(QWidget *parent, FlowType flowType)
    : QWidget(parent)
{
    Q_UNUSED(flowType)

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    toolBar = new GoToFlowToolBar(this);

    setLayout(mainLayout);

    flow = new YACReaderPageFlow3D(this);
    flow->setShowMarks(false);

    imageSize = Configuration::getConfiguration().getGotoSlideSize();

    flow->setSlideSize(imageSize);
    connect(flow, &YACReaderPageFlow3D::centerIndexChanged, this, &GoToFlowWidget::setPageNumber);
    connect(flow, &YACReaderPageFlow3D::selected, this, &GoToFlowWidget::goToPage);

    connect(toolBar, &GoToFlowToolBar::goToPage, this, &GoToFlowWidget::goToPage);
    connect(toolBar, &GoToFlowToolBar::setCenter, flow, &YACReaderPageFlow3D::setCenterIndex);

    mainLayout->addWidget(flow);
    toolBar->raise();

    resize(static_cast<int>(5 * imageSize.width()), toolBar->height() + static_cast<int>(imageSize.height() * 1.7));

    this->setCursor(QCursor(Qt::ArrowCursor));
}

GoToFlowWidget::~GoToFlowWidget()
{
    delete flow;
}

void GoToFlowWidget::reset()
{
    flow->reset();
}

void GoToFlowWidget::centerSlide(int slide)
{
    if (flow->centerIndex() != slide) {
        flow->setCenterIndex(slide);
    }
}

void GoToFlowWidget::setPageNumber(int page)
{
    toolBar->setPage(page);
}

void GoToFlowWidget::setFlowType(FlowType flowType)
{
    if (flowType == CoverFlowLike)
        flow->setPreset(presetYACReaderFlowClassicConfig);
    else if (flowType == Strip)
        flow->setPreset(presetYACReaderFlowStripeConfig);
    else if (flowType == StripOverlapped)
        flow->setPreset(presetYACReaderFlowOverlappedStripeConfig);
    else
        flow->setPreset(defaultYACReaderFlowConfig);
}

void GoToFlowWidget::setNumSlides(unsigned int slides)
{
    flow->populate(slides);
    toolBar->setTop(slides);
}

void GoToFlowWidget::setImageReady(int index, const QByteArray &imageData)
{
    flow->rawImages[index] = imageData;
    flow->imagesReady[index] = true;
}

void GoToFlowWidget::updateSize()
{
    if (Configuration::getConfiguration().getQuickNaviMode() && parentWidget() != nullptr)
        resize(parentWidget()->width(), height());
}

void GoToFlowWidget::updateConfig(QSettings *settings)
{
    toolBar->updateOptions();

    Performance performance = medium;
    switch (settings->value(PERFORMANCE).toInt()) {
    case 0:
        performance = low;
        break;
    case 1:
        performance = medium;
        break;
    case 2:
        performance = high;
        break;
    case 3:
        performance = ultraHigh;
        break;
    }

    imageSize = Configuration::getConfiguration().getGotoSlideSize();
    resize(5 * imageSize.width(), toolBar->height() + imageSize.height() * 1.7);
    updateSize();

    flow->setPerformance(performance);

    switch (settings->value(FLOW_TYPE_GL).toInt()) {
    case FlowType::CoverFlowLike:
        flow->setPreset(presetYACReaderFlowClassicConfig);
        break;
    case FlowType::Strip:
        flow->setPreset(presetYACReaderFlowStripeConfig);
        break;
    case FlowType::StripOverlapped:
        flow->setPreset(presetYACReaderFlowOverlappedStripeConfig);
        break;
    case FlowType::Modern:
        flow->setPreset(defaultYACReaderFlowConfig);
        break;
    case FlowType::Roulette:
        flow->setPreset(pressetYACReaderFlowDownConfig);
        break;
    case FlowType::Custom:
        flow->setCF_RX(settings->value(X_ROTATION).toInt());
        flow->setCF_Y(settings->value(Y_POSITION).toInt());
        flow->setX_Distance(settings->value(COVER_DISTANCE).toInt());
        flow->setCenter_Distance(settings->value(CENTRAL_DISTANCE).toInt());
        flow->setCF_Z(settings->value(ZOOM_LEVEL).toInt());
        flow->setY_Distance(settings->value(Y_COVER_OFFSET).toInt());
        flow->setZ_Distance(settings->value(Z_COVER_OFFSET).toInt());
        flow->setRotation(settings->value(COVER_ROTATION).toInt());
        flow->setFadeOutDist(settings->value(FADE_OUT_DIST).toInt());
        flow->setLightStrenght(settings->value(LIGHT_STRENGTH).toInt());
        flow->setMaxAngle(settings->value(MAX_ANGLE).toInt());
        break;
    }
    if (Configuration::getConfiguration().getQuickNaviMode())
        flow->setFadeOutDist(20);
}

void GoToFlowWidget::setFlowRightToLeft(bool b)
{
    flow->setFlowRightToLeft(b);
}

void GoToFlowWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
        QApplication::sendEvent(flow, event);
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        toolBar->goTo();
        toolBar->centerSlide();
        break;
    case Qt::Key_Space:
        toolBar->centerSlide();
        break;
    case Qt::Key_S:
        QCoreApplication::sendEvent(this->parent(), event);
        break;
    }

    event->accept();
}

void GoToFlowWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    toolBar->move(0, event->size().height() - toolBar->height());
    toolBar->setFixedWidth(width());
}
