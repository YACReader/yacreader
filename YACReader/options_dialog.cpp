#include "options_dialog.h"
#include "configuration.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QTabWidget>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QColorDialog>
#include <QCheckBox>

#include "yacreader_spin_slider_widget.h"
#include "yacreader_flow_config_widget.h"
#ifndef NO_OPENGL
#include "yacreader_gl_flow_config_widget.h"
#endif

OptionsDialog::OptionsDialog(QWidget *parent)
    : YACReaderOptionsDialog(parent)
{

    auto tabWidget = new QTabWidget();

    auto layout = new QVBoxLayout(this);

    QWidget *pageGeneral = new QWidget();
    QWidget *pageFlow = new QWidget();
    QWidget *pageImage = new QWidget();
    auto layoutGeneral = new QVBoxLayout();
    auto layoutFlow = new QVBoxLayout();
    auto layoutImageV = new QVBoxLayout();
    auto layoutImage = new QGridLayout();

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

    QGroupBox *pathBox = new QGroupBox(tr("My comics path"));

    auto path = new QHBoxLayout();
    path->addWidget(pathEdit = new QLineEdit());
    path->addWidget(pathFindButton = new QPushButton(QIcon(":/images/find_folder.png"), ""));
    pathBox->setLayout(path);

    connect(pathFindButton, &QAbstractButton::clicked, this, &OptionsDialog::findFolder);

    auto colorSelection = new QHBoxLayout;
    backgroundColor = new QLabel();
    QPalette pal = backgroundColor->palette();
    pal.setColor(backgroundColor->backgroundRole(), Qt::black);
    backgroundColor->setPalette(pal);
    backgroundColor->setAutoFillBackground(true);

    colorDialog = new QColorDialog(Qt::red, this);
    connect(colorDialog, &QColorDialog::colorSelected, this, &OptionsDialog::updateColor);

    QGroupBox *colorBox = new QGroupBox(tr("Background color"));
    // backgroundColor->setMinimumWidth(100);
    colorSelection->addWidget(backgroundColor);
    colorSelection->addWidget(selectBackgroundColorButton = new QPushButton(tr("Choose")));
    colorSelection->setStretchFactor(backgroundColor, 1);
    colorSelection->setStretchFactor(selectBackgroundColorButton, 0);
    // colorSelection->addStretch();
    connect(selectBackgroundColorButton, &QAbstractButton::clicked, colorDialog, &QWidget::show);
    colorBox->setLayout(colorSelection);

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

    quickNavi = new QCheckBox(tr("Quick Navigation Mode"));
    disableShowOnMouseOver = new QCheckBox(tr("Disable mouse over activation"));

    auto buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(new QLabel(tr("Restart is needed")));
    buttons->addWidget(accept);
    buttons->addWidget(cancel);

    layoutGeneral->addWidget(pathBox);
    layoutGeneral->addWidget(slideSizeBox);
    // layoutGeneral->addWidget(fitBox);
    layoutGeneral->addWidget(colorBox);
    layoutGeneral->addWidget(shortcutsBox);
    layoutGeneral->addStretch();

    layoutFlow->addWidget(sw);
#ifndef NO_OPENGL
    layoutFlow->addWidget(gl);
    layoutFlow->addWidget(useGL);
#endif
    layoutFlow->addWidget(quickNavi);
    layoutFlow->addWidget(disableShowOnMouseOver);
    layoutFlow->addStretch();

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
            [=](bool checked) {
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
            [=](bool checked) {
                settings->setValue(COVER_IS_SP, checked);
                emit changedImageOptions();
            });

    doublePageBoxLayout->addWidget(coverSPCheckBox);
    doublePageBox->setLayout(doublePageBoxLayout);
    layoutImageV->addWidget(doublePageBox);
    layoutImageV->addStretch();

    pageGeneral->setLayout(layoutGeneral);
    pageFlow->setLayout(layoutFlow);
    pageImage->setLayout(layoutImageV);

    tabWidget->addTab(pageGeneral, tr("General"));
    tabWidget->addTab(pageFlow, tr("Page Flow"));
    tabWidget->addTab(pageImage, tr("Image adjustment"));

    layout->addWidget(tabWidget);
    layout->addLayout(buttons);

    setLayout(layout);

    // disable vSyncCheck
#ifndef NO_OPENGL
    gl->vSyncCheck->hide();
#endif
    // restoreOptions(); //load options
    // resize(400,0);
    setModal(true);
    setWindowTitle(tr("Options"));

    this->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void OptionsDialog::findFolder()
{
    QString s = QFileDialog::getExistingDirectory(nullptr, tr("Comics directory"), ".");
    if (!s.isEmpty()) {
        pathEdit->setText(s);
    }
}

void OptionsDialog::saveOptions()
{

    settings->setValue(GO_TO_FLOW_SIZE, QSize(static_cast<int>(slideSize->sliderPosition() / SLIDE_ASPECT_RATIO), slideSize->sliderPosition()));

    if (sw->radio1->isChecked())
        settings->setValue(FLOW_TYPE_SW, 0);
    if (sw->radio2->isChecked())
        settings->setValue(FLOW_TYPE_SW, 1);
    if (sw->radio3->isChecked())
        settings->setValue(FLOW_TYPE_SW, 2);

    settings->setValue(PATH, pathEdit->text());

    settings->setValue(BACKGROUND_COLOR, colorDialog->currentColor());
    // settings->setValue(FIT_TO_WIDTH_RATIO,fitToWidthRatioS->sliderPosition()/100.0);
    settings->setValue(QUICK_NAVI_MODE, quickNavi->isChecked());
    settings->setValue(DISABLE_MOUSE_OVER_GOTO_FLOW, disableShowOnMouseOver->isChecked());

    YACReaderOptionsDialog::saveOptions();
}

void OptionsDialog::restoreOptions(QSettings *settings)
{
    YACReaderOptionsDialog::restoreOptions(settings);

    slideSize->setSliderPosition(settings->value(GO_TO_FLOW_SIZE).toSize().height());
    switch (settings->value(FLOW_TYPE_SW).toInt()) {
    case 0:
        sw->radio1->setChecked(true);
        break;
    case 1:
        sw->radio2->setChecked(true);
        break;
    case 2:
        sw->radio3->setChecked(true);
        break;
    default:
        sw->radio1->setChecked(true);
        break;
    }

    pathEdit->setText(settings->value(PATH).toString());

    updateColor(settings->value(BACKGROUND_COLOR).value<QColor>());
    // fitToWidthRatioS->setSliderPosition(settings->value(FIT_TO_WIDTH_RATIO).toFloat()*100);

    quickNavi->setChecked(settings->value(QUICK_NAVI_MODE).toBool());
    disableShowOnMouseOver->setChecked(settings->value(DISABLE_MOUSE_OVER_GOTO_FLOW).toBool());

    brightnessS->setValue(settings->value(BRIGHTNESS, 0).toInt());
    contrastS->setValue(settings->value(CONTRAST, 100).toInt());
    gammaS->setValue(settings->value(GAMMA, 100).toInt());

    scaleCheckbox->setChecked(settings->value(ENLARGE_IMAGES, true).toBool());
    coverSPCheckBox->setChecked(settings->value(COVER_IS_SP, true).toBool());
}

void OptionsDialog::updateColor(const QColor &color)
{
    QPalette pal = backgroundColor->palette();
    pal.setColor(backgroundColor->backgroundRole(), color);
    backgroundColor->setPalette(pal);
    backgroundColor->setAutoFillBackground(true);
    colorDialog->setCurrentColor(color);

    settings->setValue(BACKGROUND_COLOR, color);

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
