
#==================== lua fs ====================#

add_library(lua_fs STATIC)
target_compile_definitions(lua_fs PRIVATE
    _CRT_SECURE_NO_WARNINGS
)
target_include_directories(lua_fs PUBLIC
    luafilesystem/src
)
target_sources(lua_fs PRIVATE
    luafilesystem/src/lua_fs.h
    luafilesystem/src/lfs.h
    luafilesystem/src/lfs.c
)
target_link_libraries(lua_fs PUBLIC
    luajit
)

set_target_properties(lua_fs PROPERTIES FOLDER external)

#==================== lua extension ====================#

add_subdirectory(pugixml)

set_target_properties(pugixml-static PROPERTIES FOLDER external)

add_library(lua_xlsx_csv STATIC)
set_target_properties(lua_xlsx_csv PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(lua_xlsx_csv PUBLIC
    luaext
)
target_sources(lua_xlsx_csv PRIVATE
    luaext/lua_xlsx_csv.h
    luaext/lua_xlsx.cpp
    luaext/lua_csv.cpp
)
target_link_libraries(lua_xlsx_csv PUBLIC
    luajit
    zip
    pugixml
)

set_target_properties(lua_xlsx_csv PROPERTIES FOLDER external)
