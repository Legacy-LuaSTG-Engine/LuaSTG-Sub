# d3dcompiler

add_library(Microsoft.D3DCompiler.Redist SHARED IMPORTED GLOBAL)

# 注意：从 Windows SDK 10.0.26100 开始，d3dcompiler_47.dll 不再静态链接 CRT，而是链接到 UCRT

CPMAddPackage(
    NAME Microsoft.Windows.SDK.CPP
    VERSION 10.0.22621.3233
    URL https://www.nuget.org/api/v2/package/Microsoft.Windows.SDK.CPP/10.0.22621.3233
    URL_HASH SHA256=E4EFE1768EA61F4F999DBEF61B09895320629F975F9CEED8290A9633E0C31623
    DOWNLOAD_ONLY YES
)

if (NOT Microsoft.Windows.SDK.CPP_ADDED)
    message(FATAL_ERROR "Microsoft.Windows.SDK.CPP is required")
endif ()

set(d3dcompiler47_dll "d3dcompiler_47.dll")
if(Microsoft.Windows.SDK.CPP_ADDED)
    set(windows_sdk_redist "${Microsoft.Windows.SDK.CPP_SOURCE_DIR}/c/Redist/D3D")
    string(TOUPPER ${CMAKE_SYSTEM_PROCESSOR} system_processor)
    if(system_processor STREQUAL "AMD64")
        message(STATUS "Microsoft.D3DCompiler.Redist (AMD64)")
        set(windows_sdk_redist "${windows_sdk_redist}/x64")
    elseif(system_processor EQUAL "ARM64")
        message(STATUS "Microsoft.D3DCompiler.Redist (ARM64)")
        set(windows_sdk_redist "${windows_sdk_redist}/arm64")
    elseif(system_processor EQUAL "X86")
        message(STATUS "Microsoft.D3DCompiler.Redist (X86)")
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
