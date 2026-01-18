
function (get_component_type full_path_component out_var)
    try_run(RET_RUN RET_COMPILE 
        SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/main.cxx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
        ARGS ${full_path_component}
        )
        set(${out_var} ${RET_RUN} PARENT_SCOPE)
endfunction()

if (SIGN_SIGNING)
    if(APPLE)
        SET(SIGN_CERT "MyCertificate, OOO") #certificate name
        find_program(SIGN_BAT NAMES sign.sh HINTS ENV PATH)
        find_program(NOTARY_BAT NAMES notary.sh HINTS ENV PATH)
        find_program(PKG_SIGN NAMES pkgsign.sh HINTS ENV PATH)
    elseif(WIN32)
        find_program(SIGN_BAT NAMES sign.bat HINTS ENV PATH)
        #example sign.bat
        #signtool sign /f certificate.pfx /p password /t http://timestamp.verisign.com/scripts/timstamp.dll /v %1
        #signtool sign /fd SHA256 /f certificate.pfx /p password /tr http://sha256timestamp.ws.symantec.com/sha256/timestamp /td SHA256 /as /v %1
        if (SIGN_BAT-NOTFOUND)
            find_program(SIGN_TOOL NAMES signtool.exe HINTS ENV PATH)

            if (SIGN_TOOL)
                SET(SIGN_CERT "my.pfx" CACHE FILEPATH "Certificate path" FORCE) #certificate path
                SET(SIGN_CERT_PWD "password" CACHE STRING "Password for certificate" FORCE)
                SET(SIGN_URL_SHA1 "http://timestamp.verisign.com/scripts/timstamp.dll" CACHE STRING "SHA1 timestamp url" FORCE)
                SET(SIGN_URL_SHA256 "http://sha256timestamp.ws.symantec.com/sha256/timestamp" CACHE STRING "SHA256 timestamp url" FORCE)

                SET(SIGN_SHA1 "${SIGN_TOOL}" sign /f "${SIGN_CERT}" /p "${SIGN_CERT_PWD}" /t ${SIGN_URL_SHA1} /v) 
                SET(SIGN_SHA256 "${SIGN_TOOL}" sign  sign /fd SHA256 /f "${SIGN_CERT}" /p "${SIGN_CERT_PWD}" /tr ${SIGN_URL_SHA256} /td SHA256 /as /v)
                if (NOT EXISTS ${SIGN_CERT})
                    message(WARNING "Certificate file not found. Disabled.")
                    set(SIGN_SIGNING OFF)
                endif()
            else()
                message(WARNING "Signing tools not found. Disabled.")
                set(SIGN_SIGNING OFF)
            endif()
        endif()
    endif(APPLE)
endif()