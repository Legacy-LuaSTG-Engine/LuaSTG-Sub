# zlib

CPMAddPackage(
    NAME zlib_ng # 必须用这个名称，用来适配一些SB库
    VERSION 2.1.3
    GITHUB_REPOSITORY zlib-ng/zlib-ng
    GIT_TAG 2.1.3
    OPTIONS
    "ZLIB_ENABLE_TESTS OFF"
    "ZLIB_COMPAT OFF"
)

if(zlib_ng_ADDED)
    if(TARGET zlib-ng)
        set_target_properties(zlib-ng PROPERTIES FOLDER external)
    else()
        set_target_properties(zlib PROPERTIES FOLDER external)
    endif()
endif()

# minizip
# 读取 zip 文件
# 另外，操你妈的 minizip-ng 开发团队，
# 不打算支持同 project 内的 zlib-ng 目标是吧？老子不会自己弄一个分支吗？

CPMAddPackage(
    NAME minizip_ng
    VERSION 3.0.8
    GITHUB_REPOSITORY Demonese/minizip-ng # FUCK YOU minizip-ng
    GIT_TAG be71a37ad7223180a75d4920cf7dd6c56ea55a3c # FUCK YOU minizip-ng
    OPTIONS
    "MZ_COMPAT OFF"
    "MZ_FETCH_LIBS OFF"
    "SKIP_INSTALL_ALL ON"
)

if(minizip_ng_ADDED)
    set_target_properties(minizip-ng PROPERTIES FOLDER external)
endif()
