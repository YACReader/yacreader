#include "yacreader_sidebar.h"

#include <QPainter>
#include <QLayout>

YACReaderSideBar::YACReaderSideBar(QWidget *parent) :
    QWidget(parent)
{
	setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);
}


void YACReaderSideBar::paintEvent(QPaintEvent * event)
{
#ifdef Q_OS_MAC
	QPainter painter(this);
	
	QLinearGradient lG(0,0,0,height());
	lG.setColorAt(0,QColor("#E8ECF1"));
	lG.setColorAt(1,QColor("#D1D8E0"));

	painter.fillRect(0,0,width(),height(),lG);
#else
	QWidget::paintEvent(event);
#endif

	//QPixmap shadow(":/images/side_bar/shadow.png");
	//painter.drawPixmap(width()-shadow.width(),0,shadow.width(),height(),shadow);

	//   painter.setRenderHint(QPainter::Antialiasing);
    // painter.drawLine(rect().topLeft(), rect().bottomRight());

	//QWidget::paintEvent(event);
}

QSize YACReaderSideBar::sizeHint() const
{
	return QSize(225,200);
}