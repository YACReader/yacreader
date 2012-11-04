/**
  @file
  @author Stefan Frings
*/

#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <QMutex>
#include <QDateTime>
#include <QThread>

Logger* Logger::defaultLogger=0;


QThreadStorage<QHash<QString,QString>*> Logger::logVars;


QThreadStorage<QList<LogMessage*>*> Logger::buffers;


QMutex Logger::mutex;


Logger::Logger(QObject* parent)
    : QObject(parent),
    msgFormat("{timestamp} {type} {msg}"),
    timestampFormat("dd.MM.yyyy hh:mm:ss.zzz"),
    minLevel(QtDebugMsg),
    bufferSize(0)
    {}


Logger::Logger(const QString msgFormat, const QString timestampFormat, const QtMsgType minLevel, const int bufferSize, QObject* parent)
    :QObject(parent) {
    this->msgFormat=msgFormat;
    this->timestampFormat=timestampFormat;
    this->minLevel=minLevel;
    this->bufferSize=bufferSize;
}


void Logger::msgHandler(const QtMsgType type, const char* message) {
    static QMutex recursiveMutex(QMutex::Recursive);
    static QMutex nonRecursiveMutex(QMutex::NonRecursive);

    // Prevent multiple threads from calling this method simultaneoulsy.
    // But allow recursive calls, which is required to prevent a deadlock
    // if the logger itself produces an error message.
    recursiveMutex.lock();

    // Fall back to stderr when this method has been called recursively.
    if (defaultLogger && nonRecursiveMutex.tryLock()) {
        defaultLogger->log(type,message);
        nonRecursiveMutex.unlock();
    }
    else {
        fputs(message,stderr);
        fflush(stderr);
    }

    // Abort the program after logging a fatal message
    if (type>=QtFatalMsg) {
        //abort();
    }

    recursiveMutex.unlock();
}




Logger::~Logger() {
    if (defaultLogger==this) {
        qInstallMsgHandler(0);
        defaultLogger=0;
    }
}


void Logger::write(const LogMessage* logMessage) {
    fputs(qPrintable(logMessage->toString(msgFormat,timestampFormat)),stderr);
    fflush(stderr);
}


void Logger::installMsgHandler() {
    defaultLogger=this;
    qInstallMsgHandler(msgHandler);
}


void Logger::set(const QString& name, const QString& value) {
    mutex.lock();
    if (!logVars.hasLocalData()) {
        logVars.setLocalData(new QHash<QString,QString>);
    }
    logVars.localData()->insert(name,value);
    mutex.unlock();
}


void Logger::clear(const bool buffer, const bool variables) {
    mutex.lock();
    if (buffer && buffers.hasLocalData()) {
        QList<LogMessage*>* buffer=buffers.localData();
        while (buffer && !buffer->isEmpty()) {
            LogMessage* logMessage=buffer->takeLast();
            delete logMessage;
        }
    }
    if (variables && logVars.hasLocalData()) {
        logVars.localData()->clear();
    }
    mutex.unlock();
}


void Logger::log(const QtMsgType type, const QString& message) {
    mutex.lock();

    // If the buffer is enabled, write the message into it
    if (bufferSize>0) {
        // Create new thread local buffer, if necessary
        if (!buffers.hasLocalData()) {
            buffers.setLocalData(new QList<LogMessage*>());
        }
        QList<LogMessage*>* buffer=buffers.localData();
        // Append the decorated log message
        LogMessage* logMessage=new LogMessage(type,message,logVars.localData());
        buffer->append(logMessage);
        // Delete oldest message if the buffer became too large
        if (buffer->size()>bufferSize) {
            delete buffer->takeFirst();
        }
        // If the type of the message is high enough, print the whole buffer
        if (type>=minLevel) {
            while (!buffer->isEmpty()) {
                LogMessage* logMessage=buffer->takeFirst();
                write(logMessage);
                delete logMessage;
            }
        }
    }

    // Buffer is disabled, print the message if the type is high enough
    else {
        if (type>=minLevel) {
            LogMessage logMessage(type,message,logVars.localData());
            write(&logMessage);
        }
    }
    mutex.unlock();
}
