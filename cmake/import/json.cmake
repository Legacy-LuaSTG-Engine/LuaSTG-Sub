# nlohmann json

CPMAddPackage(
    NAME nlohmann_json
    VERSION 3.11.3
    # ！！！警告：不要直接使用 nlohmann json 的 git 仓库，否则下载耗时++++
    #GITHUB_REPOSITORY nlohmann/json
    #OPTIONS
    #"JSON_BuildTests OFF"
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/include.zip
    URL_HASH SHA256=A22461D13119AC5C78F205D3DF1DB13403E58CE1BB1794EDC9313677313F4A9D
    DOWNLOAD_ONLY YES
)

if(nlohmann_json_ADDED)
    add_library(nlohmann_json INTERFACE)
    add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
    target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
endif()
