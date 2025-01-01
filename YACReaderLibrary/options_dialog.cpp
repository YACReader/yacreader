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
    auto generalW = createGeneralTab();
    auto librariesW = createLibrariesTab();
    auto comicFlowW = createFlowTab();
    auto gridViewW = createGridTab();

    auto tabWidget = new QTabWidget();
    tabWidget->addTab(generalW, tr("General"));
    tabWidget->addTab(librariesW, tr("Libraries"));
    tabWidget->addTab(comicFlowW, tr("Comic Flow"));
#ifndef NO_OPENGL
    tabWidget->addTab(gridViewW, tr("Grid view"));
#endif

    auto buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(accept);
    buttons->addWidget(cancel);

    auto layout = new QVBoxLayout(this);
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

    displayGlobalContinueReadingBannerCheck->setChecked(settings->value(DISPLAY_GLOBAL_CONTINUE_READING_IN_GRID_VIEW, true).toBool());
    displayContinueReadingBannerCheck->setChecked(settings->value(DISPLAY_CONTINUE_READING_IN_GRID_VIEW, true).toBool());

    updateLibrariesAtStartupCheck->setChecked(settings->value(UPDATE_LIBRARIES_AT_STARTUP, false).toBool());
    detectChangesAutomaticallyCheck->setChecked(settings->value(DETECT_CHANGES_IN_LIBRARIES_AUTOMATICALLY, false).toBool());
    updateLibrariesPeriodicallyCheck->setChecked(settings->value(UPDATE_LIBRARIES_PERIODICALLY, false).toBool());
    intervalComboBox->setCurrentIndex(settings->value(UPDATE_LIBRARIES_PERIODICALLY_INTERVAL, static_cast<typename std::underlying_type<YACReader::LibrariesUpdateInterval>::type>(LibrariesUpdateInterval::Hours2)).toInt());
    updateLibrariesAtCertainTimeCheck->setChecked(settings->value(UPDATE_LIBRARIES_AT_CERTAIN_TIME, false).toBool());
    updateLibrariesTimeEdit->setTime(settings->value(UPDATE_LIBRARIES_AT_CERTAIN_TIME_TIME, "00:00").toTime());

    compareModifiedDateWhenUpdatingLibrariesCheck->setChecked(settings->value(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, false).toBool());

    thirdPartyReaderEdit->setText(settings->value(THIRD_PARTY_READER_COMMAND, "").toString());
}

