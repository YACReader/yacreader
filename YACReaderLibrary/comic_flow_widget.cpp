#include "comic_flow_widget.h"
#include <QVBoxLayout>

ComicFlowWidget::ComicFlowWidget(QWidget *parent)
    : QWidget(parent)
{
    flow = new YACReaderComicFlow3D(this);

    connect(flow, &YACReaderComicFlow3D::centerIndexChanged, this, &ComicFlowWidget::centerIndexChanged);
    connect(flow, &YACReaderComicFlow3D::selected, this, &ComicFlowWidget::selected);

    auto l = new QVBoxLayout;
    l->addWidget(flow);
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    QPalette Pal(palette());
    Pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(Pal);
}

QSize ComicFlowWidget::minimumSizeHint() const
{
    return flow->minimumSizeHint();
}

QSize ComicFlowWidget::sizeHint() const
{
    return flow->sizeHint();
}

void ComicFlowWidget::setShowMarks(bool value)
{
    flow->setShowMarks(value);
}

void ComicFlowWidget::setMarks(QVector<YACReader::YACReaderComicReadStatus> marks)
{
    flow->setMarks(marks);
}

void ComicFlowWidget::setMarkImage(QImage &image)
{
    flow->setMarkImage(image);
}

void ComicFlowWidget::markSlide(int index, YACReader::YACReaderComicReadStatus status)
{
    flow->markSlide(index, status);
}

void ComicFlowWidget::unmarkSlide(int index)
{
    flow->unmarkSlide(index);
}

void ComicFlowWidget::setSlideSize(QSize size)
{
    flow->setSlideSize(size);
}

void ComicFlowWidget::clear()
{
    flow->clear();
}

void ComicFlowWidget::setImagePaths(QStringList paths)
{
    flow->setImagePaths(paths);
}

void ComicFlowWidget::setCenterIndex(int index)
{
    flow->setCenterIndex(index);
}

void ComicFlowWidget::showSlide(int index)
{
    flow->showSlide(index);
}

int ComicFlowWidget::centerIndex()
{
    return flow->centerIndex();
}

void ComicFlowWidget::updateMarks()
{
    flow->updateMarks();
}

void ComicFlowWidget::setFlowType(YACReader::FlowType flowType)
{
    if (flowType == YACReader::CoverFlowLike)
        flow->setPreset(presetYACReaderFlowClassicConfig);
    else if (flowType == YACReader::Strip)
        flow->setPreset(presetYACReaderFlowStripeConfig);
    else if (flowType == YACReader::StripOverlapped)
        flow->setPreset(presetYACReaderFlowOverlappedStripeConfig);
    else
        flow->setPreset(defaultYACReaderFlowConfig);
}

void ComicFlowWidget::render()
{
    flow->render();
}

void ComicFlowWidget::keyPressEvent(QKeyEvent *event)
{
    flow->keyPressEvent(event);
}

void ComicFlowWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void ComicFlowWidget::mousePressEvent(QMouseEvent *event)
{
    flow->mousePressEvent(event);
}

void ComicFlowWidget::resizeEvent(QResizeEvent *event)
{
    flow->resizeGL(event->size().width(), event->size().height());
}

void ComicFlowWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    flow->mouseDoubleClickEvent(event);
}

void ComicFlowWidget::updateConfig(QSettings *settings)
{
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

    flow->setPerformance(performance);
    if (!settings->contains(V_SYNC))
        flow->useVSync(false);
    else
        flow->useVSync(settings->value(V_SYNC).toBool());

    switch (settings->value(FLOW_TYPE_GL).toInt()) {
    case 0:
        flow->setPreset(presetYACReaderFlowClassicConfig);
        return;
    case 1:
        flow->setPreset(presetYACReaderFlowStripeConfig);
        return;
    case 2:
        flow->setPreset(presetYACReaderFlowOverlappedStripeConfig);
        return;
    case 3:
        flow->setPreset(defaultYACReaderFlowConfig);
        return;
    case 4:
        flow->setPreset(pressetYACReaderFlowDownConfig);
        return;
    }

    // custom config
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
}

void ComicFlowWidget::add(const QString &path, int index)
{
    flow->add(path, index);
}

void ComicFlowWidget::remove(int cover)
{
    flow->remove(cover);
}

void ComicFlowWidget::resortCovers(QList<int> newOrder)
{
    flow->resortCovers(newOrder);
}
