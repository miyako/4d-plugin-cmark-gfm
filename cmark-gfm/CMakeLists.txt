cmake_minimum_required(VERSION 3.20)

set(PLUGIN_NAME "cmark-gfm" CACHE STRING "Plugin name")
project(${PLUGIN_NAME} LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../VERSION" PLUGIN_VERSION)
string(STRIP "${PLUGIN_VERSION}" PLUGIN_VERSION)
if(NOT PLUGIN_VERSION MATCHES "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)$")
    message(FATAL_ERROR "VERSION must be a numeric semantic version")
endif()
set(SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../4D-Plugin-SDK/4D Plugin API")
set(CMARK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cmark-gfm-src")
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# --- cmark-gfm: compiled into the plugin from the pinned cmark-gfm-src submodule -----------
#
# Upstream's own CMake project is not used: it declares cmake_minimum_required(VERSION 3.0),
# which CMake 4 rejects, and its old policy level ignores CMAKE_MSVC_RUNTIME_LIBRARY, so the
# Windows objects would use the dynamic CRT. The generated headers it would write are written
# here instead, for a static build.

file(STRINGS "${CMARK_DIR}/CMakeLists.txt" _cmark_version_lines REGEX "^set\\(PROJECT_VERSION_(MAJOR|MINOR|PATCH|GFM) ")
foreach(_line IN LISTS _cmark_version_lines)
    string(REGEX MATCH "PROJECT_VERSION_([A-Z]+) ([0-9]+)" _m "${_line}")
    set(PROJECT_VERSION_${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
endforeach()
if(NOT DEFINED PROJECT_VERSION_GFM)
    message(FATAL_ERROR "Could not read the cmark-gfm version from ${CMARK_DIR}/CMakeLists.txt")
endif()
set(CMARK_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/cmark-gfm-generated")
set(HAVE_STDBOOL_H 1)
if(NOT MSVC)
    set(HAVE___BUILTIN_EXPECT 1)
    set(HAVE___ATTRIBUTE__ 1)
endif()
configure_file("${CMARK_DIR}/src/config.h.in" "${CMARK_GEN_DIR}/config.h")
configure_file("${CMARK_DIR}/src/cmark-gfm_version.h.in" "${CMARK_GEN_DIR}/cmark-gfm_version.h")
file(WRITE "${CMARK_GEN_DIR}/cmark-gfm_export.h.tmp"
"#ifndef CMARK_GFM_EXPORT_H\n#define CMARK_GFM_EXPORT_H\n/* static build: nothing is exported */\n#define CMARK_GFM_EXPORT\n#define CMARK_GFM_NO_EXPORT\n#endif\n")
configure_file("${CMARK_GEN_DIR}/cmark-gfm_export.h.tmp" "${CMARK_GEN_DIR}/cmark-gfm_export.h" COPYONLY)

set(CMARK_SOURCES)
foreach(_f cmark node iterator blocks inlines scanners utf8 buffer references footnotes map
        render man xml html commonmark plaintext latex houdini_href_e houdini_html_e
        houdini_html_u cmark_ctype arena linked_list syntax_extension registry plugin)
    list(APPEND CMARK_SOURCES "${CMARK_DIR}/src/${_f}.c")
endforeach()
foreach(_f core-extensions table strikethrough autolink tagfilter ext_scanners tasklist)
    list(APPEND CMARK_SOURCES "${CMARK_DIR}/extensions/${_f}.c")
endforeach()
add_library(cmark_gfm_static STATIC ${CMARK_SOURCES})
target_include_directories(cmark_gfm_static PUBLIC
    "${CMARK_DIR}/src" "${CMARK_DIR}/extensions" "${CMARK_GEN_DIR}")
target_compile_definitions(cmark_gfm_static PUBLIC CMARK_GFM_STATIC_DEFINE CMARK_GFM_EXTENSIONS_STATIC_DEFINE)
set_target_properties(cmark_gfm_static PROPERTIES POSITION_INDEPENDENT_CODE ON C_VISIBILITY_PRESET hidden)
if(MSVC)
    target_compile_definitions(cmark_gfm_static PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(cmark_gfm_static PRIVATE /wd4244 /wd4267 /wd4018 /wd4996)
else()
    target_compile_options(cmark_gfm_static PRIVATE -Wno-unused-parameter -Wno-sign-compare)
endif()

# --- plugin --------------------------------------------------------------------------------

set(PLUGIN_SOURCES ${PLUGIN_NAME}-4dplugin.cpp "${SDK_DIR}/4DPluginAPI.c")
set(PLUGIN_HEADERS ${PLUGIN_NAME}-4dplugin.h 4d-helpers.h
    "${SDK_DIR}/4DPluginAPI.h" "${SDK_DIR}/EntryPoints.h"
    "${SDK_DIR}/PublicTypes.h" "${SDK_DIR}/PrivateTypes.h" "${SDK_DIR}/Flags.h")
set(PLUGIN_RESOURCES manifest.json constants.xlf)

if(APPLE)
    add_library(${PLUGIN_NAME} MODULE ${PLUGIN_SOURCES} ${PLUGIN_HEADERS} ${PLUGIN_RESOURCES})
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        BUNDLE TRUE
        BUNDLE_EXTENSION bundle
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.4d.${PLUGIN_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PLUGIN_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PLUGIN_VERSION}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}-test/Plugins"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}-test/Plugins"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}-test/Plugins")
    set_source_files_properties(${PLUGIN_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
    target_link_libraries(${PLUGIN_NAME} PRIVATE "${COREGRAPHICS_FRAMEWORK}" cmark_gfm_static)
    set_source_files_properties("${SDK_DIR}/4DPluginAPI.c" PROPERTIES
        COMPILE_FLAGS "-Wno-int-conversion -Wno-multichar")
elseif(WIN32)
    add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})
    set(PLUGIN_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}-test/Plugins/${PLUGIN_NAME}/Contents/Windows64")
    set(RESOURCE_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}-test/Plugins/${PLUGIN_NAME}/Contents/Resources")
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        SUFFIX ".4DX"
        LINK_FLAGS "/DEF:\"${SDK_DIR}/4DPluginAPI.def\""
        RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PLUGIN_OUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PLUGIN_OUT_DIR}")
    target_link_libraries(${PLUGIN_NAME} PRIVATE cmark_gfm_static)
    add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${RESOURCE_OUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
            "${CMAKE_CURRENT_SOURCE_DIR}/constants.xlf" "${RESOURCE_OUT_DIR}")
else()
    message(FATAL_ERROR "Only macOS and Windows plugins are supported")
endif()

target_include_directories(${PLUGIN_NAME} PRIVATE "${SDK_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
set_source_files_properties("${SDK_DIR}/4DPluginAPI.c" PROPERTIES LANGUAGE CXX)
target_compile_definitions(${PLUGIN_NAME} PRIVATE UNICODE _UNICODE)
if(WIN32)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()
