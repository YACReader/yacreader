#include "server_config_dialog.h"
#include <QCoreApplication>
#include <QGridLayout>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QHostAddress>

#include "startup.h"

extern Startup * s;

ServerConfigDialog::ServerConfigDialog(QWidget * parent)
	:QDialog(parent)	
{
	accept = new QPushButton(tr("Generar"));
	connect(accept,SIGNAL(clicked()),this,SLOT(generateQR()));
	qrCodeImage = new QPixmap();
	qrCode = new QLabel("xxxx",this);

	QGridLayout * gridEdits = new QGridLayout;
	gridEdits->addWidget(new QLabel(tr("IP")),0,0);
	gridEdits->addWidget(new QLabel(tr("Port")),0,1);
	ip = new QComboBox();
	connect(ip,SIGNAL(activated(const QString &)),this,SLOT(regenerateQR(const QString &)));
	gridEdits->addWidget(ip,1,0);
	port = new QLineEdit("8080");
	port->setMaximumWidth(50);
	gridEdits->addWidget(port,1,1);
	gridEdits->setColumnStretch(0,1);
	gridEdits->setColumnStretch(1,0);

	QHBoxLayout * codeLayout = new QHBoxLayout;
	codeLayout->addStretch();
	QLabel * qrMessage = new QLabel();
	qrMessage->setPixmap(QPixmap(":/images/qrMessage.png"));
	codeLayout->addWidget(qrMessage);
	codeLayout->addWidget(qrCode);

	QVBoxLayout * configLayout = new QVBoxLayout;
	configLayout->addLayout(gridEdits);
	configLayout->addLayout(codeLayout);
	configLayout->addStretch();
	configLayout->setSpacing(5);

	QHBoxLayout * elementsLayout = new QHBoxLayout;

	QLabel * iphone = new QLabel();
	iphone->setPixmap(QPixmap(":/images/iphoneConfig.png"));
	elementsLayout->setSpacing(40);
	elementsLayout->addWidget(iphone);
	elementsLayout->addStretch();
	elementsLayout->addLayout(configLayout);

	QHBoxLayout * buttons = new QHBoxLayout;
	buttons->addStretch();
	buttons->addWidget(accept);

	QVBoxLayout *	mainLayout = new QVBoxLayout;
	mainLayout->addLayout(elementsLayout);
	mainLayout->addLayout(buttons);
	//mainLayout->addWidget(qrCode,0,1);

	this->setLayout(mainLayout);
	generateQR();

	QPalette Pal(palette()); 
	// set black background
	Pal.setColor(QPalette::Background, Qt::white);
	setAutoFillBackground(true);
	setPalette(Pal);
}

void ServerConfigDialog::generateQR()
{
	//QString items;
	//foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
	//{
	//	if (~interface.flags() & QNetworkInterface::IsLoopBack)//interface.flags().testFlag(QNetworkInterface::IsRunning))
	//		foreach (QNetworkAddressEntry entry, interface.addressEntries())
	//		{
	//			if ( interface.hardwareAddress() != "00:00:00:00:00:00" &&     entry.ip().toString().contains("."))
	//				items.append(interface.name() + entry.ip().toString());
	//		}
	//}
	
	QString dir;
	QList<QHostAddress> list = QHostInfo::fromName( QHostInfo::localHostName()  ).addresses();
	QList<QString> otherAddresses;
	foreach(QHostAddress add, list)
	{
		QString tmp = add.toString();
		if(tmp.contains(".") && tmp != "127.0.0.1")
		{
			if(dir.isEmpty())
				dir = tmp;
			else
				otherAddresses.push_back(tmp);
			
		}	
	}
	if(!dir.isEmpty())
	{
		generateQR(dir+":"+s->getPort());
		ip->addItem(dir);
		ip->addItems(otherAddresses);
		port->setText(s->getPort());
	}
	else
	{

	}
	//qrCode->setText(dir+":8080");

	
}

void ServerConfigDialog::generateQR(const QString & serverAddress)
{
	qrGenerator = new QProcess();
	QStringList attributes;
	attributes << "-o" << "-" /*QCoreApplication::applicationDirPath()+"/utils/tmp.png"*/ << "-s" << "8" << "-l" << "H" << serverAddress;
	connect(qrGenerator,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(updateImage(void)));
	connect(qrGenerator,SIGNAL(error(QProcess::ProcessError)),this,SLOT(openingError(QProcess::ProcessError)));
	qrGenerator->start(QCoreApplication::applicationDirPath()+"/utils/qrencode",attributes);
}

void ServerConfigDialog::updateImage()
{
	QByteArray imgBinary = qrGenerator->readAllStandardOutput();
	//imgBinary = imgBinary.replace(0x0D0A,0x0A);

	if(!qrCodeImage->loadFromData(imgBinary))
		qrCode->setText(tr("QR generator error!"));
	else
		qrCode->setPixmap(*qrCodeImage);
	
	delete qrGenerator;

	

/*	qrCodeImage->load(QCoreApplication::applicationDirPath()+"/utils/tmp.png");
	qrCode->setPixmap(*qrCodeImage);

	delete qrGenerator;*/
}

void ServerConfigDialog::regenerateQR(const QString & ip)
{
	generateQR(ip+":"+s->getPort());
}
