# simdutf

CPMAddPackage(
    NAME simdutf
    VERSION 5.7.2
    URL https://github.com/simdutf/simdutf/releases/download/v5.7.2/singleheader.zip
    URL_HASH SHA256=38501738470DA83677C74F8690700F070074689BD26EA2D5D4F3386DD48BA5C4
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
