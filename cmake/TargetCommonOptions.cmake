function(luastg_target_common_options __TARGET__)
    target_compile_options(${__TARGET__} PRIVATE
        "/MP"
        "/utf-8"
        "$<$<CONFIG:Debug>:/ZI>"
    )
    #target_link_options(${__TARGET__} PRIVATE
    #    "/DEPENDENTLOADFLAG:0x800" # Windows 10 1607+ 强制 DLL 搜索目录为系统目录
    #)
    if(LUASTG_ARCH STREQUAL "x86")
        target_compile_options(${__TARGET__} PRIVATE
            "/arch:SSE2"
        )
        target_link_options(${__TARGET__} PRIVATE
            "$<$<CONFIG:Debug>:/SAFESEH:NO>"
        )
    endif()
    set_target_properties(${__TARGET__} PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
    target_compile_definitions(${__TARGET__} PRIVATE
        _UNICODE
        UNICODE
    )
endfunction()

function(luastg_target_common_options2 __TARGET__)
    target_compile_options(${__TARGET__} PRIVATE
        "/MP"
        "/utf-8"
        "$<$<CONFIG:Debug>:/ZI>"
    )
    #target_link_options(${__TARGET__} PRIVATE
    #    "/DEPENDENTLOADFLAG:0x800" # Windows 10 1607+ 强制 DLL 搜索目录为系统目录
    #)
    if(LUASTG_ARCH STREQUAL "x86")
        target_compile_options(${__TARGET__} PRIVATE
            "/arch:SSE2"
        )
        target_link_options(${__TARGET__} PRIVATE
            "$<$<CONFIG:Debug>:/SAFESEH:NO>"
        )
    endif()
    set_target_properties(${__TARGET__} PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
endfunction()

function(luastg_target_more_warning __TARGET__)
    target_compile_options(${__TARGET__} PRIVATE
        "/W4"
    )
endfunction()

function(luastg_target_copy_to_output_directory __AFTER_TARGET__ __TARGET__)
    add_custom_command(TARGET ${__AFTER_TARGET__} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/engine
        COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_SOURCE_DIR}/engine/"$<TARGET_FILE_NAME:${__TARGET__}>"
        COMMAND ${CMAKE_COMMAND} -E copy  "$<TARGET_FILE:${__TARGET__}>" ${CMAKE_SOURCE_DIR}/engine
    )
endfunction()

function(luastg_target_platform_windows_7 __TARGET__)
    target_compile_definitions(${__TARGET__}
    PRIVATE
        _WIN32_WINNT=0x0601       # _WIN32_WINNT_WIN7
        NTDDI_VERSION=0x06010000  # NTDDI_WIN7
    )
endfunction()
