# Cmake script for project AddInIESetup
# autor: 1C

cmake_minimum_required(VERSION 3.18...3.31 FATAL_ERROR)

if(NOT DEFINED AddInIEBuild)
    return()
endif()

project(${Addn_IESetup})
if (UNIX)
    message(FATAL_ERROR "Must Windows only")
endif(UNIX)

find_program(MNSIS makensis)

if(NOT MNSIS)
	message(WARNING "NSIS not found.")
    return()
endif()

if (NOT TARGET_PLATFORM_32)
        set(ADD_SUFFIX "x64")
    else()
        set(ADD_SUFFIX "x86")
    endif()
    set(ADD_BITS ${MySuffix2})
    
    set(ADAPTER ${Addn_IE}${MySuffix2}${LIBSUFFIX})
    if(DEFINED FROM_ZIP)
        set(EXCOMP ${Addn_Native}${LIBSUFFIX})
    else()
        set(EXCOMP ${Addn_Native}${MySuffix1}${MySuffix2}_${Addn_VERSION}${LIBSUFFIX})
    endif()
    set(PRODUCT "AddInIE")
    set(OUTFILENAME ${PROJECT_NAME})
    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/AddInIESetup.nsh.in ${CMAKE_CURRENT_BINARY_DIR}/AddInIESetup.nsh @ONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AddInIESetup.nsi ${CMAKE_CURRENT_BINARY_DIR}/AddInIESetup.nsi COPYONLY)

if (SIGN_BAT)
    set(SIGN_CMD1 ${SIGN_BAT} ${PROJECT_NAME}${MySuffix2}.exe)
    set(SIGN_CMD2 )
elseif(SIGN_TOOL AND EXISTS ${SIGN_CERT})
    set(SIGN_CMD1 ${SIGN_SHA1} ${PROJECT_NAME}${MySuffix2}.exe)
    set(SIGN_CMD2 ${SIGN_SHA256} ${PROJECT_NAME}${MySuffix2}.exe)
else()
    set(SIGN_CMD1)
    set(SIGN_CMD2)
endif()

add_custom_target(${PROJECT_NAME} ALL
            COMMAND ${MNSIS}  AddInIESetup.nsi
            COMMAND ${SIGN_CMD1}
            COMMAND ${SIGN_CMD2}
            COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${MySuffix2}.exe ${PKG_OUTPUT_DIR}/${PROJECT_NAME}${MySuffix2}.exe
            DEPENDS AddInIESetup.nsh AddInIESetup.nsi 
            COMMENT "Build setup for IE..."
            SOURCES AddInIESetup.nsi AddInIESetup.nsh
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            )
add_dependencies(${PROJECT_NAME} ${Addn_IE} ${Addn_Native})

if(COMP_EXTERNAL)
    set(pattern [=[    <component os="@OS_NAME@" path="@PROJECT_NAME@@MySuffix2@.exe" type="plugin" object="@Addn_IE@.AddInServiceEx" arch="@OS_ARCH@" client="MSIE"/>
]=])
    string(CONFIGURE ${pattern} _addin_str @ONLY ESCAPE_QUOTES)
    set(manifest_begin "${manifest_begin}${_addin_str}" PARENT_SCOPE)
    unset(_addin_str)
    unset(pattern)
endif()
