#include "notifications_label_widget.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

NotificationsLabelWidget::NotificationsLabelWidget(QWidget * parent)
	:QWidget(parent)
{
	setAttribute(Qt::WA_LayoutUsesWidgetRect,true);
	effect = new QGraphicsOpacityEffect(this);
	effect->setOpacity(1.0);

	effect2= new QGraphicsOpacityEffect(this);
	effect->setOpacity(1.0);

	anim = new QPropertyAnimation(effect,"opacity");
	anim->setDuration(500);
	anim->setStartValue(1.0);
	anim->setEndValue(0.0);
	anim->setEasingCurve(QEasingCurve::InExpo);

	anim2 = new QPropertyAnimation(effect2,"opacity");
	anim2->setDuration(500);
	anim2->setStartValue(1.0);
	anim2->setEndValue(0.0);
	anim2->setEasingCurve(QEasingCurve::InExpo);
	anim2->start();	

	connect(anim,SIGNAL(finished()),this,SLOT(hide()));

	imgLabel = new QLabel(this);
	QPixmap p(":/images/notificationsLabel.png");
	imgLabel->resize(p.size());
	imgLabel->setPixmap(p);
	imgLabel->setAttribute(Qt::WA_LayoutUsesWidgetRect,true);

	textLabel = new QLabel(this);
	textLabel->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
	textLabel->setStyleSheet("QLabel { color : white; font-size:24px; }");
	textLabel->setAttribute(Qt::WA_LayoutUsesWidgetRect,true);

	textLabel->setGeometry(imgLabel->geometry());
#ifndef Q_WS_MAC
	imgLabel->setGraphicsEffect(effect);
	textLabel->setGraphicsEffect(effect2);
#endif
	resize(p.size());
	updatePosition();

}

void NotificationsLabelWidget::flash()
{
	updatePosition();
	anim->stop();
	anim2->stop();
	anim->start();
	anim2->start();

	setVisible(true);
}

void NotificationsLabelWidget::setText(const QString & text)
{
	textLabel->setText(text);
	QRect geom = imgLabel->geometry();
	QSize size = geom.size();
	size.setHeight(size.height() - 10); //TODO remove this amazing magic number
	geom.setSize(size);
	textLabel->setGeometry(geom);
}

void NotificationsLabelWidget::updatePosition()
{
	QWidget * parent = dynamic_cast<QWidget *>(this->parent());
	if(parent == 0)
	{
		return;
	}
	move(QPoint((parent->geometry().size().width()-this->width())/2,(parent->geometry().size().height()-this->height())/2));
}