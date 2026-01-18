cmake_minimum_required(VERSION 3.18...3.31)

project(localization NONE)

file(GLOB rcb_files LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS *.rcb)

set(COMMON_NAME "common.cxx")
set(ALL_FILES)

set(all_list_to_cpp)
list(APPEND all_list_to_cpp "static const TextMap stLocalizedText =")
list(APPEND all_list_to_cpp "{")
foreach(rcb_file ${rcb_files})
    file(STRINGS ${rcb_file} rcb_content ENCODING UTF-8)
    set(_first_line TRUE)
    set(lang)
    set(list_to_rc)
    set(list_to_cpp)

    foreach(rcb_line ${rcb_content})
        string(STRIP ${rcb_line} line_strip)
        string(REGEX MATCH "^[{}/]" _comment ${line_strip})
        if(_comment)
            continue()
        endif()
        
        if(_first_line)
            set(lang ${line_strip})
            set(_first_line FALSE)
        endif()
        
        string(REGEX MATCH "(.+):[string ]*{(.+)}" _content ${line_strip})
        
        if (${CMAKE_MATCH_COUNT})
            list(APPEND list_to_cpp "\t\t{ ${CMAKE_MATCH_1},\t u${CMAKE_MATCH_2} },")
        endif()
        
    endforeach()

    list(PREPEND list_to_cpp "\t{ u\"${lang}\", {" )
    list(APPEND list_to_cpp "\t}, }," )

    list(JOIN list_to_cpp "\n" list_to_cpp)
    list(APPEND all_list_to_cpp "${list_to_cpp}" )
endforeach()

list(APPEND all_list_to_cpp "}")
list(JOIN all_list_to_cpp "\n" all_to_cpp)
string(APPEND all_to_cpp ";")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${COMMON_NAME} ${all_to_cpp})
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${COMMON_NAME} ";")
list(APPEND ALL_FILES "${CMAKE_CURRENT_BINARY_DIR}/${COMMON_NAME}")

add_custom_target(${PROJECT_NAME} ALL 
    COMMENT "Generating include files from rcb"
    DEPENDS en.rcb
    SOURCES "${ALL_FILES}"
)
