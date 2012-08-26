#include "options_dialog.h"
#include "configuration.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>


OptionsDialog::OptionsDialog(QWidget * parent)
:QDialog(parent)
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

	accept = new QPushButton(tr("Save"));
	cancel = new QPushButton(tr("Cancel"));
	connect(accept,SIGNAL(clicked()),this,SLOT(saveOptions()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(restoreOptions()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(close()));

	QGroupBox *groupBox = new QGroupBox(tr("How to show pages in GoToFlow:"));

	radio1 = new QRadioButton(tr("CoverFlow look"));
	radio2 = new QRadioButton(tr("Stripe look"));
	radio3 = new QRadioButton(tr("Overlapped Stripe look"));


	QVBoxLayout *vbox = new QVBoxLayout;
	QHBoxLayout * opt1 = new QHBoxLayout;
	opt1->addWidget(radio1);
	QLabel * lOpt1 = new QLabel();
	lOpt1->setPixmap(QPixmap(":/images/flow1.png"));
	opt1->addStretch();
	opt1->addWidget(lOpt1);
	vbox->addLayout(opt1);

	QHBoxLayout * opt2 = new QHBoxLayout;
	opt2->addWidget(radio2);
	QLabel * lOpt2 = new QLabel();
	lOpt2->setPixmap(QPixmap(":/images/flow2.png"));
	opt2->addStretch();
	opt2->addWidget(lOpt2);
	vbox->addLayout(opt2);

	QHBoxLayout * opt3 = new QHBoxLayout;
	opt3->addWidget(radio3);
	QLabel * lOpt3 = new QLabel();
	lOpt3->setPixmap(QPixmap(":/images/flow3.png"));
	opt3->addStretch();
	opt3->addWidget(lOpt3);
	vbox->addLayout(opt3);


	//vbox->addStretch(1);
	groupBox->setLayout(vbox);

	//fitToWidthRatioLabel = new QLabel(tr("Page width stretch"),this);
	QGroupBox *fitBox = new QGroupBox(tr("Page width stretch"));
	fitToWidthRatioS = new QSlider(this);
	fitToWidthRatioS->setMinimum(50);
	fitToWidthRatioS->setMaximum(100);
	fitToWidthRatioS->setPageStep(5);
	fitToWidthRatioS->setOrientation(Qt::Horizontal);
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
	//layout->addLayout(path);
	layout->addWidget(slideSizeBox);
	//layout->addWidget(slideSize);
	layout->addWidget(groupBox);
	//layout->addWidget(fitToWidthRatioLabel);
	layout->addWidget(fitBox);
	layout->addWidget(colorBox);
	//layout->addLayout(colorSelection);
	layout->addLayout(buttons);


	setLayout(layout);

	restoreOptions(); //load options
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
	if(radio1->isChecked())
		conf.setFlowType(PictureFlow::CoverFlowLike);
	if(radio2->isChecked())
		conf.setFlowType(PictureFlow::Strip);
	if(radio3->isChecked())
		conf.setFlowType(PictureFlow::StripOverlapped);
	conf.setFitToWidthRatio(fitToWidthRatioS->sliderPosition()/100.0);
	conf.setBackgroundColor(colorDialog->currentColor());
	conf.save();
	close();
	emit(accepted());
}

void OptionsDialog::restoreOptions()
{
	Configuration & conf = Configuration::getConfiguration();

	slideSize->setSliderPosition(conf.getGotoSlideSize().height());
	fitToWidthRatioS->setSliderPosition(conf.getFitToWidthRatio()*100);
	pathEdit->setText(conf.getDefaultPath());
	updateColor(Configuration::getConfiguration().getBackgroundColor());
	switch(conf.getFlowType()){
		case PictureFlow::CoverFlowLike:
			radio1->setChecked(true);
			break;
		case PictureFlow::Strip:
			radio2->setChecked(true);
			break;
		case PictureFlow::StripOverlapped:
			radio3->setChecked(true);
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