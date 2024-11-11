# tracy

CPMAddPackage(
    NAME tracy
    VERSION 0.11.1
    GITHUB_REPOSITORY wolfpld/tracy
    DOWNLOAD_ONLY YES
)

if(tracy_ADDED)
    add_library(tracy STATIC)
    luastg_target_common_options(tracy)
    if(LUASTG_LINK_TRACY_CLIENT)
        target_compile_definitions(tracy PUBLIC
            TRACY_ENABLE
        )
    endif()
    # WTF ???
    target_compile_definitions(tracy PUBLIC
        TracyFunction=__FUNCSIG__
    )
    target_include_directories(tracy PUBLIC
        ${tracy_SOURCE_DIR}/public
    )
    target_sources(tracy PRIVATE
        ${tracy_SOURCE_DIR}/public/tracy/Tracy.hpp
        ${tracy_SOURCE_DIR}/public/tracy/TracyD3D11.hpp
        ${tracy_SOURCE_DIR}/public/TracyClient.cpp
    )
    set_target_properties(tracy PROPERTIES FOLDER external)
endif()
