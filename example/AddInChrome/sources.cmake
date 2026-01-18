#--source files-------------------
set(SOURCES
    config.cpp
)

set(HEADERS
    config.h
)

set(SRC ${SOURCES})
list(APPEND SRC ${HEADERS})
if(WIN32 AND MSVC)
    list(APPEND SRC ${CMAKE_CURRENT_BINARY_DIR}/${COMMON_NAME}.exe.manifest ${CMAKE_CURRENT_BINARY_DIR}/AddInChrome.rc)
endif()
#--source files end---------------
