cmake_minimum_required(VERSION 3.20)
project(luajit)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Debug>:DLL>")

# build contorller

set(LJ_ROOT ${CMAKE_CURRENT_LIST_DIR}/luajit)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message("64 bits platform")
    set(LJ_AMD64 ON)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message("32 bits platform")
    set(LJ_AMD64 OFF)
else()
    message(FATAL_ERROR "unknown platform")
endif()

set(LJ_TEMP ${CMAKE_BINARY_DIR}/lj_h_)

function(luajit_target_common_setup __TARGET__)
    target_compile_options(${__TARGET__} PRIVATE
        "/D_CRT_STDIO_INLINE=__declspec(dllexport)__inline"
    )
    if(NOT LJ_AMD64)
        target_compile_options(${__TARGET__} PRIVATE
            "/arch:SSE2"
        )
    endif()
    target_compile_definitions(${__TARGET__} PRIVATE
        _CRT_SECURE_NO_DEPRECATE
    )
endfunction()

# minilua

add_executable(minilua)
luajit_target_common_setup(minilua)
target_sources(minilua PRIVATE
    ${LJ_ROOT}/src/host/minilua.c
)

set(LJ_DASMDIR   ${LJ_ROOT}/dynasm)
set(LJ_DASM      ${LJ_DASMDIR}/dynasm.lua)
set(LJ_DASMFLAGS -D WIN -D JIT -D FFI)
set(LJ_DASC      ${LJ_ROOT}/src/vm_x86.dasc)

if(LJ_AMD64)
    set(LJ_DASMFLAGS ${LJ_DASMFLAGS} -D P64) # add P64 option
    set(LJ_DASC      ${LJ_ROOT}/src/vm_x64.dasc) # x64
endif()

add_custom_command(TARGET minilua POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LJ_TEMP}$<CONFIG>
    COMMAND $<TARGET_FILE:minilua> ${LJ_DASM} -LN ${LJ_DASMFLAGS} -o ${LJ_TEMP}$<CONFIG>/buildvm_arch.h ${LJ_DASC}
)

# buildvm

add_executable(buildvm)
luajit_target_common_setup(buildvm)
target_include_directories(buildvm PRIVATE
    ${LJ_TEMP}$<CONFIG> # before src
    ${LJ_ROOT}/src
    ${LJ_ROOT}/dynasm
)
target_sources(buildvm PRIVATE
    ${LJ_ROOT}/src/host/buildvm_asm.c
    ${LJ_ROOT}/src/host/buildvm_fold.c
    ${LJ_ROOT}/src/host/buildvm_lib.c
    ${LJ_ROOT}/src/host/buildvm_peobj.c
    ${LJ_ROOT}/src/host/buildvm.c
)

add_dependencies(buildvm minilua)

set(LJ_ALL_LIB
    ${LJ_ROOT}/src/lib_base.c
    ${LJ_ROOT}/src/lib_math.c
    ${LJ_ROOT}/src/lib_bit.c
    ${LJ_ROOT}/src/lib_string.c
    ${LJ_ROOT}/src/lib_table.c
    ${LJ_ROOT}/src/lib_io.c
    ${LJ_ROOT}/src/lib_os.c
    ${LJ_ROOT}/src/lib_package.c
    ${LJ_ROOT}/src/lib_debug.c
    ${LJ_ROOT}/src/lib_jit.c
    ${LJ_ROOT}/src/lib_ffi.c
    ${LJ_ROOT}/src/lib_buffer.c
)
add_custom_command(TARGET buildvm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LJ_TEMP}$<CONFIG>
    
    COMMAND $<TARGET_FILE:buildvm> -m peobj   -o ${LJ_TEMP}$<CONFIG>/lj_vm.obj
    
    COMMAND $<TARGET_FILE:buildvm> -m bcdef   -o ${LJ_TEMP}$<CONFIG>/lj_bcdef.h    ${LJ_ALL_LIB}
    COMMAND $<TARGET_FILE:buildvm> -m ffdef   -o ${LJ_TEMP}$<CONFIG>/lj_ffdef.h    ${LJ_ALL_LIB}
    COMMAND $<TARGET_FILE:buildvm> -m libdef  -o ${LJ_TEMP}$<CONFIG>/lj_libdef.h   ${LJ_ALL_LIB}
    COMMAND $<TARGET_FILE:buildvm> -m recdef  -o ${LJ_TEMP}$<CONFIG>/lj_recdef.h   ${LJ_ALL_LIB}
    COMMAND $<TARGET_FILE:buildvm> -m folddef -o ${LJ_TEMP}$<CONFIG>/lj_folddef.h  ${LJ_ROOT}/src/lj_opt_fold.c
    
    COMMAND $<TARGET_FILE:buildvm> -m vmdef   -o ${LJ_ROOT}/src/jit/vmdef.lua ${LJ_ALL_LIB}
)

# lua51 (luajit) common

