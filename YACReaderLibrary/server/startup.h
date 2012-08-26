/**
  @file
  @author Stefan Frings
*/

#ifndef STARTUP_H
#define STARTUP_H

class HttpListener;
/**
  Helper class to install and run the application as a windows
  service.
*/
class Startup
{
private:
	//QTcpServer
	HttpListener * listener;
public:

    /** Constructor */
    Startup();
	/** Start the server */
    void start();
	/** Stop the server */
    void stop();
protected:
};

#endif // STARTUP_H
