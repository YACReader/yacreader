#include "options_dialog.h"
#include "configuration.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>


OptionsDialog::OptionsDialog(QWidget * parent)
:YACReaderOptionsDialog(parent)
{
	QVBoxLayout * layout = new QVBoxLayout(this);

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
	//colorSelection->addStretch();
	connect(selectBackgroundColorButton, SIGNAL(clicked()), colorDialog, SLOT(show()));
	colorBox->setLayout(colorSelection);

	QHBoxLayout * buttons = new QHBoxLayout();
	buttons->addStretch();
	buttons->addWidget(new QLabel(tr("Restart is needed")));
	buttons->addWidget(accept);
	buttons->addWidget(cancel);

	layout->addWidget(pathBox);
	layout->addWidget(slideSizeBox);
	layout->addWidget(fitBox);
	layout->addWidget(colorBox);
	layout->addWidget(sw);
	layout->addWidget(gl);
	layout->addWidget(useGL);
	layout->addLayout(buttons);

	setLayout(layout);

	//disable vSyncCheck
	gl->vSyncCheck->hide();

	//restoreOptions(); //load options
	resize(400,0);
	setModal (true);
	setWindowTitle("Options");
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
}


void OptionsDialog::updateColor(const QColor & color)
{
	QPalette pal = backgroundColor->palette();
	pal.setColor(backgroundColor->backgroundRole(), color);
	backgroundColor->setPalette(pal);
	backgroundColor->setAutoFillBackground(true);
	colorDialog->setCurrentColor(color);
}

void OptionsDialog::fitToWidthRatio(int value)
{
	Configuration::getConfiguration().setFitToWidthRatio(value/100.0);
	emit(fitToWidthRatioChanged(value/100.0));
}