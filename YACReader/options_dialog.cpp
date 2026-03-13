#include "options_dialog.h"

#include "app_language_utils.h"
#include "appearance_tab_widget.h"
#include "configuration.h"
#include "theme_factory.h"
#include "theme_manager.h"
#include "yacreader_3d_flow_config_widget.h"
#include "yacreader_spin_slider_widget.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QTabWidget>
#include <QVBoxLayout>

OptionsDialog::OptionsDialog(QWidget *parent)
    : YACReaderOptionsDialog(parent)
{
    auto tabWidget = new QTabWidget();

    auto layout = new QVBoxLayout(this);

    // GENERAL -------------------------------------------

    QWidget *pageGeneral = new QWidget();
    auto layoutGeneral = new QVBoxLayout();

    QGroupBox *pathBox = new QGroupBox(tr("My comics path"));

    auto path = new QHBoxLayout();
    path->addWidget(pathEdit = new QLineEdit());
    path->addWidget(pathFindButton = new QPushButton(""));
    pathBox->setLayout(path);

    auto *languageBox = new QGroupBox(tr("Language"));
    auto *languageLayout = new QHBoxLayout();
    languageLayout->addWidget(new QLabel(tr("Application language")));
    languageCombo = new QComboBox(this);
    languageCombo->addItem(tr("System default"), QString());
    const auto availableLanguages = YACReader::UiLanguage::availableLanguages("yacreader");
    for (const auto &language : availableLanguages) {
        languageCombo->addItem(
                QString("%1 (%2)").arg(language.displayName, language.code), language.code);
    }
    languageLayout->addWidget(languageCombo);
    languageBox->setLayout(languageLayout);

    QGroupBox *displayBox = new QGroupBox(tr("Display"));
    auto displayLayout = new QHBoxLayout();
    showTimeInInformationLabel = new QCheckBox(tr("Show time in current page information label"));
    displayLayout->addWidget(showTimeInInformationLabel);
    displayBox->setLayout(displayLayout);

    connect(pathFindButton, &QAbstractButton::clicked, this, &OptionsDialog::findFolder);

    QGroupBox *slideSizeBox = new QGroupBox(tr("\"Go to flow\" size"));
    // slideSizeLabel = new QLabel(,this);
    slideSize = new QSlider(this);
    slideSize->setMinimum(125);
    slideSize->setMaximum(350);
    slideSize->setPageStep(5);
    slideSize->setOrientation(Qt::Horizontal);
    auto slideLayout = new QHBoxLayout();
    slideLayout->addWidget(slideSize);
    slideSizeBox->setLayout(slideLayout);

    auto colorSelection = new QHBoxLayout;
    backgroundColor = new QLabel();
    QPalette pal = backgroundColor->palette();
    pal.setColor(backgroundColor->backgroundRole(), Qt::black);
    backgroundColor->setPalette(pal);
    backgroundColor->setAutoFillBackground(true);

    QGroupBox *colorBox = new QGroupBox(tr("Background color"));
    // backgroundColor->setMinimumWidth(100);
    colorSelection->addWidget(backgroundColor);
    colorSelection->addWidget(selectBackgroundColorButton = new QPushButton(tr("Choose")));
    colorSelection->addWidget(clearBackgroundColorButton = new QPushButton(tr("Clear")));
    colorSelection->setStretchFactor(backgroundColor, 1);
    colorSelection->setStretchFactor(selectBackgroundColorButton, 0);
    // colorSelection->addStretch();
    connect(selectBackgroundColorButton, &QAbstractButton::clicked, this, &OptionsDialog::showColorDialog);
    connect(clearBackgroundColorButton, &QAbstractButton::clicked, this, &OptionsDialog::clearBackgroundColor);
    colorBox->setLayout(colorSelection);

    auto scrollBox = new QGroupBox(tr("Scroll behaviour"));
    auto scrollLayout = new QVBoxLayout;

    disableScrollAnimations = new QCheckBox(tr("Disable scroll animations and smooth scrolling"));
    doNotTurnPageOnScroll = new QCheckBox(tr("Do not turn page using scroll"));
    useSingleScrollStepToTurnPage = new QCheckBox(tr("Use single scroll step to turn page"));

    scrollLayout->addWidget(disableScrollAnimations);
    scrollLayout->addWidget(doNotTurnPageOnScroll);
    scrollLayout->addWidget(useSingleScrollStepToTurnPage);

    scrollBox->setLayout(scrollLayout);

    auto mouseModeBox = new QGroupBox(tr("Mouse mode"));
    auto mouseModeLayout = new QVBoxLayout();

    normalMouseModeRadioButton = new QRadioButton(tr("Only Back/Forward buttons can turn pages"));
    leftRightNavigationMouseModeRadioButton = new QRadioButton(tr("Use the Left/Right buttons to turn pages."));
    hotAreasMouseModeRadioButton = new QRadioButton(tr("Click left or right half of the screen to turn pages."));

    mouseModeLayout->addWidget(normalMouseModeRadioButton);
    mouseModeLayout->addWidget(leftRightNavigationMouseModeRadioButton);
    mouseModeLayout->addWidget(hotAreasMouseModeRadioButton);

    mouseModeBox->setLayout(mouseModeLayout);

    layoutGeneral->addWidget(pathBox);
    layoutGeneral->addWidget(languageBox);
    layoutGeneral->addWidget(displayBox);
    layoutGeneral->addWidget(slideSizeBox);
    // layoutGeneral->addWidget(fitBox);
    layoutGeneral->addWidget(colorBox);
    layoutGeneral->addWidget(scrollBox);
    layoutGeneral->addWidget(mouseModeBox);
    layoutGeneral->addWidget(shortcutsBox);
    layoutGeneral->addStretch();

    // GENERAL END ---------------------------------------

    // PAGE FLOW -----------------------------------------

    QWidget *pageFlow = new QWidget();
    auto layoutFlow = new QVBoxLayout();

    quickNavi = new QCheckBox(tr("Quick Navigation Mode"));
    disableShowOnMouseOver = new QCheckBox(tr("Disable mouse over activation"));

    layoutFlow->addWidget(gl);

    layoutFlow->addWidget(quickNavi);
    layoutFlow->addWidget(disableShowOnMouseOver);
    layoutFlow->addStretch();

    gl->vSyncCheck->hide();

    // PAGE FLOW END -------------------------------------

    // IMAGE ADJUSTMENTS ---------------------------------

    QWidget *pageImage = new QWidget();
    auto layoutImageV = new QVBoxLayout();
    auto layoutImage = new QGridLayout();

    brightnessS = new YACReaderSpinSliderWidget(this, true);
    brightnessS->setRange(0, 100);
    // brightnessS->setText(tr("Brightness"));
    brightnessS->setTracking(false);
    connect(brightnessS, &YACReaderSpinSliderWidget::valueChanged, this, &OptionsDialog::brightnessChanged);

    contrastS = new YACReaderSpinSliderWidget(this, true);
    contrastS->setRange(0, 250);
    // contrastS->setText(tr("Contrast"));
    contrastS->setTracking(false);
    connect(contrastS, &YACReaderSpinSliderWidget::valueChanged, this, &OptionsDialog::contrastChanged);

    gammaS = new YACReaderSpinSliderWidget(this, true);
    gammaS->setRange(0, 250);
    // gammaS->setText(tr("Gamma"));
    gammaS->setTracking(false);
    connect(gammaS, &YACReaderSpinSliderWidget::valueChanged, this, &OptionsDialog::gammaChanged);
    // connect(brightnessS,SIGNAL(valueChanged(int)),this,SIGNAL(changedOptions()));

    layoutImage->addWidget(new QLabel(tr("Brightness")), 0, 0);
    layoutImage->addWidget(new QLabel(tr("Contrast")), 1, 0);
    layoutImage->addWidget(new QLabel(tr("Gamma")), 2, 0);
    layoutImage->addWidget(brightnessS, 0, 1);
    layoutImage->addWidget(contrastS, 1, 1);
    layoutImage->addWidget(gammaS, 2, 1);
    QPushButton *pushButton = new QPushButton(tr("Reset"));
    connect(pushButton, &QAbstractButton::pressed, this, &OptionsDialog::resetImageConfig);
    layoutImage->addWidget(pushButton, 3, 0);
    layoutImage->setColumnStretch(1, 1);

    QGroupBox *imageBox = new QGroupBox(tr("Image options"));
    imageBox->setLayout(layoutImage);
    layoutImageV->addWidget(imageBox);

    auto scaleBox = new QGroupBox(tr("Fit options"));
    auto scaleLayout = new QVBoxLayout();
    scaleCheckbox = new QCheckBox(tr("Enlarge images to fit width/height"));
    connect(scaleCheckbox, &QCheckBox::clicked, scaleCheckbox,
            [=, this](bool checked) {
                Configuration::getConfiguration().setEnlargeImages(checked);
                emit changedImageOptions();
            });

    scaleLayout->addWidget(scaleCheckbox);
    scaleBox->setLayout(scaleLayout);
    layoutImageV->addWidget(scaleBox);

    auto doublePageBox = new QGroupBox(tr("Double Page options"));
    auto doublePageBoxLayout = new QVBoxLayout();
    coverSPCheckBox = new QCheckBox(tr("Show covers as single page"));
    connect(coverSPCheckBox, &QCheckBox::clicked, coverSPCheckBox,
            [=, this](bool checked) {
                settings->setValue(COVER_IS_SP, checked);
                emit changedImageOptions();
            });

    doublePageBoxLayout->addWidget(coverSPCheckBox);
    doublePageBox->setLayout(doublePageBoxLayout);
    layoutImageV->addWidget(doublePageBox);

    auto scalingBox = new QGroupBox(tr("Scaling"));
    auto scalingLayout = new QHBoxLayout();
    scalingLayout->addWidget(new QLabel(tr("Scaling method")));
    scalingMethodCombo = new QComboBox();
    scalingMethodCombo->addItem(tr("Nearest (fast, low quality)"));
    scalingMethodCombo->addItem(tr("Bilinear"));
    scalingMethodCombo->addItem(tr("Lanczos (better quality)"));
    connect(scalingMethodCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        Configuration::getConfiguration().setScalingMethod(static_cast<ScaleMethod>(index));
        emit changedImageOptions();
    });
    scalingLayout->addWidget(scalingMethodCombo);
    scalingLayout->addStretch();
    scalingBox->setLayout(scalingLayout);
    layoutImageV->addWidget(scalingBox);

    layoutImageV->addStretch();

    // IMAGE ADJUSTMENTS END -----------------------------

    pageGeneral->setLayout(layoutGeneral);
    pageFlow->setLayout(layoutFlow);
    pageImage->setLayout(layoutImageV);

    // APPEARANCE ----------------------------------------

    auto *pageAppearance = new AppearanceTabWidget(
            ThemeManager::instance().getAppearanceConfiguration(),
            ThemeManager::instance().getRepository(),
            []() { return ThemeManager::instance().getCurrentTheme().sourceJson; },
            [](const QJsonObject &json) { ThemeManager::instance().setTheme(makeTheme(json)); },
            this);

    // APPEARANCE END ------------------------------------

    tabWidget->addTab(pageGeneral, tr("General"));
    tabWidget->addTab(pageFlow, tr("Page Flow"));
    tabWidget->addTab(pageImage, tr("Image adjustment"));
    tabWidget->addTab(pageAppearance, tr("Appearance"));

    layout->addWidget(tabWidget);

    auto buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(new QLabel(tr("Restart is needed")));
    buttons->addWidget(accept);
    buttons->addWidget(cancel);

    layout->addLayout(buttons);

    setLayout(layout);

    setModal(true);
    setWindowTitle(tr("Options"));

    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    initTheme(this);
}

