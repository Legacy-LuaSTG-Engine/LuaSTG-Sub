cmake_minimum_required(VERSION 3.26)
project(LuaJIT)

# common

set(LUAJIT_ROOT ${CMAKE_CURRENT_LIST_DIR}/luajit)
set(LUAJIT_TEMP ${CMAKE_CURRENT_BINARY_DIR}/luajit/$<CONFIG>)

function(luajit_target_setup target)
    set_target_properties(${target} PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
    )
    target_compile_definitions(${target}
    PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
    target_compile_options(${target}
    PRIVATE
        "/W4"
        "/wd4127" # conditional expression is constant
        "/wd4702" # unreachable code
        "/wd4706" # assignment within conditional expression (note: update value in while/for loop)
        "/sdl"
        "/permissive-"
        "/D_CRT_STDIO_INLINE=__declspec(dllexport)__inline"
    )
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_compile_options(${target}
        PRIVATE
            "/arch:SSE2"
        )
    endif()
endfunction()

# minilua

add_executable(minilua)
luajit_target_setup(minilua)
target_sources(minilua
PRIVATE
    ${LUAJIT_ROOT}/src/host/minilua.c
)

# buildvm_arch

set(LUAJIT_DASM ${LUAJIT_ROOT}/dynasm/dynasm.lua)
set(LUAJIT_DASM_FLAGS)
set(LUAJIT_BUILDVM_ARCH_H_DIR ${LUAJIT_TEMP}/host)
set(LUAJIT_BUILDVM_ARCH_H ${LUAJIT_BUILDVM_ARCH_H_DIR}/buildvm_arch.h)
set(LUAJIT_DASC)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(LUAJIT_DASM_FLAGS -D WIN -D JIT -D FFI -D ENDIAN_LE -D FPU -D P64)
    if (CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        message(STATUS "luajit arm64 (aarch64)")
        set(LUAJIT_DASC ${LUAJIT_ROOT}/src/vm_arm64.dasc)
    else()
        message(STATUS "luajit amd64 (x86_64)")
        set(LUAJIT_DASC ${LUAJIT_ROOT}/src/vm_x64.dasc)
    endif()
else()
    set(LUAJIT_DASM_FLAGS -D WIN -D JIT -D FFI -D ENDIAN_LE -D FPU)
    message(STATUS "luajit x86")
    set(LUAJIT_DASC ${LUAJIT_ROOT}/src/vm_x86.dasc)
endif()

add_custom_command(
    OUTPUT ${LUAJIT_BUILDVM_ARCH_H}
    COMMAND cmake -E make_directory ${LUAJIT_BUILDVM_ARCH_H_DIR}
    COMMAND $<TARGET_FILE:minilua> ${LUAJIT_DASM} -LN ${LUAJIT_DASM_FLAGS} -o ${LUAJIT_BUILDVM_ARCH_H} ${LUAJIT_DASC}
    DEPENDS minilua
    VERBATIM
)

# version tag

add_custom_command(
    OUTPUT ${LUAJIT_TEMP}/luajit_relver.txt
    COMMAND git show -s --format=%ct > ${LUAJIT_TEMP}/luajit_relver.txt
    WORKING_DIRECTORY ${LUAJIT_ROOT}
    VERBATIM
    USES_TERMINAL
)

add_custom_command(
    OUTPUT ${LUAJIT_TEMP}/luajit.h
    COMMAND $<TARGET_FILE:minilua> host/genversion.lua luajit_rolling.h ${LUAJIT_TEMP}/luajit_relver.txt ${LUAJIT_TEMP}/luajit.h
    DEPENDS ${LUAJIT_TEMP}/luajit_relver.txt
    WORKING_DIRECTORY ${LUAJIT_ROOT}/src
    VERBATIM
)

# buildvm

add_executable(buildvm)
luajit_target_setup(buildvm)
target_include_directories(buildvm
PRIVATE
    ${LUAJIT_BUILDVM_ARCH_H_DIR}
    ${LUAJIT_TEMP}
    ${LUAJIT_ROOT}/dynasm
    ${LUAJIT_ROOT}/src
)
target_sources(buildvm
PRIVATE
    ${LUAJIT_BUILDVM_ARCH_H}
    ${LUAJIT_TEMP}/luajit.h
    ${LUAJIT_ROOT}/src/host/buildvm_asm.c
    ${LUAJIT_ROOT}/src/host/buildvm_fold.c
    ${LUAJIT_ROOT}/src/host/buildvm_lib.c
    ${LUAJIT_ROOT}/src/host/buildvm_peobj.c
    ${LUAJIT_ROOT}/src/host/buildvm.c
)

# vm object

set(LUAJIT_BUILDVM_OBJ_DIR ${LUAJIT_TEMP})
set(LUAJIT_BUILDVM_VMDEF_DIR ${LUAJIT_BUILDVM_OBJ_DIR}/jit)
set(LUAJIT_ALL_LIB
    ${LUAJIT_ROOT}/src/lib_base.c
    ${LUAJIT_ROOT}/src/lib_math.c
    ${LUAJIT_ROOT}/src/lib_bit.c
    ${LUAJIT_ROOT}/src/lib_string.c
    ${LUAJIT_ROOT}/src/lib_table.c
    #${LUAJIT_ROOT}/src/lib_io.c
    #${LUAJIT_ROOT}/src/lib_os.c
    ${LUAJIT_ROOT}/src/lib_package.c
    ${LUAJIT_ROOT}/src/lib_debug.c
    ${LUAJIT_ROOT}/src/lib_jit.c
    ${LUAJIT_ROOT}/src/lib_ffi.c
    ${LUAJIT_ROOT}/src/lib_buffer.c
    # custom
    ${CMAKE_CURRENT_LIST_DIR}/patch/lib_io.c
    ${CMAKE_CURRENT_LIST_DIR}/patch/lib_os.c
)

add_custom_command(
    OUTPUT ${LUAJIT_BUILDVM_OBJ_DIR}/lj_vm.obj
    COMMAND cmake -E make_directory ${LUAJIT_BUILDVM_OBJ_DIR}
    COMMAND $<TARGET_FILE:buildvm> -m peobj -o ${LUAJIT_BUILDVM_OBJ_DIR}/lj_vm.obj
    DEPENDS buildvm
    VERBATIM
)

add_custom_command(
    OUTPUT ${LUAJIT_BUILDVM_OBJ_DIR}/lj_bcdef.h
    COMMAND cmake -E make_directory ${LUAJIT_BUILDVM_OBJ_DIR}
    COMMAND $<TARGET_FILE:buildvm> -m bcdef -o ${LUAJIT_BUILDVM_OBJ_DIR}/lj_bcdef.h ${LUAJIT_ALL_LIB}
    DEPENDS buildvm
    VERBATIM
)

add_custom_command(
    OUTPUT ${LUAJIT_BUILDVM_OBJ_DIR}/lj_ffdef.h
    COMMAND cmake -E make_directory ${LUAJIT_BUILDVM_OBJ_DIR}
    COMMAND $<TARGET_FILE:buildvm> -m ffdef -o ${LUAJIT_BUILDVM_OBJ_DIR}/lj_ffdef.h ${LUAJIT_ALL_LIB}
    DEPENDS buildvm
    VERBATIM
)

add_custom_command(
    OUTPUT ${LUAJIT_BUILDVM_OBJ_DIR}/lj_libdef.h
    COMMAND cmake -E make_directory ${LUAJIT_BUILDVM_OBJ_DIR}
    COMMAND $<TARGET_FILE:buildvm> -m libdef -o ${LUAJIT_BUILDVM_OBJ_DIR}/lj_libdef.h ${LUAJIT_ALL_LIB}
    DEPENDS buildvm
    VERBATIM
)

add_custom_command(
    OUTPUT ${LUAJIT_BUILDVM_OBJ_DIR}/lj_recdef.h
    COMMAND cmake -E make_directory ${LUAJIT_BUILDVM_OBJ_DIR}
    COMMAND $<TARGET_FILE:buildvm> -m recdef -o ${LUAJIT_BUILDVM_OBJ_DIR}/lj_recdef.h ${LUAJIT_ALL_LIB}
    DEPENDS buildvm
    VERBATIM
)

add_custom_command(
    OUTPUT ${LUAJIT_BUILDVM_VMDEF_DIR}/vmdef.lua
    COMMAND cmake -E make_directory ${LUAJIT_BUILDVM_VMDEF_DIR}
    COMMAND $<TARGET_FILE:buildvm> -m vmdef -o ${LUAJIT_BUILDVM_VMDEF_DIR}/vmdef.lua ${LUAJIT_ALL_LIB}
    DEPENDS buildvm
    VERBATIM
)

add_custom_command(
    OUTPUT ${LUAJIT_BUILDVM_OBJ_DIR}/lj_folddef.h
    COMMAND cmake -E make_directory ${LUAJIT_BUILDVM_OBJ_DIR}
    COMMAND $<TARGET_FILE:buildvm> -m folddef -o ${LUAJIT_BUILDVM_OBJ_DIR}/lj_folddef.h ${LUAJIT_ROOT}/src/lj_opt_fold.c
    DEPENDS buildvm
    VERBATIM
)

# lib

set(LUAJIT_LIB_GEN
    ${LUAJIT_TEMP}/luajit.h
    ${LUAJIT_BUILDVM_OBJ_DIR}/lj_vm.obj
    ${LUAJIT_BUILDVM_OBJ_DIR}/lj_bcdef.h
    ${LUAJIT_BUILDVM_OBJ_DIR}/lj_ffdef.h
    ${LUAJIT_BUILDVM_OBJ_DIR}/lj_libdef.h
    ${LUAJIT_BUILDVM_OBJ_DIR}/lj_recdef.h
    ${LUAJIT_BUILDVM_OBJ_DIR}/lj_folddef.h
)

set(LUAJIT_LIB_SRC
    ${LUAJIT_ROOT}/src/lj_alloc.c
    ${LUAJIT_ROOT}/src/lj_api.c
    ${LUAJIT_ROOT}/src/lj_asm.c
    ${LUAJIT_ROOT}/src/lj_assert.c
    ${LUAJIT_ROOT}/src/lj_bc.c
    ${LUAJIT_ROOT}/src/lj_bcread.c
    ${LUAJIT_ROOT}/src/lj_bcwrite.c
    ${LUAJIT_ROOT}/src/lj_buf.c
    ${LUAJIT_ROOT}/src/lj_carith.c
    ${LUAJIT_ROOT}/src/lj_ccall.c
    ${LUAJIT_ROOT}/src/lj_ccallback.c
    ${LUAJIT_ROOT}/src/lj_cconv.c
    ${LUAJIT_ROOT}/src/lj_cdata.c
    ${LUAJIT_ROOT}/src/lj_char.c
    ${LUAJIT_ROOT}/src/lj_clib.c
    ${LUAJIT_ROOT}/src/lj_cparse.c
    ${LUAJIT_ROOT}/src/lj_crecord.c
    ${LUAJIT_ROOT}/src/lj_ctype.c
    ${LUAJIT_ROOT}/src/lj_debug.c
    ${LUAJIT_ROOT}/src/lj_dispatch.c
    ${LUAJIT_ROOT}/src/lj_err.c
    ${LUAJIT_ROOT}/src/lj_ffrecord.c
    ${LUAJIT_ROOT}/src/lj_func.c
    ${LUAJIT_ROOT}/src/lj_gc.c
    ${LUAJIT_ROOT}/src/lj_gdbjit.c
    ${LUAJIT_ROOT}/src/lj_ir.c
    ${LUAJIT_ROOT}/src/lj_lex.c
    ${LUAJIT_ROOT}/src/lj_lib.c
    ${LUAJIT_ROOT}/src/lj_load.c
    ${LUAJIT_ROOT}/src/lj_mcode.c
    ${LUAJIT_ROOT}/src/lj_meta.c
    ${LUAJIT_ROOT}/src/lj_obj.c
    ${LUAJIT_ROOT}/src/lj_opt_dce.c
    ${LUAJIT_ROOT}/src/lj_opt_fold.c
    ${LUAJIT_ROOT}/src/lj_opt_loop.c
    ${LUAJIT_ROOT}/src/lj_opt_mem.c
    ${LUAJIT_ROOT}/src/lj_opt_narrow.c
    ${LUAJIT_ROOT}/src/lj_opt_sink.c
    ${LUAJIT_ROOT}/src/lj_opt_split.c
    ${LUAJIT_ROOT}/src/lj_parse.c
    ${LUAJIT_ROOT}/src/lj_prng.c
    ${LUAJIT_ROOT}/src/lj_profile.c
    ${LUAJIT_ROOT}/src/lj_record.c
    ${LUAJIT_ROOT}/src/lj_serialize.c
    ${LUAJIT_ROOT}/src/lj_snap.c
    ${LUAJIT_ROOT}/src/lj_state.c
    ${LUAJIT_ROOT}/src/lj_str.c
    ${LUAJIT_ROOT}/src/lj_strfmt.c
    ${LUAJIT_ROOT}/src/lj_strfmt_num.c
    ${LUAJIT_ROOT}/src/lj_strscan.c
    ${LUAJIT_ROOT}/src/lj_tab.c
    ${LUAJIT_ROOT}/src/lj_trace.c
    ${LUAJIT_ROOT}/src/lj_udata.c
    ${LUAJIT_ROOT}/src/lj_vmevent.c
    ${LUAJIT_ROOT}/src/lj_vmmath.c
    ${LUAJIT_ROOT}/src/lib_aux.c
    ${LUAJIT_ROOT}/src/lib_base.c
    ${LUAJIT_ROOT}/src/lib_bit.c
    ${LUAJIT_ROOT}/src/lib_buffer.c
    ${LUAJIT_ROOT}/src/lib_debug.c
    ${LUAJIT_ROOT}/src/lib_ffi.c
    ${LUAJIT_ROOT}/src/lib_init.c
    #${LUAJIT_ROOT}/src/lib_io.c
    ${LUAJIT_ROOT}/src/lib_jit.c
    ${LUAJIT_ROOT}/src/lib_math.c
    #${LUAJIT_ROOT}/src/lib_os.c
    ${LUAJIT_ROOT}/src/lib_package.c
    ${LUAJIT_ROOT}/src/lib_string.c
    ${LUAJIT_ROOT}/src/lib_table.c
    # custom
    patch/lib_io.c
    patch/lib_os.c
    patch/lib_utf8.c
    patch/lib_strpack.c
    patch/lj_win32.c
)

add_library(lua51_static STATIC)
luajit_target_setup(lua51_static)
target_include_directories(lua51_static
PRIVATE
    ${LUAJIT_BUILDVM_OBJ_DIR}
PUBLIC
    ${LUAJIT_ROOT}/src
    ${LUAJIT_TEMP}
)
target_sources(lua51_static
PRIVATE
    ${LUAJIT_LIB_GEN}
    ${LUAJIT_LIB_SRC}
)

# dll

add_library(lua51 SHARED)
luajit_target_setup(lua51)
target_compile_definitions(lua51
PUBLIC
    LUA_BUILD_AS_DLL
)
target_include_directories(lua51
PRIVATE
    ${LUAJIT_BUILDVM_OBJ_DIR}
PUBLIC
    ${LUAJIT_ROOT}/src
    ${LUAJIT_TEMP}
)
target_sources(lua51
PRIVATE
    ${LUAJIT_LIB_GEN}
    ${LUAJIT_LIB_SRC}
)

# luajit

add_executable(luajit)
luajit_target_setup(luajit)
target_sources(luajit
PRIVATE
    ${LUAJIT_ROOT}/src/luajit.c
)
target_link_libraries(luajit
PRIVATE
    lua51
)

# install

include(GNUInstallDirs)

install(TARGETS lua51 luajit)

install(FILES
    ${LUAJIT_ROOT}/src/lua.h
    ${LUAJIT_ROOT}/src/lualib.h
    ${LUAJIT_ROOT}/src/lauxlib.h
    ${LUAJIT_ROOT}/src/luaconf.h
    ${LUAJIT_ROOT}/src/lua.hpp
    ${LUAJIT_TEMP}/luajit.h
    TYPE INCLUDE
)
