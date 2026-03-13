#ifndef YACREADER_3D_FLOW_CONFIG_WIDGET_H
#define YACREADER_3D_FLOW_CONFIG_WIDGET_H

#include "flow_types.h" //TODO

#include <QWidget>

class QRadioButton;
class YACReaderSpinSliderWidget;
class QSlider;
class QCheckBox;
class QPushButton;
class QGroupBox;

class YACReader3DFlowConfigWidget : public QWidget
{
    Q_OBJECT
public:
    YACReader3DFlowConfigWidget(QWidget *parent = 0);

    // GL.........................
    QRadioButton *radioClassic;
    QRadioButton *radioStripe;
    QRadioButton *radioOver;
    QRadioButton *radionModern;
    QRadioButton *radioDown;

    YACReaderSpinSliderWidget *xRotation;
    YACReaderSpinSliderWidget *yPosition;
    YACReaderSpinSliderWidget *coverDistance;
    YACReaderSpinSliderWidget *centralDistance;
    YACReaderSpinSliderWidget *zoomLevel;
    YACReaderSpinSliderWidget *yCoverOffset;
    YACReaderSpinSliderWidget *zCoverOffset;
    YACReaderSpinSliderWidget *coverRotation;
    YACReaderSpinSliderWidget *fadeOutDist;
    YACReaderSpinSliderWidget *lightStrength;
    YACReaderSpinSliderWidget *maxAngle;

    QSlider *performanceSlider;
    QCheckBox *vSyncCheck;

    QPushButton *showAdvancedOptions;
    QGroupBox *optionsGroupBox;

public slots:
    void setValues(Preset preset);
    void avancedOptionToogled(bool show);
};

#endif // YACREADER_3D_FLOW_CONFIG_WIDGET_H