void OptionsDialog::applyTheme(const Theme &theme)
{
    pathFindButton->setIcon(theme.dialogIcons.findFolderIcon);
}

void OptionsDialog::findFolder()
{
    QString s = QFileDialog::getExistingDirectory(nullptr, tr("Comics directory"), ".");
    if (!s.isEmpty()) {
        pathEdit->setText(s);
    }
}

void OptionsDialog::showColorDialog()
{
    auto color = QColorDialog::getColor(currentColor, this);
    updateColor(color);
}

void OptionsDialog::saveOptions()
{
    settings->setValue(GO_TO_FLOW_SIZE, QSize(static_cast<int>(slideSize->sliderPosition() / SLIDE_ASPECT_RATIO), slideSize->sliderPosition()));

    settings->setValue(PATH, pathEdit->text());

    Configuration::getConfiguration().setShowTimeInInformation(showTimeInInformationLabel->isChecked());

    if (currentColor != theme.viewer.defaultBackgroundColor) {
        settings->setValue(BACKGROUND_COLOR, currentColor);
    } else {
        settings->remove(BACKGROUND_COLOR);
    }
    // settings->setValue(FIT_TO_WIDTH_RATIO,fitToWidthRatioS->sliderPosition()/100.0);
    settings->setValue(QUICK_NAVI_MODE, quickNavi->isChecked());
    settings->setValue(DISABLE_MOUSE_OVER_GOTO_FLOW, disableShowOnMouseOver->isChecked());

    settings->setValue(DO_NOT_TURN_PAGE_ON_SCROLL, doNotTurnPageOnScroll->isChecked());
    settings->setValue(USE_SINGLE_SCROLL_STEP_TO_TURN_PAGE, useSingleScrollStepToTurnPage->isChecked());
    settings->setValue(DISABLE_SCROLL_ANIMATION, disableScrollAnimations->isChecked());

    // get checked radio button to get the mouse mode
    YACReader::MouseMode mouseMode = Normal;
    if (normalMouseModeRadioButton->isChecked()) {
        mouseMode = Normal;
        ;
    } else if (leftRightNavigationMouseModeRadioButton->isChecked()) {
        mouseMode = LeftRightNavigation;
    } else if (hotAreasMouseModeRadioButton->isChecked()) {
        mouseMode = HotAreas;
    }
    Configuration::getConfiguration().setMouseMode(mouseMode);

    Configuration::getConfiguration().setScalingMethod(static_cast<ScaleMethod>(scalingMethodCombo->currentIndex()));
    emit changedImageOptions();

    const auto selectedLanguage = languageCombo->currentData().toString().trimmed();
    if (selectedLanguage.isEmpty())
        settings->remove(UI_LANGUAGE);
    else
        settings->setValue(UI_LANGUAGE, selectedLanguage);

    YACReaderOptionsDialog::saveOptions();
}

