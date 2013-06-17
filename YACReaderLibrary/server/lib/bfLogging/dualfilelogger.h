/**
  @file
  @author Stefan Frings
*/

#ifndef DUALFILELOGGER_H
#define DUALFILELOGGER_H

#include "logger.h"
#include "filelogger.h"
#include <QString>
#include <QSettings>
#include <QtGlobal>

/**
  Logs messages into two log files. This is specially useful to get one less detailed
  logfile for normal operation plus one more detailed file for debugging.
  @see FileLogger for a description of the required config settings.
  @see set() describes how to set logger variables
  @see LogMessage for a description of the message decoration.
*/

class DualFileLogger : public Logger {
    Q_OBJECT
    Q_DISABLE_COPY(DualFileLogger)
public:

    /**
      Constructor.
      @param firstSettings Configuration settings for the first log file, usually stored in an INI file.
      Must not be 0.
      Settings are read from the current group, so the caller must have called settings->beginGroup().
      Because the group must not change during runtime, it is recommended to provide a
      separate QSettings instance to the logger that is not used by other parts of the program.
      @param secondSettings Same as firstSettings, but for the second log file.
      @param refreshInterval Interval of checking the config settings in msec, or 0=disabled
      @param parent Parent object
    */
    DualFileLogger(QSettings* firstSettings, QSettings* secondSettings, const int refreshInterval=10000, QObject *parent = 0);

    /**
      Decorate and log a message.
      This method is thread safe.
      @param type Message type (level)
      @param message Message text
      @see LogMessage for a description of the message decoration.
    */
    virtual void log(const QtMsgType type, const QString& message);

private:

    /** First logger */
    FileLogger* firstLogger;

    /** Second logger */
    FileLogger* secondLogger;

};

#endif // DUALFILELOGGER_H
