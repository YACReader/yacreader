include(../qt_test.pri)

PATH_TO_common = ../../common

INCLUDEPATH += $$PATH_TO_common
HEADERS += $${PATH_TO_common}/concurrent_queue.h
SOURCES += \
    $${PATH_TO_common}/concurrent_queue.cpp \
    concurrent_queue_test.cpp
