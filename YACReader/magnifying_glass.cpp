#include "magnifying_glass.h"
#include "viewer.h"
#include "configuration.h"



MagnifyingGlass::MagnifyingGlass(int w, int h, QWidget * parent)
:QLabel(parent),zoomLevel(0.5)
{
	setup(QSize(w,h));
}

MagnifyingGlass::MagnifyingGlass(const QSize & size, QWidget * parent)
:QLabel(parent),zoomLevel(0.5)
{
	setup(size);
}

void MagnifyingGlass::setup(const QSize & size)
{
	resize(size);
	setScaledContents(true);
	setMouseTracking(true);
	setCursor(QCursor(QBitmap(1,1),QBitmap(1,1)));
}

void MagnifyingGlass::mouseMoveEvent(QMouseEvent * event)
{
	updateImage();
}

void MagnifyingGlass::updateImage(int x, int y)
{
	//image section augmented
	int zoomWidth = static_cast<int>(width() * zoomLevel);
	int zoomHeight = static_cast<int>(height() * zoomLevel);
	Viewer * p = (Viewer *)parent();
	int currentPos = p->verticalScrollBar()->sliderPosition();
	const QPixmap * image = p->pixmap();
	int iWidth = image->width();
	int iHeight = image->height();
	float wFactor = static_cast<float>(iWidth) / p->widget()->width();
	float hFactor = static_cast<float>(iHeight) / p->widget()->height();
	zoomWidth *= wFactor;
	zoomHeight *= hFactor;
	if(p->verticalScrollBar()->minimum()==p->verticalScrollBar()->maximum())
	{
		int xp = static_cast<int>(((x-p->widget()->pos().x())*wFactor)-zoomWidth/2);
		int yp = static_cast<int>((y-p->widget()->pos().y()+currentPos)*hFactor-zoomHeight/2);
		int xOffset=0;
		int yOffset=0;
		int zw=zoomWidth;
		int zh=zoomHeight;
		//int wOffset,hOffset=0;
		bool outImage = false;
		if(xp<0)
		{
			xOffset = -xp;
			xp=0;
			zw = zw - xOffset;
			outImage = true;
		}
		if(yp<0)
		{
			yOffset = -yp;
			yp=0;
			zh = zh - yOffset;
			outImage = true;
		}
		
		if(xp+zoomWidth >= image->width())
		{
			zw -= xp+zw - image->width();
			outImage = true;
		}
		if(yp+zoomHeight >= image->height())
		{
			zh -= yp+zh - image->height();
			outImage = true;
		}
		if(outImage)
		{
			QImage img(zoomWidth,zoomHeight,QImage::Format_RGB32);
			img.fill(Configuration::getConfiguration().getBackgroundColor());
			if(zw>0&&zh>0)
			{
				QPainter painter(&img);
				painter.drawPixmap(xOffset,yOffset,p->pixmap()->copy(xp,yp,zw,zh));
			}
			setPixmap(QPixmap().fromImage(img));
		}
		else
			setPixmap(p->pixmap()->copy(xp,yp,zoomWidth,zoomHeight));
	}
	else
	{
		int xp = static_cast<int>(((x-p->widget()->pos().x())*wFactor)-zoomWidth/2);
		int yp = static_cast<int>((y+currentPos)*hFactor-zoomHeight/2);
		int xOffset=0;
		int yOffset=0;
		int zw=zoomWidth;
		int zh=zoomHeight;
		//int wOffset,hOffset=0;
		bool outImage = false;
		if(xp<0)
		{
			xOffset = -xp;
			xp=0;
			zw = zw - xOffset;
			outImage = true;
		}
		if(yp<0)
		{
			yOffset = -yp;
			yp=0;
			zh = zh - yOffset;
			outImage = true;
		}
		
		if(xp+zoomWidth >= image->width())
		{
			zw -= xp+zw - image->width();
			outImage = true;
		}
		if(yp+zoomHeight >= image->height())
		{
			zh -= yp+zh - image->height();
			outImage = true;
		}
		if(outImage)
		{
			QImage img(zoomWidth,zoomHeight,QImage::Format_RGB32);
			img.fill(Configuration::getConfiguration().getBackgroundColor());
			if(zw>0&&zh>0)
			{
				QPainter painter(&img);
				painter.drawPixmap(xOffset,yOffset,p->pixmap()->copy(xp,yp,zw,zh));
			}
			setPixmap(QPixmap().fromImage(img));
		}
		else
			setPixmap(p->pixmap()->copy(xp,yp,zoomWidth,zoomHeight));
	}
	move(static_cast<int>(x-float(width())/2),static_cast<int>(y-float(height())/2));
}

void MagnifyingGlass::updateImage()
{
	if(isVisible())
	{
		QPoint p = QPoint(cursor().pos().x(),cursor().pos().y());
		p = this->parentWidget()->mapFromGlobal(p);
		updateImage(p.x(),p.y());
	}
}
void MagnifyingGlass::wheelEvent(QWheelEvent * event)
{
    switch(event->modifiers())
    {
	//size
	case Qt::NoModifier:
	    if(event->delta()<0)
		sizeUp();
	    else
		sizeDown();
	break;
	//size height
	case Qt::ControlModifier:
	    if(event->delta()<0)
		heightUp();
	    else
		heightDown();
	break;
	//size width
	case Qt::AltModifier:
	    if(event->delta()<0)
		widthUp();
	    else
		widthDown();
	break;
	//zoom level
	case Qt::ShiftModifier:
	    if(event->delta()<0)
		zoomIn();
	    else
		zoomOut();
	break;
    }
    updateImage();
    event->setAccepted(true);
}
void MagnifyingGlass::zoomIn()
{
	if(zoomLevel>0.2)
		zoomLevel -= 0.025;
}

void MagnifyingGlass::zoomOut()
{
	if(zoomLevel<0.9)
		zoomLevel += 0.025;
}

void MagnifyingGlass::sizeUp()
{
	Viewer * p = (Viewer *)parent();
	if(width()<(p->width()*0.90))
		resize(width()+30,height()+15);
}

void MagnifyingGlass::sizeDown()
{
	if(width()>175)
		resize(width()-30,height()-15);
}

void MagnifyingGlass::heightUp()
{
    Viewer * p = (Viewer *)parent();
    if(height()<(p->height()*0.90))
	    resize(width(),height()+15);
}

void MagnifyingGlass::heightDown()
{
    if(height()>80)
	    resize(width(),height()-15);
}

void MagnifyingGlass::widthUp()
{
    Viewer * p = (Viewer *)parent();
    if(width()<(p->width()*0.90))
	    resize(width()+30,height());
}

void MagnifyingGlass::widthDown()
{
    if(width()>175)
	    resize(width()-30,height());
}

void MagnifyingGlass::keyPressEvent(QKeyEvent *event)
{
	bool validKey = false;
	switch (event->key())
	{
	case Qt::Key_Plus:
		sizeUp();
		validKey = true;
		break;
	case Qt::Key_Minus:
		sizeDown();
		validKey = true;
		break;
	case Qt::Key_Underscore:
		zoomOut();
		validKey = true;
		break;
	case Qt::Key_Asterisk:
		zoomIn();
		validKey = true;
		break;
	}
	updateImage();
	if(validKey)
		event->setAccepted(true);
}