void OptionsDialog::restoreOptions(QSettings *settings)
{
    YACReaderOptionsDialog::restoreOptions(settings);

    slideSize->setSliderPosition(settings->value(GO_TO_FLOW_SIZE).toSize().height());

    pathEdit->setText(settings->value(PATH).toString());

    const auto selectedLanguage = settings->value(UI_LANGUAGE).toString().trimmed();
    int languageIndex = languageCombo->findData(selectedLanguage);
    if (languageIndex < 0)
        languageIndex = 0;
    languageCombo->setCurrentIndex(languageIndex);

    showTimeInInformationLabel->setChecked(Configuration::getConfiguration().getShowTimeInInformation());

    updateColor(settings->value(BACKGROUND_COLOR, theme.viewer.defaultBackgroundColor).value<QColor>());
    // fitToWidthRatioS->setSliderPosition(settings->value(FIT_TO_WIDTH_RATIO).toFloat()*100);

    quickNavi->setChecked(settings->value(QUICK_NAVI_MODE).toBool());
    disableShowOnMouseOver->setChecked(settings->value(DISABLE_MOUSE_OVER_GOTO_FLOW).toBool());

    brightnessS->setValue(settings->value(BRIGHTNESS, 0).toInt());
    contrastS->setValue(settings->value(CONTRAST, 100).toInt());
    gammaS->setValue(settings->value(GAMMA, 100).toInt());

    scaleCheckbox->setChecked(settings->value(ENLARGE_IMAGES, true).toBool());
    coverSPCheckBox->setChecked(settings->value(COVER_IS_SP, true).toBool());

    doNotTurnPageOnScroll->setChecked(settings->value(DO_NOT_TURN_PAGE_ON_SCROLL, false).toBool());
    useSingleScrollStepToTurnPage->setChecked(settings->value(USE_SINGLE_SCROLL_STEP_TO_TURN_PAGE, false).toBool());

#ifdef Q_OS_MACOS
    auto defaultDisableScrollAnimationsValue = true;
#else
    auto defaultDisableScrollAnimationsValue = false;
#endif
    disableScrollAnimations->setChecked(settings->value(DISABLE_SCROLL_ANIMATION, defaultDisableScrollAnimationsValue).toBool());

    {
        QSignalBlocker blocker(scalingMethodCombo);
        scalingMethodCombo->setCurrentIndex(static_cast<int>(Configuration::getConfiguration().getScalingMethod()));
    }

    auto mouseMode = Configuration::getConfiguration().getMouseMode();

    switch (mouseMode) {
    case Normal:
        normalMouseModeRadioButton->setChecked(true);
        break;
    case LeftRightNavigation:
        leftRightNavigationMouseModeRadioButton->setChecked(true);
        break;
    case HotAreas:
        hotAreasMouseModeRadioButton->setChecked(true);
        break;
    }
}

