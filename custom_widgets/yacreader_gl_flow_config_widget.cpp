#include "yacreader_gl_flow_config_widget.h"

#include "yacreader_spin_slider_widget.h"
#include "yacreader_flow_gl.h" //TODO

#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>


YACReaderGLFlowConfigWidget::YACReaderGLFlowConfigWidget(QWidget * parent /* = 0 */)
	:QWidget(parent)
{
	QVBoxLayout * layout = new QVBoxLayout(this);

	//PRESETS------------------------------------------------------------------
	QGroupBox *groupBox = new QGroupBox(tr("Presets:"));

	radioClassic = new QRadioButton(tr("Classic look"));
	//connect(radioClassic,SIGNAL(toggled(bool)),this,SLOT(setClassicConfig()));

	radioStripe  = new QRadioButton(tr("Stripe look"));
	//connect(radioStripe,SIGNAL(toggled(bool)),this,SLOT(setStripeConfig()));

	radioOver    = new QRadioButton(tr("Overlapped Stripe look"));
	//connect(radioOver,SIGNAL(toggled(bool)),this,SLOT(setOverlappedStripeConfig()));

	radionModern = new QRadioButton(tr("Modern look"));
	//connect(radionModern,SIGNAL(toggled(bool)),this,SLOT(setModernConfig()));

	radioDown    = new QRadioButton(tr("Roulette look"));
	//connect(radioDown,SIGNAL(toggled(bool)),this,SLOT(setRouletteConfig()));

	QVBoxLayout *vbox = new QVBoxLayout;
	QHBoxLayout * opt1 = new QHBoxLayout;
	opt1->addWidget(radioClassic);
	QLabel * lOpt1 = new QLabel();
	lOpt1->setPixmap(QPixmap(":/images/flow1.png"));
	opt1->addStretch();
	opt1->addWidget(lOpt1);
	vbox->addLayout(opt1);

	QHBoxLayout * opt2 = new QHBoxLayout;
	opt2->addWidget(radioStripe);
	QLabel * lOpt2 = new QLabel();
	lOpt2->setPixmap(QPixmap(":/images/flow2.png"));
	opt2->addStretch();
	opt2->addWidget(lOpt2);
	vbox->addLayout(opt2);

	QHBoxLayout * opt3 = new QHBoxLayout;
	opt3->addWidget(radioOver);
	QLabel * lOpt3 = new QLabel();
	lOpt3->setPixmap(QPixmap(":/images/flow3.png"));
	opt3->addStretch();
	opt3->addWidget(lOpt3);
	vbox->addLayout(opt3);

	QHBoxLayout * opt4 = new QHBoxLayout;
	opt4->addWidget(radionModern);
	QLabel * lOpt4 = new QLabel();
	lOpt4->setPixmap(QPixmap(":/images/flow4.png"));
	opt4->addStretch();
	opt4->addWidget(lOpt4);
	vbox->addLayout(opt4);

	QHBoxLayout * opt5 = new QHBoxLayout;
	opt5->addWidget(radioDown);
	QLabel * lOpt5 = new QLabel();
	lOpt5->setPixmap(QPixmap(":/images/flow5.png"));
	opt5->addStretch();
	opt5->addWidget(lOpt5);
	vbox->addLayout(opt5);

	showAdvancedOptions = new QPushButton(tr("Show advanced settings"));
	showAdvancedOptions->setCheckable(true);
	connect(showAdvancedOptions,SIGNAL(toggled(bool)),this,SLOT(avancedOptionToogled(bool)));
	
	vbox->addWidget(showAdvancedOptions,0,Qt::AlignRight);
	
	groupBox->setLayout(vbox);

	//OPTIONS------------------------------------------------------------------
	optionsGroupBox = new QGroupBox(tr("Custom:"));

	xRotation = new YACReaderSpinSliderWidget(this);
	xRotation->setText(tr("View angle"));
	xRotation->setRange(0,90);
	//connect(xRotation,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(xRotation,SIGNAL(valueChanged(int)),this,SLOT(saveXRotation(int)));

	yPosition = new YACReaderSpinSliderWidget(this);
	yPosition->setText(tr("Position"));
	yPosition->setRange(-100,100);
	//connect(yPosition,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(yPosition,SIGNAL(valueChanged(int)),this,SLOT(saveYPosition(int)));

	coverDistance = new YACReaderSpinSliderWidget(this);
	coverDistance->setText(tr("Cover gap"));
	coverDistance->setRange(0,150);
	//connect(coverDistance,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(coverDistance,SIGNAL(valueChanged(int)),this,SLOT(saveCoverDistance(int)));

	centralDistance = new YACReaderSpinSliderWidget(this);
	centralDistance->setText(tr("Central gap"));
	centralDistance->setRange(0,150);
	//connect(centralDistance,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(centralDistance,SIGNAL(valueChanged(int)),this,SLOT(saveCentralDistance(int)));

	zoomLevel = new YACReaderSpinSliderWidget(this);
	zoomLevel->setText(tr("Zoom"));
	zoomLevel->setRange(-20,0);
	//connect(zoomLevel,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(zoomLevel,SIGNAL(valueChanged(int)),this,SLOT(saveZoomLevel(int)));

	yCoverOffset = new YACReaderSpinSliderWidget(this);
	yCoverOffset->setText(tr("Y offset"));
	yCoverOffset->setRange(-50,50);
	//connect(yCoverOffset,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(yCoverOffset,SIGNAL(valueChanged(int)),this,SLOT(saveYCoverOffset(int)));

	zCoverOffset = new YACReaderSpinSliderWidget(this);
	zCoverOffset->setText(tr("Z offset"));
	zCoverOffset->setRange(-50,50);
	//connect(zCoverOffset,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(zCoverOffset,SIGNAL(valueChanged(int)),this,SLOT(saveZCoverOffset(int)));

	coverRotation = new YACReaderSpinSliderWidget(this);
	coverRotation->setText(tr("Cover Angle"));
	coverRotation->setRange(0,360);
	//connect(coverRotation,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(coverRotation,SIGNAL(valueChanged(int)),this,SLOT(saveCoverRotation(int)));

	fadeOutDist = new YACReaderSpinSliderWidget(this);
	fadeOutDist->setText(tr("Visibility"));
	fadeOutDist->setRange(0,10);
	//connect(fadeOutDist,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(fadeOutDist,SIGNAL(valueChanged(int)),this,SLOT(saveFadeOutDist(int)));

	lightStrength = new YACReaderSpinSliderWidget(this);
	lightStrength->setText(tr("Light"));
	lightStrength->setRange(0,10);
	//connect(lightStrength,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(lightStrength,SIGNAL(valueChanged(int)),this,SLOT(saveLightStrength(int)));

	maxAngle = new YACReaderSpinSliderWidget(this);
	maxAngle->setText(tr("Max angle"));
	maxAngle->setRange(0,90);
	//connect(maxAngle,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(maxAngle,SIGNAL(valueChanged(int)),this,SLOT(saveMaxAngle(int)));

	QVBoxLayout *optionsLayoutStretch = new QVBoxLayout;
	optionsLayoutStretch->setContentsMargins(0,0,0,0);
	QGridLayout *optionsLayout = new QGridLayout;
	optionsLayout->addWidget(xRotation,0,0);
	optionsLayout->addWidget(yPosition,0,1);
	optionsLayout->addWidget(coverDistance,1,0);
	optionsLayout->addWidget(centralDistance,1,1);
	optionsLayout->addWidget(zoomLevel,2,0);
	optionsLayout->addWidget(yCoverOffset,2,1);
	optionsLayout->addWidget(zCoverOffset,3,0);
	optionsLayout->addWidget(coverRotation,3,1);
	optionsLayout->addWidget(fadeOutDist,4,0);
	optionsLayout->addWidget(lightStrength,4,1);
	optionsLayout->addWidget(maxAngle,5,0);
	
	optionsLayoutStretch->addLayout(optionsLayout);
	optionsLayoutStretch->addStretch();

	optionsGroupBox->setLayout(optionsLayoutStretch);

	QHBoxLayout * groupBoxesLayout = new QHBoxLayout;
	groupBoxesLayout->addWidget(groupBox);
	groupBoxesLayout->addWidget(optionsGroupBox);

	optionsGroupBox->hide();

	QHBoxLayout * performanceSliderLayout = new QHBoxLayout;
	performanceSliderLayout->addWidget(new QLabel(tr("Low Performance")));
	performanceSliderLayout->addWidget(performanceSlider = new QSlider(Qt::Horizontal));
	performanceSliderLayout->addWidget(new QLabel(tr("High Performance")));

	performanceSlider->setMinimum(0);
	performanceSlider->setMaximum(3);
	performanceSlider->setSingleStep(1);
	performanceSlider->setPageStep(1);
	performanceSlider->setTickInterval(1);
	performanceSlider->setTickPosition(QSlider::TicksRight);

	QHBoxLayout * vSyncLayout = new QHBoxLayout;

	vSyncCheck = new QCheckBox(tr("Use VSync (improve the image quality in fullscreen mode, worse performance)"));
	vSyncLayout->addStretch();
	vSyncLayout->addWidget(vSyncCheck);

	QVBoxLayout * performanceLayout = new QVBoxLayout;
	performanceLayout->addLayout(performanceSliderLayout);
	performanceLayout->addLayout(vSyncLayout);

	QGroupBox *performanceGroupBox = new QGroupBox(tr("Performance:"));

	//connect(performanceSlider, SIGNAL(valueChanged(int)),this,SLOT(savePerformance(int)));
	//connect(performanceSlider, SIGNAL(valueChanged(int)),this,SLOT(optionsChanged()));

	performanceGroupBox->setLayout(performanceLayout);

	layout->addLayout(groupBoxesLayout);
	layout->addWidget(performanceGroupBox);

	layout->setContentsMargins(0,0,0,0);

	setLayout(layout);


}

void YACReaderGLFlowConfigWidget::avancedOptionToogled(bool show)
{
	if(show)
		optionsGroupBox->show();
	else
		optionsGroupBox->hide();
}

void YACReaderGLFlowConfigWidget::setValues(Preset preset)
{
	xRotation->setValue(preset.cfRX);
	yPosition->setValue(preset.cfY*100);
	coverDistance->setValue(preset.xDistance*100);
	centralDistance->setValue(preset.centerDistance*100);
	zoomLevel->setValue(preset.cfZ);
	yCoverOffset->setValue(preset.yDistance*100);
	zCoverOffset->setValue(preset.zDistance*100);
	coverRotation->setValue(preset.rotation*-1);
	fadeOutDist->setValue(preset.animationFadeOutDist);
	lightStrength->setValue(preset.viewRotateLightStrenght);
	maxAngle->setValue(preset.viewAngle);
}
