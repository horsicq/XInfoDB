include_directories(${CMAKE_CURRENT_LIST_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/../XScanEngine/xscanengine.cmake)
# mb TODO XCapstone
set(XINFODB_SOURCES
    ${XSCANENGINE_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/xinfodb.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xinfodb.h
)
