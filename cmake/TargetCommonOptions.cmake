function(luastg_target_common_options __TARGET__)
    target_compile_options(${__TARGET__} PRIVATE
        "/MP"
        "/utf-8"
    )
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_compile_options(${__TARGET__} PRIVATE
            "/arch:SSE2"
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
    )
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_compile_options(${__TARGET__} PRIVATE
            "/arch:SSE2"
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
