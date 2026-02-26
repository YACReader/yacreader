# Third-party libraries for YACReader

# --- QsLog (logging) ---
add_library(QsLog STATIC
    QsLog/QsLogDest.cpp
    QsLog/QsLog.cpp
    QsLog/QsLogDestConsole.cpp
    QsLog/QsLogDestFile.cpp
    QsLog/QsLogDestFunctor.cpp
    QsLog/QsLogMessage.cpp
    QsLog/QsLogLevel.cpp
    QsLog/QsLogDest.h
    QsLog/QsLog.h
    QsLog/QsLogDestConsole.h
    QsLog/QsLogLevel.h
    QsLog/QsLogDestFile.h
    QsLog/QsLogDisableForThisFile.h
    QsLog/QsLogDestFunctor.h
    QsLog/QsLogMessage.h
    QsLog/QsLogSharedLibrary.h
)
target_include_directories(QsLog PUBLIC QsLog)
target_compile_definitions(QsLog PUBLIC QS_LOG_SEPARATE_THREAD)
target_link_libraries(QsLog PUBLIC Qt::Core)

# --- QrCode (QR code generation, no Qt dependency) ---
add_library(QrCode STATIC
    QrCode/qrcodegen.cpp
    QrCode/qrcodegen.hpp
)
target_include_directories(QrCode PUBLIC QrCode)

# --- QtWebApp HTTP server ---
add_library(QtWebApp_httpserver STATIC
    QtWebApp/httpserver/httpglobal.cpp
    QtWebApp/httpserver/httplistener.cpp
    QtWebApp/httpserver/httpconnectionhandler.cpp
    QtWebApp/httpserver/httpconnectionhandlerpool.cpp
    QtWebApp/httpserver/httprequest.cpp
    QtWebApp/httpserver/httpresponse.cpp
    QtWebApp/httpserver/httpcookie.cpp
    QtWebApp/httpserver/httprequesthandler.cpp
    QtWebApp/httpserver/httpsession.cpp
    QtWebApp/httpserver/httpsessionstore.cpp
    QtWebApp/httpserver/staticfilecontroller.cpp
    QtWebApp/httpserver/httpglobal.h
    QtWebApp/httpserver/httplistener.h
    QtWebApp/httpserver/httpconnectionhandler.h
    QtWebApp/httpserver/httpconnectionhandlerpool.h
    QtWebApp/httpserver/httprequest.h
    QtWebApp/httpserver/httpresponse.h
    QtWebApp/httpserver/httpcookie.h
    QtWebApp/httpserver/httprequesthandler.h
    QtWebApp/httpserver/httpsession.h
    QtWebApp/httpserver/httpsessionstore.h
    QtWebApp/httpserver/staticfilecontroller.h
)
target_include_directories(QtWebApp_httpserver PUBLIC QtWebApp/httpserver)
target_link_libraries(QtWebApp_httpserver PUBLIC Qt::Network)

# --- QtWebApp template engine ---
add_library(QtWebApp_templateengine STATIC
    QtWebApp/templateengine/template.cpp
    QtWebApp/templateengine/templateloader.cpp
    QtWebApp/templateengine/templatecache.cpp
    QtWebApp/templateengine/templateglobal.h
    QtWebApp/templateengine/template.h
    QtWebApp/templateengine/templateloader.h
    QtWebApp/templateengine/templatecache.h
)
target_include_directories(QtWebApp_templateengine PUBLIC QtWebApp/templateengine)
target_link_libraries(QtWebApp_templateengine PUBLIC Qt::Core5Compat)

# --- KDSignalThrottler (from KDToolBox) ---
add_library(KDSignalThrottler STATIC
    KDToolBox/KDSignalThrottler.cpp
    KDToolBox/KDSignalThrottler.h
)
target_include_directories(KDSignalThrottler PUBLIC KDToolBox)
target_link_libraries(KDSignalThrottler PUBLIC Qt::Core)
