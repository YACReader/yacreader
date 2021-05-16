/**
  @file
  @author Stefan Frings
*/

#include "static.h"
#include "startup.h"
//#include "dualfilelogger.h"
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

using stefanfrings::HttpListener;
using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

using stefanfrings::HttpSessionStore;
using stefanfrings::StaticFileController;
using stefanfrings::TemplateCache;

void Startup::start(quint16 port)
{
    // Initialize the core application
    QCoreApplication *app = QCoreApplication::instance();
    QString configFileName = YACReader::getSettingsPath() + "/" + QCoreApplication::applicationName() + ".ini";

    // Configure template loader and cache
    auto templateSettings = new QSettings(configFileName, QSettings::IniFormat, app);
    templateSettings->beginGroup("templates");

    if (templateSettings->value("cacheSize").isNull())
        templateSettings->setValue("cacheSize", "160000");

    QString baseTemplatePath = QString("./server/templates");
    QString templatePath;

#if defined Q_OS_UNIX && !defined Q_OS_MAC
    templatePath = QFileInfo(QString(DATADIR) + "/yacreader", baseTemplatePath).absoluteFilePath();
#else
    templatePath = QFileInfo(QCoreApplication::applicationDirPath(), baseTemplatePath).absoluteFilePath();
#endif

    if (templateSettings->value("path").isNull())
        templateSettings->setValue("path", templatePath);

    Static::templateLoader = new TemplateCache(templateSettings, app);

    // Configure session store
    auto sessionSettings = new QSettings(configFileName, QSettings::IniFormat, app);
    sessionSettings->beginGroup("sessions");

    if (sessionSettings->value("expirationTime").isNull())
        sessionSettings->setValue("expirationTime", 864000000);

    Static::sessionStore = new HttpSessionStore(sessionSettings, app);

    Static::yacreaderSessionStore = new YACReaderHttpSessionStore(Static::sessionStore, app);

    // Configure static file controller
    auto fileSettings = new QSettings(configFileName, QSettings::IniFormat, app);
    fileSettings->beginGroup("docroot");

    QString basedocroot = "./server/docroot";
    QString docroot;

#if defined Q_OS_UNIX && !defined Q_OS_MAC
    QFileInfo configFile(QString(DATADIR) + "/yacreader");
    docroot = QFileInfo(QString(DATADIR) + "/yacreader", basedocroot).absoluteFilePath();
#else
    QFileInfo configFile(QCoreApplication::applicationDirPath());
    docroot = QFileInfo(QCoreApplication::applicationDirPath(), basedocroot).absoluteFilePath();
#endif

    if (fileSettings->value("path").isNull())
        fileSettings->setValue("path", docroot);

    Static::staticFileController = new StaticFileController(fileSettings, app);

    // Configure and start the TCP listener
    qDebug("ServiceHelper: Starting service");
    auto listenerSettings = new QSettings(configFileName, QSettings::IniFormat, app);
    listenerSettings->beginGroup("listener");

    if (listenerSettings->value("maxRequestSize").isNull())
        listenerSettings->setValue("maxRequestSize", "32000000");

    if (listenerSettings->value("maxMultiPartSize").isNull())
        listenerSettings->setValue("maxMultiPartSize", "32000000");

    if (listenerSettings->value("cleanupInterval").isNull())
        listenerSettings->setValue("cleanupInterval", 10000);

    if (listenerSettings->value("minThreads").isNull())
        listenerSettings->setValue("maxThreads", 1000);

    if (listenerSettings->value("minThreads").isNull())
        listenerSettings->setValue("minThreads", 50);

    if (listenerSettings->value("port").isNull())
        listenerSettings->setValue("port", 8080);

    // start with a temporary port
    if (port != 0) {
        // cache saved port
        if (!listenerSettings->contains("cachedPort")) {
            listenerSettings->setValue("cachedPort", listenerSettings->value("port").toInt());
        }
        listenerSettings->setValue("port", port);
    } else {
        // restore saved port
        if (listenerSettings->contains("cachedPort")) {
            listenerSettings->setValue("port", listenerSettings->value("cachedPort"));
            listenerSettings->remove("cachedPort");
        }
    }
    // test if port is working
    {
        QTcpServer testServer;
        if (!testServer.listen(QHostAddress::Any, listenerSettings->value("port").toInt())) {
            qDebug("Port is busy.");
            // get random port
            testServer.listen(QHostAddress::Any, 0);
            listenerSettings->setValue("port", testServer.serverPort());
        }
        testServer.close();
    }

    listener = new HttpListener(listenerSettings, new RequestMapper(app), app);

    if (listener->isListening()) {
        qDebug("ServiceHelper: Service has started");
    } else {
        qDebug("ServiceHelper: Could not launch service");
    }
}

void Startup::stop()
{
    qDebug("ServiceHelper: Service has been stopped");
    // QCoreApplication destroys all objects that have been created in start().
    if (listener != nullptr) {
        listener->close();
        delete listener;
        listener = nullptr;
    }
}

Startup::Startup()
    : listener(nullptr)
{
}

QString Startup::getPort()
{
    return QString("%1").arg(listener->serverPort());
}