void OptionsDialog::updateColor(const QColor &color)
{
    QPalette pal = backgroundColor->palette();
    pal.setColor(backgroundColor->backgroundRole(), color);
    backgroundColor->setPalette(pal);
    backgroundColor->setAutoFillBackground(true);
    currentColor = color;

    emit changedOptions();
}

void OptionsDialog::brightnessChanged(int value)
{
    QSettings settings(YACReader::getSettingsPath() + "/YACReader.ini", QSettings::IniFormat);
    settings.setValue(BRIGHTNESS, value);
    emit changedFilters(brightnessS->getValue(), contrastS->getValue(), gammaS->getValue());
    // emit(changedImageOptions());
}

void OptionsDialog::contrastChanged(int value)
{
    QSettings settings(YACReader::getSettingsPath() + "/YACReader.ini", QSettings::IniFormat);
    settings.setValue(CONTRAST, value);
    emit changedFilters(brightnessS->getValue(), contrastS->getValue(), gammaS->getValue());
    /// emit(changedImageOptions());
}

void OptionsDialog::gammaChanged(int value)
{
    QSettings settings(YACReader::getSettingsPath() + "/YACReader.ini", QSettings::IniFormat);
    settings.setValue(GAMMA, value);
    emit changedFilters(brightnessS->getValue(), contrastS->getValue(), gammaS->getValue());
    // emit(changedImageOptions());
}

