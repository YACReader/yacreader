#include "options_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QTextStream>
#include <QCoreApplication>
#include <QFile>
#include <QMessageBox>

PictureFlow::FlowType flowType = PictureFlow::Strip;

OptionsDialog::OptionsDialog(QWidget * parent)
:QDialog()
{
	QVBoxLayout * layout = new QVBoxLayout(this);

	accept = new QPushButton(tr("Save"));
	cancel = new QPushButton(tr("Cancel"));
	connect(accept,SIGNAL(clicked()),this,SLOT(saveOptions()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(restoreOptions()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(close()));

        QGroupBox *groupBox = new QGroupBox(tr("How to show covers:"));

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

	QHBoxLayout * buttons = new QHBoxLayout();
	buttons->addStretch();
	buttons->addWidget(accept);
	buttons->addWidget(cancel);

     layout->addWidget(groupBox);
	layout->addLayout(buttons);

	setLayout(layout);

	restoreOptions(); //load options
        resize(200,0);
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
	QFile f(QCoreApplication::applicationDirPath()+"/YACReaderLibrary.conf");
        if(!f.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(NULL,tr("Saving config file...."),tr("There was a problem saving YACReaderLibrary configuration. Please, check if you have enough permissions in the YACReader root folder."));
        }
        else
        {
	QTextStream txtS(&f);
	if(radio1->isChecked())
	{
		txtS << "FLOW_TYPE" << "\n" << (int)PictureFlow::CoverFlowLike << "\n";
		flowType = PictureFlow::CoverFlowLike;
	}
	if(radio2->isChecked())
	{
		txtS << "FLOW_TYPE" << "\n" << (int)PictureFlow::Strip << "\n";
		flowType = PictureFlow::Strip;
	}
	if(radio3->isChecked())
	{
		txtS << "FLOW_TYPE" << "\n" << (int)PictureFlow::StripOverlapped << "\n";
		flowType = PictureFlow::StripOverlapped;
	}
	f.close();
	close();
	emit(optionsChanged());
    }
}

void OptionsDialog::restoreOptions()
{
	QFile f(QCoreApplication::applicationDirPath()+"/YACReaderLibrary.conf");
	if(f.exists())
	{
	f.open(QIODevice::ReadOnly);
	QTextStream txtS(&f);
	QString content = txtS.readAll();
	
	QStringList lines = content.split('\n');
	if(lines.count()>0){
	QString name = lines.at(1);

	switch(flowType=(PictureFlow::FlowType)name.toInt()){
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
	else
		flowType=PictureFlow::Strip;
	}
	else
		flowType=PictureFlow::Strip;

}
