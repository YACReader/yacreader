TEMPLATE = app
CONFIG += console testcase
QT += core testlib

SOURCES += \
    main.cpp \
    ../../common/image_decoders.cpp

HEADERS += \
    ../../common/image_decoders.h

unix:!mac {
    CONFIG += link_pkgconfig
    PKGCONFIG += libavif libjxl libjxl_threads
}
win32 {
    LIBS += -lavif -ljxl -ljxl_threads
}
mac {
    LIBS += -lavif -ljxl -ljxl_threads
}

RESOURCES += \
    test_images.qrc
