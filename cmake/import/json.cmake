# nlohmann json

CPMAddPackage(
    NAME nlohmann_json
    VERSION 3.12.0
    # ！！！警告：不要直接使用 nlohmann json 的 git 仓库，否则下载耗时++++
    #GITHUB_REPOSITORY nlohmann/json
    #OPTIONS
    #"JSON_BuildTests OFF"
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/include.zip
    URL_HASH SHA256=B8CB0EF2DD7F57F18933997C9934BB1FA962594F701CD5A8D3C2C80541559372
    DOWNLOAD_ONLY YES
)

if(nlohmann_json_ADDED)
    add_library(nlohmann_json INTERFACE)
    add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
    target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
endif()
