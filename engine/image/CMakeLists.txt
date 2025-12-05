# Image

set(lib_name "Core.Image")

file(GLOB_RECURSE
    lib_src
    RELATIVE ${CMAKE_CURRENT_LIST_DIR}
    core/*.hpp
    core/*.cpp
    backend/*.hpp
    backend/*.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${lib_src})

add_library(${lib_name})
luastg_target_common_options(${lib_name})
luastg_target_more_warning(${lib_name})
target_include_directories(${lib_name} PUBLIC .)
target_sources(${lib_name} PRIVATE ${lib_src})
target_link_libraries(${lib_name} PRIVATE options_compile_utf8)
target_link_libraries(${lib_name} PUBLIC
    Core.ReferenceCounted
    Core.Logging
    Core.FileSystem
    Core.Math
    Microsoft::DirectXTex
    libqoi
    win32
)
if (LUASTG_IMAGE_JPEG_ENABLE)
    target_compile_definitions(${lib_name} PUBLIC LUASTG_IMAGE_JPEG_ENABLE)
    target_link_libraries(${lib_name} PUBLIC libjpeg-turbo::turbojpeg-static)
endif ()
if (LUASTG_IMAGE_PNG_ENABLE)
    target_compile_definitions(${lib_name} PUBLIC LUASTG_IMAGE_PNG_ENABLE)
    target_link_libraries(${lib_name} PUBLIC PNG::png_static)
endif ()
if (LUASTG_IMAGE_WEBP_ENABLE)
    target_compile_definitions(${lib_name} PUBLIC LUASTG_IMAGE_WEBP_ENABLE)
    target_link_libraries(${lib_name} PUBLIC WebP::webp)
endif ()
if (LUASTG_IMAGE_WINDOWS_IMAGING_COMPONENT_ENABLE)
    target_compile_definitions(${lib_name} PUBLIC LUASTG_IMAGE_WINDOWS_IMAGING_COMPONENT_ENABLE)
    target_link_libraries(${lib_name} PUBLIC windowscodecs.lib)
endif ()

set_target_properties(${lib_name} PROPERTIES FOLDER engine)

# Image Test

set(test_name "Core.Image.Test")

add_executable(${test_name})
luastg_target_common_options(${test_name})
luastg_target_more_warning(${test_name})
target_compile_features(${test_name} PRIVATE cxx_std_23)
target_sources(${test_name} PRIVATE test/test.cpp)
target_link_libraries(${test_name} PRIVATE options_compile_utf8 ${lib_name} GTest::gtest_main)

set_target_properties(${test_name} PROPERTIES FOLDER engine/test)
