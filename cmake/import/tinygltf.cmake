# tinygltf

CPMAddPackage(
    NAME tinygltf
    VERSION 2.9.3
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
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/tinygltf)
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tinygltf)
    endif ()
    file(COPY_FILE
        ${tinygltf_SOURCE_DIR}/tiny_gltf.h
        ${CMAKE_BINARY_DIR}/tinygltf/tiny_gltf.h
        ONLY_IF_DIFFERENT
    )
    # 配置 include 路径，避免使用自带的 json 和 stb 库
    target_include_directories(tinygltf PUBLIC
        ${CMAKE_BINARY_DIR}/tinygltf
        ${nlohmann_json_SOURCE_DIR}/include/nlohmann # 非常傻逼
    )
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/tinygltf/tiny_gltf.cpp)
        file(WRITE ${CMAKE_BINARY_DIR}/tinygltf/tiny_gltf.cpp
            "#define TINYGLTF_IMPLEMENTATION\n"
            "#include \"tiny_gltf.h\"\n"
        )
    endif ()
    target_sources(tinygltf PRIVATE
        ${CMAKE_BINARY_DIR}/tinygltf/tiny_gltf.h
        ${CMAKE_BINARY_DIR}/tinygltf/tiny_gltf.cpp
    )
    target_link_libraries(tinygltf PUBLIC
        nlohmann_json
        nothings_stb
    )
    set_target_properties(tinygltf PROPERTIES FOLDER external)
endif()
