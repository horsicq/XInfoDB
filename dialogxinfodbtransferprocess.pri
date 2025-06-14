INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xinfodboptionswidget.h \
    $$PWD/xinfomenu.h

SOURCES += \
    $$PWD/xinfodboptionswidget.cpp \
    $$PWD/xinfomenu.cpp

FORMS += \
    $$PWD/xinfodboptionswidget.ui

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
