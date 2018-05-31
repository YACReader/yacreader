#include "yacreader_options_dialog.h"

#include "yacreader_flow_config_widget.h"
#ifndef NO_OPENGL
#include "yacreader_gl_flow_config_widget.h"
#else
#include "pictureflow.h"
#endif
#include "yacreader_spin_slider_widget.h"
#include "yacreader_global.h"

#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QSlider>
#include <QSettings>
#include <QGroupBox>
#include <QVBoxLayout>

YACReaderOptionsDialog::YACReaderOptionsDialog(QWidget * parent)
	:QDialog(parent)
{

	sw = new YACReaderFlowConfigWidget(this);
#ifndef NO_OPENGL
	gl = new YACReaderGLFlowConfigWidget(this);
#endif
	accept = new QPushButton(tr("Save"));
	cancel = new QPushButton(tr("Cancel"));

	cancel->setDefault(true);


    QVBoxLayout * shortcutsLayout = new QVBoxLayout();
    QPushButton * shortcutsButton = new QPushButton(tr("Edit shortcuts"));
    shortcutsLayout->addWidget(shortcutsButton);

    shortcutsBox = new QGroupBox(tr("Shortcuts"));
    shortcutsBox->setLayout(shortcutsLayout);

    connect(shortcutsButton,SIGNAL(clicked()),this,SIGNAL(editShortcuts()));

	connect(accept,SIGNAL(clicked()),this,SLOT(saveOptions()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(restoreOptions())); //TODO fix this
	connect(cancel,SIGNAL(clicked()),this,SLOT(close()));
#ifndef NO_OPENGL
	useGL = new QCheckBox(tr("Use hardware acceleration (restart needed)"));
	connect(useGL,SIGNAL(stateChanged(int)),this,SLOT(saveUseGL(int)));
#endif
#ifdef FORCE_ANGLE
    useGL->setHidden(true);
#endif
	//sw CONNECTIONS
	connect(sw->radio1,SIGNAL(toggled(bool)),this,SLOT(setClassicConfigSW()));
	connect(sw->radio2,SIGNAL(toggled(bool)),this,SLOT(setStripeConfigSW()));
	connect(sw->radio3,SIGNAL(toggled(bool)),this,SLOT(setOverlappedStripeConfigSW()));
#ifndef NO_OPENGL
	//gl CONNECTIONS
	connect(gl->radioClassic,SIGNAL(toggled(bool)),this,SLOT(setClassicConfig()));
	connect(gl->radioStripe,SIGNAL(toggled(bool)),this,SLOT(setStripeConfig()));
	connect(gl->radioOver,SIGNAL(toggled(bool)),this,SLOT(setOverlappedStripeConfig()));
	connect(gl->radionModern,SIGNAL(toggled(bool)),this,SLOT(setModernConfig()));
	connect(gl->radioDown,SIGNAL(toggled(bool)),this,SLOT(setRouletteConfig()));

	connect(gl->radioClassic,SIGNAL(toggled(bool)),this,SIGNAL(optionsChanged()));
	connect(gl->radioStripe,SIGNAL(toggled(bool)),this,SIGNAL(optionsChanged()));
	connect(gl->radioOver,SIGNAL(toggled(bool)),this,SIGNAL(optionsChanged()));
	connect(gl->radionModern,SIGNAL(toggled(bool)),this,SIGNAL(optionsChanged()));
	connect(gl->radioDown,SIGNAL(toggled(bool)),this,SIGNAL(optionsChanged()));

	connect(gl->xRotation,SIGNAL(valueChanged(int)),this,SLOT(saveXRotation(int)));
	connect(gl->xRotation,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->yPosition,SIGNAL(valueChanged(int)),this,SLOT(saveYPosition(int)));
	connect(gl->yPosition,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->coverDistance,SIGNAL(valueChanged(int)),this,SLOT(saveCoverDistance(int)));
	connect(gl->coverDistance,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->centralDistance,SIGNAL(valueChanged(int)),this,SLOT(saveCentralDistance(int)));
	connect(gl->centralDistance,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->zoomLevel,SIGNAL(valueChanged(int)),this,SLOT(saveZoomLevel(int)));
	connect(gl->zoomLevel,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->yCoverOffset,SIGNAL(valueChanged(int)),this,SLOT(saveYCoverOffset(int)));
	connect(gl->yCoverOffset,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->zCoverOffset,SIGNAL(valueChanged(int)),this,SLOT(saveZCoverOffset(int)));
	connect(gl->zCoverOffset,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->coverRotation,SIGNAL(valueChanged(int)),this,SLOT(saveCoverRotation(int)));
	connect(gl->coverRotation,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->fadeOutDist,SIGNAL(valueChanged(int)),this,SLOT(saveFadeOutDist(int)));
	connect(gl->fadeOutDist,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->lightStrength,SIGNAL(valueChanged(int)),this,SLOT(saveLightStrength(int)));
	connect(gl->lightStrength,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->maxAngle,SIGNAL(valueChanged(int)),this,SLOT(saveMaxAngle(int)));
	connect(gl->maxAngle,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->performanceSlider, SIGNAL(valueChanged(int)),this,SLOT(savePerformance(int)));
	connect(gl->performanceSlider, SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));

	connect(gl->vSyncCheck,SIGNAL(stateChanged(int)),this,SLOT(saveUseVSync(int)));
#endif
}

