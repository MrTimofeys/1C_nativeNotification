# Cmake script for project AddInIESetup
# autor: 1C

cmake_minimum_required(VERSION 3.18...3.31 FATAL_ERROR)

if(NOT DEFINED AddInChrBuild)
    return()
endif()

project(${Addn_ChromeSetup})

if (WIN32)
    find_program(NSIS makensis)

    if(NOT NSIS)
        message(WARNING "NSIS not found.")
        return()
    endif()

    if (NOT TARGET_PLATFORM_32)
        set(ADD_SUFFIX "x64")
    else()
        set(ADD_SUFFIX "x86")
    endif()
    set(ADD_BITS ${MySuffix2})
    
    set(JSON_FF "${Addn_Mainfest}.ff.json")
    set(JSON_CHR "${Addn_Mainfest}.json")
    set(ADAPTER ${Addn_Chrome}${MySuffix1}${MySuffix2}${EXESUFFIX})
    if(DEFINED FROM_ZIP)
        set(EXCOMP ${Addn_Native}${LIBSUFFIX})
    else()
        set(EXCOMP ${Addn_Native}${MySuffix1}${MySuffix2}_${Addn_VERSION}${LIBSUFFIX})
    endif()
    set(PRODUCT "AddInChrome")
    set(OUTFILENAME ${PROJECT_NAME})

    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/manifest.ff.json.in ${CMAKE_CURRENT_BINARY_DIR}/${JSON_FF} @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/manifest.json.in ${CMAKE_CURRENT_BINARY_DIR}/${JSON_CHR} @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/AddInChromeSetup.nsh.in ${CMAKE_CURRENT_BINARY_DIR}/AddInChromeSetup.nsh @ONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AddInChromeSetup.nsi ${CMAKE_CURRENT_BINARY_DIR}/AddInChromeSetup.nsi COPYONLY)

    if (SIGN_BAT)
        set(SIGN_CMD1 ${SIGN_BAT} ${PROJECT_NAME}${MySuffix2}${EXESUFFIX})
        set(SIGN_CMD2 )
    elseif(SIGN_TOOL AND EXISTS ${SIGN_CERT})
        set(SIGN_CMD1 ${SIGN_TOOL} ${SIGN_SHA1} ${PROJECT_NAME}${MySuffix2}${EXESUFFIX})
        set(SIGN_CMD2 ${SIGN_TOOL} ${SIGN_SHA256} ${PROJECT_NAME}${MySuffix2}${EXESUFFIX})
    else()
        set(SIGN_CMD1)
        set(SIGN_CMD2)
    endif()
    
    add_custom_target(${PROJECT_NAME} ALL
                COMMAND ${NSIS} AddInChromeSetup.nsi 
                COMMAND ${SIGN_CMD1}
                COMMAND ${SIGN_CMD2}
                COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${MySuffix2}${EXESUFFIX} ${PKG_OUTPUT_DIR}/${PROJECT_NAME}${MySuffix2}${EXESUFFIX}
                DEPENDS ${JSON_FF} ${JSON_CHR}
                COMMENT "Build setup for Google Chrome and Mozilla Firefox..."
                SOURCES AddInChromeSetup.nsi AddInChromeSetup.nsh
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                )
    add_dependencies(${PROJECT_NAME} ${Addn_Chrome} ${Addn_Native})
elseif(UNIX AND NOT APPLE)
    #create makeself 
    FIND_PROGRAM(MKSLF NAMES makeself.sh makeself HINTS ENV PATH)

    if(NOT MKSLF)
        message(WARNING "makeself not found!")
        return()
    ENDIF()
    
    set(ADAPTER "HOST_PATH")
    set(JSON_FF "${Addn_Mainfest}.ff.json")
    set(JSON_CHR "${Addn_Mainfest}.json")

    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/install_host.sh.in ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.sh @ONLY)
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.19)
        file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.sh 
            FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endif()
    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/manifest.ff.json.in ${CMAKE_CURRENT_BINARY_DIR}/${JSON_FF} @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/conf_data_in/manifest.json.in ${CMAKE_CURRENT_BINARY_DIR}/${JSON_CHR} @ONLY)
    
    add_custom_target( copy_json DEPENDS ${Addn_Chrome} ${Addn_Native}
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${JSON_FF} ${TMP_DIR}/${JSON_FF}
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${JSON_CHR} ${TMP_DIR}/${JSON_CHR}
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.sh ${TMP_DIR}/${PROJECT_NAME}.sh
            COMMENT "Copy json files"
        )

    add_custom_target(${PROJECT_NAME} ALL COMMAND ${MKSLF} --nox11 
                ${TMP_DIR}/ #archive dir
                ${PKG_OUTPUT_DIR}/${PROJECT_NAME}${MySuffix1}${MySuffix2}${MySuffix2Append}.sh #file name
                "${PROJECT_NAME}" #label
                ./${PROJECT_NAME}.sh #startup script
                DEPENDS copy_json
                WORKING_DIRECTORY ${TMP_DIR}
                )
    add_dependencies(${PROJECT_NAME} ${Addn_Chrome} ${Addn_Native})
endif()

if(COMP_EXTERNAL AND NOT APPLE)
    if(WIN32)
        set(EXES ${EXESUFFIX})
        set(_name "${PROJECT_NAME}${MySuffix2}")
    elseif(UNIX)
        set(_name "{PROJECT_NAME}${MySuffix1}${MySuffix2}${MySuffix2Append}")
        set(EXES ".sh")
    endif()
    set(pattern [=[    <component os="@OS_NAME@" path="@_name@@EXES@" type="plugin" object="@Addn_Mainfest@" arch="@OS_ARCH@" client="Firefox" clientVersion="40.*"/> 
    <component os="@OS_NAME@" path="@_name@@EXES@" type="plugin" object="@Addn_Mainfest@" arch="@OS_ARCH@" client="Chrome" />
    <component os="@OS_NAME@" path="@_name@@EXES@" type="plugin" object="@Addn_Mainfest@" arch="@OS_ARCH@" client="AnyChromiumBased" />
]=])
    
    string(CONFIGURE ${pattern} _addin_str ESCAPE_QUOTES)
    set(manifest_begin "${manifest_begin}${_addin_str}" PARENT_SCOPE)
    unset(_addin_str)
    unset(pattern)
endif()
