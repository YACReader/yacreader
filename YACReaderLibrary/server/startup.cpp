/**
  @file
  @author Stefan Frings
*/

#include "static.h"
#include "startup.h"
#include "dualfilelogger.h"
#include "httplistener.h"
#include "requestmapper.h"
#include "staticfilecontroller.h"

#include "yacreader_global.h"

#include <QDir>
#include <QCoreApplication>

/** Name of this application */
#define APPNAME "YACReaderLibrary"

/** Publisher of this application */
#define ORGANISATION "YACReader"

/** Short description of this application */
#define DESCRIPTION "Comic reader and organizer"

void Startup::start() {
	// Initialize the core application
    QCoreApplication* app = QCoreApplication::instance();

	QString configFileName=YACReader::getSettingsPath()+"/"+QCoreApplication::applicationName()+".ini";

	// Configure logging into files
	QSettings* mainLogSettings=new QSettings(configFileName,QSettings::IniFormat,app);
	mainLogSettings->beginGroup("mainLogFile");
    //QSettings* debugLogSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    //debugLogSettings->beginGroup("debugLogFile");

    if(mainLogSettings->value("fileName").isNull())
        mainLogSettings->setValue("fileName", QFileInfo(YACReader::getSettingsPath(), "server_log.log").absoluteFilePath());

    if(mainLogSettings->value("maxSize").isNull())
        mainLogSettings->setValue("maxSize",1048576);

    if(mainLogSettings->value("maxBackups").isNull())
        mainLogSettings->setValue("maxBackups",1);

    if(mainLogSettings->value("minLevel").isNull())
        mainLogSettings->setValue("minLevel",QtCriticalMsg);

    Logger* logger=new FileLogger(mainLogSettings,10000,app);
    logger->installMsgHandler();

	// Configure template loader and cache
	QSettings* templateSettings=new QSettings(configFileName,QSettings::IniFormat,app);
	templateSettings->beginGroup("templates");

    if(templateSettings->value("cacheSize").isNull())
        templateSettings->setValue("cacheSize","160000");

    QString baseTemplatePath = QString("./server/templates");
    QString templatePath;

    #if defined Q_OS_UNIX && !defined Q_OS_MAC
            templatePath=QFileInfo(QString(DATADIR)+"/yacreader",baseTemplatePath).absoluteFilePath();
    #else
            templatePath=QFileInfo(QCoreApplication::applicationDirPath(),baseTemplatePath).absoluteFilePath();
    #endif

    if(templateSettings->value("path").isNull())
        templateSettings->setValue("path",templatePath);

	Static::templateLoader=new TemplateCache(templateSettings,app);

	// Configure session store
	QSettings* sessionSettings=new QSettings(configFileName,QSettings::IniFormat,app);
	sessionSettings->beginGroup("sessions");
	Static::sessionStore=new HttpSessionStore(sessionSettings,app);

    Static::yacreaderSessionStore = new YACReaderHttpSessionStore(Static::sessionStore, app);

	// Configure static file controller
	QSettings* fileSettings=new QSettings(configFileName,QSettings::IniFormat,app);
	fileSettings->beginGroup("docroot");

    QString basedocroot = "./server/docroot";
    QString docroot;

    #if defined Q_OS_UNIX && ! defined Q_OS_MAC
        QFileInfo configFile(QString(DATADIR)+"/yacreader");
        docroot=QFileInfo(QString(DATADIR)+"/yacreader",basedocroot).absoluteFilePath();
    #else
        QFileInfo configFile(QCoreApplication::applicationDirPath());
        docroot=QFileInfo(QCoreApplication::applicationDirPath(),basedocroot).absoluteFilePath();
    #endif

    if(fileSettings->value("path").isNull())
        fileSettings->setValue("path",docroot);

	Static::staticFileController=new StaticFileController(fileSettings,app);

	// Configure and start the TCP listener
	qDebug("ServiceHelper: Starting service");
	QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,app);
	listenerSettings->beginGroup("listener");
	listener = new HttpListener(listenerSettings,new RequestMapper(app),app);

	qDebug("ServiceHelper: Service has started");
}


void Startup::stop() {
    qDebug("ServiceHelper: Service has been stopped");
    // QCoreApplication destroys all objects that have been created in start().
    if(listener!=nullptr)
    {
        listener->close();
        delete listener;
        listener = nullptr;
    }
}


Startup::Startup()
{

}

QString Startup::getPort()
{
	return QString("%1").arg(listener->serverPort());
}


