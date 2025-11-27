# Microsoft.DirectXTex

CPMAddPackage(
    NAME Microsoft.DirectXTex
    VERSION 2024.10.1
    GITHUB_REPOSITORY microsoft/DirectXTex
    GIT_TAG oct2024
    DOWNLOAD_ONLY YES
)

if(Microsoft.DirectXTex_ADDED)
    set(DirectXTex_DIR ${Microsoft.DirectXTex_SOURCE_DIR})
    add_library(DirectXTexMini STATIC)
    luastg_target_common_options(DirectXTexMini)
    luastg_target_platform_windows_7(DirectXTexMini)
    target_include_directories(DirectXTexMini PUBLIC
        ${DirectXTex_DIR}
        ${DirectXTex_DIR}/ScreenGrab
        ${DirectXTex_DIR}/WICTextureLoader
        ${DirectXTex_DIR}/DDSTextureLoader
    )
    target_sources(DirectXTexMini PRIVATE
        ${DirectXTex_DIR}/ScreenGrab/ScreenGrab11.h
        ${DirectXTex_DIR}/ScreenGrab/ScreenGrab11.cpp
        ${DirectXTex_DIR}/WICTextureLoader/WICTextureLoader11.h
        ${DirectXTex_DIR}/WICTextureLoader/WICTextureLoader11.cpp
        ${DirectXTex_DIR}/DDSTextureLoader/DDSTextureLoader11.h
        ${DirectXTex_DIR}/DDSTextureLoader/DDSTextureLoader11.cpp
    )
    target_link_libraries(DirectXTexMini PUBLIC
        dxguid.lib
    )

    add_library(DirectX::TextureMini ALIAS DirectXTexMini)
    set_target_properties(DirectXTexMini PROPERTIES FOLDER external)

    # external build

    set(DirectXTex_source_directory  ${Microsoft.DirectXTex_SOURCE_DIR})
    set(DirectXTex_build_directory   ${CMAKE_CURRENT_BINARY_DIR}/DirectXTex/$<CONFIG>)
    set(DirectXTex_install_directory ${CMAKE_CURRENT_BINARY_DIR}/install/$<CONFIG>)
    set(DirectXTex_library           ${DirectXTex_install_directory}/lib/DirectXTex.lib)

    add_custom_command(
        OUTPUT ${DirectXTex_library}
        COMMAND echo ${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_GENERATOR_TOOLSET} $<CONFIG>
        COMMAND ${CMAKE_COMMAND}
            # basic
            -S ${DirectXTex_source_directory}
            -B ${DirectXTex_build_directory}
            # generator
            -G ${CMAKE_GENERATOR}
            $<$<BOOL:${CMAKE_GENERATOR_PLATFORM}>:-A> ${CMAKE_GENERATOR_PLATFORM}
            $<$<BOOL:${CMAKE_GENERATOR_TOOLSET}>:-T> ${CMAKE_GENERATOR_TOOLSET}
            # install
            -DCMAKE_INSTALL_PREFIX=${DirectXTex_install_directory}
            # crt
            -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
            -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
            # win7
            "-DCMAKE_C_FLAGS=/D_WIN32_WINNT=0x0601 /DWINVER=0x0601"
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
        DEPENDS ${DirectXTex_library}
    )
    add_custom_target(DirectXTex_clean
        COMMAND cmake -E rm -rf ${DirectXTex_build_directory}
        COMMAND cmake -E rm -f  ${DirectXTex_library}
        VERBATIM
    )
    set_target_properties(DirectXTex_build PROPERTIES FOLDER external)
    set_target_properties(DirectXTex_clean PROPERTIES FOLDER external)

    # fuck cmake

    file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/include/placeholder)
    file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/install/Release/include/placeholder)
    file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/install/RelWithDebInfo/include/placeholder)
    file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/install/MinSizeRel/include/placeholder)

    # import

    add_library(DirectXTex_target STATIC IMPORTED GLOBAL)
    add_library(Microsoft::DirectXTex ALIAS DirectXTex_target)
    target_include_directories(DirectXTex_target
    INTERFACE
        ${DirectXTex_install_directory}/include
    )
    set_target_properties(DirectXTex_target PROPERTIES
        IMPORTED_LOCATION       ${CMAKE_CURRENT_BINARY_DIR}/install/Release/lib/DirectXTex.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/install/Debug/lib/DirectXTex.lib
    )
    add_dependencies(DirectXTex_target DirectXTex_build)
else()
    message(FATAL_ERROR "Microsoft.DirectXTex is requied")
endif()
