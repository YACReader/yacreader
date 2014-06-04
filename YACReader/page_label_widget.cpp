#include "page_label_widget.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QApplication>
#include <QDesktopWidget>

PageLabelWidget::PageLabelWidget(QWidget * parent)
	:QWidget(parent)
	{
	animation = new QPropertyAnimation(this,"pos");
    animation->setDuration(150);
	
	int verticalRes = QApplication::desktop()->screenGeometry().height();
		
	imgLabel = new QLabel(this);
	QPixmap p;
	if (verticalRes <= 1024)
		p.load(":/images/numPagesLabel.png");
	else if (verticalRes <= 1200)
		p.load(":/images/numPagesLabelMedium.png");
	else
		p.load(":/images/numPagesLabelBig.png");
	imgLabel->resize(p.size());
	imgLabel->setPixmap(p);
	
	textLabel = new QLabel(this);
	textLabel->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
	if(verticalRes <= 1024)
		textLabel->setStyleSheet("QLabel { color : white; font-size:12px; padding-left:8px; }");
	else if (verticalRes <= 1200)
		textLabel->setStyleSheet("QLabel { color : white; font-size:16px; padding-left:8px;}");
	else
		textLabel->setStyleSheet("QLabel { color : white; font-size:20px; padding-left:8px; }");

	//informationLabel->setAutoFillBackground(true);
	//textLabel->setFont(QFont("courier new bold", 12));
	//textLabel->resize(100,25);

	resize(p.size());
	//por defecto aparece oculto
	if(parent != 0)
		move(QPoint((parent->geometry().size().width()-this->width()),-this->height()));
	/*QSize size = textLabel->sizeHint();

	int w = width();   // returns screen width
	int h = height();  // returns screen height
	int mw = size.width();
	int mh = size.height();
	int cw = (w-mw)/2;
	int ch = 0;
	textLabel->move(cw,ch);*/
	}

void PageLabelWidget::show()
{
	if(this->pos().y() <= 0  && animation->state()!=QPropertyAnimation::Running)
	{
		QWidget * parent = dynamic_cast<QWidget *>(this->parent());
		if(parent == 0)
		{
			return;
		}

		QWidget::show();
		//connect(animation,SIGNAL(finished()),this,SLOT(QWidget::hide()));
		animation->disconnect();

		animation->setStartValue(QPoint((parent->geometry().size().width()-this->width()),-this->height()));
		animation->setEndValue(QPoint((parent->geometry().size().width()-this->width()),0));
		animation->start();
	}
}

void PageLabelWidget::hide()
{

	if(this->pos().y() >= 0 && animation->state()!=QPropertyAnimation::Running)
	{
		QWidget * parent = dynamic_cast<QWidget *>(this->parent());
		if(parent == 0)
		{
			return;
		}
		//connect(animation,SIGNAL(finished()),this,SLOT(setHidden()));
		animation->setStartValue(QPoint((parent->geometry().size().width()-this->width()),0));
		animation->setEndValue(QPoint((parent->geometry().size().width()-this->width()),-this->height()));
		animation->start();
	}
}

void PageLabelWidget::setText(const QString & text)
{
	textLabel->setText(text);
	QRect geom = imgLabel->geometry();
	QSize size = geom.size();
	size.setHeight(size.height() - 10);//TODO remove this amazing magic number
	geom.setSize(size);
	textLabel->setGeometry(geom);
}

/*void PageLabelWidget::resizeEvent(QResizeEvent * event)
{
	move(QPoint((((QWidget *) parent())->geometry().size().width()-this->width())/2,0));
}*/

void PageLabelWidget::updatePosition()
{
	QWidget * parent = dynamic_cast<QWidget *>(this->parent());
	if(parent == 0)
	{
        return;
	}

    animation->stop();
    if (animation->endValue().toPoint().y() == 0)
        move(QPoint((parent->geometry().size().width()-this->width()),0));
    else
       move(QPoint((parent->geometry().size().width()-this->width()),-this->height()));
}
