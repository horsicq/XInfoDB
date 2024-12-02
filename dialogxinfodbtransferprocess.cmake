include_directories(${CMAKE_CURRENT_LIST_DIR})

if (NOT DEFINED XDIALOGPROCESS_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../FormatDialogs/xdialogprocess.cmake)
    set(DIALOGXINFODBTRANSFERPROCESS_SOURCES ${DIALOGXINFODBTRANSFERPROCESS_SOURCES} ${XDIALOGPROCESS_SOURCES})
endif()

# TODO Check includes
set(DIALOGXINFODBTRANSFERPROCESS_SOURCES
    ${DIALOGXINFODBTRANSFERPROCESS_SOURCES}
    ${XDIALOGPROCESS_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/dialogxinfodbtransferprocess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dialogxinfodbtransferprocess.h
    ${CMAKE_CURRENT_LIST_DIR}/xinfodbtransfer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xinfodbtransfer.h
    ${CMAKE_CURRENT_LIST_DIR}/xinfodb.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xinfodb.h
    ${CMAKE_CURRENT_LIST_DIR}/xinfodboptionswidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xinfodboptionswidget.h
    ${CMAKE_CURRENT_LIST_DIR}/xinfodboptionswidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/xinfomenu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xinfomenu.h
)
