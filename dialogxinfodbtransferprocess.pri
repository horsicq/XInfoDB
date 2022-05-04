INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

FORMS += \
    $$PWD/dialogxinfodbtransferprocess.ui

HEADERS += \
    $$PWD/dialogxinfodbtransferprocess.h

SOURCES += \
    $$PWD/dialogxinfodbtransferprocess.cpp

!contains(XCONFIG, xinfodbtransfer) {
    XCONFIG += xinfodbtransfer
    include($$PWD/xinfodbtransfer.pri)
}

DISTFILES += \
    $$PWD/dialogxinfodbtransferprocess.cmake
