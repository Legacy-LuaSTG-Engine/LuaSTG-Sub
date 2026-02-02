# 设置 C 运行时库
# * DynamicCRT
#     - 链接到 UCRT 和 VCRT 的动态链接库
# * StaticCRT
#     - 链接到 UCRT 和 VCRT 的静态链接库
# * HybridCRT
#     - 链接到 UCRT 的动态链接库
#     - 链接到 VCRT 的静态链接库

function(msvc_dynamic_runtime_library)
    if(MSVC)
        message(STATUS "MSVC detected, using DynamicCRT")
        cmake_policy(SET CMP0091 NEW)
        set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
            PARENT_SCOPE
        )
    endif()
endfunction()

function(msvc_static_runtime_library)
    if(MSVC)
        message(STATUS "MSVC detected, using StaticCRT")
        cmake_policy(SET CMP0091 NEW)
        set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>"
            PARENT_SCOPE
        )
    endif()
endfunction()

function(msvc_hybrid_runtime_library)
    if(MSVC)
        message(STATUS "MSVC detected, using HybridCRT")
        cmake_policy(SET CMP0091 NEW)
        set(CMAKE_MSVC_RUNTIME_LIBRARY
            # Statically link the C++ runtime libraries, but partially override this below
            "MultiThreaded$<$<CONFIG:Debug>:Debug>"
            PARENT_SCOPE
        )
        add_link_options(
            "/DEFAULTLIB:ucrt$<$<CONFIG:Debug>:d>.lib" # include the dynamic UCRT
            "/NODEFAULTLIB:libucrt$<$<CONFIG:Debug>:d>.lib" # ignore the static UCRT
        )
    endif()
endfunction()
