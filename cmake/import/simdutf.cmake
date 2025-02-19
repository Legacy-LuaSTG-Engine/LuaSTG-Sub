# simdutf

CPMAddPackage(
    NAME simdutf
    VERSION 6.2.0
    URL https://github.com/simdutf/simdutf/releases/download/v6.2.0/singleheader.zip
    URL_HASH SHA256=66C85F591133E3BAA23CC441D6E2400DD2C94C4902820734DDBCD9E04DD3988B
    DOWNLOAD_ONLY YES
)

if (NOT simdutf_ADDED)
    message(FATAL_ERROR "simdutf is requied")
endif ()

set(_root ${simdutf_SOURCE_DIR})

add_library(simdutf STATIC)
add_library(simdutf::simdutf ALIAS simdutf)
target_compile_features(simdutf PRIVATE c_std_17 cxx_std_20)
target_include_directories(simdutf PUBLIC ${_root})
target_sources(simdutf PRIVATE ${_root}/simdutf.h ${_root}/simdutf.cpp)

set_target_properties(simdutf PROPERTIES FOLDER external)
