#ifndef YACREADER_GL_FLOW_CONFIG_WIDGET_H
#define YACREADER_GL_FLOW_CONFIG_WIDGET_H

#include "yacreader_flow_gl.h" //TODO
#include <QWidget>

class QRadioButton;
class YACReaderSpinSliderWidget;
class QSlider;
class QCheckBox;
class QPushButton;
class QGroupBox;

class YACReaderGLFlowConfigWidget : public QWidget
{
    Q_OBJECT
public:
    YACReaderGLFlowConfigWidget(QWidget *parent = 0);

    //GL.........................
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

#endif // YACREADER_GL_FLOW_CONFIG_WIDGET_H