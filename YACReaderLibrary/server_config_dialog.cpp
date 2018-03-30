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
#include <QBitmap>
#include <QPainter>

#include "startup.h"
#include "yacreader_global_gui.h"

#include "qnaturalsorting.h"

#include <algorithm>

//192.168 (most comon local subnet for ips are always put first)
//IPs are sorted using natoral sorting
bool ipComparator(const QString & ip1, const QString & ip2)
{
    if(ip1.startsWith("192.168") && ip2.startsWith("192.168"))
        return naturalSortLessThanCI(ip1, ip2);

    if(ip1.startsWith("192.168"))
        return true;

    if(ip2.startsWith("192.168"))
        return false;

    return naturalSortLessThanCI(ip1, ip2);
}

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
		if (ifa ->ifa_addr) {
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
	qrCode = new QLabel(this);
    qrCode->move(64, 112);
    qrCode->setFixedSize(200,200);
    qrCode->setScaledContents(true);

    QLabel * title1 = new QLabel(tr("Server connectivity information"),this);
    title1->move(332, 61);
    title1->setStyleSheet("QLabel {color:#474747; font-size:30px; font-family: Arial;}");

    QLabel * qrMessage = new QLabel(tr("Scan it!"),this);
    qrMessage->move(135,388);//373,627);
    qrMessage->setStyleSheet("QLabel {color:#A3A3A3; font-size:18px; font-family: Arial;}");
	qrMessage->setWordWrap(true);
	qrMessage->setFixedWidth(200);

    QLabel * propaganda = new QLabel(tr("YACReader is available for iOS devices. <a href='http://ios.yacreader.com' style='color:rgb(193, 148, 65)'> Discover it! </a>"),this);
    propaganda->move(332,505);
    propaganda->setStyleSheet("QLabel {color:#4D4D4D; font-size:13px; font-family: Arial; font-style: italic;}");
    /*propaganda->setWordWrap(true);
    propaganda->setFixedWidth(590);*/
	propaganda->setOpenExternalLinks(true);

	//FORM---------------------------------------------------------------------

    QLabel * ipLabel = new QLabel(tr("Choose an IP address"),this);
    ipLabel->move(332,117);
    ipLabel->setStyleSheet("QLabel {color:#575757; font-size:18px; font-family: Arial;}");

	QLabel * portLabel = new QLabel(tr("Port"),this);
    portLabel->move(332, 211);
    portLabel->setStyleSheet("QLabel {color:#575757; font-size:18px; font-family: Arial;}");

	ip = new QComboBox(this);
	connect(ip,SIGNAL(activated(const QString &)),this,SLOT(regenerateQR(const QString &)));

    ip->setFixedWidth(200);
    ip->move(332,153);


	port = new QLineEdit("8080",this);
	port->setReadOnly(false);
    //port->setFixedWidth(100);
    //port->move(332, 244);

	//port->move(520,110);
	QValidator *validator = new QIntValidator(1024, 65535, this);
	port->setValidator(validator);

    QWidget * portWidget = new QWidget(this);
    QHBoxLayout * portWidgetLayout = new QHBoxLayout(this);
    portWidgetLayout->addWidget(port);
    portWidgetLayout->addWidget(accept);
    portWidgetLayout->setMargin(0);
    portWidget->setLayout(portWidgetLayout);
    portWidget->move(332, 244);
	//accept->move(514,149);
	connect(accept,SIGNAL(pressed()),this,SLOT(updatePort()));
	//END FORM-----------------------------------------------------------------

	check = new QCheckBox(this);
    check->move(332,314);
	check->setText(tr("enable the server"));
    check->setStyleSheet("QCheckBox {color:#262626; font-size:13px; font-family: Arial;}");

    performanceWorkaroundCheck = new QCheckBox(this);
    performanceWorkaroundCheck->move(332,354);
    performanceWorkaroundCheck->setText(tr("display less information about folders in the browser\nto improve the performance"));
    performanceWorkaroundCheck->setStyleSheet("QCheckBox {color:#262626; font-size:13px; font-family: Arial;}");

	QPalette Pal(palette());
	// set black background
	QPalette palette;
	QImage image(":/images/serverConfigBackground.png");
	palette.setBrush(this->backgroundRole(), QBrush(image));

	setPalette(palette);

	this->setFixedSize(image.size());

	QSettings * settings = new QSettings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
	settings->beginGroup("libraryConfig");

	if(settings->value(SERVER_ON,true).toBool())
	{
		check->setChecked(true);
		generateQR();
	}
	else
		check->setChecked(false);

    performanceWorkaroundCheck->setChecked(settings->value(REMOTE_BROWSE_PERFORMANCE_WORKAROUND,false).toBool());

	  settings->endGroup();

    connect(check,SIGNAL(stateChanged(int)),this,SLOT(enableServer(int)));
    connect(performanceWorkaroundCheck,SIGNAL(stateChanged(int)),this,SLOT(enableperformanceWorkaround(int)));
}

