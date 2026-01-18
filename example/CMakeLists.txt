# Cmake script for project AddIn
# autor: 1C
cmake_minimum_required (VERSION 3.18...3.31)

if(NOT UNIX)
    # Placing automatically generated targets (ALL_BUILD, ZERO_CHECK, etc) into separate folder
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Build configurations" FORCE)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build configurations" FORCE)
endif()
include(CMakePrintHelpers)

project (AddIn)
list(APPEND CMAKE_VS_GLOBALS "XPDeprecationWarning=false")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(settings)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(compilers)

set(OS_NAME ${CMAKE_SYSTEM_NAME} CACHE STRING "OS name")
if (UNIX)
    set(MySuffix1 "Lin" CACHE STRING "OS")
    if (APPLE)
        set(MySuffix1 "Mac" CACHE STRING "OS" FORCE)
        set(OS_NAME "MacOS" CACHE STRING "OS name" FORCE)
    endif(APPLE)
else(UNIX)
    set(MySuffix1 "Win" CACHE STRING "OS")
endif(UNIX)

set(MySuffix2Append "" CACHE STRING "") # common

set(LIBSUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
set(LIBPREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
set(EXESUFFIX ${CMAKE_EXECUTABLE_SUFFIX})

set(OS ${MySuffix1})
set(ARCH ${MySuffix2})
set(ARCHSUFFIX ${MySuffix2Append})
if (NOT EXESUFFIX)
    set(EXESUFFIX "")
endif()
set(manifest_file "MANIFEST.XML")

include(dirs)
#==============================================================================#
string(TIMESTAMP YEAR "%Y")
#==============================================================================#

option(SIGN_SIGNING "Signing binaries" ON)
include(utils)
#==============================================================================#
#seting you version file
if(EXISTS "${CMAKE_SOURCE_DIR}/AddInVersion.txt")
    FILE(STRINGS "${CMAKE_SOURCE_DIR}/AddInVersion.txt" APP_VERSION LIMIT_COUNT 1)
else()
    set(APP_VERSION 01)
ENDIF()
set(Addn_VERSION ${APP_VERSION} CACHE STRING "Version of external component" )
set(ADDNVER "${Addn_VERSION}")
set(Addn_VERSION_FULL
    "${Addn_VERSION_MAJOR}.${Addn_VERSION_MINOR}.${Addn_VERSION_BUILD}.${Addn_VERSION}" CACHE 
    STRING "Full version of external component")
STRING(REPLACE "." "," Addn_VERSION_FULL_C ${Addn_VERSION_FULL})
STRING(REPLACE "." "_" Addn_VERSION_FULL_U ${Addn_VERSION_FULL})
#==============================================================================#
string(TOLOWER ${MYCOMPANY} prefix_lower)
set(PKG_ID "com.${prefix_lower}.pkg.${Addn_ChromeSetup}-${Addn_VERSION_FULL}")

set(COMP_EXTERNAL OFF)
if (DEFINED FROM_ZIP)
    # exists addin from zip
    if (NOT IS_ABSOLUTE(${FROM_ZIP}))
        file(REAL_PATH ${FROM_ZIP} FROM_ZIP EXPAND_TILDE)
        file(TO_NATIVE_PATH "${FROM_ZIP}" FROM_ZIP)
    endif()
    set(COMP_EXTERNAL ON)
    set(zip_path ${CMAKE_CURRENT_BINARY_DIR}/zip)
    get_filename_component(EXT ${FROM_ZIP} LAST_EXT)
    if(${EXT} STREQUAL ".zip")
        file(ARCHIVE_EXTRACT INPUT ${FROM_ZIP} DESTINATION ${zip_path} PATTERNS *${LIBSUFFIX})
    else()
        file(COPY ${FROM_ZIP} DESTINATION ${zip_path} FILES_MATCHING PATTERN "*${LIBSUFFIX}")
    endif()
    unset(EXT)
    file(GLOB so_files LIST_DIRECTORIES false CONFIGURE_DEPENDS ${zip_path}/*${LIBSUFFIX})
    list(LENGTH so_files count_files)
    set(COMP_NOT_FOUND ON)
    foreach(so_file ${so_files})
        get_component_type(${so_file} comp_type)
        if(${comp_type} GREATER 1)
            get_filename_component(_Addn_Native ${so_file} NAME_WE)
            get_filename_component(ADDNNATIVE ${so_file} NAME)
            set(COMP_NOT_FOUND 0)
            set(COMP_FILE ${so_file})
            file(COPY ${so_file} DESTINATION ${LIB_DIR})
            file(COPY ${so_file} DESTINATION ${PKG_OUTPUT_DIR})
            break()
        endif()
    endforeach()
    if(COMP_NOT_FOUND)
        message(WARNING "AddIn not determinate. Exit.")
        return()
    endif()
    unset(COMP_NOT_FOUND)
    set(ext_type "native")
    if(comp_type EQUAL 3)
        set(COM_TYPE 1)
        set(ext_type "com")
    endif()
    set(Addn_Native ${_Addn_Native} CACHE STRING "Name for NativeAPI project" FORCE)
    add_library(${Addn_Native} SHARED IMPORTED)
    set_target_properties(${Addn_Native} PROPERTIES IMPORTED_LOCATION ${COMP_FILE})
    #=========================================================================
    #generating manifest
    if(count_files LESS 2)
        set(manifest_begin [=[<?xml version="1.0" encoding="UTF-8"?>
<bundle xmlns="http://v8.1c.ru/8.2/addin/bundle">
]=])
        set(manifest_end [=[
</bundle>]=])
    else()
        if(NOT EXISTS ${PKG_OUTPUT_DIR}/${manifest_file})
            set(manifest_begin [=[<?xml version="1.0" encoding="UTF-8"?>
<bundle xmlns="http://v8.1c.ru/8.2/addin/bundle">
]=])
            set(manifest_end "")
        else()
            set(manifest_begin "")
            set(manifest_end [=[
</bundle>]=])
        endif()
    endif()

    set(pattern [=[    <component os="@OS_NAME@" path="@ADDNNATIVE@" type="@ext_type@" arch="@OS_ARCH@"/>
]=])
    string(CONFIGURE ${pattern} _addin_str @ONLY ESCAPE_QUOTES)
    string(APPEND manifest_begin ${_addin_str})
    unset(_addin_str)
    unset(comp_type)
    unset(pattern)
endif()
#==============================================================================#
set(MIMETYPE ${Addn_MimeType})
set(ADDNCHROME ${Addn_Chrome})
set(ADDNNATIVE ${Addn_Native})
set(ADDNIE ${Addn_IE})

configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/AddInVersion.h.in ${CMAKE_BINARY_DIR}/AddInVersion.h @ONLY)

UNSET(MIMITYPE)
UNSET(ADDNCHROME)
UNSET(ADDNNATIVE)
UNSET(ADDNIE)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

if(UNIX AND NOT APPLE)
    set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH ON)
    find_program(STRIP NAME eu-strip HINTS ENV PATH
        DOC "eu-strip executable" )
    if (NOT STRIP)
        message(WARNING "eu-strip not found!")
    endif()
    
    set(TMP_DIR ${CMAKE_BINARY_DIR}/${MySuffix2})
elseif(APPLE)
    set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH ON)
    find_program(STRIP NAME dsymutil HINTS ENV PATH
        DOC "dsymutil executable" )
    if (NOT STRIP)
        message(WARNING "dsymutil not found!")
    else()
        find_program(STRIP2 NAME strip HINTS ENV PATH
        DOC "strip executable" )
    endif()
endif()
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_subdirectory (NativeAPI)
add_subdirectory (AddInChrome)
add_subdirectory (AddInChromeSetup)

if (WIN32)
    add_subdirectory(AddInIE)
    if(NOT DEFINED FROM_ZIP)
        add_subdirectory(COM)
    endif()
    add_subdirectory(AddInIESetup)
endif (WIN32)


if(COMP_EXTERNAL)
    string(APPEND manifest_begin ${manifest_end})
    file(APPEND ${PKG_OUTPUT_DIR}/${manifest_file} ${manifest_begin})
else()
    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/${manifest_file}.in ${PKG_OUTPUT_DIR}/${manifest_file} @ONLY)
endif()
