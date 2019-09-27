#include "width_slider.h"

#include <QtWidgets>

#include "configuration.h"

YACReaderSliderAction::YACReaderSliderAction(QWidget *parent)
    : QWidgetAction(parent)
{

    widget = new YACReaderSlider();
    setDefaultWidget(widget);

    connect(widget, SIGNAL(zoomRatioChanged(int)), this, SIGNAL(zoomRatioChanged(int)));
}

void YACReaderSliderAction::updateText(int value)
{
    widget->updateText(value);
}

void YACReaderSliderAction::updateZoomRatio(int value)
{
    widget->updateZoomRatio(value);
}

YACReaderSlider::YACReaderSlider(QWidget *parent)
    : QWidget(parent)
{
    const int sliderWidth = 200;
    const int contentsMargin = 10;
    const int elementsSpacing = 10;
    const int percentageLabelWidth = 30;

    setFocusPolicy(Qt::StrongFocus);

    auto pLayout = new QHBoxLayout();

    pLayout->addStretch();

    percentageLabel = new QLabel();
    percentageLabel->setStyleSheet("QLabel { color : white; }");
    percentageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    slider = new QSlider();
    slider->setOrientation(Qt::Horizontal);

    slider->setMinimumWidth(sliderWidth);

    QPushButton *resetButton = new QPushButton(tr("Reset"));
    resetButton->setStyleSheet("QPushButton {border: 1px solid #BB242424; background: #BB2E2E2E; color:white; padding: 3px 5px 5px 5px;}");
    connect(resetButton, &QPushButton::clicked, this, &YACReaderSlider::resetValueToDefault);

    pLayout->addWidget(percentageLabel, 1, Qt::AlignHCenter);
    pLayout->addWidget(slider, 0, Qt::AlignHCenter | Qt::AlignBottom);
    pLayout->addWidget(resetButton, 1, Qt::AlignHCenter | Qt::AlignBottom);
    pLayout->setSpacing(elementsSpacing);

    pLayout->setMargin(0);

    setLayout(pLayout);
    setAutoFillBackground(false);

    setContentsMargins(contentsMargin, contentsMargin, contentsMargin, contentsMargin);
    setFixedSize(sliderWidth + 2 * contentsMargin + 2 * elementsSpacing + percentageLabelWidth + resetButton->sizeHint().width(), 45);

    slider->setMinimum(30);
    slider->setMaximum(500);
    slider->setPageStep(5);

    slider->setFocusPolicy(Qt::NoFocus);
    resetButton->setFocusPolicy(Qt::NoFocus);

    slider->setValue(100);
    percentageLabel->setText(QString("%1%").arg(100));
    connect(slider, &QSlider::valueChanged, this, &YACReaderSlider::updateText);
}

void YACReaderSlider::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.fillRect(0, 0, width(), height(), QColor("#BB000000"));
}

void YACReaderSlider::show()
{
    QWidget::show();
    setFocus();
}

void YACReaderSlider::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    hide();
}

void YACReaderSlider::updateText(int value)
{
    percentageLabel->setText(QString("%1%").arg(value));
    emit zoomRatioChanged(value);
}

void YACReaderSlider::updateZoomRatio(int value)
{
    slider->setValue(value);
    percentageLabel->setText(QString("%1%").arg(value));
}

void YACReaderSlider::resetValueToDefault()
{
    slider->setValue(100);
}
