INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/dialogxinfodbtransferprocess.h

SOURCES += \
    $$PWD/dialogxinfodbtransferprocess.cpp

FORMS += \
    $$PWD/dialogxinfodbtransferprocess.ui

!contains(XCONFIG, xinfodbtransfer) {
    XCONFIG += xinfodbtransfer
    include($$PWD/xinfodbtransfer.pri)
}

!contains(XCONFIG, xdialogprocess) {
    XCONFIG += xdialogprocess
    include($$PWD/../FormatDialogs/xdialogprocess.pri)
}

DISTFILES += \
    $$PWD/LICENSE \
    $$PWD/README.md \
    $$PWD/dialogxinfodbtransferprocess.cmake
