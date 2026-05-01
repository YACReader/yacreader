#include "yacreader_options_dialog.h"

#include "yacreader_3d_flow_config_widget.h"
#include "yacreader_global_gui.h"
#include "yacreader_spin_slider_widget.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QSlider>
#include <QVBoxLayout>

YACReaderOptionsDialog::YACReaderOptionsDialog(QWidget *parent)
    : QDialog(parent)
{
    gl = new YACReader3DFlowConfigWidget(this);
    accept = new QPushButton(tr("Save"));
    cancel = new QPushButton(tr("Cancel"));

    cancel->setDefault(true);

    QVBoxLayout *shortcutsLayout = new QVBoxLayout();
    QPushButton *shortcutsButton = new QPushButton(tr("Edit shortcuts"));
    shortcutsLayout->addWidget(shortcutsButton);

    shortcutsBox = new QGroupBox(tr("Shortcuts"));
    shortcutsBox->setLayout(shortcutsLayout);

    connect(shortcutsButton, &QAbstractButton::clicked, this, &YACReaderOptionsDialog::editShortcuts);

    connect(accept, &QAbstractButton::clicked, this, &YACReaderOptionsDialog::saveOptions);
    connect(cancel, &QAbstractButton::clicked, this, QOverload<>::of(&YACReaderOptionsDialog::restoreOptions));
    connect(cancel, &QAbstractButton::clicked, this, &QWidget::close);

    // gl CONNECTIONS
    connect(gl->radioClassic, &QAbstractButton::toggled, this, &YACReaderOptionsDialog::setClassicConfig);
    connect(gl->radioStripe, &QAbstractButton::toggled, this, &YACReaderOptionsDialog::setStripeConfig);
    connect(gl->radioOver, &QAbstractButton::toggled, this, &YACReaderOptionsDialog::setOverlappedStripeConfig);
    connect(gl->radionModern, &QAbstractButton::toggled, this, &YACReaderOptionsDialog::setModernConfig);
    connect(gl->radioDown, &QAbstractButton::toggled, this, &YACReaderOptionsDialog::setRouletteConfig);

    connect(gl->radioClassic, &QAbstractButton::toggled, this, &YACReaderOptionsDialog::optionsChanged);
    connect(gl->radioStripe, &QAbstractButton::toggled, this, &YACReaderOptionsDialog::optionsChanged);
    connect(gl->radioOver, &QAbstractButton::toggled, this, &YACReaderOptionsDialog::optionsChanged);
    connect(gl->radionModern, &QAbstractButton::toggled, this, &YACReaderOptionsDialog::optionsChanged);
    connect(gl->radioDown, &QAbstractButton::toggled, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->xRotation, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::saveXRotation);
    connect(gl->xRotation, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->yPosition, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::saveYPosition);
    connect(gl->yPosition, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->coverDistance, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::saveCoverDistance);
    connect(gl->coverDistance, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->centralDistance, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::saveCentralDistance);
    connect(gl->centralDistance, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->zoomLevel, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::saveZoomLevel);
    connect(gl->zoomLevel, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->yCoverOffset, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::saveYCoverOffset);
    connect(gl->yCoverOffset, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->zCoverOffset, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::saveZCoverOffset);
    connect(gl->zCoverOffset, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->coverRotation, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::saveCoverRotation);
    connect(gl->coverRotation, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->fadeOutDist, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::saveFadeOutDist);
    connect(gl->fadeOutDist, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->lightStrength, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::saveLightStrength);
    connect(gl->lightStrength, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->maxAngle, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::saveMaxAngle);
    connect(gl->maxAngle, &YACReaderSpinSliderWidget::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->performanceSlider, &QAbstractSlider::valueChanged, this, &YACReaderOptionsDialog::savePerformance);
    connect(gl->performanceSlider, &QAbstractSlider::valueChanged, this, &YACReaderOptionsDialog::optionsChanged);

    connect(gl->vSyncCheck, &QCheckBox::checkStateChanged, this, &YACReaderOptionsDialog::saveUseVSync);
}

void YACReaderOptionsDialog::savePerformance(int value)
{
    settings->setValue(PERFORMANCE, value);
}

void YACReaderOptionsDialog::saveUseVSync(int b)
{
    settings->setValue(V_SYNC, b);
}

void YACReaderOptionsDialog::saveFlowParameters()
{
    settings->setValue(X_ROTATION, gl->xRotation->getValue());
    settings->setValue(Y_POSITION, gl->yPosition->getValue());
    settings->setValue(COVER_DISTANCE, gl->coverDistance->getValue());
    settings->setValue(CENTRAL_DISTANCE, gl->centralDistance->getValue());
    settings->setValue(ZOOM_LEVEL, gl->zoomLevel->getValue());
    settings->setValue(Y_COVER_OFFSET, gl->yCoverOffset->getValue());
    settings->setValue(Z_COVER_OFFSET, gl->zCoverOffset->getValue());
    settings->setValue(COVER_ROTATION, gl->coverRotation->getValue());
    settings->setValue(FADE_OUT_DIST, gl->fadeOutDist->getValue());
    settings->setValue(LIGHT_STRENGTH, gl->lightStrength->getValue());
    settings->setValue(MAX_ANGLE, gl->maxAngle->getValue());
}

void YACReaderOptionsDialog::saveOptions()
{
    emit optionsChanged();
    close();
}

