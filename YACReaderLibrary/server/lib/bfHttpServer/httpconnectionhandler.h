/**
  @file
  @author Stefan Frings
*/

#ifndef HTTPCONNECTIONHANDLER_H
#define HTTPCONNECTIONHANDLER_H

#include <QTcpSocket>
#include <QSettings>
#include <QTimer>
#include <QThread>
#include "httprequest.h"
#include "httprequesthandler.h"

/**
  The connection handler accepts incoming connections and dispatches incoming requests to to a
  request mapper. Since HTTP clients can send multiple requests before waiting for the response,
  the incoming requests are queued and processed one after the other.
  <p>
  Example for the required configuration settings:
  <code><pre>
  readTimeout=60000
  maxRequestSize=16000
  maxMultiPartSize=1000000
  </pre></code>
  <p>
  The readTimeout value defines the maximum time to wait for a complete HTTP request.
  @see HttpRequest for description of config settings maxRequestSize and maxMultiPartSize
*/

class HttpConnectionHandler : public QThread {
    Q_OBJECT
    Q_DISABLE_COPY(HttpConnectionHandler)
public:

    /**
      Constructor.
      @param settings Configuration settings of the HTTP webserver
      @param requestHandler handler that will process each incomin HTTP request
    */
    HttpConnectionHandler(QSettings* settings, HttpRequestHandler* requestHandler);

    /** Destructor */
    virtual ~HttpConnectionHandler();

    /** Returns true, if this handler is in use. */
    bool isBusy();

    /** Mark this handler as busy */
    void setBusy();

private:

    /** Configuration settings */
    QSettings* settings;

    /** TCP socket of the current connection */
    QTcpSocket socket;

    /** Time for read timeout detection */
    QTimer readTimer;

    /** Storage for the current incoming HTTP request */
    HttpRequest* currentRequest;

    /** Dispatches received requests to services */
    HttpRequestHandler* requestHandler;

    /** This shows the busy-state from a very early time */
    bool busy;

    /** Executes the htreads own event loop */
    void run();

public slots:

    /**
      Received from from the listener, when the handler shall start processing a new connection.
      @param socketDescriptor references the accepted connection.
    */
    void handleConnection(int socketDescriptor);

private slots:

    /** Received from the socket when a read-timeout occured */
    void readTimeout();

    /** Received from the socket when incoming data can be read */
    void read();

    /** Received from the socket when a connection has been closed */
    void disconnected();

};

#endif // HTTPCONNECTIONHANDLER_H
