add_library(QrCode STATIC QrCode/qrcodegen.hpp QrCode/qrcodegen.cpp)
target_include_directories(QrCode PUBLIC QrCode)

add_library(
  QtWebApp_httpserver STATIC
  QtWebApp/httpserver/httpconnectionhandler.cpp
  QtWebApp/httpserver/httpconnectionhandler.h
  QtWebApp/httpserver/httpconnectionhandlerpool.cpp
  QtWebApp/httpserver/httpconnectionhandlerpool.h
  QtWebApp/httpserver/httpcookie.cpp
  QtWebApp/httpserver/httpcookie.h
  QtWebApp/httpserver/httpglobal.cpp
  QtWebApp/httpserver/httpglobal.h
  QtWebApp/httpserver/httplistener.cpp
  QtWebApp/httpserver/httplistener.h
  QtWebApp/httpserver/httprequest.cpp
  QtWebApp/httpserver/httprequest.h
  QtWebApp/httpserver/httprequesthandler.cpp
  QtWebApp/httpserver/httprequesthandler.h
  QtWebApp/httpserver/httpresponse.cpp
  QtWebApp/httpserver/httpresponse.h
  QtWebApp/httpserver/httpsession.cpp
  QtWebApp/httpserver/httpsession.h
  QtWebApp/httpserver/httpsessionstore.cpp
  QtWebApp/httpserver/httpsessionstore.h
  QtWebApp/httpserver/staticfilecontroller.cpp
  QtWebApp/httpserver/staticfilecontroller.h)

target_link_libraries(QtWebApp_httpserver PUBLIC Qt::Network)
target_include_directories(QtWebApp_httpserver PUBLIC QtWebApp/httpserver)
# target_compile_definitions(QtWebApp SUPERVERBOSE)

add_library(
  QtWebApp_templateengine STATIC
  QtWebApp/templateengine/template.cpp
  QtWebApp/templateengine/template.h
  QtWebApp/templateengine/templatecache.cpp
  QtWebApp/templateengine/templatecache.h
  QtWebApp/templateengine/templateglobal.h
  QtWebApp/templateengine/templateloader.cpp
  QtWebApp/templateengine/templateloader.h)

target_link_libraries(QtWebApp_templateengine Qt::Core5Compat)
target_include_directories(QtWebApp_templateengine
                           PUBLIC QtWebApp/templateengine)

add_library(
  QsLog
  QsLog/QsLog.cpp
  QsLog/QsLog.h
  QsLog/QsLogDest.cpp
  QsLog/QsLogDest.h
  QsLog/QsLogDestConsole.cpp
  QsLog/QsLogDestConsole.h
  QsLog/QsLogDestFile.cpp
  QsLog/QsLogDestFile.h
  QsLog/QsLogDestFunctor.cpp
  QsLog/QsLogDestFunctor.h
  QsLog/QsLogDisableForThisFile.h
  QsLog/QsLogLevel.cpp
  QsLog/QsLogLevel.h
  QsLog/QsLogMessage.cpp
  QsLog/QsLogMessage.h
  QsLog/QsLogSharedLibrary.h)
target_compile_definitions(QsLog PRIVATE QS_LOG_SEPARATE_THREAD)
target_include_directories(QsLog PUBLIC QsLog)
target_link_libraries(QsLog Qt::Core)

add_library(KDSignalThrottler KDToolBox/KDSignalThrottler.h
                              KDToolBox/KDSignalThrottler.cpp)
target_include_directories(KDSignalThrottler PUBLIC KDToolBox)
target_link_libraries(KDSignalThrottler Qt::Core)
