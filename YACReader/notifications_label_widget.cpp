#include "notifications_label_widget.h"

#include <QtWidgets>

NotificationsLabelWidget::NotificationsLabelWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    setAttribute(Qt::WA_LayoutUsesWidgetRect, true);
    effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(1.0);

    anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(500);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InExpo);

    connect(anim, SIGNAL(finished()), this, SLOT(hide()));

    textLabel = new QLabel(this);
    textLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    textLabel->setStyleSheet("QLabel { color : white; font-size:24px; }");
    textLabel->setAttribute(Qt::WA_LayoutUsesWidgetRect, true);

    textLabel->setFixedSize(200, 120);

    //TODO check if the effects still be broken in OSX yet
#ifndef Q_OS_MAC
    this->setGraphicsEffect(effect);
#endif

    layout->addWidget(textLabel);
    setLayout(layout);

    setFixedSize(200, 120);
    updatePosition();
}

void NotificationsLabelWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, width(), height()), 5.0, 5.0);
    painter.setPen(Qt::NoPen);
    painter.fillPath(path, QColor("#BB000000"));
    painter.drawPath(path);
}

void NotificationsLabelWidget::flash()
{
    updatePosition();
    anim->stop();
    anim->start();

    setVisible(true);
}

void NotificationsLabelWidget::setText(const QString &text)
{
    textLabel->setText(text);
}

void NotificationsLabelWidget::updatePosition()
{
    QWidget *parent = dynamic_cast<QWidget *>(this->parent());
    if (parent == nullptr) {
        return;
    }
    move(QPoint((parent->geometry().size().width() - this->width()) / 2, (parent->geometry().size().height() - this->height()) / 2));
}
