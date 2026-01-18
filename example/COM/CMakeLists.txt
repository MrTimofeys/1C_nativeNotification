# Cmake script for project AddInCOM
# autor: 1C

cmake_minimum_required(VERSION 3.18...3.31 FATAL_ERROR)

if (UNIX)
    message(WARNING "Must Windows only")
    return()
ENDIF()

project(${Addn_COM})
set(CMAKE_INCLUDE_CURRENT_DIR ON)

configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/AddIn.rc.in ${CMAKE_CURRENT_BINARY_DIR}/AddIn.rc)
configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/AddIn.def.in ${CMAKE_CURRENT_BINARY_DIR}/AddIn.def)
#----------------------------------------------------------------------------------------------------------------#
include(${CMAKE_CURRENT_SOURCE_DIR}/sources.cmake)
set(AddnCOM_SRC ${SRC})
#----------------------------------------------------------------------------------------------------------------#

ADD_DEFINITIONS(-DWIN32 -DUNICODE -D_USRDLL -D_MERGE_PROXYSTUB)

if(NOT TARGET_PLATFORM_32)
    set(AMD_64 "/amd64")
else()
    set(AMD_64 "/win32")
ENDIF()

set(MIDL_OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/AddIn_i.h
    ${CMAKE_CURRENT_BINARY_DIR}/AddIn_i.c
    ${CMAKE_CURRENT_BINARY_DIR}/AddIn.tlb
    #${CMAKE_CURRENT_BINARY_DIR}/dlldata.c
    )
set(MIDL_FILE
    ${CMAKE_CURRENT_LIST_DIR}/AddIn.idl
    )
add_custom_command(
    OUTPUT ${MIDL_OUTPUT}
    COMMAND midl /h ${CMAKE_CURRENT_BINARY_DIR}/AddIn_i.h 
        /proxy ${CMAKE_CURRENT_BINARY_DIR}/AddIn_p.c 
        /iid ${CMAKE_CURRENT_BINARY_DIR}/AddIn_i.c 
        /dlldata ${CMAKE_CURRENT_BINARY_DIR}/dlldata.c 
        /tlb ${CMAKE_CURRENT_BINARY_DIR}/AddIn.tlb
        ${AMD_64} ${MIDL_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    MAIN_DEPENDENCY ${MIDL_FILE}
    BYPRODUCTS ${MIDL_OUTPUT}
    VERBATIM
    )
set(ADDITIONAL_CLEAN_FILES ${MIDL_OUTPUT})
ADD_DEFINITIONS( -D_WINDLL -D_CRT_SECURE_NO_WARNINGS)



set(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/../lib)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/../lib)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../include)

add_library(${PROJECT_NAME} SHARED ${AddnCOM_SRC} ${MIDL_OUTPUT})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} RpcRT4.lib oleaut32.lib)

SET_TARGET_PROPERTIES( ${PROJECT_NAME} PROPERTIES
    PREFIX ""
   CLEAN_DIRECT_OUTPUT 1
   OUTPUT_NAME ${PROJECT_NAME}${MySuffix2}_${Addn_VERSION}
   )
if (SIGN_BAT)
    set(SIGN_CMD1 ${SIGN_BAT} $<TARGET_FILE:${PROJECT_NAME}>)
    set(SIGN_CMD2 )
elseif(SIGN_TOOL AND EXISTS ${SIGN_CERT})
    set(SIGN_CMD1 ${SIGN_SHA1} $<TARGET_FILE:${PROJECT_NAME}>)
    set(SIGN_CMD2 ${SIGN_SHA256} $<TARGET_FILE:${PROJECT_NAME}>)
else()
    set(SIGN_CMD1)
    set(SIGN_CMD2)
endif()
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
        COMMAND ${SIGN_CMD1}
        COMMAND ${SIGN_CMD2}
        COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:${PROJECT_NAME}> ${LIB_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>
        COMMENT "Copy ${PROJECT_NAME}"
        )
