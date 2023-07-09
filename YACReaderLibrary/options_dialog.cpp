#include "options_dialog.h"

#ifndef NO_OPENGL
#include "yacreader_flow_gl.h"
#include "yacreader_gl_flow_config_widget.h"
#endif
#include "yacreader_flow_config_widget.h"
#include "api_key_dialog.h"

#include "yacreader_global_gui.h"

#ifndef NO_OPENGL
FlowType flowType = Strip;
#endif

OptionsDialog::OptionsDialog(QWidget *parent)
    : YACReaderOptionsDialog(parent)
{
    auto tabWidget = new QTabWidget();

    auto layout = new QVBoxLayout(this);

    auto flowLayout = new QVBoxLayout;
    auto gridViewLayout = new QVBoxLayout();
    auto generalLayout = new QVBoxLayout();

    auto switchFlowType = new QHBoxLayout();
    switchFlowType->addStretch();
#ifndef NO_OPENGL
    switchFlowType->addWidget(useGL);
#endif
    auto buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(accept);
    buttons->addWidget(cancel);

    flowLayout->addWidget(sw);
#ifndef NO_OPENGL
    flowLayout->addWidget(gl);
#endif
    flowLayout->addLayout(switchFlowType);

#ifndef NO_OPENGL
    sw->hide();
#endif
    // Tray icon settings
    QGroupBox *trayIconBox = new QGroupBox(tr("Tray icon settings (experimental)"));
    QVBoxLayout *trayLayout = new QVBoxLayout();

    trayIconCheckbox = new QCheckBox(tr("Close to tray"));
    startToTrayCheckbox = new QCheckBox(tr("Start into the system tray"));

    connect(trayIconCheckbox, &QCheckBox::clicked, this,
            [=](bool checked) {
                settings->setValue(CLOSE_TO_TRAY, checked);
                startToTrayCheckbox->setEnabled(checked);
                emit optionsChanged();
            });
    connect(startToTrayCheckbox, &QCheckBox::clicked, this,
            [=](bool checked) {
                settings->setValue(START_TO_TRAY, checked);
            });

    trayLayout->addWidget(trayIconCheckbox);
    trayLayout->addWidget(startToTrayCheckbox);
    trayIconBox->setLayout(trayLayout);

    auto apiKeyLayout = new QVBoxLayout();
    auto apiKeyButton = new QPushButton(tr("Edit Comic Vine API key"));
    apiKeyLayout->addWidget(apiKeyButton);

    auto apiKeyBox = new QGroupBox(tr("Comic Vine API key"));
    apiKeyBox->setLayout(apiKeyLayout);

    connect(apiKeyButton, &QAbstractButton::clicked, this, &OptionsDialog::editApiKey);

    auto comicInfoXMLBox = new QGroupBox(tr("ComicInfo.xml legacy support"));

    comicInfoXMLCheckbox = new QCheckBox(tr("Import metada from ComicInfo.xml when adding new comics"));
    connect(comicInfoXMLCheckbox, &QCheckBox::clicked, this,
            [=](bool checked) {
                settings->setValue(IMPORT_COMIC_INFO_XML_METADATA, checked);
            });

    auto comicInfoXMLBoxLayout = new QVBoxLayout();
    comicInfoXMLBoxLayout->addWidget(comicInfoXMLCheckbox);
    comicInfoXMLBox->setLayout(comicInfoXMLBoxLayout);

    auto recentlyAddedBox = new QGroupBox(tr("Consider 'recent' items added or updated since X days ago"));
    recentIntervalSlider = new QSlider(Qt::Horizontal);
    recentIntervalSlider->setRange(1, 30);
    auto recentlyAddedLayout = new QHBoxLayout();
    numDaysLabel = new QLabel();
    numDaysLabel->setMidLineWidth(50);
    recentlyAddedLayout->addWidget(numDaysLabel);
    recentlyAddedLayout->addWidget(recentIntervalSlider);
    recentlyAddedBox->setLayout(recentlyAddedLayout);

    connect(recentIntervalSlider, &QAbstractSlider::valueChanged, this, &OptionsDialog::numDaysToConsiderRecentChanged);

    auto libraryUpdatesBox = new QGroupBox(tr("Library update"));

    compareModifiedDateWhenUpdatingLibrariesCheck = new QCheckBox(tr("Compare the modified date of files when updating a library"));
    connect(compareModifiedDateWhenUpdatingLibrariesCheck, &QCheckBox::clicked, this,
            [=](bool checked) {
                settings->setValue(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, checked);
            });

    auto libraryUpdatesBoxLayout = new QVBoxLayout();
    libraryUpdatesBoxLayout->addWidget(compareModifiedDateWhenUpdatingLibrariesCheck);
    libraryUpdatesBox->setLayout(libraryUpdatesBoxLayout);

    // grid view background config
    useBackgroundImageCheck = new QCheckBox(tr("Enable background image"));

    opacityLabel = new QLabel(tr("Opacity level"));

    backgroundImageOpacitySlider = new QSlider(Qt::Horizontal);
    backgroundImageOpacitySlider->setRange(5, 100);

    blurLabel = new QLabel(tr("Blur level"));

    backgroundImageBlurRadiusSlider = new QSlider(Qt::Horizontal);
    backgroundImageBlurRadiusSlider->setRange(0, 100);

    useCurrentComicCoverCheck = new QCheckBox(tr("Use selected comic cover as background"));

    resetButton = new QPushButton(tr("Restore defautls"));

    auto gridBackgroundLayout = new QVBoxLayout();
    gridBackgroundLayout->addWidget(useBackgroundImageCheck);
    gridBackgroundLayout->addWidget(opacityLabel);
    gridBackgroundLayout->addWidget(backgroundImageOpacitySlider);
    gridBackgroundLayout->addWidget(blurLabel);
    gridBackgroundLayout->addWidget(backgroundImageBlurRadiusSlider);
    gridBackgroundLayout->addWidget(useCurrentComicCoverCheck);
    gridBackgroundLayout->addWidget(resetButton, 0, Qt::AlignRight);

    auto gridBackgroundGroup = new QGroupBox(tr("Background"));
    gridBackgroundGroup->setLayout(gridBackgroundLayout);

    displayContinueReadingBannerCheck = new QCheckBox(tr("Display continue reading banner"));

    auto continueReadingLayout = new QVBoxLayout();
    continueReadingLayout->addWidget(displayContinueReadingBannerCheck);

    auto continueReadingGroup = new QGroupBox(tr("Continue reading"));
    continueReadingGroup->setLayout(continueReadingLayout);

    gridViewLayout->addWidget(gridBackgroundGroup);
    gridViewLayout->addWidget(continueReadingGroup);
    gridViewLayout->addStretch();

    connect(useBackgroundImageCheck, &QAbstractButton::clicked, this, &OptionsDialog::useBackgroundImageCheckClicked);
    connect(backgroundImageOpacitySlider, &QAbstractSlider::valueChanged, this, &OptionsDialog::backgroundImageOpacitySliderChanged);
    connect(backgroundImageBlurRadiusSlider, &QAbstractSlider::valueChanged, this, &OptionsDialog::backgroundImageBlurRadiusSliderChanged);
    connect(useCurrentComicCoverCheck, &QCheckBox::clicked, this, &OptionsDialog::useCurrentComicCoverCheckClicked);
    connect(resetButton, &QPushButton::clicked, this, &OptionsDialog::resetToDefaults);
    // end grid view background config

    connect(displayContinueReadingBannerCheck, &QCheckBox::clicked, this, [this]() {
        this->settings->setValue(DISPLAY_CONTINUE_READING_IN_GRID_VIEW, this->displayContinueReadingBannerCheck->isChecked());

        emit optionsChanged();
    });

    auto comicFlowW = new QWidget;
    comicFlowW->setLayout(flowLayout);

    auto gridViewW = new QWidget;
    gridViewW->setLayout(gridViewLayout);

    auto generalW = new QWidget;
    generalW->setLayout(generalLayout);
    generalLayout->addWidget(trayIconBox);
    generalLayout->addWidget(shortcutsBox);
    generalLayout->addWidget(apiKeyBox);
    generalLayout->addWidget(comicInfoXMLBox);
    generalLayout->addWidget(recentlyAddedBox);
    generalLayout->addWidget(libraryUpdatesBox);
    generalLayout->addStretch();

    tabWidget->addTab(generalW, tr("General"));
    tabWidget->addTab(comicFlowW, tr("Comic Flow"));
#ifndef NO_OPENGL
    tabWidget->addTab(gridViewW, tr("Grid view"));
#endif

    layout->addWidget(tabWidget);
    layout->addLayout(buttons);
    setLayout(layout);
    setModal(true);
    setWindowTitle(tr("Options"));

    this->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void OptionsDialog::editApiKey()
{
    ApiKeyDialog d;
    d.exec();
}

void OptionsDialog::restoreOptions(QSettings *settings)
{
    YACReaderOptionsDialog::restoreOptions(settings);

    trayIconCheckbox->setChecked(settings->value(CLOSE_TO_TRAY, false).toBool());
    startToTrayCheckbox->setChecked(settings->value(START_TO_TRAY, false).toBool());
    startToTrayCheckbox->setEnabled(trayIconCheckbox->isChecked());

    comicInfoXMLCheckbox->setChecked(settings->value(IMPORT_COMIC_INFO_XML_METADATA, false).toBool());

    recentIntervalSlider->setValue(settings->value(NUM_DAYS_TO_CONSIDER_RECENT, 1).toInt());

    compareModifiedDateWhenUpdatingLibrariesCheck->setChecked(settings->value(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, false).toBool());

    bool useBackgroundImage = settings->value(USE_BACKGROUND_IMAGE_IN_GRID_VIEW, true).toBool();

    useBackgroundImageCheck->setChecked(useBackgroundImage);
    backgroundImageOpacitySlider->setValue(settings->value(OPACITY_BACKGROUND_IMAGE_IN_GRID_VIEW, 0.2).toFloat() * 100);
    backgroundImageBlurRadiusSlider->setValue(settings->value(BLUR_RADIUS_BACKGROUND_IMAGE_IN_GRID_VIEW, 75).toInt());
    useCurrentComicCoverCheck->setChecked(settings->value(USE_SELECTED_COMIC_COVER_AS_BACKGROUND_IMAGE_IN_GRID_VIEW, false).toBool());

    backgroundImageOpacitySlider->setVisible(useBackgroundImage);
    backgroundImageBlurRadiusSlider->setVisible(useBackgroundImage);
    opacityLabel->setVisible(useBackgroundImage);
    blurLabel->setVisible(useBackgroundImage);
    useCurrentComicCoverCheck->setVisible(useBackgroundImage);

    displayContinueReadingBannerCheck->setChecked(settings->value(DISPLAY_CONTINUE_READING_IN_GRID_VIEW, true).toBool());
}

void OptionsDialog::useBackgroundImageCheckClicked(bool checked)
{
    settings->setValue(USE_BACKGROUND_IMAGE_IN_GRID_VIEW, checked);

    backgroundImageOpacitySlider->setVisible(checked);
    backgroundImageBlurRadiusSlider->setVisible(checked);
    opacityLabel->setVisible(checked);
    blurLabel->setVisible(checked);
    useCurrentComicCoverCheck->setVisible(checked);

    emit optionsChanged();
}

void OptionsDialog::backgroundImageOpacitySliderChanged(int value)
{
    settings->setValue(OPACITY_BACKGROUND_IMAGE_IN_GRID_VIEW, value / 100.0);

    emit optionsChanged();
}

void OptionsDialog::backgroundImageBlurRadiusSliderChanged(int value)
{
    settings->setValue(BLUR_RADIUS_BACKGROUND_IMAGE_IN_GRID_VIEW, value);

    emit optionsChanged();
}

void OptionsDialog::useCurrentComicCoverCheckClicked(bool checked)
{
    settings->setValue(USE_SELECTED_COMIC_COVER_AS_BACKGROUND_IMAGE_IN_GRID_VIEW, checked);

    emit optionsChanged();
}

void OptionsDialog::numDaysToConsiderRecentChanged(int value)
{
    settings->setValue(NUM_DAYS_TO_CONSIDER_RECENT, value);

    numDaysLabel->setText(QString("%1").arg(value));

    emit optionsChanged();
}

void OptionsDialog::resetToDefaults()
{
    settings->setValue(OPACITY_BACKGROUND_IMAGE_IN_GRID_VIEW, 0.2);
    settings->setValue(BLUR_RADIUS_BACKGROUND_IMAGE_IN_GRID_VIEW, 75);
    settings->setValue(USE_SELECTED_COMIC_COVER_AS_BACKGROUND_IMAGE_IN_GRID_VIEW, false);

    restoreOptions(settings);

    emit optionsChanged();
}
