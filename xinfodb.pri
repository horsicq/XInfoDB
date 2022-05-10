INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xinfodb.h

SOURCES += \
    $$PWD/xinfodb.cpp

!contains(XCONFIG, xformats) {
    XCONFIG += xformats
    include($$PWD/../Formats/xformats.pri)
}

DISTFILES += \
    $$PWD/xinfodb.cmake

