# simdutf

CPMAddPackage(
    NAME simdutf
    VERSION 5.6.2
    URL https://github.com/simdutf/simdutf/releases/download/v5.6.2/singleheader.zip
    URL_HASH SHA256=B03B42EDB8A872E90329686BB3A73BE916A0A111157CCFA1B361224E1DFABF17
    DOWNLOAD_ONLY YES
)

if (NOT simdutf_ADDED)
    message(FATAL_ERROR "simdutf is requied")
endif ()

set(_root ${simdutf_SOURCE_DIR})

add_library(simdutf STATIC)
target_compile_features(simdutf PRIVATE c_std_17 cxx_std_20)
target_include_directories(simdutf PUBLIC ${_root})
target_sources(simdutf PRIVATE ${_root}/simdutf.h ${_root}/simdutf.cpp)

set_target_properties(simdutf PROPERTIES FOLDER external)
