INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
    $$PWD/resize_image.cpp

HEADERS += \
    $$PWD/resize_image.h

# include opencv, and link against it (core and imgproc)
# windows
win32 {
    INCLUDEPATH += $$PWD/opencv/build/include
    DEPENDPATH += $$PWD/opencv/build/include
}
# release
win32:CONFIG(release, debug|release) {

    LIBS += -L$$PWD/opencv/build/x64/vc16/lib \
            -lopencv_world490
}
# debug
win32:CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/opencv/build/x64/vc16/lib \
            -lopencv_world490d
}


#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../opencv/build/x64/vc15/lib/ -lopencv_world341
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../opencv/build/x64/vc15/lib/ -lopencv_world341d

#INCLUDEPATH += $$PWD/../../../../opencv/build/include
#DEPENDPATH += $$PWD/../../../../opencv/build/include
