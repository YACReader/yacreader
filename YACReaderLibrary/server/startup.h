/**
  @file
  @author Stefan Frings
*/

#ifndef STARTUP_H
#define STARTUP_H

#include <QString>

namespace stefanfrings {
class HttpListener;
}

/**
  Helper class to install and run the application as a windows
  service.
*/
class Startup
{
private:
    //QTcpServer
    stefanfrings::HttpListener *listener;

public:
    /** Constructor */
    Startup();
    /** Start the server */
    void start(quint16 port = 0);
    /** Stop the server */
    void stop();

    QString getPort();

protected:
};

#endif // STARTUP_H
