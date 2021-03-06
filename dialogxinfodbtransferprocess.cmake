include_directories(${CMAKE_CURRENT_LIST_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/../FormatDialogs/xdialogprocess.cmake)

set(DIALOGXINFODBTRANSFERPROCESS_SOURCES
    ${XDIALOGPROCESS_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/dialogxinfodbtransferprocess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dialogxinfodbtransferprocess.ui
    ${CMAKE_CURRENT_LIST_DIR}/xinfodbtransfer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xinfodb.cpp
)
