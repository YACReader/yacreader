#include "goto_flow_toolbar.h"

#include <QtWidgets>

#include "configuration.h"

GoToFlowToolBar::GoToFlowToolBar(QWidget * parent)
	:QStackedWidget(parent)
{
	//elementos interactivos
	QWidget * normal = new QWidget(this);  // container widget
	QWidget * quickNavi = new QWidget(this);  // container widget
	addWidget(normal);
	addWidget(quickNavi);
	QHBoxLayout * normalLayout = new QHBoxLayout(normal);
	QHBoxLayout * naviLayout = new QHBoxLayout(quickNavi);
	normal->setLayout(normalLayout);
	quickNavi->setLayout(naviLayout);

	slider = new QSlider(Qt::Horizontal,this);
	slider->setStyleSheet(
		"QSlider::groove:horizontal {"
        "  border: 1px solid #22FFFFFF;"
        "  border-radius: 1px;"
        "  background: #77000000;"
        "  margin: 2px 0;"
        "  padding: 1px;"
		"}"
		"QSlider::handle:horizontal {"
        "  background: #55FFFFFF;"
        "  width: 48px;"
        "  border-radius: 1px;"
		"}"
	);
	connect(slider, &QSlider::valueChanged, this, [&](int v) { emit(setCenter(v)); });

	pageHint = new QLabel("<b>" + tr("Page : ") + "</b>",this);
	v = new QIntValidator(this);
	v->setBottom(1);
	edit = new QLineEdit(this);
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
	centerButton = new QPushButton(this);
	//centerButton->setIcon(QIcon(":/images/center.png"));
	centerButton->setStyleSheet(centerButtonCSS); 
	centerButton->setFixedSize(26,50);
    centerButton->setAttribute(Qt::WA_LayoutUsesWidgetRect,true);
	connect(centerButton,SIGNAL(clicked()),this,SLOT(centerSlide()));

	QString goToButtonCSS = "QPushButton {background-image: url(:/images/imgGoToSlide.png); width: 100%; height:100%; background-repeat: none; border: none;} "
		                    "QPushButton:focus { border: none; outline: none;}"
		                    "QPushButton:pressed  {background-image: url(:/images/imgGoToSlidePressed.png); width: 100%; height:100%; background-repeat: none; border: none;} ";
	goToButton = new QPushButton(this);
	//goToButton->setIcon(QIcon(":/images/goto.png"));
	goToButton->setStyleSheet(goToButtonCSS); 
	goToButton->setFixedSize(32,50);
	goToButton->setAttribute(Qt::WA_LayoutUsesWidgetRect,true);
	
	connect(goToButton,SIGNAL(clicked()),this,SLOT(goTo()));

	normalLayout->setMargin(0);
	normalLayout->setSpacing(0);
	normalLayout->addStretch();
	normalLayout->addWidget(pageHint);
	normalLayout->addWidget(edit);
	normalLayout->addWidget(centerButton);
	normalLayout->addWidget(goToButton);
	normalLayout->addStretch();

	naviLayout->setContentsMargins(5, 0, 0, 0);
	naviLayout->setSpacing(2);
	naviLayout->addWidget(slider);
	naviLayout->addWidget(goToButton);

	switchLayout();

    setFixedHeight(50);
}

void GoToFlowToolBar::switchLayout()
{
	if (Configuration::getConfiguration().getQuickNaviMode())
		setCurrentIndex(1);
	else
		setCurrentIndex(0);
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
	slider->setValue(pageNumber);
}

void GoToFlowToolBar::setTop(int numPages)
{
	v->setTop(numPages);
	slider->setMaximum(numPages-1);  // min is 0
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
