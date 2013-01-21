#include "page_label_widget.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QPropertyAnimation>

PageLabelWidget::PageLabelWidget(QWidget * parent)
	:QWidget(parent)
	{
	animation = new QPropertyAnimation(this,"pos");
	animation->setDuration(150);
		
		
	imgLabel = new QLabel(this);
	QPixmap p(":/images/numPagesLabel.png");
	imgLabel->resize(p.size());
	imgLabel->setPixmap(QPixmap(":/images/numPagesLabel.png"));
	
	textLabel = new QLabel(this);
	textLabel->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
	textLabel->setStyleSheet("QLabel { color : white; }");
	//informationLabel->setAutoFillBackground(true);
	//textLabel->setFont(QFont("courier new bold", 12));
	//textLabel->resize(100,25);

	textLabel->setGeometry(imgLabel->geometry());

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

		animation->setStartValue(QPoint((parent->geometry().size().width()-this->width())/2,-this->height()));
		animation->setEndValue(QPoint((parent->geometry().size().width()-this->width())/2,0));
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
		animation->setStartValue(QPoint((parent->geometry().size().width()-this->width())/2,0));
		animation->setEndValue(QPoint((parent->geometry().size().width()-this->width())/2,-this->height()));
		animation->start();
	}
}

void PageLabelWidget::setText(const QString & text)
{
	textLabel->setText(text);
	QRect geom = imgLabel->geometry();
	QSize size = geom.size();
	size.setHeight(size.height() - 10); //TODO remove this amazing magic number
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
	move(QPoint((parent->geometry().size().width()-this->width())/2,this->pos().y()));
}