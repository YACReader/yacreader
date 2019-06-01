#ifndef __OPTIONS_DIALOG_H
#define __OPTIONS_DIALOG_H

#include "yacreader_options_dialog.h"

class QDialog;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QPushButton;
class QRadioButton;
class QColorDialog;
class YACReaderSpinSliderWidget;

class OptionsDialog : public YACReaderOptionsDialog
{
    Q_OBJECT
public:
    OptionsDialog(QWidget *parent = nullptr);

private:
    //QLabel * pathLabel;
    QLineEdit *pathEdit;
    QPushButton *pathFindButton;
    QCheckBox *quickNavi;
    QCheckBox *disableShowOnMouseOver;

    QLabel *magGlassSizeLabel;

    QLabel *zoomLevel;

    //QLabel * slideSizeLabel;
    QSlider *slideSize;

    //QLabel * fitToWidthRatioLabel;
    //QSlider * fitToWidthRatioS;

    QLabel *backgroundColor;
    QPushButton *selectBackgroundColorButton;

    QColorDialog *colorDialog;

    YACReaderSpinSliderWidget *brightnessS;

    YACReaderSpinSliderWidget *contrastS;

    YACReaderSpinSliderWidget *gammaS;

public slots:
    void saveOptions() override;
    void restoreOptions(QSettings *settings) override;
    void findFolder();
    void updateColor(const QColor &color);
    //void fitToWidthRatio(int value);
    void brightnessChanged(int value);
    void contrastChanged(int value);
    void gammaChanged(int value);
    void resetImageConfig();
    void show();
    void setFilters(int brightness, int contrast, int gamma);

signals:
    void changedOptions();
    void changedImageOptions();
    void changedFilters(int brightness, int contrast, int gamma);
    //void fitToWidthRatioChanged(float ratio);
};

#endif
