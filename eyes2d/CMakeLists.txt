add_library(eyes2d STATIC)

set_target_properties(eyes2d PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_compile_options(eyes2d PRIVATE
    "/MP"
    "/utf-8"
)

target_include_directories(eyes2d PUBLIC
    .
)

target_compile_definitions(eyes2d PRIVATE
    UNICODE
    _UNICODE
)

target_sources(eyes2d PRIVATE
    E2DGlobal.hpp
    E2DException.hpp
    
    E2DLogSystem.hpp
    E2DLogSystem.cpp
    
    E2DMath.hpp
    E2DCodePage.hpp
    E2DFilePath.hpp
    
    E2DFileManager.hpp
    E2DFileManager.cpp
)

target_link_libraries(eyes2d PRIVATE
    fancylib
    zip
)
