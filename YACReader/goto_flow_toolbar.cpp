#include "goto_flow_toolbar.h"

#include <QtWidgets>

GoToFlowToolBar::GoToFlowToolBar(QWidget * parent)
	:QWidget(parent)
{
	//elementos interactivos
    QVBoxLayout * mainLayout = new QVBoxLayout;
	bar = new QWidget(this);
	QHBoxLayout * bottom = new QHBoxLayout(bar);
	bottom->addStretch();
    bottom->addWidget(new QLabel("<b>" + tr("Page : ") + "</b>",bar));
	bottom->addWidget(edit = new QLineEdit(bar));
	v = new QIntValidator(bar);
	v->setBottom(1);
	edit->setValidator(v);
	edit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    edit->setStyleSheet("QLineEdit {border: 1px solid #77000000; background: #55000000; color: white; padding: 3px 5px 5px 5px; margin: 13px 5px 12px 5px; font-weight:bold}");
	QPixmap p(":/images/imgEdit.png");
	edit->setFixedSize(54,50);
	edit->setAttribute(Qt::WA_MacShowFocusRect,false);
    //edit->setAttribute(Qt::WA_LayoutUsesWidgetRect,true);
	//edit->resize(QSize(54,50));
	edit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed));
    //edit->setAutoFillBackground(false);
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

    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(bar);

    setLayout(mainLayout);

    setFixedHeight(50);
}

void GoToFlowToolBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

#ifdef YACREADER_LEGACY_FLOW_GL
    painter.fillRect(0,0,width(),height(),QColor("#FF000000"));
#else
    painter.fillRect(0,0,width(),height(),QColor("#99000000"));
#endif
}

void GoToFlowToolBar::setPage(int pageNumber)
{
	edit->setText(QString::number(pageNumber+1));
}

void GoToFlowToolBar::setTop(int numPages)
{
	v->setTop(numPages);
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
