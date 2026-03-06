
if(EXISTS ${LUASTG_RESDIR}/Custom.cmake)
    message(STATUS "LuaSTG will using custom build configuration: " ${LUASTG_RESDIR}/Custom.cmake)
    include(${LUASTG_RESDIR}/Custom.cmake)
elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${LUASTG_RESDIR}/Custom.cmake)
    message(STATUS "LuaSTG will using custom build configuration: " ${CMAKE_CURRENT_LIST_DIR}/${LUASTG_RESDIR}/Custom.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/${LUASTG_RESDIR}/Custom.cmake)
else()
    message(FATAL_ERROR "LuaSTG can not find custom build configuration")
endif()

if (LUASTG_LINK_LUASOCKET)
    target_compile_definitions(LuaSTG PRIVATE LUASTG_LINK_LUASOCKET)
    target_link_libraries(LuaSTG PRIVATE luasocket)
    message(STATUS "[LuaSTG] Link: luasocket")
endif ()

configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/import_luastg_options.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/luastg_config_generated.h"
    @ONLY
)
target_include_directories(LuaSTG PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(Setting PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
set(luastg_generated_sources "${CMAKE_CURRENT_BINARY_DIR}/luastg_config_generated.h")
source_group(TREE "${CMAKE_CURRENT_BINARY_DIR}" PREFIX "generated" FILES ${luastg_generated_sources})
target_sources(LuaSTG PRIVATE ${luastg_generated_sources})
