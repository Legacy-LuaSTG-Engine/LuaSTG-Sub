# CPM.cmake
# 建议给系统或用户设置一个环境变量 CPM_SOURCE_CACHE 指向一个目录
# 可以让多个项目共享包缓存

function(CPMInitialize)
    set(CPM_DOWNLOAD_VERSION 0.38.0)

    if(CPM_SOURCE_CACHE)
        set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    elseif(DEFINED ENV{CPM_SOURCE_CACHE})
        set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    else()
        set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    endif()

    if(NOT(EXISTS ${CPM_DOWNLOAD_LOCATION}))
        message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
        file(DOWNLOAD
            https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
            ${CPM_DOWNLOAD_LOCATION}
        )
    endif()

    include(${CPM_DOWNLOAD_LOCATION})
endfunction()

CPMInitialize()

# nlohmann json
# 读取和输出 json 文件

CPMAddPackage(
    NAME nlohmann_json
    VERSION 3.11.2
    GITHUB_REPOSITORY nlohmann/json
    OPTIONS
    "JSON_BuildTests OFF"
)

# gabime spdlog
# 打印日志到文件和调试器

CPMAddPackage(
    NAME spdlog
    #VERSION 1.11.0
    #GITHUB_REPOSITORY gabime/spdlog
    GITHUB_REPOSITORY Demonese/spdlog
    GIT_TAG f2ec878bc31f3935e5caf45b76d3a81af9a3f5fe
    OPTIONS
    "SPDLOG_WCHAR_FILENAMES ON"
    "SPDLOG_MSVC_SINK_UTF8_SUPPORT ON"
    "SPDLOG_DISABLE_DEFAULT_LOGGER ON"
)

if(TARGET spdlog)
    if(MSVC)
        target_compile_options(spdlog PUBLIC
            "/DSPDLOG_SHORT_LEVEL_NAMES={\"V\",\"D\",\"I\",\"W\",\"E\",\"F\",\"O\"}"
        )
    endif()
    set_target_properties(spdlog PROPERTIES FOLDER external)
endif()

# pugixml
# 用于读取 xml 文件

CPMAddPackage(
    NAME pugixml
    VERSION 1.13
    GITHUB_REPOSITORY zeux/pugixml
    DOWNLOAD_ONLY YES
)

if(pugixml_ADDED)
    # pugixml 提供的 CMake 支持有点脏，我们自己来
    add_library(pugixml STATIC)
    target_include_directories(pugixml PUBLIC
        ${pugixml_SOURCE_DIR}/src
    )
    target_sources(pugixml PRIVATE
        ${pugixml_SOURCE_DIR}/src/pugiconfig.hpp
        ${pugixml_SOURCE_DIR}/src/pugixml.hpp
        ${pugixml_SOURCE_DIR}/src/pugixml.cpp
    )
    target_sources(pugixml PUBLIC
        ${pugixml_SOURCE_DIR}/scripts/natvis/pugixml.natvis
    )
    set_target_properties(pugixml PROPERTIES FOLDER external)
endif()

# nothings stb
# 各种杂项工具

CPMAddPackage(
    NAME nothings_stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG 5736b15f7ea0ffb08dd38af21067c314d6a3aae9
    DOWNLOAD_ONLY YES
)

