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
    $$PWD/controllers/sessioncontroller.h \
	$$PWD/controllers/comiccontroller.h \ 
	$$PWD/controllers/errorcontroller.h \
	$$PWD/controllers/foldercontroller.h \
	$$PWD/controllers/folderinfocontroller.h \
	$$PWD/controllers/librariescontroller.h \
	$$PWD/controllers/pagecontroller.h \
	$$PWD/controllers/sessionmanager.h \
	$$PWD/controllers/covercontroller.h 

SOURCES += \
    $$PWD/static.cpp \
    $$PWD/startup.cpp \
    $$PWD/requestmapper.cpp \
    $$PWD/controllers/dumpcontroller.cpp \
    $$PWD/controllers/templatecontroller.cpp \
    $$PWD/controllers/formcontroller.cpp \
    $$PWD/controllers/fileuploadcontroller.cpp \
    $$PWD/controllers/sessioncontroller.cpp \
	$$PWD/controllers/comiccontroller.cpp \
	$$PWD/controllers/errorcontroller.cpp \
	$$PWD/controllers/foldercontroller.cpp \
	$$PWD/controllers/folderinfocontroller.cpp \
	$$PWD/controllers/librariescontroller.cpp \
	$$PWD/controllers/pagecontroller.cpp \
	$$PWD/controllers/sessionmanager.cpp \
	$$PWD/controllers/covercontroller.cpp
	
include(lib/bfLogging/bfLogging.pri)
include(lib/bfHttpServer/bfHttpServer.pri)
include(lib/bfTemplateEngine/bfTemplateEngine.pri)
