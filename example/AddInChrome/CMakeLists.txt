# Cmake script for project AddInChr
# autor: 1C

cmake_minimum_required(VERSION 3.18...3.31 FATAL_ERROR)

project(${Addn_Chrome})

if (NOT UNIX)
    if(NOT "${CMAKE_GENERATOR_TOOLSET}" STREQUAL "${VS_TOOLSET_FOR_BROWSERS_REQUIRED}")
        message(WARNING "Building ${PROJECT_NAME} disabled! Supported Visual Studio: 15 (v140_xp), 14 (v140_xp), 15 (v140_xp) or above with v140_xp toolset")
        RETURN()
    endif()
endif(NOT UNIX)

set(AddInChrBuild TRUE PARENT_SCOPE)
set(COMMON_NAME ${PROJECT_NAME}$CACHE{MySuffix1}$CACHE{MySuffix2})
if(WIN32)
    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/AddInChrome.rc.in ${CMAKE_CURRENT_BINARY_DIR}/AddInChrome.rc)
    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/Addn.exe.manifest.in ${CMAKE_CURRENT_BINARY_DIR}/${COMMON_NAME}.exe.manifest @ONLY)
endif()
#----------------------------------------------------------------------------------------------------------------#
include(${CMAKE_CURRENT_SOURCE_DIR}/sources.cmake)
set(AddInChr_SRC ${SRC})
#----------------------------------------------------------------------------------------------------------------#

if (TARGET_PLATFORM_32)
    set(FIND_LIBRARY_USE_LIB64_PATHS OFF)
    set(FIND_LIBRARY_USE_LIB32_PATHS ON)
    set(CMAKE_SIZEOF_VOID_P 4)
else()
    set(FIND_LIBRARY_USE_LIB64_PATHS ON)
    set(FIND_LIBRARY_USE_LIB32_PATHS OFF)
    set(CMAKE_SIZEOF_VOID_P 8)
endif()

if (UNIX)
    if (NOT APPLE)
        set_source_files_properties(${AddInChr_SRC}
              PROPERTIES COMPILE_FLAGS
              "-fshort-wchar")
    else()
        set_source_files_properties(${AddInChr_SRC}
              PROPERTIES COMPILE_FLAGS
              "-fshort-wchar -stdlib=libc++")
        ADD_DEFINITIONS(-D__APPLE__ )
    endif()
else()
    ADD_DEFINITIONS(-DWIN32 )
endif()

if (APPLE)
    set(MAC_LIBRARIES
            "-framework Carbon"
            "-framework Cocoa"
            "-framework CoreFoundation"
            #"-framework AudioToolbox"
            "-framework IOKit"
            "-framework OpenGL"
            "-framework System"
            "-framework WebKit"
            "-lobjc")
endif()
include(CMakePrintHelpers)
if (UNIX AND NOT APPLE)
    find_library(GTK3 NAMES gtk-3 gtk-3.0 REQUIRED)
    list(APPEND GTK_LIBS ${GTK3} )

    find_library(GOBJ NAMES gobject-2 gobject-2.0 REQUIRED)
    list(APPEND GTK_LIBS ${GOBJ})
endif (UNIX AND NOT APPLE)

set(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/../lib)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/../lib)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../include)

