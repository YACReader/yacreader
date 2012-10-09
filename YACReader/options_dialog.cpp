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
	Configuration & conf = Configuration::getConfiguration();
	conf.setDefaultPath(pathEdit->text());
	conf.setGotoSlideSize(QSize(static_cast<int>(slideSize->sliderPosition()*SLIDE_ASPECT_RATIO),slideSize->sliderPosition()));
	if(sw->radio1->isChecked())
		conf.setFlowType(PictureFlow::CoverFlowLike);
	if(sw->radio2->isChecked())
		conf.setFlowType(PictureFlow::Strip);
	if(sw->radio3->isChecked())
		conf.setFlowType(PictureFlow::StripOverlapped);
	conf.setFitToWidthRatio(fitToWidthRatioS->sliderPosition()/100.0);
	conf.setBackgroundColor(colorDialog->currentColor());
	conf.save();
	close();
	emit(accepted());
}

void OptionsDialog::restoreOptions(QSettings * settings)
{
	YACReaderOptionsDialog::restoreOptions(settings);

	Configuration & conf = Configuration::getConfiguration();

	slideSize->setSliderPosition(conf.getGotoSlideSize().height());
	fitToWidthRatioS->setSliderPosition(conf.getFitToWidthRatio()*100);
	pathEdit->setText(conf.getDefaultPath());
	updateColor(Configuration::getConfiguration().getBackgroundColor());
	switch(conf.getFlowType()){
		case PictureFlow::CoverFlowLike:
			sw->radio1->setChecked(true);
			break;
		case PictureFlow::Strip:
			sw->radio2->setChecked(true);
			break;
		case PictureFlow::StripOverlapped:
			sw->radio3->setChecked(true);
			break;
	}
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