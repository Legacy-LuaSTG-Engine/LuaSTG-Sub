# prepare cmake external build install directories

function(luastg_cmake_external_build_prepare_directories dir)
    list(LENGTH CMAKE_CONFIGURATION_TYPES config_count)
    if (config_count GREATER_EQUAL 1)
        #message(STATUS "luastg_cmake_external_build_prepare_directories -- multi-config mode (${config_count} configs)")
        foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
            #message(STATUS "luastg_cmake_external_build_prepare_directories -- -- ${config}")
            file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/install/${config}/${dir})
        endforeach()
    else ()
        #message(STATUS "luastg_cmake_external_build_prepare_directories -- single-config mode")
        #message(STATUS "luastg_cmake_external_build_prepare_directories -- -- ${CMAKE_BUILD_TYPE}")
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/install/${CMAKE_BUILD_TYPE}/${dir})
    endif ()
endfunction()

luastg_cmake_external_build_prepare_directories(include)

# collect cmake external build generator options

set(_options -G ${CMAKE_GENERATOR})
if (DEFINED CMAKE_GENERATOR_PLATFORM AND (NOT (CMAKE_GENERATOR_PLATFORM STREQUAL "")))
    set(_options ${_options} -A ${CMAKE_GENERATOR_PLATFORM})
endif ()
if (DEFINED CMAKE_GENERATOR_TOOLSET AND (NOT (CMAKE_GENERATOR_TOOLSET STREQUAL "")))
    set(_options ${_options} -T ${CMAKE_GENERATOR_TOOLSET})
endif ()
if (DEFINED CMAKE_C_COMPILER AND (NOT (CMAKE_C_COMPILER STREQUAL "")))
    set(_options ${_options} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER})
endif ()
if (DEFINED CMAKE_CXX_COMPILER AND (NOT (CMAKE_CXX_COMPILER STREQUAL "")))
    set(_options ${_options} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER})
endif ()
set(LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS ${_options})
unset(_options)

# libraries

set(LUASTG_CMAKE_SUB_BUILD_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/zlib_ng.cmake)
include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/minizip_ng.cmake)
include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/libogg.cmake)
include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/libvorbis.cmake)
include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/libflac.cmake)
include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/freetype.cmake)
if (LUASTG_IMAGE_PNG_ENABLE)
    include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/png.cmake)
endif ()
if (LUASTG_IMAGE_WEBP_ENABLE)
    include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/webp.cmake)
endif ()
include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/directx_tex.cmake)
include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/gtest.cmake)

#include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/harfbuzz.cmake)
