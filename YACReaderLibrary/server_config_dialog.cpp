#include "server_config_dialog.h"
#include <QCoreApplication>
#include <QGridLayout>


ServerConfigDialog::ServerConfigDialog(QWidget * parent)
	:QDialog(parent)	
{
	accept = new QPushButton(tr("Generar"));
	connect(accept,SIGNAL(clicked()),this,SLOT(generateQR()));
	qrCodeImage = new QPixmap();
	qrCode = new QLabel("xxxx",this);

	QGridLayout *	mainLayout = new QGridLayout;
	mainLayout->addWidget(accept,0,0);
	mainLayout->addWidget(qrCode,0,1);


	this->setLayout(mainLayout);
}

void ServerConfigDialog::generateQR()
{
	generateQR("192.168.2.110:8080");
}

void ServerConfigDialog::generateQR(const QString & serverAddress)
{
	qrGenerator = new QProcess();
	QStringList attributes;
	attributes << "-o" << QCoreApplication::applicationDirPath()+"/utils/tmp.png" << "-s" << "8" << "-l" << "H" << serverAddress;
	connect(qrGenerator,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(updateImage(void)));
	connect(qrGenerator,SIGNAL(error(QProcess::ProcessError)),this,SLOT(openingError(QProcess::ProcessError)));
	qrGenerator->start(QCoreApplication::applicationDirPath()+"/utils/qrcode",attributes);
}

void ServerConfigDialog::updateImage()
{
	//QByteArray imgBinary = qrGenerator->readAllStandardOutput();
	//imgBinary = imgBinary.replace(0x0D0A,0x0A);
	//qrCodeImage->loadFromData(imgBinary);
	//if(imgBinary.isEmpty())
	//	qrCode->setText("yyyyy");
	//else
	//	qrCode->setText("")
	//delete qrGenerator;

	qrCodeImage->load(QCoreApplication::applicationDirPath()+"/utils/tmp.png");
	qrCode->setPixmap(*qrCodeImage);

	delete qrGenerator;
}