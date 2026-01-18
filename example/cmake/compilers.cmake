if (WIN32 AND MSVC)
#-----------------------------------------------------------------------------
# Minimum compiler version check: Microsoft C/C++ >= 18.0 (aka VS 2013 aka VS 12.0)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0)
        message(FATAL_ERROR "Only Microsoft Visual Studio 2017 or later is currently supported.")
    endif ()
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.15.0)
        cmake_policy(SET CMP0091 NEW)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"  CACHE STRING "Runtime lib" FORCE)
    else()
        foreach(flag
           CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
           CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(${flag} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
                set(${flag} ${${flag}} CACHE "STRING" "Compilation flags" FORCE)
            endif()
        endforeach()
    endif()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /FS")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif(WIN32 AND MSVC)

set(VS_TOOLSET_FOR_BROWSERS_REQUIRED "v141_xp")
#-----------------------------------------------------------------------------
# Minimum compiler version check: GCC >= 4.8
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
  message(FATAL_ERROR "GCC 4.8 or later is required.")
endif ()

if(UNIX AND NOT APPLE)
    set(CMAKE_LINK_WHAT_YOU_USE ON)
endif()
set(CMAKE_SKIP_RPATH ON CACHE BOOL "")

set(_target OFF)
if (WIN32)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8 OR "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "x64")
        set(_target OFF)
    else()
        set(_target ON)
    endif()
else()
    set(_target OFF)
endif()
option(TARGET_PLATFORM_32 "Build target for i386" ${_target})
mark_as_advanced(TARGET_PLATFORM_32)

set(ARCH "${CMAKE_SYSTEM_PROCESSOR}" CACHE STRING "SYSTEM PROCESSOR" FORCE)

if (ARCH MATCHES aarch64|arm64)
    set(MySuffix2 "ARM64" CACHE STRING "Arch" FORCE)
    set(OS_ARCH "${MySuffix2}" CACHE STRING "Arch OS" FORCE)
    SET(ARCH_FLAG "")
elseif (ARCH STREQUAL e2k)
    set(MySuffix2 "E2K" CACHE STRING "Arch" FORCE) # short suffix
    set(OS_ARCH "${MySuffix2}" CACHE STRING "Arch OS" FORCE)
    set(MySuffix2Append "-8C"  CACHE STRING "Arch suffix" FORCE) # full suffix
    SET(ARCH_FLAG "")
elseif (TARGET_PLATFORM_32)
    set(MySuffix2 "32" CACHE STRING "Arch" FORCE)
    set(OS_ARCH "i386" CACHE STRING "Arch OS" FORCE)
    if (UNIX)
        SET(ARCH_FLAG "-m32")
    endif()
else()
    set(MySuffix2 "64" CACHE STRING "Arch" FORCE)
    set(OS_ARCH "x86_64" CACHE STRING "Arch OS" FORCE)
    if (UNIX)
        SET(ARCH_FLAG "-m64")
    endif()
endif()

if (UNIX AND NOT APPLE AND NOT ARCH STREQUAL aarch64)
    foreach(flag
        CMAKE_EXE_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS CMAKE_C_FLAGS CMAKE_CXX_FLAGS )
        if(NOT "${flag}" MATCHES "${ARCH_FLAG}")
            set(${flag} ${${flag}} ${ARCH_FLAG} CACHE "STRING" "Linker flags" FORCE)
        endif()
    endforeach()
endif()

