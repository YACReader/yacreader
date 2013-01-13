/**
  @file
  @author Stefan Frings
*/

#include "logmessage.h"
#include <QThread>

LogMessage::LogMessage(const QtMsgType type, const QString& message, QHash<QString,QString>* logVars) {
    this->type=type;
    this->message=message;
    timestamp=QDateTime::currentDateTime();
    threadId=QThread::currentThreadId();

    // Copy the logVars if not null,
    // so that later changes in the original do not affect the copy
    if (logVars) {
        this->logVars=*logVars;
    }
}

QString LogMessage::toString(const QString& msgFormat, const QString& timestampFormat) const {
    QString decorated=msgFormat+"\n";
    decorated.replace("{msg}",message);

    if (decorated.contains("{timestamp}")) {
        decorated.replace("{timestamp}",QDateTime::currentDateTime().toString(timestampFormat));
    }

    QString typeNr;
    typeNr.setNum(type);
    decorated.replace("{typeNr}",typeNr);

    switch (type) {
    case QtDebugMsg:
        decorated.replace("{type}","DEBUG");
        break;
    case QtWarningMsg:
        decorated.replace("{type}","WARNING");
        break;
    case QtCriticalMsg:
        decorated.replace("{type}","CRITICAL");
        break;
    case QtFatalMsg:
        decorated.replace("{type}","FATAL");
        break;
    default:
        decorated.replace("{type}",typeNr);
    }

    QString threadId;
    threadId.setNum((quint64)QThread::currentThreadId()); //CAMBIADo unsigned int por quint64, evita error de compilación en máquinas de 64bit
    decorated.replace("{thread}",threadId);

    // Fill in variables
    if (decorated.contains("{") && !logVars.isEmpty()) {
        QList<QString> keys=logVars.keys();
        foreach (QString key, keys) {
            decorated.replace("{"+key+"}",logVars.value(key));
        }
    }

    return decorated;
}

QtMsgType LogMessage::getType() const {
    return type;
}
