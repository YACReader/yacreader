/**
  @file
  @author Stefan Frings
*/

/**
  @mainpage
  This project provies libraries for writing server-side web application in C++
  based on the Qt toolkit. It is a light-weight implementation that works like 
  Java Servlets from the programmers point of view.
  <p>
  Features:

  - HTTP 1.1 web server
      - persistent connections
      - chunked and non-chunked transfer
      - file uploads (multipart encoded forms)
      - cookies
      - sessions
      - uses dynamic thread pool
      - optional file cache
  - Template engine 
      - multi-language
      - optional file cache
  - Logger
      - configurable without program restart
      - automatic backup and file rotation
      - configurable message format
      - messages may contain thread-local variables
      - optional buffer for writing history details in case of an error
  - Example application
      - Install and run as Windows service, unix daemon or at the command-line
      - Search config file in multiple common directories
      - Demonstrates how to write servlets for common use-cases

  If you write a real application based on this source, take a look into startup.cpp,
  which contains startup and shutdown procedures. The example source sets
  up a single listener on port 8080, however multiple listeners with individual
  configuration could be set up.
  <p>
  Incoming requests are mapped to controller classes in requestmapper.cpp, based on the
  requested path. If you want to learn form the example, then focus on these classes.
  <p>
  High-availability and HTTPS encryption can be easily added by putting an Apache HTTP server
  in front of the self-written web application using the mod-proxy module with sticky sessions.
*/

#include "startup.h"

/**
  Entry point of the program.
  Passes control to the service helper.
*/
int main(int argc, char *argv[]) {
    // Use a qtservice wrapper to start the application as a Windows service or Unix daemon
    Startup startup(argc, argv);
    return startup.exec();
}
