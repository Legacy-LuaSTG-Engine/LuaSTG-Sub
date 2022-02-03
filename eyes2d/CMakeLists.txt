# legacy eyes2d

add_library(eyes2d STATIC)
luastg_target_common_options(eyes2d)
target_include_directories(eyes2d PUBLIC
    .
)
target_sources(eyes2d PRIVATE
    E2DGlobal.hpp
    E2DException.hpp
    
    E2DFilePath.hpp
    
    E2DFileManager.hpp
    E2DFileManager.cpp
)
target_link_libraries(eyes2d PRIVATE
    fancylib
    zip
    utility
)
