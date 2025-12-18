# YY-Thunks
# 日你妈的 Windows 7

CPMAddPackage(
    NAME YY_Thunks
    VERSION 1.1.4
    #GITHUB_REPOSITORY Chuyu-Team/YY-Thunks
    URL https://github.com/Chuyu-Team/YY-Thunks/releases/download/v1.1.4/YY-Thunks-1.1.4-Objs.zip
    URL_HASH SHA256=F9850A35D1F2B0CAE15E64DA516DE0EC5E798C060B320E91788AA33B853E85BA
    DOWNLOAD_ONLY YES
)

if(YY_Thunks_ADDED)
    add_library(YY_Thunks INTERFACE)
    if (LUASTG_ARCH STREQUAL "amd64")
        target_link_libraries(YY_Thunks INTERFACE
            ${YY_Thunks_SOURCE_DIR}/objs/x64/YY_Thunks_for_Win7.obj
        )
    elseif (LUASTG_ARCH STREQUAL "x86")
        target_link_libraries(YY_Thunks INTERFACE
            ${YY_Thunks_SOURCE_DIR}/objs/x86/YY_Thunks_for_Win7.obj
        )
    else ()
        message(FATAL_ERROR "YY_Thunks: unsupported architecture")
    endif()
    message(STATUS "LuaSTG: YY_Thunks: Enabled")
endif()
