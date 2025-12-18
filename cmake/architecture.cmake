if (NOT (DEFINED LUASTG_ARCH))
    try_compile(LUASTG_IS_X86
        ${CMAKE_CURRENT_BINARY_DIR}/is_x86
        SOURCES ${CMAKE_CURRENT_LIST_DIR}/architecture/is_x86.c
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
    )
    try_compile(LUASTG_IS_AMD64
        ${CMAKE_CURRENT_BINARY_DIR}/is_amd64
        SOURCES ${CMAKE_CURRENT_LIST_DIR}/architecture/is_amd64.c
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
    )
    try_compile(LUASTG_IS_ARM64
        ${CMAKE_CURRENT_BINARY_DIR}/is_arm64
        SOURCES ${CMAKE_CURRENT_LIST_DIR}/architecture/is_arm64.c
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
    )
    if (LUASTG_IS_X86 AND (NOT LUASTG_IS_AMD64) AND (NOT LUASTG_IS_ARM64))
        set(LUASTG_ARCH "x86" CACHE STRING "LuaSTG: current architecture" FORCE)
    elseif ((NOT LUASTG_IS_X86) AND LUASTG_IS_AMD64 AND (NOT LUASTG_IS_ARM64))
        set(LUASTG_ARCH "amd64" CACHE STRING "LuaSTG: current architecture" FORCE)
    elseif ((NOT LUASTG_IS_X86) AND (NOT LUASTG_IS_AMD64) AND LUASTG_IS_ARM64)
        set(LUASTG_ARCH "arm64" CACHE STRING "LuaSTG: current architecture" FORCE)
    else ()
        message(FATAL_ERROR "LuaSTG: what's your architecture?")
    endif ()
    message(STATUS "LuaSTG: architecture test result")
    message(STATUS "-- x86  : ${LUASTG_IS_X86}")
    message(STATUS "-- amd64: ${LUASTG_IS_AMD64}")
    message(STATUS "-- arm64: ${LUASTG_IS_ARM64}")
endif ()

message(STATUS "LuaSTG: current architecture is ${LUASTG_ARCH}")
