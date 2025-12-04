# Microsoft.DirectXTex

if (NOT Microsoft.DirectXTex_ADDED)
    message(FATAL_ERROR "Microsoft.DirectXTex is requied")
endif ()

# prepare

set(DirectXTex_source_directory  ${Microsoft.DirectXTex_SOURCE_DIR})
set(DirectXTex_build_directory   ${CMAKE_CURRENT_BINARY_DIR}/build/DirectXTex/$<CONFIG>)
set(DirectXTex_install_directory ${CMAKE_CURRENT_BINARY_DIR}/install/$<CONFIG>)
set(DirectXTex_library_file      ${DirectXTex_install_directory}/lib/DirectXTex.lib)

# external cmake build

add_custom_command(
    OUTPUT ${DirectXTex_library_file}
    COMMAND echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        # basic
        -S ${DirectXTex_source_directory}
        -B ${DirectXTex_build_directory}
        # generator
        ${LUASTG_CMAKE_EXTERNAL_BUILD_GENERATOR_OPTIONS}
        # crt
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
        # install
        -DCMAKE_INSTALL_PREFIX=${DirectXTex_install_directory}
        # options
        -DBUILD_TOOLS=OFF
        -DBUILD_SAMPLE=OFF
        -DBUILD_DX12=OFF
        -DBC_USE_OPENMP=OFF
    COMMAND ${CMAKE_COMMAND}
        --build ${DirectXTex_build_directory}
        --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
        --install ${DirectXTex_build_directory}
        --config $<CONFIG>
        --prefix ${DirectXTex_install_directory}
    VERBATIM
)
add_custom_target(DirectXTex_build ALL
    DEPENDS ${DirectXTex_library_file}
)
set_target_properties(DirectXTex_build PROPERTIES FOLDER external/DirectXTex)

# external cmake build clean

add_custom_target(DirectXTex_clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${DirectXTex_build_directory}
    COMMAND ${CMAKE_COMMAND} -E rm -f  ${DirectXTex_library_file}
    VERBATIM
)
set_target_properties(DirectXTex_clean PROPERTIES FOLDER external/DirectXTex)

# import

add_library(Microsoft_DirectXTex STATIC IMPORTED GLOBAL)
add_library(Microsoft::DirectXTex ALIAS Microsoft_DirectXTex)
target_include_directories(Microsoft_DirectXTex
INTERFACE
    ${DirectXTex_install_directory}/include
)
set_target_properties(Microsoft_DirectXTex PROPERTIES
    IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/DirectXTex.lib
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/DirectXTex.lib
)
add_dependencies(Microsoft_DirectXTex DirectXTex_build)
