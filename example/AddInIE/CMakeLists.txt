# Cmake script for project AddInIE
# autor: 1C

cmake_minimum_required(VERSION 3.18...3.31 FATAL_ERROR)

project(${Addn_IE})

if (NOT UNIX)
    set(CMAKE_BUILD_TYPE Release)
    if(NOT "${CMAKE_GENERATOR_TOOLSET}" STREQUAL "${VS_TOOLSET_FOR_BROWSERS_REQUIRED}")
        message(WARNING "Building ${PROJECT_NAME} disabled! Supported Visual Studio: 12 (v120_xp), 14 (v120_xp), 15 (v120_xp) or above with v120_xp toolset")
        RETURN()
    endif()
else()
    message(WARNING "Must Windows only")
    RETURN()
endif(NOT UNIX)

set(AddInIEBuild TRUE PARENT_SCOPE)
configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/AddInIE.rc.in ${CMAKE_CURRENT_BINARY_DIR}/AddInIE.rc)
configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/AddInIE.def.in ${CMAKE_CURRENT_BINARY_DIR}/AddInIE.def)
#----------------------------------------------------------------------------------------------------------------#
include(${CMAKE_CURRENT_SOURCE_DIR}/sources.cmake)
set(AddInIE_SRC ${SRC})
#----------------------------------------------------------------------------------------------------------------#
add_definitions(-DWIN32 -DUNICODE -D_USRDLL -D_MERGE_PROXYSTUB)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT TARGET_PLATFORM_32)
    set(AMD_64 "/amd64")
else()
    set(AMD_64 "/win32")
endif()

set(MIDL_OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/AddInIE_i.h
    ${CMAKE_CURRENT_BINARY_DIR}/AddInIE_i.c
    ${CMAKE_CURRENT_BINARY_DIR}/AddInIE.tlb
    )
set(MIDL_FILE
    ${CMAKE_CURRENT_LIST_DIR}/AddInIE.idl
    )
add_custom_command(
    OUTPUT ${MIDL_OUTPUT}
    COMMAND midl /h ${CMAKE_CURRENT_BINARY_DIR}/AddInIE_i.h 
        /iid ${CMAKE_CURRENT_BINARY_DIR}/AddInIE_i.c
        /proxy ${CMAKE_CURRENT_BINARY_DIR}/AddInIE_p.c 
        /dlldata ${CMAKE_CURRENT_BINARY_DIR}/dlldata.c 
        /tlb ${CMAKE_CURRENT_BINARY_DIR}/AddInIE.tlb
        ${AMD_64} ${MIDL_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    MAIN_DEPENDENCY ${MIDL_FILE}
    BYPRODUCTS ${MIDL_OUTPUT}
    VERBATIM
    )

set(ADDITIONAL_CLEAN_FILES ${MIDL_OUTPUT})
set(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/../lib)
link_directories(${CMAKE_SOURCE_DIR}/../lib)
include_directories(${CMAKE_SOURCE_DIR}/../include)

add_library(${PROJECT_NAME} SHARED ${AddInIE_SRC} ${MIDL_OUTPUT})

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_START_STATIC 1)
find_library(1CEADNWEBIE "1CEAdnWebIE${MySuffix2}")
target_link_libraries(${PROJECT_NAME} ${1CEADNWEBIE})
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_END_STATIC 1)

target_link_libraries(${PROJECT_NAME}  RpcRT4.lib oleaut32.lib)

set_target_properties( ${PROJECT_NAME} PROPERTIES
   OUTPUT_NAME ${PROJECT_NAME}${MySuffix2} 
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
