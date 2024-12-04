set(LUASTG_CMAKE_IMPORT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/cpm.cmake)

if(LUASTG_LINK_YY_THUNKS)
    include(${CMAKE_CURRENT_LIST_DIR}/yy_thunks.cmake)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/spdlog.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/tracy.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/pugixml.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/stb.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/dr_libs.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/tinygltf.cmake) # depends json, stb
include(${CMAKE_CURRENT_LIST_DIR}/tinyobjloader.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/pcg.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/xxhash.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/simdutf.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/wil.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/directx_tk.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/directx_tex.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/xaudio2.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/d3dcompiler.cmake)
