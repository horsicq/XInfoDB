INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xinfodbtransfer.h

SOURCES += \
    $$PWD/xinfodbtransfer.cpp

!contains(XCONFIG, xinfodb) {
    XCONFIG += xinfodb
    include($$PWD/xinfodb.pri)
}

DISTFILES += \
    $$PWD/LICENSE \
    $$PWD/README.md
