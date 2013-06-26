#include "goto_flow_toolbar.h"

#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>

GoToFlowToolBar::GoToFlowToolBar(QWidget * parent)
	:QWidget(parent)
{
	//fondo
	QBoxLayout * background = new QHBoxLayout(this);

	QLabel * imgBottomLeft = new QLabel(this);
	QLabel * imgBottomRight = new QLabel(this);
	QLabel * imgBottomMiddle = new QLabel(this);
	QPixmap pBL(":/images/imgBottomLeft.png");
	QPixmap pBM(":/images/imgBottomMiddle.png");
	QPixmap pBR(":/images/imgBottomRight.png");
	imgBottomLeft->setPixmap(pBL);
	imgBottomRight->setPixmap(pBR);
	imgBottomMiddle->setPixmap(pBM);
	imgBottomMiddle->setScaledContents(true);
	//imgTop->setStyleSheet("background-image: url(:/images/numPagesLabel.png); width: 100%; height:100%; background-repeat: none; border: none"); 

	background->addWidget(imgBottomLeft);
	background->addWidget(imgBottomMiddle);
	background->addWidget(imgBottomRight);
	background->setStretchFactor(imgBottomLeft,0);
	background->setStretchFactor(imgBottomMiddle,1);
	background->setStretchFactor(imgBottomRight,0);

	background->setMargin(0);
	background->setSpacing(0);

	//elementos interactivos
	//QVBoxLayout * mainLayout = new QVBoxLayout;
	bar = new QWidget(this);
	QHBoxLayout * bottom = new QHBoxLayout(bar);
	bottom->addStretch();
	bottom->addWidget(new QLabel(tr("Page : "),bar));
	bottom->addWidget(edit = new QLineEdit(bar));
	v = new QIntValidator(bar);
	v->setBottom(1);
	edit->setValidator(v);
	edit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	edit->setStyleSheet("background-image: url(:/images/imgEdit.png); width: 100%; height:100%; background-repeat: none; border: none; padding: 3px; color: white;"); 
	QPixmap p(":/images/imgEdit.png");
	edit->setFixedSize(54,50);
	edit->setAttribute(Qt::WA_MacShowFocusRect,false);
	edit->setAttribute(Qt::WA_LayoutUsesWidgetRect,true);
	//edit->resize(QSize(54,50));
	edit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed));
	edit->setAutoFillBackground(false);
	connect(edit,SIGNAL(returnPressed()),this,SLOT(goTo()));

	QString centerButtonCSS = "QPushButton {background-image: url(:/images/imgCenterSlide.png); width: 100%; height:100%; background-repeat: none; border: none;} "
		                      "QPushButton:focus { border: none; outline: none;}"
		                      "QPushButton:pressed  {background-image: url(:/images/imgCenterSlidePressed.png); width: 100%; height:100%; background-repeat: none; border: none;} ";
	centerButton = new QPushButton(bar);
	//centerButton->setIcon(QIcon(":/images/center.png"));
	centerButton->setStyleSheet(centerButtonCSS); 
	centerButton->setFixedSize(26,50);
	centerButton->setAttribute(Qt::WA_LayoutUsesWidgetRect,true);
	connect(centerButton,SIGNAL(clicked()),this,SLOT(centerSlide()));
	bottom->addWidget(centerButton);

	QString goToButtonCSS = "QPushButton {background-image: url(:/images/imgGoToSlide.png); width: 100%; height:100%; background-repeat: none; border: none;} "
		                    "QPushButton:focus { border: none; outline: none;}"
		                    "QPushButton:pressed  {background-image: url(:/images/imgGoToSlidePressed.png); width: 100%; height:100%; background-repeat: none; border: none;} ";
	goToButton = new QPushButton(bar);
	//goToButton->setIcon(QIcon(":/images/goto.png"));
	goToButton->setStyleSheet(goToButtonCSS); 
	goToButton->setFixedSize(32,50);
	goToButton->setAttribute(Qt::WA_LayoutUsesWidgetRect,true);
	
	connect(goToButton,SIGNAL(clicked()),this,SLOT(goTo()));
	bottom->addWidget(goToButton);

	bottom->addStretch();
	bottom->setMargin(0);
	bottom->setSpacing(0);
	
	bar->setLayout(bottom);
	//mainLayout->addWidget(bar);
	setLayout(background);
	bar->setGeometry(QRect(0,0,400,50));
	
}
	
void GoToFlowToolBar::setPage(int pageNumber)
{
	edit->setText(QString::number(pageNumber+1));
}

void GoToFlowToolBar::setTop(int numPages)
{
	v->setTop(numPages);
}

void GoToFlowToolBar::resizeEvent(QResizeEvent * event)
{

	bar->setGeometry(QRect(0,(event->size().height()-50)+((50-bar->height())/2),event->size().width(),50));

	QWidget::resizeEvent(event);
}

void GoToFlowToolBar::goTo()
{
	if(edit->text().toInt()!=0)
		emit(goTo(edit->text().toInt()-1));
}

void GoToFlowToolBar::centerSlide()
{
	if(edit->text().toInt()!=0)
		emit(setCenter(edit->text().toInt()-1));
}