add_executable(${PROJECT_NAME} ${AddInChr_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES 
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
        )

if (UNIX)
    set(LibName 1CEAddInWebChr$CACHE{MySuffix1}$CACHE{MySuffix2})
else()
    set(LibName 1CEAdnWebChr$CACHE{MySuffix1}$CACHE{MySuffix2})
endif()
find_library(1CEADNWEBChr ${LibName} REQUIRED NO_CACHE)
target_link_libraries(${PROJECT_NAME} ${1CEADNWEBChr})

add_dependencies(${PROJECT_NAME} ${Addn_Native})

if (UNIX)
    if (APPLE)
        #set_target_properties( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ext/Application\ Support/${MYCOMPANY}/${ADN_NAME})
        target_link_libraries(${PROJECT_NAME} dl Threads::Threads ${MAC_LIBRARIES} panel c++ )
    else()
        target_link_libraries(${PROJECT_NAME} dl Threads::Threads ${GTK_LIBS})
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
    endif()
else()
    target_link_libraries(${PROJECT_NAME} RpcRT4.lib)
endif()

set_target_properties( ${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${COMMON_NAME}
   )

if(UNIX AND NOT APPLE)
    set(STRIP_CMD)
    if (STRIP)
        set(STRIP_CMD ${STRIP} $<TARGET_FILE:${PROJECT_NAME}> -f ${PDB_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>.debug)
    endif()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
            COMMAND ${STRIP_CMD}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${TMP_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${TMP_DIR}/
            COMMENT "Copy ${PROJECT_NAME}"
            )
elseif(APPLE)
    if (SIGN_SIGNING)
        set(SIGN_CMD1 ${SIGN_BAT} $<TARGET_FILE_NAME:${PROJECT_NAME}>)
    else()
        set(SIGN_CMD1)
    endif()
    
    set(STRIP_CMD)
    if (STRIP AND STRIP2)
        set(STRIP_CMD ${STRIP} $<TARGET_FILE:${PROJECT_NAME}> -o ${PDB_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>.debug)
        set(STRIP_CMD2 ${STRIP2} -x $<TARGET_FILE:${PROJECT_NAME}>)
    endif()
    
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
            COMMAND ${STRIP_CMD}
            COMMAND ${STRIP_CMD2}
            COMMAND ${SIGN_CMD1}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/ext/Application\ Support/${MYCOMPANY}/${ADN_NAME}/
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/ext/Application\ Support/${MYCOMPANY}/${ADN_NAME}/
            COMMENT "Signing and copy ${PROJECT_NAME}"
            WORKING_DIRECTORY ${EXE_DIR}
            )
elseif(WIN32)
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
endif()

get_target_property(APPNAME ${PROJECT_NAME} OUTPUT_NAME)

if (APPLE)
    set(PKG_NAME "${Addn_Chrome}${OS_NAME}${MySuffix2}"  CACHE STRING "Name for Apple package")
    set(JSON_FF "${Addn_Mainfest}.ff.json")
    set(JSON_CHR "${Addn_Mainfest}.json")
    set(ADAPTER "/Library/Application Support/${MYCOMPANY}/${ADN_NAME}/${APPNAME}")
    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/manifest.json.in 
        ${CMAKE_CURRENT_BINARY_DIR}/chr/Google/Chrome/NativeMessagingHosts/${JSON_CHR} @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/manifest.json.in 
        ${CMAKE_CURRENT_BINARY_DIR}/chromium/Application\ Support/Chromium/NativeMessagingHosts/${JSON_CHR} @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/manifest.ff.json.in 
        ${CMAKE_CURRENT_BINARY_DIR}/ff/Application\ Support/Mozilla/NativeMessagingHosts/${JSON_CHR} @ONLY)

    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/distribution.plist.in ${CMAKE_CURRENT_BINARY_DIR}/distribution.plist @ONLY)

    #copy library
    get_target_property(NATIVE_LIB ${Addn_Native} OUTPUT_NAME)
    set(NATIVE ${CMAKE_SHARED_LIBRARY_PREFIX}${NATIVE_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX})
    
    add_custom_target(copy-lib ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ext/Application\ Support/${MYCOMPANY}/${ADN_NAME}/${NATIVE})
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ext/Application\ Support/${MYCOMPANY}/${ADN_NAME}/${NATIVE}
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${Addn_Native}>
                        ${CMAKE_CURRENT_BINARY_DIR}/ext/Application\ Support/${MYCOMPANY}/${ADN_NAME}/${NATIVE}
                   DEPENDS ${Addn_Native}
                   COMMENT "Copy target $<TARGET_FILE:${Addn_Native}>"
                )

    set(CUR_VER "8.3.0.${Addn_VERSION}")
    add_custom_target( pkgbuild DEPENDS ${PROJECT_NAME} copy-lib
        COMMAND pkgbuild --install-location /Library --version ${CUR_VER} --identifier "$<LOWER_CASE:${Addn_Mainfest}>.ff" --root ${CMAKE_CURRENT_BINARY_DIR}/ff ${CMAKE_CURRENT_BINARY_DIR}/ff.pkg

        COMMAND pkgbuild --install-location /Library --version ${CUR_VER} --identifier "$<LOWER_CASE:${Addn_Mainfest}>.chr" --root ${CMAKE_CURRENT_BINARY_DIR}/chr ${CMAKE_CURRENT_BINARY_DIR}/chrome.pkg

        COMMAND pkgbuild --install-location /Library --version ${CUR_VER} --identifier "$<LOWER_CASE:${Addn_Mainfest}>.chromium" --root ${CMAKE_CURRENT_BINARY_DIR}/chromium ${CMAKE_CURRENT_BINARY_DIR}/chromium.pkg

        COMMAND pkgbuild --install-location /Library --version ${CUR_VER} --identifier "$<LOWER_CASE:${Addn_Mainfest}>.ext" --root ${CMAKE_CURRENT_BINARY_DIR}/ext ${CMAKE_CURRENT_BINARY_DIR}/${ADN_NAME}.pkg
    )
    add_custom_target( productbuild DEPENDS pkgbuild
        COMMAND productbuild --distribution ${CMAKE_CURRENT_BINARY_DIR}/distribution.plist --package-path ${CMAKE_CURRENT_BINARY_DIR}/ff.pkg --package-path ${CMAKE_CURRENT_BINARY_DIR}/chrome.pkg --package-path ${CMAKE_CURRENT_BINARY_DIR}/chromium.pkg --package-path ${CMAKE_CURRENT_BINARY_DIR}/${ADN_NAME}.pkg ${CMAKE_CURRENT_BINARY_DIR}/${PKG_NAME}.pkg
    )
    if (SIGN_SIGNING)
        set(SIGN_CMD1 ${PKG_SIGN} ${PKG_NAME}.pkg)
        add_custom_target( productsign ALL DEPENDS productbuild
            COMMAND ${SIGN_CMD1}
            COMMAND ${NOTARY_BAT} ${PKG_NAME}.pkg ${PKG_ID}
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${PKG_NAME}.pkg ${PKG_OUTPUT_DIR}/${PKG_NAME}.pkg
            COMMENT "Notarizing ${PKG_NAME}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()
    add_custom_target( zipdebug DEPENDS ${PROGNAME}
        COMMAND dsymutil -o ${PROJECT_NAME}.dSYM $<TARGET_FILE:${PROJECT_NAME}>
        COMMAND zip -r ${PDB_DIR}/${PROJECT_NAME}_${Addn_VERSION}.zip ${PROJECT_NAME}.dSYM
    )
endif()

if(COMP_EXTERNAL AND APPLE)
    if(WIN32)
        set(EXES ${EXESUFFIX})
        set(_name "${PROJECT_NAME}${MySuffix2}")
    elseif(UNIX)
        set(_name "{PROJECT_NAME}${MySuffix1}${MySuffix2}${MySuffix2Append}")
        set(EXES ".sh")
    endif()
    set(pattern [=[    <component os="@OS_NAME@" path="@PKG_NAME@.pkg" type="plugin" object="@Addn_Mainfest@" arch="@OS_ARCH@" client="Firefox" clientVersion="40.*"/> 
    <component os="@OS_NAME@" path="@PKG_NAME@.pkg" type="plugin" object="@Addn_Mainfest@" arch="@OS_ARCH@" client="Chrome" />
    <component os="@OS_NAME@" path="@PKG_NAME@.pkg" type="plugin" object="@Addn_Mainfest@" arch="@OS_ARCH@" client="AnyChromiumBased" />
]=])
    
    string(CONFIGURE ${pattern} _addin_str ESCAPE_QUOTES)
    set(manifest_begin "${manifest_begin}${_addin_str}" PARENT_SCOPE)
    unset(_addin_str)
    unset(pattern)
endif()