void OptionsDialog::resetImageConfig()
{
    brightnessS->setValue(0);
    contrastS->setValue(100);
    gammaS->setValue(100);
    QSettings settings(YACReader::getSettingsPath() + "/YACReader.ini", QSettings::IniFormat);
    settings.setValue(BRIGHTNESS, 0);
    settings.setValue(CONTRAST, 100);
    settings.setValue(GAMMA, 100);
    emit changedFilters(brightnessS->getValue(), contrastS->getValue(), gammaS->getValue());
    // emit(changedImageOptions());
}

void OptionsDialog::show()
{
    // TODO solucionar el tema de las settings, esto sólo debería aparecer en una única línea de código
    QSettings *s = new QSettings(YACReader::getSettingsPath() + "/YACReader.ini", QSettings::IniFormat);
    // fitToWidthRatioS->disconnect();
    // fitToWidthRatioS->setSliderPosition(settings->value(FIT_TO_WIDTH_RATIO).toFloat()*100);
    // connect(fitToWidthRatioS,SIGNAL(valueChanged(int)),this,SLOT(fitToWidthRatio(int)));
    QDialog::show();
    delete s;
}

void OptionsDialog::setFilters(int brightness, int contrast, int gamma)
{
    if (brightness != -1)
        brightnessS->setValue(brightness);
    else
        brightnessS->setValue(0);
    if (contrast != -1)
        contrastS->setValue(contrast);
    else
        contrastS->setValue(100);
    if (gamma != -1)
        gammaS->setValue(gamma);
    else
        gammaS->setValue(100);
}

void OptionsDialog::clearBackgroundColor()
{
    settings->remove(BACKGROUND_COLOR);

    updateColor(theme.viewer.defaultBackgroundColor);
}
