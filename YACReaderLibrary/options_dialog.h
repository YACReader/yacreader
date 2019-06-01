#ifndef __OPTIONS_DIALOG_H
#define __OPTIONS_DIALOG_H

#include <QtWidgets>

#include "yacreader_options_dialog.h"

#include "yacreader_global.h"

using namespace YACReader;

class OptionsDialog : public YACReaderOptionsDialog
{
    Q_OBJECT
public:
    OptionsDialog(QWidget *parent = nullptr);

public slots:
    void editApiKey();
    void restoreOptions(QSettings *settings) override;

private slots:
    void useBackgroundImageCheckClicked(bool checked);
    void backgroundImageOpacitySliderChanged(int value);
    void backgroundImageBlurRadiusSliderChanged(int value);
    void useCurrentComicCoverCheckClicked(bool checked);
    void showCurrentComicBannerCheckClicked(bool checked);
    void resetToDefaults();

private:
    QCheckBox *useBackgroundImageCheck;
    QCheckBox *useCurrentComicCoverCheck;
    QSlider *backgroundImageOpacitySlider;
    QSlider *backgroundImageBlurRadiusSlider;
    QLabel *opacityLabel;
    QLabel *blurLabel;
    QPushButton *resetButton;
    QCheckBox *displayContinueReadingBannerCheck;
    QCheckBox *trayIconCheckbox;
    QCheckBox *startToTrayCheckbox;
};

#endif
