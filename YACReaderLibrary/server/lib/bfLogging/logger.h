/**
  @file
  @author Stefan Frings
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <QtGlobal>
#include <QThreadStorage>
#include <QHash>
#include <QStringList>
#include <QMutex>
#include "logmessage.h"

/**
  Decorates and writes log messages to the console, stderr.
  <p>
  The decorator uses a predefined msgFormat string to enrich log messages
  with additional information (e.g. timestamp).
  <p>
  The msgFormat string and also the message text may contain additional
  variable names in the form  <i>{name}</i> that are filled by values
  taken from a static thread local dictionary.
  <p>
  The buffer stores log messages of any level from the time before an error occurs. 
  It can be used to provide detailed debug information when an error occurs, while 
  keeping the logfile clean as long no error occurs. Using this buffer may 
  reduce performance significantly.
  <p>
  The logger can be registered to handle messages from
  the static global functions qDebug(), qWarning(), qCritical() and qFatal().
  @see set() describes how to set logger variables
  @see LogMessage for a description of the message decoration.
  @warning You should prefer a derived class, for example FileLogger,
  because logging to the console is less useful.
*/

class Logger : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Logger)
public:

    /**
      Constructor.
      Uses the same defaults as the other constructor.
      @param parent Parent object
    */
    Logger(QObject* parent);


    /**
      Constructor.
      @param msgFormat Format of the decoration, e.g. "{timestamp} {type} thread={thread}: {msg}"
      @param timestampFormat Format of timestamp, e.g. "dd.MM.yyyy hh:mm:ss.zzz"
      @param minLevel Minimum type of messages that are written out.
      @param bufferSize Size of the backtrace buffer, number of messages per thread. 0=disabled.
      @param parent Parent object
      @see LogMessage for a description of the message decoration.
    */
    Logger(const QString msgFormat="{timestamp} {type} {msg}", const QString timestampFormat="dd.MM.yyyy hh:mm:ss.zzz", const QtMsgType minLevel=QtDebugMsg, const int bufferSize=0, QObject* parent = 0);

    /** Destructor */
    virtual ~Logger();

    /**
      Decorate and log the message, if type>=minLevel.
      This method is thread safe.
      @param type Message type (level)
      @param message Message text
      @see LogMessage for a description of the message decoration.
    */
    virtual void log(const QtMsgType type, const QString& message);

    /**
      Installs this logger as the default message handler, so it
      can be used through the global static logging functions (e.g. qDebug()).
    */
    void installMsgHandler();

    /**
      Sets a thread-local variable that may be used to decorate log messages.
      This method is thread safe.
      @param name Name of the variable
      @param value Value of the variable
    */
    static void set(const QString& name, const QString& value);

    /**
      Clear the thread-local data of the current thread.
      @param buffer Whether to clear the backtrace buffer
      @param variables Whether to clear the log variables
    */
    static void clear(const bool buffer=true, const bool variables=true);

protected:

    /** Format string for message decoration */
    QString msgFormat;

    /** Format string of timestamps */
    QString timestampFormat;

    /** Minimum level of message types that are written out */
    QtMsgType minLevel;

    /** Size of backtrace buffer, number of messages per thread. 0=disabled */
    int bufferSize;

    /** Used to synchronize access to the static members */
    static QMutex mutex;

    /**
      Decorate and write a log message to stderr. Override this method
      to provide a different output medium.
    */
    virtual void write(const LogMessage* logMessage);

private:

    /** Pointer to the default logger, used by msgHandler() */
    static Logger* defaultLogger;

    /**
      Message Handler for the global static logging functions (e.g. qDebug()).
      Forward calls to the default logger.
      <p>
      In case of a fatal message, the program will abort.
      Variables in the in the message are replaced by their values.
      This method is thread safe.
      @param type Message type (level)
      @param message Message text
    */
    static void msgHandler(const QtMsgType type, const char* message);

    /** Thread local variables to be used in log messages */
    static QThreadStorage<QHash<QString,QString>*> logVars;

    /** Thread local backtrace buffers */
    static QThreadStorage<QList<LogMessage*>*> buffers;

};

#endif // LOGGER_H
