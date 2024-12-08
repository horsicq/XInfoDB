include_directories(${CMAKE_CURRENT_LIST_DIR})

if (NOT DEFINED XSCANENGINE_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../XScanEngine/xscanengine.cmake)
    set(XINFODB_SOURCES ${XINFODB_SOURCES} ${XSCANENGINE_SOURCES})
endif()

# mb TODO XCapstone
set(XINFODB_SOURCES
    ${XINFODB_SOURCES}
    ${XSCANENGINE_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/xinfodb.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xinfodb.h
)
