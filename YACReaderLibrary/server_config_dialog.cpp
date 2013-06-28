#include "server_config_dialog.h"
#include <QCoreApplication>
#include <QGridLayout>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QHostAddress>
#include <QSettings>
#include <QPalette>
#include <QIntValidator>
#include <QFormLayout>

#include "startup.h"
#include "yacreader_global.h"

#ifndef Q_OS_WIN32

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>

QList<QString> addresses()
{
	struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;
	
	QList<QString> localAddreses;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            QString add(addressBuffer);
			localAddreses.push_back(QString(addressBuffer));
            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
        } else if (ifa->ifa_addr->sa_family==AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
        } 
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return localAddreses;
}

#endif

extern Startup * s;

ServerConfigDialog::ServerConfigDialog(QWidget * parent)
	:QDialog(parent)	
{
	accept = new QPushButton(tr("set port"),this);
	qrCodeImage = new QPixmap();
	qrCode = new QLabel(this);
	qrCode->move(196,73);
	qrCode->setFixedSize(200,200);
	qrCode->setScaledContents(true);

	QLabel * title1 = new QLabel(tr("EASY SERVER CONNECTION"),this);
	title1->move(37,28);
	title1->setStyleSheet("QLabel {color:#1F1F1F; font-size:18px; font-family: Arial; font-weight: bold;}");

	QLabel * title2 = new QLabel(tr("SERVER ADDRESS"),this);
	title2->move(451,28);
	title2->setStyleSheet("QLabel {color:#1F1F1F; font-size:18px; font-family: Arial; font-weight: bold;}");

	QLabel * qrMessage = new QLabel(tr("just scan the code with your device!!"),this);
	qrMessage->move(194,290);//373,627);
	qrMessage->setStyleSheet("QLabel {color:#1F1F1F; font-size:16px; font-family: Arial; font-style: italic;}");
	qrMessage->setWordWrap(true);
	qrMessage->setFixedWidth(200);

	QLabel * propaganda = new QLabel(tr("YACReader is now available for iOS devices, the best comic reading experience now in your iPad, iPhone or iPod touch. <a href='http://ios.yacreader.com' style='color:rgb(193, 148, 65)'> Discover it! </a>"),this);
	propaganda->move(36,375);
	propaganda->setStyleSheet("QLabel {color:#1F1F1F; font-size:16px; font-family: Arial; font-style: italic;}"
		"QLabel::a {color:#1A1A1A}");
	propaganda->setWordWrap(true);
	propaganda->setFixedWidth(590);
	propaganda->setOpenExternalLinks(true);

	//FORM---------------------------------------------------------------------
	QWidget * form = new QWidget(this);
    QFormLayout * formLayout = new QFormLayout;

	/*QLabel * ipLabel = new QLabel(tr("IP address"),this);
	ipLabel->move(452,75);
	ipLabel->setStyleSheet("QLabel {color:#1F1F1F; font-size:13px; font-family: Arial; font-weight: bold;}");

	QLabel * portLabel = new QLabel(tr("Port"),this);
	portLabel->move(452, 114);
	portLabel->setStyleSheet("QLabel {color:#1F1F1F; font-size:13px; font-family: Arial; font-weight: bold;}");*/

	ip = new QComboBox(this);
	connect(ip,SIGNAL(activated(const QString &)),this,SLOT(regenerateQR(const QString &)));
    //ip->move(520,71);
#ifndef Q_OS_WIN32
    ip->setStyleSheet("QComboBox{font-size:10px;}");
#endif
    ip->setSizeAdjustPolicy(QComboBox::AdjustToContents);
#ifdef Q_OS_WIN32
    ip->setMinimumWidth(120);
#else
    ip->setFixedSize(120,ip->height());
#endif

	port = new QLineEdit("8080",this);
	port->setReadOnly(false);
    port->setMaximumWidth(50);
#ifndef Q_OS_WIN32
    port->setStyleSheet("QLineEdit{font-size:10px;}");
#endif
	//port->move(520,110);
	QValidator *validator = new QIntValidator(1024, 65535, this);
	port->setValidator(validator);

	//accept->move(514,149);
	connect(accept,SIGNAL(pressed()),this,SLOT(updatePort()));
	
    formLayout->addRow(tr("IP address"),ip);
	formLayout->addRow(tr("Port"),port);
	formLayout->addRow("",accept);

	form->setLayout(formLayout);
#ifdef Q_OS_WIN32
	form->move(444,70);
#else
    form->move(435,70);
#endif
	//END FORM-----------------------------------------------------------------

	check = new QCheckBox(this);
	check->move(453,314);
	check->setText(tr("enable the server"));
	check->setStyleSheet("QCheckBox {color:#1F1F1F; font-size:13px; font-family: Arial; font-weight: bold;}");
	
	//check->setLayoutDirection(Qt::RightToLeft);
	
	//elementsLayout->setSpacing(40);
	//elementsLayout->addWidget(iphone);
	//elementsLayout->addStretch();
	//elementsLayout->addLayout(configLayout);

	//QVBoxLayout *	mainLayout = new QVBoxLayout;
	//mainLayout->addLayout(elementsLayout);
	//mainLayout->addLayout(buttons);
	//mainLayout->addWidget(qrCode,0,1);

	//this->setLayout(mainLayout);

	QPalette Pal(palette()); 
	// set black background
	QPalette palette;
	QImage image(":/images/serverConfigBackground.png");
	palette.setBrush(this->backgroundRole(), QBrush(image));

	setPalette(palette);

	this->setFixedSize(image.size());
	
	QSettings * settings = new QSettings(QCoreApplication::applicationDirPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
	settings->beginGroup("libraryConfig");

	if(settings->value(SERVER_ON,true).toBool())
	{
		check->setChecked(true);
		generateQR();
	}
	else
		check->setChecked(false);

	settings->endGroup();

	connect(check,SIGNAL(stateChanged(int)),this,SLOT(enableServer(int)));
}

void ServerConfigDialog::enableServer(int status)
{
	QSettings * settings = new QSettings(QCoreApplication::applicationDirPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
	settings->beginGroup("libraryConfig");

	if(status == Qt::Checked)
	{
		s->start();
		this->generateQR();
		settings->setValue(SERVER_ON,true);
	}
	else
	{
		s->stop();
		qrCode->setPixmap(QPixmap());
		ip->clear();
		port->setText("");
		settings->setValue(SERVER_ON,false);
	}
	settings->endGroup();
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
	ip->clear();
	QString dir;
#ifdef Q_OS_WIN32
	QList<QHostAddress> list = QHostInfo::fromName( QHostInfo::localHostName()  ).addresses();

	QList<QString> otherAddresses;
	foreach(QHostAddress add, list)
	{
		QString tmp = add.toString();
		if(tmp.contains(".") && !tmp.startsWith("127"))
		{
			if(dir.isEmpty() && tmp.startsWith("192.168"))
				dir = tmp;
			else
				otherAddresses.push_back(tmp);
			
		}	
	}

#else
	QList<QString> list = addresses();
	
	QList<QString> otherAddresses;
	foreach(QString add, list)
	{
		QString tmp = add;
		if(tmp.contains(".") && !tmp.startsWith("127"))
		{
			if(dir.isEmpty() && tmp.startsWith("192.168"))
				dir = tmp;
			else
				otherAddresses.push_back(tmp);
			
		}	
	}
#endif
	if(otherAddresses.length()>0 || !dir.isEmpty())
	{
		if(!dir.isEmpty())
		{
		generateQR(dir+":"+s->getPort());
		
		ip->addItem(dir);
		}
		else
		{
			generateQR(otherAddresses.first()+":"+s->getPort());
		}
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
	qrCode->clear();
	qrGenerator = new QProcess();
	QStringList attributes;
	attributes << "-o" << "-" /*QCoreApplication::applicationDirPath()+"/utils/tmp.png"*/ << "-s" << "8" << "-l" << "H" << "-m" << "0" << serverAddress;
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

void ServerConfigDialog::updatePort()
{

	QSettings * settings = new QSettings(QCoreApplication::applicationDirPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
	settings->beginGroup("listener");
	settings->setValue("port",port->text().toInt());
	settings->endGroup();

	s->stop();
	s->start();

	generateQR(ip->currentText()+":"+port->text());

}
