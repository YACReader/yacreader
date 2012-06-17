#include "properties_dialog.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QSizePolicy>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QPlainTextEdit>

PropertiesDialog::PropertiesDialog(QWidget * parent)
:QDialog(parent)
{

	createCoverBox();
    createGeneralInfoBox();
    createAuthorsBox();
	createPublishingBox();
	createButtonBox();
	createPlotBox();

	createTabBar();

	mainLayout = new QGridLayout;
	mainLayout->addWidget(coverBox,0,0);
	mainLayout->addWidget(tabBar,0,1);
	mainLayout->setColumnStretch(1,1);
	/*mainLayout->addWidget(authorsBox,1,1);
	mainLayout->addWidget(publishingBox,2,1);*/
	mainLayout->addWidget(buttonBox,1,1,Qt::AlignBottom);

	this->setLayout(mainLayout);
	mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
	this->setWindowTitle(tr("Comic properties"));

	int heightDesktopResolution = QApplication::desktop()->screenGeometry().height();
	int widthDesktopResolution = QApplication::desktop()->screenGeometry().width();
	int sHeight,sWidth;
	sHeight = static_cast<int>(heightDesktopResolution*0.65);
	sWidth = static_cast<int>(sHeight*1.4);
	setCover(QPixmap(":/images/notCover.png"));
	this->resize(sWidth,this->height());
	this->move(QPoint((widthDesktopResolution-sWidth)/2,((heightDesktopResolution-sHeight)-40)/2));
	repaint();
}

void PropertiesDialog::createTabBar()
{
	tabBar = new QTabWidget;
	tabBar->addTab(generalInfoBox,tr("General info"));
	tabBar->addTab(authorsBox,tr("Authors"));
	tabBar->addTab(publishingBox,tr("Publishing"));
	tabBar->addTab(plotBox,tr("Plot"));
}

void PropertiesDialog::createCoverBox()
{
	coverBox = new QGroupBox(tr("Cover"));

	sa = new QScrollArea();
	cover = new QLabel();
	cover->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	cover->setScaledContents(false);
	cover->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
	

	/*sa->setWidget(cover);
	sa->setBackgroundRole(QPalette::Dark);
	sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	sa->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	sa->setFrameStyle(QFrame::NoFrame);
	sa->setAlignment(Qt::AlignCenter);*/

	QVBoxLayout * coverLayout = new QVBoxLayout();
	coverLayout->addWidget(cover);
	coverLayout->addWidget(new QLineEdit());

	coverBox->setLayout(coverLayout);
}

QFrame * createLine()
{
		QFrame * line = new QFrame();
    line->setObjectName(QString::fromUtf8("line"));
    //line->setGeometry(QRect(320, 150, 118, 3));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

	return line;
}

void PropertiesDialog::createGeneralInfoBox()
{
	generalInfoBox = new QWidget;

	QFormLayout *generalInfoLayout = new QFormLayout;





	//generalInfoLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
	generalInfoLayout->addRow(tr("Title:"), title = new QLineEdit());
	

	QHBoxLayout * number = new QHBoxLayout;
	number->addWidget(new QLineEdit());
	number->addWidget(new QLabel("Bis:"));
	number->addWidget(new QCheckBox());
	number->addWidget(new QLabel("of:"));
	number->addWidget(new QLineEdit());
	number->addStretch(1);
	/*generalInfoLayout->addRow(tr("&Issue number:"), );
	generalInfoLayout->addRow(tr("&Bis:"), );*/
	generalInfoLayout->addRow(tr("Issue number:"), number);
	
	generalInfoLayout->addRow(tr("&Volume:"), pages = new QLineEdit());

	QHBoxLayout * arc = new QHBoxLayout;
	arc->addWidget(new QLineEdit());
	arc->addWidget(new QLabel("Arc number:"));
	arc->addWidget(new QLineEdit());
	arc->addWidget(new QLabel("of:"));
	arc->addWidget(new QLineEdit());
	arc->addStretch(1);
	generalInfoLayout->addRow(tr("&Story arc:"), arc);
	
	generalInfoLayout->addRow(tr("&Genere:"),  new QLineEdit());
	
	generalInfoLayout->addRow(tr("&Size:"), size = new QLabel("size"));

	generalInfoBox->setLayout(generalInfoLayout);
}

