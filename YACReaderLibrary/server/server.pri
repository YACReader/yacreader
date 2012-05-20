INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/static.h \
    $$PWD/startup.h \
    $$PWD/requestmapper.h \
    $$PWD/controllers/dumpcontroller.h \
    $$PWD/controllers/templatecontroller.h \
    $$PWD/controllers/formcontroller.h \
    $$PWD/controllers/fileuploadcontroller.h \
    $$PWD/controllers/sessioncontroller.h

SOURCES += \
    $$PWD/static.cpp \
    $$PWD/startup.cpp \
    $$PWD/requestmapper.cpp \
    $$PWD/controllers/dumpcontroller.cpp \
    $$PWD/controllers/templatecontroller.cpp \
    $$PWD/controllers/formcontroller.cpp \
    $$PWD/controllers/fileuploadcontroller.cpp \
    $$PWD/controllers/sessioncontroller.cpp
	
include(lib/bfLogging/bfLogging.pri)
include(lib/bfHttpServer/bfHttpServer.pri)
include(lib/bfTemplateEngine/bfTemplateEngine.pri)
