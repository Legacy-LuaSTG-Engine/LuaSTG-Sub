
target_include_directories(LuaSTG PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
set(_LuaSTG_res
    ${CMAKE_CURRENT_LIST_DIR}/AppFrameLuaEx.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Config.h
    ${CMAKE_CURRENT_LIST_DIR}/ResourcePassword.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app.ico
    ${CMAKE_CURRENT_LIST_DIR}/resource.h
    ${CMAKE_CURRENT_LIST_DIR}/resource.rc
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} PREFIX res FILES ${_LuaSTG_res})
target_sources(LuaSTG PRIVATE
    ${_LuaSTG_res}
)
