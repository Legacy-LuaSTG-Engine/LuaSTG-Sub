# prepare cmake external build install directories

file(WRITE ${CMAKE_BINARY_DIR}/install/Debug/include/placeholder          "")
file(WRITE ${CMAKE_BINARY_DIR}/install/Release/include/placeholder        "")
file(WRITE ${CMAKE_BINARY_DIR}/install/RelWithDebInfo/include/placeholder "")
file(WRITE ${CMAKE_BINARY_DIR}/install/MinSizeRel/include/placeholder     "")

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

include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/z.cmake)
include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/xiph.cmake)
include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/freetype.cmake)
#include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/harfbuzz.cmake)
include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/webp.cmake)
include(${LUASTG_CMAKE_SUB_BUILD_ROOT}/gtest.cmake)