void YACReaderOptionsDialog::saveXRotation(int value)
{
    settings->setValue(FLOW_TYPE_GL, YACReader::Custom);
    settings->setValue(X_ROTATION, value);
}
void YACReaderOptionsDialog::saveYPosition(int value)
{
    settings->setValue(FLOW_TYPE_GL, YACReader::Custom);
    settings->setValue(Y_POSITION, value);
}
void YACReaderOptionsDialog::saveCoverDistance(int value)
{
    settings->setValue(FLOW_TYPE_GL, YACReader::Custom);
    settings->setValue(COVER_DISTANCE, value);
}
void YACReaderOptionsDialog::saveCentralDistance(int value)
{
    settings->setValue(FLOW_TYPE_GL, YACReader::Custom);
    settings->setValue(CENTRAL_DISTANCE, value);
}
void YACReaderOptionsDialog::saveZoomLevel(int value)
{
    settings->setValue(FLOW_TYPE_GL, YACReader::Custom);
    settings->setValue(ZOOM_LEVEL, value);
}
void YACReaderOptionsDialog::saveYCoverOffset(int value)
{
    settings->setValue(FLOW_TYPE_GL, YACReader::Custom);
    settings->setValue(Y_COVER_OFFSET, value);
}
void YACReaderOptionsDialog::saveZCoverOffset(int value)
{
    settings->setValue(FLOW_TYPE_GL, YACReader::Custom);
    settings->setValue(Z_COVER_OFFSET, value);
}
void YACReaderOptionsDialog::saveCoverRotation(int value)
{
    settings->setValue(FLOW_TYPE_GL, YACReader::Custom);
    settings->setValue(COVER_ROTATION, value);
}
void YACReaderOptionsDialog::saveFadeOutDist(int value)
{
    settings->setValue(FLOW_TYPE_GL, YACReader::Custom);
    settings->setValue(FADE_OUT_DIST, value);
}
void YACReaderOptionsDialog::saveLightStrength(int value)
{
    settings->setValue(FLOW_TYPE_GL, YACReader::Custom);
    settings->setValue(LIGHT_STRENGTH, value);
}

void YACReaderOptionsDialog::saveMaxAngle(int value)
{
    settings->setValue(FLOW_TYPE_GL, YACReader::Custom);
    settings->setValue(MAX_ANGLE, value);
}

void YACReaderOptionsDialog::restoreOptions(QSettings *settings)
{
    this->settings = settings;

    // FLOW CONFIG

    if (!settings->contains(FLOW_TYPE_GL)) {
        setClassicConfig();
        gl->radioClassic->setChecked(true);
        gl->performanceSlider->setValue(1);
        return;
    }

    if (settings->contains(V_SYNC) && settings->value(V_SYNC).toInt() == Qt::Checked)
        gl->vSyncCheck->setChecked(true);
    else
        gl->vSyncCheck->setChecked(false);

    gl->performanceSlider->setValue(settings->value(PERFORMANCE).toInt());

    YACReader::FlowType flowType;
    switch (settings->value(FLOW_TYPE_GL).toInt()) {
    case 0:
        flowType = YACReader::CoverFlowLike;
        break;
    case 1:
        flowType = YACReader::Strip;
        break;
    case 2:
        flowType = YACReader::StripOverlapped;
        break;
    case 3:
        flowType = YACReader::Modern;
        break;
    case 4:
        flowType = YACReader::Roulette;
        break;
    case 5:
        flowType = YACReader::Custom;
        break;
    default:
        flowType = YACReader::CoverFlowLike;
    }

    if (flowType == YACReader::Custom) {
        loadConfig();
        return;
    }

    if (flowType == YACReader::CoverFlowLike) {
        setClassicConfig();
        gl->radioClassic->setChecked(true);
        return;
    }

    if (flowType == YACReader::Strip) {
        setStripeConfig();
        gl->radioStripe->setChecked(true);
        return;
    }

    if (flowType == YACReader::StripOverlapped) {
        setOverlappedStripeConfig();
        gl->radioOver->setChecked(true);
        return;
    }

    if (flowType == YACReader::Modern) {
        setModernConfig();
        gl->radionModern->setChecked(true);
        return;
    }

    if (flowType == YACReader::Roulette) {
        setRouletteConfig();
        gl->radioDown->setChecked(true);
        return;
    }

    // END FLOW CONFIG
}

void YACReaderOptionsDialog::restoreOptions()
{
    restoreOptions(settings);
}

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

void YACReaderOptionsDialog::setClassicConfig()
{
    gl->setValues(presetYACReaderFlowClassicConfig);

    saveFlowParameters();

    settings->setValue(FLOW_TYPE_GL, YACReader::CoverFlowLike);
}

void YACReaderOptionsDialog::setStripeConfig()
{
    gl->setValues(presetYACReaderFlowStripeConfig);

    saveFlowParameters();

    settings->setValue(FLOW_TYPE_GL, YACReader::Strip);
}

void YACReaderOptionsDialog::setOverlappedStripeConfig()
{
    gl->setValues(presetYACReaderFlowOverlappedStripeConfig);

    saveFlowParameters();

    settings->setValue(FLOW_TYPE_GL, YACReader::StripOverlapped);
}

void YACReaderOptionsDialog::setModernConfig()
{
    gl->setValues(defaultYACReaderFlowConfig);

    saveFlowParameters();

    settings->setValue(FLOW_TYPE_GL, YACReader::Modern);
}

void YACReaderOptionsDialog::setRouletteConfig()
{
    gl->setValues(pressetYACReaderFlowDownConfig);

    saveFlowParameters();

    settings->setValue(FLOW_TYPE_GL, YACReader::Roulette);
}
