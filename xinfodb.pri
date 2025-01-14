INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

!contains(XCONFIG, use_dex) {
    XCONFIG += use_dex
}

!contains(XCONFIG, use_pdf) {
    XCONFIG += use_pdf
}

!contains(XCONFIG, use_archive) {
    XCONFIG += use_archive
}

!contains(XCONFIG, xscanengine) {
    XCONFIG += xscanengine
    include($$PWD/../XScanEngine/xscanengine.pri)
}

!contains(XCONFIG, xdisasmcore) {
    XCONFIG += xdisasmcore
    include($$PWD/../XDisasmCore/xdisasmcore.pri)
}

HEADERS += \
    $$PWD/xinfodb.h

SOURCES += \
    $$PWD/xinfodb.cpp

DISTFILES += \
    $$PWD/LICENSE \
    $$PWD/README.md \
    $$PWD/info/PE.sections.txt \
    $$PWD/info/ELF.sections.txt \
    $$PWD/xinfodb.cmake
