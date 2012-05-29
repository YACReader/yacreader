#include "properties_dialog.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QSizePolicy>


PropertiesDialog::PropertiesDialog(QWidget * parent)
:QDialog(parent)
{
	QHBoxLayout * l = new QHBoxLayout();

	sa = new QScrollArea();
	_cover = new QLabel();
	_cover->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	_cover->setScaledContents(true);
	_cover->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
	

	sa->setWidget(_cover);
	sa->setBackgroundRole(QPalette::Dark);
	sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	sa->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	sa->setFrameStyle(QFrame::NoFrame);
	sa->setAlignment(Qt::AlignCenter);

	QVBoxLayout * coverLayout = new QVBoxLayout();
	coverLayout->addWidget(sa);
	//coverLayout->addStretch();

	l->addLayout(coverLayout);

	QVBoxLayout * info = new QVBoxLayout();
	info->addWidget(_name = new QLabel("name"));
	info->addWidget(_pages = new QLabel("pages"));
	info->addWidget(_size = new QLabel("size"));
	info->addStretch();
	//coverLayout->setSizeConstraint(QLayout::SetMaximumSize);
	l->addLayout(info);
	l->addStretch();
	//l->setSizeConstraint(QLayout::SetNoConstraint);
	this->setLayout(l);
	this->setWindowTitle(tr("Comic properties"));

	int heightDesktopResolution = QApplication::desktop()->screenGeometry().height();
	int widthDesktopResolution = QApplication::desktop()->screenGeometry().width();
	int sHeight,sWidth;
	sHeight = static_cast<int>(heightDesktopResolution*0.5);
	sWidth = static_cast<int>(sHeight*1.4);
	this->resize(sWidth,sHeight);
}

void PropertiesDialog::setComics(QList<Comic> comics)
{

}

void PropertiesDialog::updateComics(QList<Comic> comics)
{

}
//Deprecated
void PropertiesDialog::setCover(const QPixmap & cover)
{
	_cover->setPixmap(cover);
	float aspectRatio = (float)cover.width()/cover.height();
	int heightDesktopResolution = QApplication::desktop()->screenGeometry().height();
	int widthDesktopResolution = QApplication::desktop()->screenGeometry().width();
	int sHeight,sWidth;
	sHeight = static_cast<int>(heightDesktopResolution*0.5);

	if(aspectRatio<1)
	{
		sWidth = static_cast<int>(sHeight*1.4);
		//this->resize(sWidth,sHeight);
		this->move(QPoint((widthDesktopResolution-sWidth)/2,((heightDesktopResolution-sHeight)-40)/2));
		_cover->resize(static_cast<int>((sa->height())*aspectRatio),
			(sa->height()));
	}
	else
	{
		sWidth = static_cast<int>(sHeight/1.16);
		//this->resize(sWidth,sHeight);
		this->move(QPoint((widthDesktopResolution-sWidth)/2,((heightDesktopResolution-sHeight)-40)/2));
		_cover->resize((width()-25),
			static_cast<int>((width()-25)/aspectRatio));
	}
}
void PropertiesDialog::setFilename(const QString & name)
{
	_name->setText(tr("Name : ") + name);
}
void PropertiesDialog::setNumpages(int pages)
{
	_pages->setText(tr("Number of pages : ") + QString::number(pages));
}
void PropertiesDialog::setSize(float size)
{
	 
	_size->setText(tr("Size : ") + QString::number(size,'f',2) + " MB");
}