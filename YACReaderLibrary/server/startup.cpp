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
#include <QDir>
#include <QApplication>

/** Name of this application */
#define APPNAME "YACReaderLibrary"

/** Publisher of this application */
#define ORGANISATION "Butterfly"

/** Short description of this application */
#define DESCRIPTION "Web service based on Qt"

void Startup::start() {
    // Initialize the core application
    QCoreApplication* app = QApplication::instance();
    app->setApplicationName(APPNAME);
    //app->setOrganizationName(ORGANISATION);
    QString configFileName=Static::getConfigDir()+"/"+QCoreApplication::applicationName()+".ini";

    // Configure logging into files
    QSettings* mainLogSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    mainLogSettings->beginGroup("mainLogFile");
    QSettings* debugLogSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    debugLogSettings->beginGroup("debugLogFile");
    Logger* logger=new DualFileLogger(mainLogSettings,debugLogSettings,10000,app);
    logger->installMsgHandler();

    // Configure template loader and cache
    QSettings* templateSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    templateSettings->beginGroup("templates");
    Static::templateLoader=new TemplateCache(templateSettings,app);

    // Configure session store
    QSettings* sessionSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    sessionSettings->beginGroup("sessions");
    Static::sessionStore=new HttpSessionStore(sessionSettings,app);

    // Configure static file controller
    QSettings* fileSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    fileSettings->beginGroup("docroot");
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
	listener->close();
}


Startup::Startup()
{

}



