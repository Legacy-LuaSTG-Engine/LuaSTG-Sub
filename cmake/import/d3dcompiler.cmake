# d3dcompiler

add_library(Microsoft.D3DCompiler.Redist SHARED IMPORTED GLOBAL)

message(STATUS "Microsoft.D3DCompiler.Redist:")
message(STATUS "-- CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")

# from installed Windows SDK

set(windows_10_sdk_root "C:/Program Files (x86)/Windows Kits/10")
set(windows_10_sdk_bin "${windows_10_sdk_root}/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(windows_10_sdk_bin "${windows_10_sdk_bin}/x64")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(windows_10_sdk_bin "${windows_10_sdk_bin}/x86")
else ()
    message(FATAL_ERROR "-- unknown platform")
endif ()
set(d3dcompiler47_dll "${windows_10_sdk_bin}/d3dcompiler_47.dll")

# from nuget package Microsoft.Windows.SDK.CPP

if (NOT EXISTS ${d3dcompiler47_dll})
    message(STATUS "-- d3dcompiler_47.dll not exits, download nuget package Microsoft.Windows.SDK.CPP")
    CPMAddPackage(
        NAME Microsoft.Windows.SDK.CPP
        VERSION 10.0.26100.3323
        URL https://www.nuget.org/api/v2/package/Microsoft.Windows.SDK.CPP/10.0.26100.3323
        URL_HASH SHA256=CDABF218DB25A5DB214FA7C4826DAE6B00AEE6EFB61BC1630D8F7E9151EDC791
        DOWNLOAD_ONLY YES
    )
    if(Microsoft.Windows.SDK.CPP_ADDED)
        set(windows_sdk_redist "${Microsoft.Windows.SDK.CPP_SOURCE_DIR}/c/Redist/D3D")
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(windows_sdk_redist "${windows_sdk_redist}/x64")
        elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(windows_sdk_redist "${windows_sdk_redist}/x86")
        else ()
            message(FATAL_ERROR "-- unknown platform")
        endif ()
        set(d3dcompiler47_dll "${windows_sdk_redist}/d3dcompiler_47.dll")
    endif()
    if (NOT EXISTS ${d3dcompiler47_dll})
        message(FATAL_ERROR "-- download nuget package Microsoft.Windows.SDK.CPP failed")
    endif()
endif ()

# import

message(STATUS "-- IMPORTED_LOCATION: ${d3dcompiler47_dll}")
set_target_properties(Microsoft.D3DCompiler.Redist PROPERTIES
    IMPORTED_IMPLIB d3dcompiler.lib
    IMPORTED_LOCATION ${d3dcompiler47_dll}
)