#ifndef NO_OPENGL
void YACReaderOptionsDialog::savePerformance(int value)
{
	settings->setValue(PERFORMANCE,value);
}

void YACReaderOptionsDialog::saveUseVSync(int b)
{
	settings->setValue(V_SYNC,b);
}

void YACReaderOptionsDialog::saveFlowParameters()
{
	settings->setValue(X_ROTATION,gl->xRotation->getValue());
	settings->setValue(Y_POSITION,gl->yPosition->getValue());
	settings->setValue(COVER_DISTANCE,gl->coverDistance->getValue());
	settings->setValue(CENTRAL_DISTANCE,gl->centralDistance->getValue());
	settings->setValue(ZOOM_LEVEL,gl->zoomLevel->getValue());
	settings->setValue(Y_COVER_OFFSET,gl->yCoverOffset->getValue());
	settings->setValue(Z_COVER_OFFSET,gl->zCoverOffset->getValue());
	settings->setValue(COVER_ROTATION,gl->coverRotation->getValue());
	settings->setValue(FADE_OUT_DIST,gl->fadeOutDist->getValue());
	settings->setValue(LIGHT_STRENGTH,gl->lightStrength->getValue());
	settings->setValue(MAX_ANGLE,gl->maxAngle->getValue());
}
#endif

void YACReaderOptionsDialog::saveOptions()
{
	emit(optionsChanged());
	close();
}

#ifndef NO_OPENGL
void YACReaderOptionsDialog::saveUseGL(int b)
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

	settings->setValue(USE_OPEN_GL,b);
}
#endif

#ifndef NO_OPENGL
void YACReaderOptionsDialog::saveXRotation(int value)
{
	settings->setValue(FLOW_TYPE_GL,Custom);
	settings->setValue(X_ROTATION,value);
}
void YACReaderOptionsDialog::saveYPosition(int value)
{
	settings->setValue(FLOW_TYPE_GL,Custom);
	settings->setValue(Y_POSITION,value);
}
void YACReaderOptionsDialog::saveCoverDistance(int value)
{
	settings->setValue(FLOW_TYPE_GL,Custom);
	settings->setValue(COVER_DISTANCE,value);
}
void YACReaderOptionsDialog::saveCentralDistance(int value)
{
	settings->setValue(FLOW_TYPE_GL,Custom);
	settings->setValue(CENTRAL_DISTANCE,value);
}
void YACReaderOptionsDialog::saveZoomLevel(int value)
{
	settings->setValue(FLOW_TYPE_GL,Custom);
	settings->setValue(ZOOM_LEVEL,value);
}
void YACReaderOptionsDialog::saveYCoverOffset(int value)
{
	settings->setValue(FLOW_TYPE_GL,Custom);
	settings->setValue(Y_COVER_OFFSET,value);
}
void YACReaderOptionsDialog::saveZCoverOffset(int value)
{
	settings->setValue(FLOW_TYPE_GL,Custom);
	settings->setValue(Z_COVER_OFFSET,value);
}
void YACReaderOptionsDialog::saveCoverRotation(int value)
{
	settings->setValue(FLOW_TYPE_GL,Custom);
	settings->setValue(COVER_ROTATION,value);
}
void YACReaderOptionsDialog::saveFadeOutDist(int value)
{
	settings->setValue(FLOW_TYPE_GL,Custom);
	settings->setValue(FADE_OUT_DIST,value);
}
void YACReaderOptionsDialog::saveLightStrength(int value)
{
	settings->setValue(FLOW_TYPE_GL,Custom);
	settings->setValue(LIGHT_STRENGTH,value);
}

