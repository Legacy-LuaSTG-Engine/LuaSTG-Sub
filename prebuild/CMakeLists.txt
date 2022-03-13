
# ---- luajit

add_library(luajit SHARED IMPORTED GLOBAL)
target_compile_definitions(luajit INTERFACE
    LUA_BUILD_AS_DLL
)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_include_directories(luajit INTERFACE
        amd64/include
    )
    set_target_properties(luajit PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/amd64/lib/lua51.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/amd64/bin/lua51.dll
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_include_directories(luajit INTERFACE
        x86/include
    )
    set_target_properties(luajit PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/x86/lib/lua51.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/x86/bin/lua51.dll
    )
else()
    message(FATAL_ERROR "unknown platform")
endif()
