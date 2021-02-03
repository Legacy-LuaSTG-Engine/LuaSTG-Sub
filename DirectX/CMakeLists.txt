add_library(DirectXToolkit STATIC)

target_compile_options(DirectXToolkit PRIVATE
    "/MP"
)

target_compile_definitions(DirectXToolkit PRIVATE
    UNICODE
    _UNICODE
)

target_include_directories(DirectXToolkit PUBLIC
    .
)

target_sources(DirectXToolkit PRIVATE
    WICTextureLoader9.cpp
    WICTextureLoader9.h
)
