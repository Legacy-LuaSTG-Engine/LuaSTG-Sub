# d3dcompiler

add_library(Microsoft.D3DCompiler.Redist SHARED IMPORTED GLOBAL)

CPMAddPackage(
    NAME Microsoft.Windows.SDK.CPP
    VERSION 10.0.26100.7463
    URL https://www.nuget.org/api/v2/package/Microsoft.Windows.SDK.CPP/10.0.26100.7463
    URL_HASH SHA256=54C5E6EBFFF5E1A8A84CDBF8AB7A4E72419620A5B2D430A3AA88734C4C557CAB
    DOWNLOAD_ONLY YES
)

if (NOT Microsoft.Windows.SDK.CPP_ADDED)
    message(FATAL_ERROR "Microsoft.Windows.SDK.CPP is required")
endif ()

set(d3dcompiler47_dll "d3dcompiler_47.dll")
if(Microsoft.Windows.SDK.CPP_ADDED)
    set(windows_sdk_redist "${Microsoft.Windows.SDK.CPP_SOURCE_DIR}/c/Redist/D3D")
    if(LUASTG_ARCH STREQUAL "amd64")
        message(STATUS "Microsoft.D3DCompiler.Redist (amd64)")
        set(windows_sdk_redist "${windows_sdk_redist}/x64")
    elseif(LUASTG_ARCH STREQUAL "arm64")
        message(STATUS "Microsoft.D3DCompiler.Redist (arm64)")
        set(windows_sdk_redist "${windows_sdk_redist}/arm64")
    elseif(LUASTG_ARCH STREQUAL "x86")
        message(STATUS "Microsoft.D3DCompiler.Redist (x86)")
        set(windows_sdk_redist "${windows_sdk_redist}/x86")
    else ()
        message(FATAL_ERROR "unknown platform")
    endif ()
    set(d3dcompiler47_dll "${windows_sdk_redist}/d3dcompiler_47.dll")
endif()

if (NOT EXISTS ${d3dcompiler47_dll})
    message(FATAL_ERROR "d3dcompiler_47.dll not exists in nuget package Microsoft.Windows.SDK.CPP")
endif()

# import

set_target_properties(Microsoft.D3DCompiler.Redist PROPERTIES
    IMPORTED_IMPLIB d3dcompiler.lib
    IMPORTED_LOCATION ${d3dcompiler47_dll}
)