void YACReaderOptionsDialog::saveMaxAngle(int value)
{
	settings->setValue(FLOW_TYPE_GL,Custom);
	settings->setValue(MAX_ANGLE,value);
}
#endif
void YACReaderOptionsDialog::restoreOptions(QSettings * settings)
{
	this->settings = settings;

	//FLOW CONFIG
#ifndef NO_OPENGL
	if(settings->contains(USE_OPEN_GL) && settings->value(USE_OPEN_GL).toInt() == Qt::Checked)
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


	if(!settings->contains(FLOW_TYPE_GL))
	{
		setClassicConfig();
		gl->radioClassic->setChecked(true);
		gl->performanceSlider->setValue(1);
		return;
	}

	if(settings->contains(V_SYNC) && settings->value(V_SYNC).toInt() == Qt::Checked)
		gl->vSyncCheck->setChecked(true);
	else
		gl->vSyncCheck->setChecked(false);

	gl->performanceSlider->setValue(settings->value(PERFORMANCE).toInt());

	FlowType flowType;
	switch(settings->value(FLOW_TYPE_GL).toInt())
	{
	case 0:
		flowType = CoverFlowLike;
		break;
	case 1:
		flowType = Strip;
		break;
	case 2:
		flowType = StripOverlapped;
		break;
	case 3:
		flowType = Modern;
		break;
	case 4:
		flowType = Roulette;
		break;
	case 5:
		flowType = Custom;
		break;
	default:
		flowType = CoverFlowLike;
	}


	if(flowType == Custom)
	{
		loadConfig();
		return;
	}

	if(flowType == CoverFlowLike)
	{
		setClassicConfig();
		gl->radioClassic->setChecked(true);
		return;
	}

	if(flowType == Strip)
	{
		setStripeConfig();
		gl->radioStripe->setChecked(true);
		return;
	}

	if(flowType == StripOverlapped)
	{
		setOverlappedStripeConfig();
		gl->radioOver->setChecked(true);
		return;
	}

	if(flowType == Modern)
	{
		setModernConfig();
		gl->radionModern->setChecked(true);
		return;
	}

	if(flowType == Roulette)
	{
		setRouletteConfig();
		gl->radioDown->setChecked(true);
		return;
	}

	//END FLOW CONFIG
#endif
}

void YACReaderOptionsDialog::restoreOptions()
{
    restoreOptions(settings);
}

#ifndef NO_OPENGL
void YACReaderOptionsDialog::loadConfig()
{
	gl->xRotation->setValue(settings->value(X_ROTATION).toInt());
	gl->yPosition->setValue(settings->value(Y_POSITION).toInt());
	gl->coverDistance->setValue(settings->value(COVER_DISTANCE).toInt());
	gl->centralDistance->setValue(settings->value(CENTRAL_DISTANCE).toInt());
	gl->zoomLevel->setValue(settings->value(ZOOM_LEVEL).toInt());
	gl->yCoverOffset->setValue(settings->value(Y_COVER_OFFSET).toInt());
	gl->zCoverOffset->setValue(settings->value(Z_COVER_OFFSET).toInt());
	gl->coverRotation->setValue(settings->value(COVER_ROTATION).toInt());
	gl->fadeOutDist->setValue(settings->value(FADE_OUT_DIST).toInt());
	gl->lightStrength->setValue(settings->value(LIGHT_STRENGTH).toInt());
	gl->maxAngle->setValue(settings->value(MAX_ANGLE).toInt());
}
#endif
void YACReaderOptionsDialog::setClassicConfigSW()
{
	settings->setValue(FLOW_TYPE_SW,CoverFlowLike);
}

void YACReaderOptionsDialog::setStripeConfigSW()
{
	settings->setValue(FLOW_TYPE_SW,Strip);
}

void YACReaderOptionsDialog::setOverlappedStripeConfigSW()
{
	settings->setValue(FLOW_TYPE_SW,StripOverlapped);
}

#ifndef NO_OPENGL
void YACReaderOptionsDialog::setClassicConfig()
{
	gl->setValues(presetYACReaderFlowClassicConfig);

	saveFlowParameters();

    settings->setValue(FLOW_TYPE_GL,CoverFlowLike);
}

void YACReaderOptionsDialog::setStripeConfig()
{
	gl->setValues(presetYACReaderFlowStripeConfig);

	saveFlowParameters();

    settings->setValue(FLOW_TYPE_GL,Strip);
}

void YACReaderOptionsDialog::setOverlappedStripeConfig()
{
	gl->setValues(presetYACReaderFlowOverlappedStripeConfig);

	saveFlowParameters();

    settings->setValue(FLOW_TYPE_GL,StripOverlapped);
}

void YACReaderOptionsDialog::setModernConfig()
{
	gl->setValues(defaultYACReaderFlowConfig);

	saveFlowParameters();

    settings->setValue(FLOW_TYPE_GL,Modern);
}

void YACReaderOptionsDialog::setRouletteConfig()
{
	gl->setValues(pressetYACReaderFlowDownConfig);

	saveFlowParameters();

    settings->setValue(FLOW_TYPE_GL,Roulette);
}
#endif