set(LJ_SRC
    ${LJ_ROOT}/src/lj_alloc.c
    ${LJ_ROOT}/src/lj_api.c
    ${LJ_ROOT}/src/lj_asm.c
    ${LJ_ROOT}/src/lj_assert.c
    ${LJ_ROOT}/src/lj_bc.c
    ${LJ_ROOT}/src/lj_bcread.c
    ${LJ_ROOT}/src/lj_bcwrite.c
    ${LJ_ROOT}/src/lj_buf.c
    ${LJ_ROOT}/src/lj_carith.c
    ${LJ_ROOT}/src/lj_ccall.c
    ${LJ_ROOT}/src/lj_ccallback.c
    ${LJ_ROOT}/src/lj_cconv.c
    ${LJ_ROOT}/src/lj_cdata.c
    ${LJ_ROOT}/src/lj_char.c
    ${LJ_ROOT}/src/lj_clib.c
    ${LJ_ROOT}/src/lj_cparse.c
    ${LJ_ROOT}/src/lj_crecord.c
    ${LJ_ROOT}/src/lj_ctype.c
    ${LJ_ROOT}/src/lj_debug.c
    ${LJ_ROOT}/src/lj_dispatch.c
    ${LJ_ROOT}/src/lj_err.c
    ${LJ_ROOT}/src/lj_ffrecord.c
    ${LJ_ROOT}/src/lj_func.c
    ${LJ_ROOT}/src/lj_gc.c
    ${LJ_ROOT}/src/lj_gdbjit.c
    ${LJ_ROOT}/src/lj_ir.c
    ${LJ_ROOT}/src/lj_lex.c
    ${LJ_ROOT}/src/lj_lib.c
    ${LJ_ROOT}/src/lj_load.c
    ${LJ_ROOT}/src/lj_mcode.c
    ${LJ_ROOT}/src/lj_meta.c
    ${LJ_ROOT}/src/lj_obj.c
    ${LJ_ROOT}/src/lj_opt_dce.c
    ${LJ_ROOT}/src/lj_opt_fold.c
    ${LJ_ROOT}/src/lj_opt_loop.c
    ${LJ_ROOT}/src/lj_opt_mem.c
    ${LJ_ROOT}/src/lj_opt_narrow.c
    ${LJ_ROOT}/src/lj_opt_sink.c
    ${LJ_ROOT}/src/lj_opt_split.c
    ${LJ_ROOT}/src/lj_parse.c
    ${LJ_ROOT}/src/lj_prng.c
    ${LJ_ROOT}/src/lj_profile.c
    ${LJ_ROOT}/src/lj_record.c
    ${LJ_ROOT}/src/lj_serialize.c
    ${LJ_ROOT}/src/lj_snap.c
    ${LJ_ROOT}/src/lj_state.c
    ${LJ_ROOT}/src/lj_str.c
    ${LJ_ROOT}/src/lj_strfmt_num.c
    ${LJ_ROOT}/src/lj_strfmt.c
    ${LJ_ROOT}/src/lj_strscan.c
    ${LJ_ROOT}/src/lj_tab.c
    ${LJ_ROOT}/src/lj_trace.c
    ${LJ_ROOT}/src/lj_udata.c
    ${LJ_ROOT}/src/lj_vmevent.c
    ${LJ_ROOT}/src/lj_vmmath.c
    ${LJ_ROOT}/src/lib_aux.c
    ${LJ_ROOT}/src/lib_base.c
    ${LJ_ROOT}/src/lib_bit.c
    ${LJ_ROOT}/src/lib_buffer.c
    ${LJ_ROOT}/src/lib_debug.c
    ${LJ_ROOT}/src/lib_ffi.c
    ${LJ_ROOT}/src/lib_init.c
    #${LJ_ROOT}/src/lib_io.c
    ${LJ_ROOT}/src/lib_jit.c
    ${LJ_ROOT}/src/lib_math.c
    #${LJ_ROOT}/src/lib_os.c
    ${LJ_ROOT}/src/lib_package.c
    ${LJ_ROOT}/src/lib_string.c
    ${LJ_ROOT}/src/lib_table.c
    patch/lib_io.c
    patch/lib_os.c
    patch/lj_win32.c
)

# lua51 (luajit) lib

add_library(lua51static STATIC)
luajit_target_common_setup(lua51static)
target_include_directories(lua51static PRIVATE
    ${LJ_TEMP}$<CONFIG> # before src
)
target_include_directories(lua51static PUBLIC
    ${LJ_ROOT}/src
    patch
)
target_sources(lua51static PRIVATE
    ${LJ_SRC}
)
target_link_libraries(lua51static PRIVATE
    ${LJ_TEMP}$<CONFIG>/lj_vm.obj
)

add_dependencies(lua51static buildvm)

# lua51 (luajit) dll

add_library(lua51 SHARED)
luajit_target_common_setup(lua51)
target_compile_definitions(lua51 PUBLIC
    LUA_BUILD_AS_DLL
)
target_include_directories(lua51 PRIVATE
    ${LJ_TEMP}$<CONFIG> # before src
    ${LJ_ROOT}/src
    patch
)
target_sources(lua51 PRIVATE
    ${LJ_SRC}
)
target_link_libraries(lua51 PRIVATE
    ${LJ_TEMP}$<CONFIG>/lj_vm.obj
)

add_dependencies(lua51 buildvm)

# luajit

add_executable(luajit)
luajit_target_common_setup(luajit)
target_compile_definitions(luajit PRIVATE
    LUA_BUILD_AS_DLL
)
target_include_directories(luajit PRIVATE
    ${LJ_TEMP}$<CONFIG> # before src
    ${LJ_ROOT}/src
)
target_sources(luajit PRIVATE
    ${LJ_ROOT}/src/luajit.c
)
target_link_libraries(luajit PRIVATE
    lua51
)

# install

include(GNUInstallDirs)
install(TARGETS lua51static
    RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_BINDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}"
)
install(TARGETS lua51
    RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_BINDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}"
)
install(TARGETS luajit
    RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_BINDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}"
)
set(LJ_PUBLIC_H
    ${LJ_ROOT}/src/luaconf.h
    ${LJ_ROOT}/src/lua.h
    ${LJ_ROOT}/src/lauxlib.h
    ${LJ_ROOT}/src/lualib.h
    ${LJ_ROOT}/src/lua.hpp
    ${LJ_ROOT}/src/luajit.h
)
install(FILES ${LJ_PUBLIC_H} DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR})
