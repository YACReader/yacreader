#include "options_dialog.h"
#include "configuration.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QTabWidget>


OptionsDialog::OptionsDialog(QWidget * parent)
:YACReaderOptionsDialog(parent)
{

	QTabWidget * tabWidget = new QTabWidget();

	QVBoxLayout * layout = new QVBoxLayout(this);

	QWidget * pageGeneral = new QWidget();
	QWidget * pageFlow = new QWidget();
	QWidget * pageImage = new QWidget();
	QVBoxLayout * layoutGeneral = new QVBoxLayout();
	QVBoxLayout * layoutFlow = new QVBoxLayout();
	QVBoxLayout * layoutImageV = new QVBoxLayout();
	QGridLayout * layoutImage = new QGridLayout();

	QGroupBox *slideSizeBox = new QGroupBox(tr("\"Go to flow\" size"));
	//slideSizeLabel = new QLabel(,this);
	slideSize = new QSlider(this);
	slideSize->setMinimum(125);
	slideSize->setMaximum(350);
	slideSize->setPageStep(5);
	slideSize->setOrientation(Qt::Horizontal);
	QHBoxLayout * slideLayout = new QHBoxLayout();
	slideLayout->addWidget(slideSize);
	slideSizeBox->setLayout(slideLayout);

	QGroupBox *pathBox = new QGroupBox(tr("My comics path"));

	QHBoxLayout * path = new QHBoxLayout();
	path->addWidget(pathEdit = new QLineEdit());
	path->addWidget(pathFindButton = new QPushButton(QIcon(":/images/comicFolder.png"),""));
	pathBox->setLayout(path);

	connect(pathFindButton,SIGNAL(clicked()),this,SLOT(findFolder()));

	//fitToWidthRatioLabel = new QLabel(tr("Page width stretch"),this);
	QGroupBox *fitBox = new QGroupBox(tr("Page width stretch"));
	fitToWidthRatioS = new QSlider(this);
	fitToWidthRatioS->setMinimum(50);
	fitToWidthRatioS->setMaximum(100);
	fitToWidthRatioS->setPageStep(5);
	fitToWidthRatioS->setOrientation(Qt::Horizontal);
	connect(fitToWidthRatioS,SIGNAL(valueChanged(int)),this,SLOT(fitToWidthRatio(int)));
	QHBoxLayout * fitLayout = new QHBoxLayout;
	fitLayout->addWidget(fitToWidthRatioS);
	fitBox->setLayout(fitLayout);

	QHBoxLayout * colorSelection = new QHBoxLayout;
	backgroundColor = new QLabel();
	QPalette pal = backgroundColor->palette();
	pal.setColor(backgroundColor->backgroundRole(), Qt::black);
	backgroundColor->setPalette(pal);
	backgroundColor->setAutoFillBackground(true);

	colorDialog = new QColorDialog(Qt::red,this);
	connect(colorDialog,SIGNAL(colorSelected(QColor)),this,SLOT(updateColor(QColor)));

	QGroupBox *colorBox = new QGroupBox(tr("Background color"));
	//backgroundColor->setMinimumWidth(100);
	colorSelection->addWidget(backgroundColor);
	colorSelection->addWidget(selectBackgroundColorButton = new QPushButton(tr("Choose")));
	colorSelection->setStretchFactor(backgroundColor,1);
	colorSelection->setStretchFactor(selectBackgroundColorButton,0);
	//colorSelection->addStretch();
	connect(selectBackgroundColorButton, SIGNAL(clicked()), colorDialog, SLOT(show()));
	colorBox->setLayout(colorSelection);

	brightnessS = new YACReaderSpinSliderWidget(this,true);
	brightnessS->setRange(0,100);
	//brightnessS->setText(tr("Brightness"));
	brightnessS->setTracking(false);
	connect(brightnessS,SIGNAL(valueChanged(int)),this,SLOT(brightnessChanged(int)));
	
	contrastS = new YACReaderSpinSliderWidget(this,true);
	contrastS->setRange(0,250);
	//contrastS->setText(tr("Contrast"));
	contrastS->setTracking(false);
	connect(contrastS,SIGNAL(valueChanged(int)),this,SLOT(contrastChanged(int)));

	gammaS = new YACReaderSpinSliderWidget(this,true);
	gammaS->setRange(0,250);
	//gammaS->setText(tr("Gamma"));
	gammaS->setTracking(false);
	connect(gammaS,SIGNAL(valueChanged(int)),this,SLOT(gammaChanged(int)));
	//connect(brightnessS,SIGNAL(valueChanged(int)),this,SIGNAL(changedOptions()));

	QHBoxLayout * buttons = new QHBoxLayout();
	buttons->addStretch();
	buttons->addWidget(new QLabel(tr("Restart is needed")));
	buttons->addWidget(accept);
	buttons->addWidget(cancel);

	layoutGeneral->addWidget(pathBox);
	layoutGeneral->addWidget(slideSizeBox);
	layoutGeneral->addWidget(fitBox);
	layoutGeneral->addWidget(colorBox);
	layoutGeneral->addStretch();
	layoutFlow->addWidget(sw);
	layoutFlow->addWidget(gl);
	layoutFlow->addWidget(useGL);
	layoutFlow->addStretch();
	layoutImage->addWidget(new QLabel(tr("Brightness")),0,0);
	layoutImage->addWidget(new QLabel(tr("Contrast")),1,0);
	layoutImage->addWidget(new QLabel(tr("Gamma")),2,0);
	layoutImage->addWidget(brightnessS,0,1);
	layoutImage->addWidget(contrastS,1,1);
	layoutImage->addWidget(gammaS,2,1);
	QPushButton * pushButton = new QPushButton(tr("Reset"));
	connect(pushButton,SIGNAL(pressed()),this,SLOT(resetImageConfig()));
	layoutImage->addWidget(pushButton,3,0);
	layoutImage->setColumnStretch(1,1);


	QGroupBox *imageBox = new QGroupBox(tr("Image options"));
	imageBox->setLayout(layoutImage);
	layoutImageV->addWidget(imageBox);
	layoutImageV->addStretch();


	pageGeneral->setLayout(layoutGeneral);
	pageFlow->setLayout(layoutFlow);
	pageImage->setLayout(layoutImageV);

	tabWidget->addTab(pageGeneral,tr("General"));
	tabWidget->addTab(pageFlow,tr("Page Flow"));
	tabWidget->addTab(pageImage,tr("Image adjustment"));

	layout->addWidget(tabWidget);
	layout->addLayout(buttons);

	setLayout(layout);

	//disable vSyncCheck
	gl->vSyncCheck->hide();

	//restoreOptions(); //load options
	resize(400,0);
	setModal (true);
	setWindowTitle(tr("Options"));

	this->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void OptionsDialog::findFolder()
{
	QString s = QFileDialog::getExistingDirectory(0,tr("Comics directory"),".");
	if(!s.isEmpty())
	{
		pathEdit->setText(s);
	}
}

void OptionsDialog::saveOptions()
{
	
	settings->setValue(GO_TO_FLOW_SIZE,QSize(static_cast<int>(slideSize->sliderPosition()/SLIDE_ASPECT_RATIO),slideSize->sliderPosition()));

	if(sw->radio1->isChecked())
		settings->setValue(FLOW_TYPE_SW,0);
	if(sw->radio2->isChecked())
		settings->setValue(FLOW_TYPE_SW,1);
	if(sw->radio3->isChecked())
		settings->setValue(FLOW_TYPE_SW,2);

	settings->setValue(PATH,pathEdit->text());

	settings->setValue(BACKGROUND_COLOR,colorDialog->currentColor());
	settings->setValue(FIT_TO_WIDTH_RATIO,fitToWidthRatioS->sliderPosition()/100.0);

	YACReaderOptionsDialog::saveOptions();
}

void OptionsDialog::restoreOptions(QSettings * settings)
{
	YACReaderOptionsDialog::restoreOptions(settings);
	
	slideSize->setSliderPosition(settings->value(GO_TO_FLOW_SIZE).toSize().height());
	switch(settings->value(FLOW_TYPE_SW).toInt())
	{
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
	fitToWidthRatioS->setSliderPosition(settings->value(FIT_TO_WIDTH_RATIO).toFloat()*100);

	brightnessS->setValue(settings->value(BRIGHTNESS,0).toInt());
	contrastS->setValue(settings->value(CONTRAST,100).toInt());
	gammaS->setValue(settings->value(GAMMA,100).toInt());
}


void OptionsDialog::updateColor(const QColor & color)
{
	QPalette pal = backgroundColor->palette();
	pal.setColor(backgroundColor->backgroundRole(), color);
	backgroundColor->setPalette(pal);
	backgroundColor->setAutoFillBackground(true);
	colorDialog->setCurrentColor(color);
	
	settings->setValue(BACKGROUND_COLOR,color);

	emit(changedOptions());
}

void OptionsDialog::fitToWidthRatio(int value)
{
	Configuration::getConfiguration().setFitToWidthRatio(value/100.0);
	emit(fitToWidthRatioChanged(value/100.0));
}

void OptionsDialog::brightnessChanged(int value)
{
	QSettings settings(QCoreApplication::applicationDirPath()+"/YACReader.ini",QSettings::IniFormat);
	settings.setValue(BRIGHTNESS,value);
	emit(changedImageOptions());
}

void OptionsDialog::contrastChanged(int value)
{
	QSettings settings(QCoreApplication::applicationDirPath()+"/YACReader.ini",QSettings::IniFormat);
	settings.setValue(CONTRAST,value);
	emit(changedImageOptions());
}

void OptionsDialog::gammaChanged(int value)
{
	QSettings settings(QCoreApplication::applicationDirPath()+"/YACReader.ini",QSettings::IniFormat);
	settings.setValue(GAMMA,value);
	emit(changedImageOptions());
}

void OptionsDialog::resetImageConfig()
{
	brightnessS->setValue(0);
	contrastS->setValue(100);
	gammaS->setValue(100);
	QSettings settings(QCoreApplication::applicationDirPath()+"/YACReader.ini",QSettings::IniFormat);
	settings.setValue(BRIGHTNESS,0);
	settings.setValue(CONTRAST,100);
	settings.setValue(GAMMA,100);
	emit(changedImageOptions());
}