void PropertiesDialog::createAuthorsBox()
{
	authorsBox = new QWidget;
	
	QVBoxLayout *authorsLayout = new QVBoxLayout;

	//authorsLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
	QHBoxLayout * h1 = new QHBoxLayout;
	QVBoxLayout * vl1 = new QVBoxLayout;
	QVBoxLayout * vr1 = new QVBoxLayout;
	vl1->addWidget(new QLabel(tr("Writer(s):")));
	vl1->addWidget(new QPlainTextEdit());
	h1->addLayout(vl1);
	vr1->addWidget(new QLabel(tr("Penciller(s):")));
	vr1->addWidget(new QPlainTextEdit());
	h1->addLayout(vr1);
	//authorsLayout->addRow(tr("Writer(s):"), new QPlainTextEdit());
	//authorsLayout->addRow(tr("Penciller(s):"), new QPlainTextEdit());
	QHBoxLayout * h2 = new QHBoxLayout;
	QVBoxLayout * vl2 = new QVBoxLayout;
	QVBoxLayout * vr2 = new QVBoxLayout;
	vl2->addWidget(new QLabel(tr("Inker(s):")));
	vl2->addWidget(new QPlainTextEdit());
	h2->addLayout(vl2);
	vr2->addWidget(new QLabel(tr("Colorist(s):")));
	vr2->addWidget(new QPlainTextEdit());
	h2->addLayout(vr2);
	
	//authorsLayout->addRow(tr("Inker(s):"), new QPlainTextEdit());
	//authorsLayout->addRow(tr("Colorist(s):"), new QPlainTextEdit());

	QHBoxLayout * h3 = new QHBoxLayout;
	QVBoxLayout * vl3 = new QVBoxLayout;
	QVBoxLayout * vr3 = new QVBoxLayout;
	vl3->addWidget(new QLabel(tr("Letterer(es):")));
	vl3->addWidget(new QPlainTextEdit());
	h3->addLayout(vl3);
	vr3->addWidget(new QLabel(tr("Cover Artist(s):")));
	vr3->addWidget(new QPlainTextEdit());
	h3->addLayout(vr3);
	//authorsLayout->addRow(tr("Letterer(es):"), new QPlainTextEdit());
	//authorsLayout->addRow(tr("Cover Artist(s):"), new QPlainTextEdit());

	authorsLayout->addLayout(h1);
	authorsLayout->addLayout(h2);
	authorsLayout->addLayout(h3);
	authorsLayout->addStretch(1);
	authorsBox->setLayout(authorsLayout);

}

void PropertiesDialog::createPublishingBox()
{
	publishingBox = new QWidget;
	
	QFormLayout *publishingLayout = new QFormLayout;

	publishingLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
	publishingLayout->addRow(tr("Year:"), new QLineEdit());
	publishingLayout->addRow(tr("Month:"), new QLineEdit());
	publishingLayout->addRow(tr("Publisher:"), new QLineEdit());
	publishingLayout->addRow(tr("Format:"), new QLineEdit());
	publishingLayout->addRow(tr("Color/BW:"), new QLineEdit());
	publishingLayout->addRow(tr("Age rating:"), new QLineEdit());

	publishingBox->setLayout(publishingLayout);
}

void PropertiesDialog::createPlotBox()
{
	plotBox = new QWidget;

	QFormLayout *plotLayout = new QFormLayout;

	plotLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
	plotLayout->addRow(tr("Synopsis:"), new QPlainTextEdit());
	plotLayout->addRow(tr("Characters:"), new QPlainTextEdit());
	plotLayout->addRow(tr("Notes:"), new QPlainTextEdit());

	plotBox->setLayout(plotLayout);

}

void PropertiesDialog::createButtonBox()
{
    buttonBox = new QDialogButtonBox;

    closeButton = buttonBox->addButton(QDialogButtonBox::Close);
	saveButton = buttonBox->addButton(QDialogButtonBox::Save);
    //rotateWidgetsButton = buttonBox->addButton(tr("Rotate &Widgets"),QDialogButtonBox::ActionRole);

    //connect(rotateWidgetsButton, SIGNAL(clicked()), this, SLOT(rotateWidgets()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
}

void PropertiesDialog::setComics(QList<Comic> comics)
{
	if(comics.length() > 1)
	{
	}
	else
	{
		Comic comic = comics.at(0);
		title->setText(comic.name);
	}
}



void PropertiesDialog::updateComics(QList<Comic> comics)
{

}
//Deprecated
void PropertiesDialog::setCover(const QPixmap & coverImage)
{
	cover->setPixmap(coverImage.scaledToWidth(125,Qt::SmoothTransformation));
	//cover->repaint();

	//float aspectRatio = (float)coverImage.width()/coverImage.height();
	//int heightDesktopResolution = QApplication::desktop()->screenGeometry().height();
	//int widthDesktopResolution = QApplication::desktop()->screenGeometry().width();
	//int sHeight,sWidth;
	//sHeight = static_cast<int>(heightDesktopResolution*0.65);

	//if(aspectRatio<1)
	//{
	//	sWidth = static_cast<int>(sHeight*1.4);
	//	//this->resize(sWidth,sHeight);
	//	this->move(QPoint((widthDesktopResolution-sWidth)/2,((heightDesktopResolution-sHeight)-40)/2));
	//	//sa->resize(sa->width(),sa->width()*1.333);
	//	/*cover->resize(static_cast<int>((sa->height())*aspectRatio),
	//		(sa->height()));*/

	//}
	//else
	//{
	//	sWidth = static_cast<int>(sHeight/1.16);
	//	//this->resize(sWidth,sHeight);
	//	this->move(QPoint((widthDesktopResolution-sWidth)/2,((heightDesktopResolution-sHeight)-40)/2));
	//	cover->resize((width()-25),
	//		static_cast<int>((width()-25)/aspectRatio));
	//}
	
}
void PropertiesDialog::setFilename(const QString & nameString)
{
	title->setText(nameString);
}
void PropertiesDialog::setNumpages(int pagesNum)
{
	pages->setText(QString::number(pagesNum));
}
void PropertiesDialog::setSize(float sizeFloat)
{
	 
	size->setText(QString::number(sizeFloat,'f',2) + " MB");
}