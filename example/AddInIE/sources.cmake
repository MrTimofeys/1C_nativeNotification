#--source files-------------------
set(SOURCES
    config.cpp
    stdafx.cpp
)

set(HEADERS
    stdafx.h
    targetver.h
    Resource.h
    config.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/AddInIE.rc
    AddInSite.rgs
    AddInWeb.rgs
    ${CMAKE_CURRENT_BINARY_DIR}/AddInIE.def
)

set_source_files_properties(AddInWeb.rgs AddInSite.rgs PROPERTIES HEADER_FILE_ONLY TRUE)

set(SRC ${SOURCES})
list(APPEND SRC ${HEADERS})
#--source files end---------------