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
    void numDaysToConsiderRecentChanged(int value);
    void resetToDefaults();

private:
    // General tabs
    QCheckBox *displayContinueReadingBannerCheck;
    QCheckBox *trayIconCheckbox;
    QCheckBox *startToTrayCheckbox;
    QCheckBox *comicInfoXMLCheckbox;
    QSlider *recentIntervalSlider;
    QLabel *numDaysLabel;

    // Libraries tab
    QCheckBox *updateLibrariesAtStartupCheck;
    QCheckBox *detectChangesAutomaticallyCheck;
    QCheckBox *updateLibrariesPeriodicallyCheck;
    QComboBox *intervalComboBox;
    QCheckBox *updateLibrariesAtCertainTimeCheck;
    QTimeEdit *updateLibrariesTimeEdit;
    QCheckBox *compareModifiedDateWhenUpdatingLibrariesCheck;

    // Grid tab
    QCheckBox *useBackgroundImageCheck;
    QCheckBox *useCurrentComicCoverCheck;
    QSlider *backgroundImageOpacitySlider;
    QSlider *backgroundImageBlurRadiusSlider;
    QLabel *opacityLabel;
    QLabel *blurLabel;
    QPushButton *resetButton;

    QWidget *createGeneralTab();
    QWidget *createLibrariesTab();
    QWidget *createFlowTab();
    QWidget *createGridTab();
};

#endif