void OptionsDialog::saveOptions()
{
    settings->setValue(THIRD_PARTY_READER_COMMAND, thirdPartyReaderEdit->text());
    YACReaderOptionsDialog::saveOptions();
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

QWidget *OptionsDialog::createGeneralTab()
{
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

    comicInfoXMLCheckbox = new QCheckBox(tr("Import metadata from ComicInfo.xml when adding new comics"));
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

    auto thirdPartyReaderBox = new QGroupBox(tr("Third party reader"));
    thirdPartyReaderEdit = new QLineEdit();
    thirdPartyReaderEdit->setPlaceholderText(tr("Write {comic_file_path} where the path should go in the command"));
    auto clearButton = new QPushButton(tr("Clear"));
    auto thirdPartyReaderLayout = new QHBoxLayout();
    thirdPartyReaderLayout->addWidget(thirdPartyReaderEdit, 1);
    thirdPartyReaderLayout->addWidget(clearButton);
    thirdPartyReaderBox->setLayout(thirdPartyReaderLayout);
    connect(clearButton, &QPushButton::clicked, thirdPartyReaderEdit, &QLineEdit::clear);

    auto generalLayout = new QVBoxLayout();
    generalLayout->addWidget(trayIconBox);
    generalLayout->addWidget(shortcutsBox);
    generalLayout->addWidget(apiKeyBox);
    generalLayout->addWidget(comicInfoXMLBox);
    generalLayout->addWidget(recentlyAddedBox);
    generalLayout->addWidget(thirdPartyReaderBox);
    generalLayout->addStretch();

    auto generalW = new QWidget;
    generalW->setLayout(generalLayout);

    return generalW;
}

QWidget *OptionsDialog::createLibrariesTab()
{
    updateLibrariesAtStartupCheck = new QCheckBox(tr("Update libraries at startup"));
    connect(updateLibrariesAtStartupCheck, &QCheckBox::clicked, this,
            [=](bool checked) {
                settings->setValue(UPDATE_LIBRARIES_AT_STARTUP, checked);
            });
    detectChangesAutomaticallyCheck = new QCheckBox(tr("Try to detect changes automatically"));
    connect(detectChangesAutomaticallyCheck, &QCheckBox::clicked, this,
            [=](bool checked) {
                settings->setValue(DETECT_CHANGES_IN_LIBRARIES_AUTOMATICALLY, checked);
            });
    updateLibrariesPeriodicallyCheck = new QCheckBox(tr("Update libraries periodically"));
    connect(updateLibrariesPeriodicallyCheck, &QCheckBox::clicked, this,
            [=](bool checked) {
                settings->setValue(UPDATE_LIBRARIES_PERIODICALLY, checked);
            });

    auto intervalLabel = new QLabel(tr("Interval:"));
    intervalComboBox = new QComboBox;
    intervalComboBox->addItem(tr("30 minutes"), static_cast<typename std::underlying_type<YACReader::LibrariesUpdateInterval>::type>(LibrariesUpdateInterval::Minutes30));
    intervalComboBox->addItem(tr("1 hour"), static_cast<typename std::underlying_type<YACReader::LibrariesUpdateInterval>::type>(LibrariesUpdateInterval::Hourly));
    intervalComboBox->addItem(tr("2 hours"), static_cast<typename std::underlying_type<YACReader::LibrariesUpdateInterval>::type>(LibrariesUpdateInterval::Hours2));
    intervalComboBox->addItem(tr("4 hours"), static_cast<typename std::underlying_type<YACReader::LibrariesUpdateInterval>::type>(LibrariesUpdateInterval::Hours4));
    intervalComboBox->addItem(tr("8 hours"), static_cast<typename std::underlying_type<YACReader::LibrariesUpdateInterval>::type>(LibrariesUpdateInterval::Hours8));
    intervalComboBox->addItem(tr("12 hours"), static_cast<typename std::underlying_type<YACReader::LibrariesUpdateInterval>::type>(LibrariesUpdateInterval::Hours12));
    intervalComboBox->addItem(tr("daily"), static_cast<typename std::underlying_type<YACReader::LibrariesUpdateInterval>::type>(LibrariesUpdateInterval::Daily));

    connect(intervalComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [=](int index) {
                settings->setValue(UPDATE_LIBRARIES_PERIODICALLY_INTERVAL, index);
            });

    updateLibrariesAtCertainTimeCheck = new QCheckBox(tr("Update libraries at certain time"));
    connect(updateLibrariesAtCertainTimeCheck, &QCheckBox::clicked, this,
            [=](bool checked) {
                settings->setValue(UPDATE_LIBRARIES_AT_CERTAIN_TIME, checked);
            });

    auto timeLabel = new QLabel(tr("Time:"));
    updateLibrariesTimeEdit = new QTimeEdit;
    updateLibrariesTimeEdit->setDisplayFormat("hh:mm");
    connect(updateLibrariesTimeEdit, &QTimeEdit::timeChanged, this,
            [=](const QTime &time) {
                settings->setValue(UPDATE_LIBRARIES_AT_CERTAIN_TIME_TIME, time.toString("hh:mm"));
            });

    auto updateLibrariesPeriodicallyLayout = new QHBoxLayout;
    updateLibrariesPeriodicallyLayout->addWidget(intervalLabel);
    updateLibrariesPeriodicallyLayout->addWidget(intervalComboBox);
    updateLibrariesPeriodicallyLayout->addStretch();

    auto updateLibrariesAtCertainTimeLayout = new QHBoxLayout;
    updateLibrariesAtCertainTimeLayout->addWidget(timeLabel);
    updateLibrariesAtCertainTimeLayout->addWidget(updateLibrariesTimeEdit);
    updateLibrariesAtCertainTimeLayout->addStretch();

    auto librariesBoxLayout = new QVBoxLayout();
    librariesBoxLayout->addWidget(updateLibrariesAtStartupCheck);
    // TODO: try to use `QFileSystemWatcher`
    // librariesBoxLayout->addWidget(detectChangesAutomaticallyCheck);
    librariesBoxLayout->addWidget(updateLibrariesPeriodicallyCheck);
    librariesBoxLayout->addLayout(updateLibrariesPeriodicallyLayout);
    librariesBoxLayout->addWidget(updateLibrariesAtCertainTimeCheck);
    librariesBoxLayout->addLayout(updateLibrariesAtCertainTimeLayout);

    librariesBoxLayout->addWidget(new QLabel(tr("WARNING! During library updates writes to the database are disabled!\n"
                                                "Don't schedule updates while you may be using the app actively.\n"
                                                "During automatic updates the app will block some of the actions until the update is finished.\n"
                                                "To stop an automatic update tap on the loading indicator next to the Libraries title.")));

    auto librariesBox = new QGroupBox(tr("Libraries"));
    librariesBox->setLayout(librariesBoxLayout);

    auto libraryModificationsBox = new QGroupBox(tr("Modifications detection"));

    compareModifiedDateWhenUpdatingLibrariesCheck = new QCheckBox(tr("Compare the modified date of files when updating a library (not recommended)"));
    connect(compareModifiedDateWhenUpdatingLibrariesCheck, &QCheckBox::clicked, this,
            [=](bool checked) {
                settings->setValue(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, checked);
            });

    auto libraryUpdatesBoxLayout = new QVBoxLayout();
    libraryUpdatesBoxLayout->addWidget(compareModifiedDateWhenUpdatingLibrariesCheck);
    libraryModificationsBox->setLayout(libraryUpdatesBoxLayout);

    auto layout = new QVBoxLayout();
    layout->addWidget(librariesBox);
    layout->addWidget(libraryModificationsBox);
    layout->addStretch();

    auto librariesW = new QWidget;
    librariesW->setLayout(layout);

    return librariesW;
}

QWidget *OptionsDialog::createFlowTab()
{
    auto switchFlowType = new QHBoxLayout();
    switchFlowType->addStretch();
#ifndef NO_OPENGL
    switchFlowType->addWidget(useGL);
#endif

    auto flowLayout = new QVBoxLayout;
    flowLayout->addWidget(sw);
#ifndef NO_OPENGL
    flowLayout->addWidget(gl);
#endif
    flowLayout->addLayout(switchFlowType);

#ifndef NO_OPENGL
    sw->hide();
#endif

    auto comicFlowW = new QWidget;
    comicFlowW->setLayout(flowLayout);

    return comicFlowW;
}

QWidget *OptionsDialog::createGridTab()
{
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

    displayGlobalContinueReadingBannerCheck = new QCheckBox(tr("Display continue reading banner"));
    displayContinueReadingBannerCheck = new QCheckBox(tr("Display current comic banner"));

    auto continueReadingLayout = new QVBoxLayout();
    continueReadingLayout->addWidget(displayGlobalContinueReadingBannerCheck);
    continueReadingLayout->addWidget(displayContinueReadingBannerCheck);

    auto continueReadingGroup = new QGroupBox(tr("Continue reading"));
    continueReadingGroup->setLayout(continueReadingLayout);

    connect(useBackgroundImageCheck, &QAbstractButton::clicked, this, &OptionsDialog::useBackgroundImageCheckClicked);
    connect(backgroundImageOpacitySlider, &QAbstractSlider::valueChanged, this, &OptionsDialog::backgroundImageOpacitySliderChanged);
    connect(backgroundImageBlurRadiusSlider, &QAbstractSlider::valueChanged, this, &OptionsDialog::backgroundImageBlurRadiusSliderChanged);
    connect(useCurrentComicCoverCheck, &QCheckBox::clicked, this, &OptionsDialog::useCurrentComicCoverCheckClicked);
    connect(resetButton, &QPushButton::clicked, this, &OptionsDialog::resetToDefaults);
    // end grid view background config

    connect(displayGlobalContinueReadingBannerCheck, &QCheckBox::clicked, this, [this]() {
        this->settings->setValue(DISPLAY_GLOBAL_CONTINUE_READING_IN_GRID_VIEW, this->displayGlobalContinueReadingBannerCheck->isChecked());

        emit optionsChanged();
    });

    connect(displayContinueReadingBannerCheck, &QCheckBox::clicked, this, [this]() {
        this->settings->setValue(DISPLAY_CONTINUE_READING_IN_GRID_VIEW, this->displayContinueReadingBannerCheck->isChecked());

        emit optionsChanged();
    });

    auto gridViewLayout = new QVBoxLayout();
    gridViewLayout->addWidget(gridBackgroundGroup);
    gridViewLayout->addWidget(continueReadingGroup);
    gridViewLayout->addStretch();

    auto gridViewW = new QWidget;
    gridViewW->setLayout(gridViewLayout);

    return gridViewW;
}
