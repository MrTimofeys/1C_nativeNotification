# Cmake script for project AddInNative
# autor: 1C

cmake_minimum_required(VERSION 3.18...3.31 FATAL_ERROR)

#----------------------------------------------------------------------------------------------------------------#
include(${CMAKE_CURRENT_SOURCE_DIR}/sources.cmake)
set(AddInNative_SRC ${SRC})
#----------------------------------------------------------------------------------------------------------------#
include_directories(${CMAKE_SOURCE_DIR}/../include)
#----------------------------------------------------------------------------------------------------------------#
if(NOT TARGET ${Addn_Native})
    project(${Addn_Native} CXX)
    add_subdirectory(localization)
    if(WIN32)
        configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/AddInNative.rc.in ${CMAKE_CURRENT_BINARY_DIR}/AddInNative.rc)
        configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/AddInNative.def.in ${CMAKE_CURRENT_BINARY_DIR}/AddInNative.def)
        enable_language(RC)
    endif()

    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_SOURCE_DIR})
    set(AddInDef_SRC)

    if (UNIX)
        if (APPLE)
            add_definitions(-D__APPLE__ )
        endif()
    else()
        add_definitions(-DUNICODE -DWIN32 -D_USRDLL -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -DADDINCPP_EXPORTS)
        if(TARGET_PLATFORM_32)
            add_definitions(-D_USE_32BIT_TIME_T )
        endif()
    endif()
    add_library(${PROJECT_NAME} SHARED ${AddInNative_SRC})
    target_compile_features(${PROJECT_NAME} PUBLIC cxx_nullptr cxx_override cxx_unicode_literals)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    if (WIN32)
        target_precompile_headers(${PROJECT_NAME} PUBLIC stdafx.h)
    endif()

    set_target_properties( ${PROJECT_NAME} PROPERTIES
           OUTPUT_NAME ${PROJECT_NAME}${MySuffix1}${MySuffix2}${MySuffix2Append}_${Addn_VERSION}
           POSITION_INDEPENDENT_CODE ON 
           CXX_STANDARD 11
           CXX_STANDARD_REQUIRED ON
       )
    if (UNIX AND NOT APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,--no-undefined -Xlinker --version-script -Xlinker ${CMAKE_CURRENT_SOURCE_DIR}/version.script" )
    endif()

endif()
if(APPLE)
    set(BNDL ${PROJECT_NAME}Bndl)

    add_library(${BNDL} MODULE ${AddInNative_SRC})
    target_include_directories(${BNDL} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    set_target_properties(${BNDL} PROPERTIES BUNDLE TRUE)
    set_target_properties( ${BNDL} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")

    set(MACOSX_BUNDLE_COPYRIGHT "Copyright ${YEAR} ${MYCOMPANY}.")
    TARGET_LINK_LIBRARIES(${BNDL} "-framework CoreFoundation")
    set_target_properties(${BNDL} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist.in)
    
    #Safari pkg
    set(PKG_NAME "${Addn_Safari}${OS_NAME}${MySuffix2}" CACHE STRING "Name for Safari package")
    get_target_property(BUNDLE_NAME ${BNDL} OUTPUT_NAME)

    set(MIMETYPE "${Addn_MimeType}")
    file(COPY ${CMAKE_SOURCE_DIR}/../lib/1CEAdnWebNPAPISafOSX.bundle DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/1CEAdnWebNPAPISafOSX.bundle/Contents/Info.plist)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/1CEAdnWebNPAPISafOSX.bundle/Contents/PlugIns)
    
    #FIXME Get BUNDLE full name
    add_custom_command(TARGET ${BNDL} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIB_DIR}/${BUNDLE_NAME}.bundle ${CMAKE_CURRENT_BINARY_DIR}/1CEAdnWebNPAPISafOSX.bundle/Contents/PlugIns/${BUNDLE_NAME}.bundle
        COMMENT "Copy ${BNDL}"
    )
    add_custom_target(saf_productbuild ALL DEPENDS ${BNDL}
        COMMAND productbuild --component ${CMAKE_CURRENT_BINARY_DIR}/1CEAdnWebNPAPISafOSX.bundle "/Library/Internet Plug-Ins/" ${CMAKE_CURRENT_BINARY_DIR}/${PKG_NAME}.pkg
        COMMENT "Create package for Safari"
    )
    add_custom_target( saf_productsign ALL DEPENDS saf_productbuild
        COMMAND productsign --sign "Developer ID Installer: ${SIGNCERT}"  ${CMAKE_CURRENT_BINARY_DIR}/${PKG_NAME}.pkg ${PKG_OUTPUT_DIR}/${PKG_NAME}.pkg
        COMMENT "Signing package for Safari"
    )
    if(COMP_EXTERNAL)
        set(pattern [=[    <component os="@OS_NAME@" path="@PKG_NAME@.pkg" type="plugin" object="@Addn_MimeType@" arch="@OS_ARCH@" client="Safari" />
]=])
        string(CONFIGURE ${pattern} _addin_str ESCAPE_QUOTES)
        set(manifest_begin "${manifest_begin}${_addin_str}" PARENT_SCOPE)
    endif()
endif()

if(NOT COMP_EXTERNAL)
    if(UNIX AND NOT APPLE)
        set(STRIP_CMD)
        if (STRIP)
            set(STRIP_CMD ${STRIP} $<TARGET_FILE:${PROJECT_NAME}> -f ${PDB_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>.debug)
        endif()
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
                COMMAND ${STRIP_CMD}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${TMP_DIR}
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${TMP_DIR}/
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${PKG_OUTPUT_DIR}/
                COMMENT "Copy ${PROJECT_NAME}"
                MAIN_DEPENDENCY ${PROJECT_NAME}
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
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${PKG_OUTPUT_DIR}/
                COMMENT "Signing and copy ${PROJECT_NAME}"
                WORKING_DIRECTORY ${LIB_DIR}
                )
    elseif(WIN32)
        if (SIGN_BAT)
            set(SIGN_CMD1 ${SIGN_BAT} $<TARGET_FILE:${PROJECT_NAME}>)
            set(SIGN_CMD2 )
        else()
            set(SIGN_CMD1)
            set(SIGN_CMD2)
        endif()
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
                COMMAND ${SIGN_CMD1}
                COMMAND ${SIGN_CMD2}
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${PKG_OUTPUT_DIR}/
                COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:${PROJECT_NAME}> ${LIB_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>
                COMMENT "Copy ${PROJECT_NAME}"
                )
    endif()
endif()
