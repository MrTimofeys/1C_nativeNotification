#--source files-------------------
set(SOURCES
    AddInCOM.cpp
    AddIn.cpp
    dllmain.cpp
    stdafx.cpp
    dlldatax.c
)

set(HEADERS
    AddInCOM.h
    dlldatax.h
    dllmain.h
    resource.h
    stdafx.h
    targetver.h
)

list(APPEND SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/AddIn.rc
    AddInCOM.rgs
    AddIn.rgs
    ${CMAKE_CURRENT_BINARY_DIR}/AddIn.def
)

set_source_files_properties(AddInCOM.rgs AddIn.rgs PROPERTIES HEADER_FILE_ONLY TRUE)

set(SRC ${SOURCES})
list(APPEND SRC ${HEADERS})
#--source files end---------------