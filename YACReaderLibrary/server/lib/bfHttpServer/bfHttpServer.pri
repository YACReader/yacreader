INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/httplistener.h $$PWD/httpconnectionhandler.h $$PWD/httpconnectionhandlerpool.h $$PWD/httprequest.h $$PWD/httpresponse.h $$PWD/httpcookie.h $$PWD/httprequesthandler.h
HEADERS += $$PWD/httpsession.h $$PWD/httpsessionstore.h
HEADERS += $$PWD/staticfilecontroller.h

SOURCES += $$PWD/httplistener.cpp $$PWD/httpconnectionhandler.cpp $$PWD/httpconnectionhandlerpool.cpp $$PWD/httprequest.cpp $$PWD/httpresponse.cpp $$PWD/httpcookie.cpp $$PWD/httprequesthandler.cpp
SOURCES += $$PWD/httpsession.cpp $$PWD/httpsessionstore.cpp
SOURCES += $$PWD/staticfilecontroller.cpp

OTHER_FILES += $$PWD/../doc/readme.txt