void ServerConfigDialog::enableServer(int status)
{
	QSettings * settings = new QSettings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
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

void ServerConfigDialog::enableperformanceWorkaround(int status)
{
    QSettings * settings = new QSettings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
    settings->beginGroup("libraryConfig");

    if(status == Qt::Checked)
    {
        settings->setValue(REMOTE_BROWSE_PERFORMANCE_WORKAROUND,true);
    }
    else
    {
        settings->setValue(REMOTE_BROWSE_PERFORMANCE_WORKAROUND,false);
    }
    settings->endGroup();
}

void ServerConfigDialog::generateQR()
{
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
            otherAddresses.push_back(tmp);
		}
	}
#endif

    std::sort(otherAddresses.begin(),otherAddresses.end(),ipComparator);

    if(!otherAddresses.isEmpty())
    {
        dir = otherAddresses.first();
        otherAddresses.pop_front();
    }

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
}

void ServerConfigDialog::generateQR(const QString & serverAddress)
{
    qrCode->clear();

    QrEncoder encoder;
    QBitmap image = encoder.encode(serverAddress);
    if (image.isNull())
    {
        qrCode->setText(tr("Could not load libqrencode."));
    }
    else
    {
        image = image.scaled(qrCode->size()*devicePixelRatio());

        QPixmap pMask(image.size());
        pMask.fill( QColor(66, 66, 66) );
        pMask.setMask(image.createMaskFromColor(Qt::white));
        pMask.setDevicePixelRatio(devicePixelRatio());

        qrCode->setPixmap(pMask);
    }
}

void ServerConfigDialog::regenerateQR(const QString & ip)
{
	generateQR(ip+":"+s->getPort());
}

void ServerConfigDialog::updatePort()
{

	QSettings * settings = new QSettings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
	settings->beginGroup("listener");
	settings->setValue("port",port->text().toInt());
	settings->endGroup();

	s->stop();
	s->start();

	generateQR(ip->currentText()+":"+port->text());

}

QrEncoder::QrEncoder()
{
#ifdef Q_OS_MACOS
    QLibrary encoder(QCoreApplication::applicationDirPath() + "/utils/libqrencode.dylib");
#else
    QLibrary encoder("qrencode");
#endif
    QRcode_encodeString8bit = (_QRcode_encodeString8bit) encoder.resolve("QRcode_encodeString8bit");
    QRcode_free = (_QRcode_free) encoder.resolve("QRcode_free");
}

QBitmap QrEncoder::encode(const QString & string)
{
    if (!QRcode_encodeString8bit)
    {
        return QBitmap();
    }
    QRcode *code;
    code = QRcode_encodeString8bit(string.toUtf8().data(), 0, 0);
    QBitmap result(code->width, code->width);
    result.fill();
    /* convert to QBitmap */
    QPainter painter;
    painter.begin(&result);
    unsigned char * pointer = code->data;
    for (int x=0; x < code->width; x++) {
        for (int y=0; y < code->width; y++) {
            if ((*pointer++ & 0x1)==1)
            {
                painter.drawPoint(x, y);
            }
        }
    }
    painter.end();
    QRcode_free(code);
    return result;
}
