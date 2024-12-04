# d3dcompiler

add_library(Microsoft.D3DCompiler.Redist SHARED IMPORTED GLOBAL)

message(STATUS "Microsoft.D3DCompiler.Redist:")
message(STATUS "-- CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")

set(windows_10_sdk_root "C:/Program Files (x86)/Windows Kits/10") # TODO: REMOVE MAGIC HARD CODE
set(windows_10_sdk_bin "${windows_10_sdk_root}/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(windows_10_sdk_bin "${windows_10_sdk_bin}/x64")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(windows_10_sdk_bin "${windows_10_sdk_bin}/x86")
else ()
    message(FATAL_ERROR "unknown platform")
endif ()

set(d3dcompiler47_dll "${windows_10_sdk_bin}/d3dcompiler_47.dll")
message(STATUS "-- IMPORTED_LOCATION: ${d3dcompiler47_dll}")
if (NOT EXISTS ${d3dcompiler47_dll})
    message(FATAL_ERROR "d3dcompiler_47.dll (amd64) is required")
endif ()
set_target_properties(Microsoft.D3DCompiler.Redist PROPERTIES
    IMPORTED_IMPLIB d3dcompiler.lib
    IMPORTED_LOCATION ${d3dcompiler47_dll}
)
