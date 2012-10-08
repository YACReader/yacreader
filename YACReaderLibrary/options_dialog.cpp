#include "options_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QTextStream>
#include <QCoreApplication>
#include <QFile>
#include <QMessageBox>
#include <QCheckBox>

#include "custom_widgets.h"
#include "yacreader_flow_gl.h"

PictureFlow::FlowType flowType = PictureFlow::Strip;

OptionsDialog::OptionsDialog(QWidget * parent)
:QDialog()
{
	QVBoxLayout * layout = new QVBoxLayout;
	QLayout * layout1 = setupLayoutSW();
	QLayout * layout2 = setupLayoutGL();
	
	sw = new QWidget(this);
	layout1->setContentsMargins(0,0,0,0);
	sw->setLayout(layout1);

	gl = new QWidget(this);
	layout2->setContentsMargins(0,0,0,0);
	gl->setLayout(layout2);

	QHBoxLayout * switchFlowType = new QHBoxLayout;
	switchFlowType->addStretch();
	switchFlowType->addWidget(useGL = new QCheckBox(tr("Use hardware acceleration (restart needed)")));

	connect(useGL,SIGNAL(stateChanged(int)),this,SLOT(saveUseGL(int)));

	accept = new QPushButton(tr("Save"));
	cancel = new QPushButton(tr("Cancel"));
	connect(accept,SIGNAL(clicked()),this,SLOT(saveOptions()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(restoreOptions()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(close()));

	QHBoxLayout * buttons = new QHBoxLayout();
	buttons->addStretch();
	buttons->addWidget(accept);
	buttons->addWidget(cancel);

	layout->addWidget(sw);
	layout->addWidget(gl);
	layout->addLayout(switchFlowType);
	layout->addLayout(buttons);

	sw->hide();

	setLayout(layout);
	//restoreOptions(settings); //load options
    //resize(200,0);
	setModal (true);
	setWindowTitle("Options");
}

QLayout * OptionsDialog::setupLayoutSW()
{
	QVBoxLayout * layout = new QVBoxLayout(this);

	QGroupBox *groupBox = new QGroupBox(tr("How to show covers:"));

	radio1 = new QRadioButton(tr("CoverFlow look"));
	radio2 = new QRadioButton(tr("Stripe look"));
	radio3 = new QRadioButton(tr("Overlapped Stripe look"));


	QVBoxLayout *vbox = new QVBoxLayout;
	QHBoxLayout * opt1 = new QHBoxLayout;
	opt1->addWidget(radio1);
	QLabel * lOpt1 = new QLabel();
	lOpt1->setPixmap(QPixmap(":/images/flow1.png"));
	opt1->addStretch();
	opt1->addWidget(lOpt1);
	vbox->addLayout(opt1);

	QHBoxLayout * opt2 = new QHBoxLayout;
	opt2->addWidget(radio2);
	QLabel * lOpt2 = new QLabel();
	lOpt2->setPixmap(QPixmap(":/images/flow2.png"));
	opt2->addStretch();
	opt2->addWidget(lOpt2);
	vbox->addLayout(opt2);

	QHBoxLayout * opt3 = new QHBoxLayout;
	opt3->addWidget(radio3);
	QLabel * lOpt3 = new QLabel();
	lOpt3->setPixmap(QPixmap(":/images/flow3.png"));
	opt3->addStretch();
	opt3->addWidget(lOpt3);
	vbox->addLayout(opt3);


	//vbox->addStretch(1);
	groupBox->setLayout(vbox);

	layout->addWidget(groupBox);

	return layout;
}
QLayout * OptionsDialog::setupLayoutGL()
{
	QVBoxLayout * layout = new QVBoxLayout(this);

	//PRESETS------------------------------------------------------------------
	QGroupBox *groupBox = new QGroupBox(tr("Presets:"));

	radioClassic = new QRadioButton(tr("Classic look"));
	connect(radioClassic,SIGNAL(toggled(bool)),this,SLOT(setClassicConfig()));

	radioStripe  = new QRadioButton(tr("Stripe look"));
	connect(radioStripe,SIGNAL(toggled(bool)),this,SLOT(setStripeConfig()));

	radioOver    = new QRadioButton(tr("Overlapped Stripe look"));
	connect(radioOver,SIGNAL(toggled(bool)),this,SLOT(setOverlappedStripeConfig()));

	radionModern = new QRadioButton(tr("Modern look"));
	connect(radionModern,SIGNAL(toggled(bool)),this,SLOT(setModernConfig()));

	radioDown    = new QRadioButton(tr("Roulette look"));
	connect(radioDown,SIGNAL(toggled(bool)),this,SLOT(setRouletteConfig()));

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
	lOpt4->setPixmap(QPixmap(":/images/flow3.png"));
	opt4->addStretch();
	opt4->addWidget(lOpt4);
	vbox->addLayout(opt4);

	QHBoxLayout * opt5 = new QHBoxLayout;
	opt5->addWidget(radioDown);
	QLabel * lOpt5 = new QLabel();
	lOpt5->setPixmap(QPixmap(":/images/flow3.png"));
	opt5->addStretch();
	opt5->addWidget(lOpt5);
	vbox->addLayout(opt5);
	
	groupBox->setLayout(vbox);

	//OPTIONS------------------------------------------------------------------
	QGroupBox *optionsGroupBox = new QGroupBox(tr("Custom:"));

	xRotation = new YACReaderSpinSliderWidget(this);
	xRotation->setText(tr("View angle"));
	xRotation->setRange(0,90);
	connect(xRotation,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(xRotation,SIGNAL(valueChanged(int)),this,SLOT(saveXRotation(int)));

	yPosition = new YACReaderSpinSliderWidget(this);
	yPosition->setText(tr("Position"));
	yPosition->setRange(-100,100);
	connect(yPosition,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(yPosition,SIGNAL(valueChanged(int)),this,SLOT(saveYPosition(int)));

	coverDistance = new YACReaderSpinSliderWidget(this);
	coverDistance->setText(tr("Cover gap"));
	coverDistance->setRange(0,150);
	connect(coverDistance,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(coverDistance,SIGNAL(valueChanged(int)),this,SLOT(saveCoverDistance(int)));

	centralDistance = new YACReaderSpinSliderWidget(this);
	centralDistance->setText(tr("Central gap"));
	centralDistance->setRange(0,150);
	connect(centralDistance,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(centralDistance,SIGNAL(valueChanged(int)),this,SLOT(saveCentralDistance(int)));

	zoomLevel = new YACReaderSpinSliderWidget(this);
	zoomLevel->setText(tr("Zoom"));
	zoomLevel->setRange(-20,0);
	connect(zoomLevel,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(zoomLevel,SIGNAL(valueChanged(int)),this,SLOT(saveZoomLevel(int)));

	yCoverOffset = new YACReaderSpinSliderWidget(this);
	yCoverOffset->setText(tr("Y offset"));
	yCoverOffset->setRange(-50,50);
	connect(yCoverOffset,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(yCoverOffset,SIGNAL(valueChanged(int)),this,SLOT(saveYCoverOffset(int)));

	zCoverOffset = new YACReaderSpinSliderWidget(this);
	zCoverOffset->setText(tr("Z offset"));
	zCoverOffset->setRange(-50,50);
	connect(zCoverOffset,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(zCoverOffset,SIGNAL(valueChanged(int)),this,SLOT(saveZCoverOffset(int)));

	coverRotation = new YACReaderSpinSliderWidget(this);
	coverRotation->setText(tr("Cover Angle"));
	coverRotation->setRange(0,360);
	connect(coverRotation,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(coverRotation,SIGNAL(valueChanged(int)),this,SLOT(saveCoverRotation(int)));

	fadeOutDist = new YACReaderSpinSliderWidget(this);
	fadeOutDist->setText(tr("Visibility"));
	fadeOutDist->setRange(0,10);
	connect(fadeOutDist,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(fadeOutDist,SIGNAL(valueChanged(int)),this,SLOT(saveFadeOutDist(int)));

	lightStrength = new YACReaderSpinSliderWidget(this);
	lightStrength->setText(tr("Light"));
	lightStrength->setRange(-10,10);
	connect(lightStrength,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(lightStrength,SIGNAL(valueChanged(int)),this,SLOT(saveLightStrength(int)));

	maxAngle = new YACReaderSpinSliderWidget(this);
	maxAngle->setText(tr("Max angle"));
	maxAngle->setRange(0,90);
	connect(maxAngle,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(maxAngle,SIGNAL(valueChanged(int)),this,SLOT(saveMaxAngle(int)));

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

	QHBoxLayout * performance = new QHBoxLayout;
	performance->addWidget(new QLabel(tr("Low Performance")));
	performance->addWidget(performanceSlider = new QSlider(Qt::Horizontal));
	performance->addWidget(new QLabel(tr("High Performance")));

	performanceSlider->setMinimum(0);
	performanceSlider->setMaximum(3);
	performanceSlider->setSingleStep(1);
	performanceSlider->setPageStep(1);
	performanceSlider->setTickInterval(1);
	performanceSlider->setTickPosition(QSlider::TicksRight);

	connect(performanceSlider, SIGNAL(valueChanged(int)),this,SLOT(savePerformance(int)));
	connect(performanceSlider, SIGNAL(valueChanged(int)),this,SLOT(optionsChanged()));


	layout->addLayout(groupBoxesLayout);
	layout->addLayout(performance);

	return layout;
}

void OptionsDialog::savePerformance(int value)
{
	settings->setValue("performance",value);
}

void OptionsDialog::saveUseGL(int b)
{
	if(Qt::Checked == b)
	{
		sw->setVisible(false);
		gl->setVisible(true);
	}
	else
	{
		gl->setVisible(false);
		sw->setVisible(true);
	}
	resize(0,0);
	settings->setValue("useOpenGL",b);
}

void OptionsDialog::saveXRotation(int value)
{
	settings->setValue("flowType",PictureFlow::Custom);
	settings->setValue("xRotation",xRotation->getValue());
}
void OptionsDialog::saveYPosition(int value)
{
	settings->setValue("flowType",PictureFlow::Custom);
	settings->setValue("yPosition",yPosition->getValue());
}
void OptionsDialog::saveCoverDistance(int value)
{
	settings->setValue("flowType",PictureFlow::Custom);
	settings->setValue("coverDistance",coverDistance->getValue());
}
void OptionsDialog::saveCentralDistance(int value)
{
	settings->setValue("flowType",PictureFlow::Custom);
	settings->setValue("centralDistance",centralDistance->getValue());
}
void OptionsDialog::saveZoomLevel(int value)
{
	settings->setValue("flowType",PictureFlow::Custom);
	settings->setValue("zoomLevel",zoomLevel->getValue());
}
void OptionsDialog::saveYCoverOffset(int value)
{
	settings->setValue("flowType",PictureFlow::Custom);
	settings->setValue("yCoverOffset",yCoverOffset->getValue());
}
void OptionsDialog::saveZCoverOffset(int value)
{
	settings->setValue("flowType",PictureFlow::Custom);
	settings->setValue("zCoverOffset",zCoverOffset->getValue());
}
void OptionsDialog::saveCoverRotation(int value)
{
	settings->setValue("flowType",PictureFlow::Custom);
	settings->setValue("coverRotation",coverRotation->getValue());
}
void OptionsDialog::saveFadeOutDist(int value)
{
	settings->setValue("flowType",PictureFlow::Custom);
	settings->setValue("fadeOutDist",fadeOutDist->getValue());
}
void OptionsDialog::saveLightStrength(int value)
{
	settings->setValue("flowType",PictureFlow::Custom);
	settings->setValue("lightStrength",lightStrength->getValue());
}

void OptionsDialog::saveMaxAngle(int value)
{
	settings->setValue("flowType",PictureFlow::Custom);
	settings->setValue("maxAngle",maxAngle->getValue());
}


void OptionsDialog::saveOptions()
{
	emit(optionsChanged());
	close();
}

void OptionsDialog::restoreOptions(QSettings * settings)
{
	this->settings = settings;

	if(settings->contains("useOpenGL") && settings->value("useOpenGL").toInt() == Qt::Checked)
	{
		sw->setVisible(false);
		gl->setVisible(true);
		useGL->setChecked(true);
	}
	else
	{
		gl->setVisible(false);
		sw->setVisible(true);
		useGL->setChecked(false);
	}
		

	if(!settings->contains("flowType"))
	{
		setClassicConfig();
		radioClassic->setChecked(true);
		performanceSlider->setValue(1);
		return;
	}

	performanceSlider->setValue(settings->value("performance").toInt());
	PictureFlow::FlowType flowType;
	switch(settings->value("flowType").toInt())
	{
	case 0:
		flowType = PictureFlow::CoverFlowLike;
		break;
	case 1:
		flowType = PictureFlow::Strip;
		break;
	case 2:
		flowType = PictureFlow::StripOverlapped;
		break;
	case 3:
		flowType = PictureFlow::Modern;
		break;
	case 4:
		flowType = PictureFlow::Roulette;
		break;
	case 5:
		flowType = PictureFlow::Custom;
		break;
	}
	

	if(flowType == PictureFlow::Custom)
	{
		loadConfig();
		return;
	}

	if(flowType == PictureFlow::CoverFlowLike)
	{
		setClassicConfig();
		radioClassic->setChecked(true);
		return;
	}

	if(flowType == PictureFlow::Strip)
	{
		setStripeConfig();
		radioStripe->setChecked(true);
		return;
	}

	if(flowType == PictureFlow::StripOverlapped)
	{
		setOverlappedStripeConfig();
		radioOver->setChecked(true);
		return;
	}

	if(flowType == PictureFlow::Modern)
	{
		setModernConfig();
		radionModern->setChecked(true);
		return;
	}
	
	if(flowType == PictureFlow::Roulette)
	{
		setRouletteConfig();
		radioDown->setChecked(true);
		return;
	}
}

void OptionsDialog::loadConfig()
{

	xRotation->setValue(settings->value("xRotation").toInt());
	yPosition->setValue(settings->value("yPosition").toInt());
	coverDistance->setValue(settings->value("coverDistance").toInt());
	centralDistance->setValue(settings->value("centralDistance").toInt());
	zoomLevel->setValue(settings->value("zoomLevel").toInt());
	yCoverOffset->setValue(settings->value("yCoverOffset").toInt());
	zCoverOffset->setValue(settings->value("zCoverOffset").toInt());
	coverRotation->setValue(settings->value("coverRotation").toInt());
	fadeOutDist->setValue(settings->value("fadeOutDist").toInt());
	lightStrength->setValue(settings->value("lightStrength").toInt());
	maxAngle->setValue(settings->value("maxAngle").toInt());
}
void OptionsDialog::setClassicConfig()
{

	settings->setValue("flowType",PictureFlow::CoverFlowLike);

	xRotation->setValue(presetYACReaderFlowClassicConfig.cfRX);
	yPosition->setValue(presetYACReaderFlowClassicConfig.cfY*100);
	coverDistance->setValue(presetYACReaderFlowClassicConfig.xDistance*100);
	centralDistance->setValue(presetYACReaderFlowClassicConfig.centerDistance*100);
	zoomLevel->setValue(presetYACReaderFlowClassicConfig.cfZ);
	yCoverOffset->setValue(presetYACReaderFlowClassicConfig.yDistance*100);
	zCoverOffset->setValue(presetYACReaderFlowClassicConfig.zDistance*100);
	coverRotation->setValue(presetYACReaderFlowClassicConfig.rotation*-1);
	fadeOutDist->setValue(presetYACReaderFlowClassicConfig.animationFadeOutDist);
	lightStrength->setValue(presetYACReaderFlowClassicConfig.viewRotateLightStrenght);
	maxAngle->setValue(presetYACReaderFlowClassicConfig.viewAngle);
}

void OptionsDialog::setStripeConfig()
{

	settings->setValue("flowType",PictureFlow::Strip);

	xRotation->setValue(presetYACReaderFlowStripeConfig.cfRX);
	yPosition->setValue(presetYACReaderFlowStripeConfig.cfY*100);
	coverDistance->setValue(presetYACReaderFlowStripeConfig.xDistance*100);
	centralDistance->setValue(presetYACReaderFlowStripeConfig.centerDistance*100);
	zoomLevel->setValue(presetYACReaderFlowStripeConfig.cfZ);
	yCoverOffset->setValue(presetYACReaderFlowStripeConfig.yDistance*100);
	zCoverOffset->setValue(presetYACReaderFlowStripeConfig.zDistance*100);
	coverRotation->setValue(presetYACReaderFlowStripeConfig.rotation*-1);
	fadeOutDist->setValue(presetYACReaderFlowStripeConfig.animationFadeOutDist);
	lightStrength->setValue(presetYACReaderFlowStripeConfig.viewRotateLightStrenght);
	maxAngle->setValue(presetYACReaderFlowStripeConfig.viewAngle);
}

void OptionsDialog::setOverlappedStripeConfig()
{
	settings->setValue("flowType",PictureFlow::StripOverlapped);

	xRotation->setValue(presetYACReaderFlowOverlappedStripeConfig.cfRX);
	yPosition->setValue(presetYACReaderFlowOverlappedStripeConfig.cfY*100);
	coverDistance->setValue(presetYACReaderFlowOverlappedStripeConfig.xDistance*100);
	centralDistance->setValue(presetYACReaderFlowOverlappedStripeConfig.centerDistance*100);
	zoomLevel->setValue(presetYACReaderFlowOverlappedStripeConfig.cfZ);
	yCoverOffset->setValue(presetYACReaderFlowOverlappedStripeConfig.yDistance*100);
	zCoverOffset->setValue(presetYACReaderFlowOverlappedStripeConfig.zDistance*100);
	coverRotation->setValue(presetYACReaderFlowOverlappedStripeConfig.rotation*-1);
	fadeOutDist->setValue(presetYACReaderFlowOverlappedStripeConfig.animationFadeOutDist);
	lightStrength->setValue(presetYACReaderFlowOverlappedStripeConfig.viewRotateLightStrenght);
	maxAngle->setValue(presetYACReaderFlowOverlappedStripeConfig.viewAngle);
}

void OptionsDialog::setModernConfig()
{
	settings->setValue("flowType",PictureFlow::Modern);

	xRotation->setValue(defaultYACReaderFlowConfig.cfRX);
	yPosition->setValue(defaultYACReaderFlowConfig.cfY*100);
	coverDistance->setValue(defaultYACReaderFlowConfig.xDistance*100);
	centralDistance->setValue(defaultYACReaderFlowConfig.centerDistance*100);
	zoomLevel->setValue(defaultYACReaderFlowConfig.cfZ);
	yCoverOffset->setValue(defaultYACReaderFlowConfig.yDistance*100);
	zCoverOffset->setValue(defaultYACReaderFlowConfig.zDistance*100);
	coverRotation->setValue(defaultYACReaderFlowConfig.rotation*-1);
	fadeOutDist->setValue(defaultYACReaderFlowConfig.animationFadeOutDist);
	lightStrength->setValue(defaultYACReaderFlowConfig.viewRotateLightStrenght);
	maxAngle->setValue(defaultYACReaderFlowConfig.viewAngle);
}

void OptionsDialog::setRouletteConfig()
{
	settings->setValue("flowType",PictureFlow::Roulette);

	xRotation->setValue(pressetYACReaderFlowDownConfig.cfRX);
	yPosition->setValue(pressetYACReaderFlowDownConfig.cfY*100);
	coverDistance->setValue(pressetYACReaderFlowDownConfig.xDistance*100);
	centralDistance->setValue(pressetYACReaderFlowDownConfig.centerDistance*100);
	zoomLevel->setValue(pressetYACReaderFlowDownConfig.cfZ);
	yCoverOffset->setValue(pressetYACReaderFlowDownConfig.yDistance*100);
	zCoverOffset->setValue(pressetYACReaderFlowDownConfig.zDistance*100);
	coverRotation->setValue(pressetYACReaderFlowDownConfig.rotation*-1);
	fadeOutDist->setValue(pressetYACReaderFlowDownConfig.animationFadeOutDist);
	lightStrength->setValue(pressetYACReaderFlowDownConfig.viewRotateLightStrenght);
	maxAngle->setValue(pressetYACReaderFlowDownConfig.viewAngle);
}
