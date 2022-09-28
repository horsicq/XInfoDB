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

!contains(XCONFIG, xdialogprocess) {
    XCONFIG += xdialogprocess
    include($$PWD/../FormatDialogs/xdialogprocess.pri)
}

DISTFILES += \
    $$PWD/LICENSE \
    $$PWD/README.md \
    $$PWD/dialogxinfodbtransferprocess.cmake