if(nothings_stb_ADDED)
    # stb 是仅头文件的库，且没有提供 CMake 支持
    add_library(nothings_stb STATIC)
    target_include_directories(nothings_stb PUBLIC
        ${nothings_stb_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/nothings_stb/include
    )
    file(WRITE ${CMAKE_BINARY_DIR}/nothings_stb/include/stb_vorbis.h
        "#define STB_VORBIS_HEADER_ONLY\n"
        "#include \"stb_vorbis.c\"\n"
        "#undef STB_VORBIS_HEADER_ONLY\n"
    )
    file(WRITE ${CMAKE_BINARY_DIR}/nothings_stb/nothings_stb.c
        "#define STB_IMAGE_IMPLEMENTATION\n"
        "#include \"stb_image.h\"\n"
        "#include \"stb_vorbis.c\"\n"
    )
    target_sources(nothings_stb PRIVATE
        ${CMAKE_BINARY_DIR}/nothings_stb/include/stb_vorbis.h
        ${nothings_stb_SOURCE_DIR}/stb_image.h
        ${CMAKE_BINARY_DIR}/nothings_stb/nothings_stb.c
    )
    set_target_properties(nothings_stb PROPERTIES FOLDER external)
endif()

# dr_libs
# 解码 wav 和 mp3 音频文件

CPMAddPackage(
    NAME dr_libs
    GITHUB_REPOSITORY mackron/dr_libs
    GIT_TAG dd762b861ecadf5ddd5fb03e9ca1db6707b54fbb
    DOWNLOAD_ONLY YES
)

if(dr_libs_ADDED)
    # dr_libs 是仅头文件的库，且没有提供 CMake 支持
    add_library(dr_libs STATIC)
    target_include_directories(dr_libs PUBLIC
        ${dr_libs_SOURCE_DIR}
    )
    file(WRITE ${CMAKE_BINARY_DIR}/dr_libs/dr_libs.c
        "#define DR_WAV_IMPLEMENTATION\n"
        "#define DR_MP3_IMPLEMENTATION\n"
        "#define DR_FLAC_IMPLEMENTATION\n"
        "#include \"dr_wav.h\"\n"
        "#include \"dr_mp3.h\"\n"
        "#include \"dr_flac.h\"\n"
    )
    target_sources(dr_libs PRIVATE
        ${dr_libs_SOURCE_DIR}/dr_wav.h
        ${dr_libs_SOURCE_DIR}/dr_mp3.h
        ${dr_libs_SOURCE_DIR}/dr_flac.h
        ${CMAKE_BINARY_DIR}/dr_libs/dr_libs.c
    )
    set_target_properties(dr_libs PROPERTIES FOLDER external)
endif()

# tinygltf
# 解析 gltf 2.0 模型格式

CPMAddPackage(
    NAME tinygltf
    VERSION 2.8.3
    GITHUB_REPOSITORY syoyo/tinygltf
    #OPTIONS
    #"TINYGLTF_BUILD_LOADER_EXAMPLE OFF"
    #"TINYGLTF_INSTALL OFF"
    DOWNLOAD_ONLY YES
)

if(tinygltf_ADDED)
    # tinygltf 提供的 CMake 支持有点脏，我们自己来
    add_library(tinygltf STATIC)
    target_compile_definitions(tinygltf PUBLIC
        TINYGLTF_NO_STB_IMAGE_WRITE
    )
    # 为了避免其使用自带的 json 和 stb 库，首先得把头文件拉到一个单独的文件夹
    file(WRITE ${CMAKE_BINARY_DIR}/tinygltf/tiny_gltf.h "PLACEHOLD")
    file(REMOVE
        ${CMAKE_BINARY_DIR}/tinygltf/tiny_gltf.h
    )
    file(COPY_FILE
        ${tinygltf_SOURCE_DIR}/tiny_gltf.h
        ${CMAKE_BINARY_DIR}/tinygltf/tiny_gltf.h
    )
    # 配置 include 路径，避免使用自带的 json 和 stb 库
    target_include_directories(tinygltf PUBLIC
        ${CMAKE_BINARY_DIR}/tinygltf
    )
    target_sources(tinygltf PRIVATE
        ${CMAKE_BINARY_DIR}/tinygltf/tiny_gltf.h
        ${tinygltf_SOURCE_DIR}/tiny_gltf.cc
    )
    target_link_libraries(tinygltf PRIVATE
        nlohmann_json
        nothings_stb
    )
    set_target_properties(tinygltf PROPERTIES FOLDER external)
endif()

# tinyobjloader
# 解析 obj 模型格式

CPMAddPackage(
    NAME tinyobjloader
    #VERSION 2.0.0
    GITHUB_REPOSITORY tinyobjloader/tinyobjloader
    GIT_TAG v2.0.0rc10
    DOWNLOAD_ONLY YES
)

if(tinyobjloader_ADDED)
    # tinyobjloader 提供的 CMake 支持有点脏，我们自己来
    add_library(tinyobjloader STATIC)
    target_include_directories(tinyobjloader PUBLIC
        ${tinyobjloader_SOURCE_DIR}
    )
    target_sources(tinyobjloader PRIVATE
        ${tinyobjloader_SOURCE_DIR}/tiny_obj_loader.h
        ${tinyobjloader_SOURCE_DIR}/tiny_obj_loader.cc
    )
    set_target_properties(tinyobjloader PROPERTIES FOLDER external)
endif()
