/**
  @file
  @author Stefan Frings
*/

#include "static.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>

QString Static::configDir=0;

TemplateLoader* Static::templateLoader=0;

HttpSessionStore* Static::sessionStore=0;

StaticFileController* Static::staticFileController=0;

QString Static::getConfigFileName() {
    return QString("%1/%2.ini").arg(getConfigDir()).arg(QCoreApplication::applicationName());
}

QString Static::getConfigDir() {
    if (!configDir.isNull()) {
        return configDir;
    }
    // Search config file
    #if defined Q_OS_UNIX && !defined Q_OS_MAC
    QString binDir=(QString(DATADIR) + "/YACReader");
    #else
    QString binDir=QCoreApplication::applicationDirPath();
    #endif
    QString organization=QCoreApplication::organizationName();
    QString configFileName=QCoreApplication::applicationName()+".ini";

    QStringList searchList;
    searchList.append(QDir::cleanPath(binDir));
    searchList.append(QDir::cleanPath(binDir+"/../etc"));
    searchList.append(QDir::cleanPath(binDir+"/../../etc")); // for development under windows
    searchList.append(QDir::rootPath()+"etc/xdg/"+organization);
    searchList.append(QDir::rootPath()+"etc/opt");
    searchList.append(QDir::rootPath()+"etc");

    foreach (QString dir, searchList) {
        QFile file(dir+"/"+configFileName);
        if (file.exists()) {
            // found
            configDir=dir;
            qDebug("Using config file %s",qPrintable(file.fileName()));
            return configDir;
        }
    }

    // not found
    foreach (QString dir, searchList) {
        qWarning("%s/%s not found",qPrintable(dir),qPrintable(configFileName));
    }
    qWarning("Cannot find config file %s",qPrintable(configFileName)); //TODO establecer los valores por defecto

    return 0;
}

