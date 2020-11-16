cmake_minimum_required(VERSION 3.15)

aux_source_directory(src DSHOW_SRC)

add_library(strmbasd)

target_compile_options(strmbasd PRIVATE
    "/MP4"
)

target_include_directories(strmbasd PRIVATE
    src
)

target_compile_definitions(strmbasd PRIVATE
    UNICODE
    _UNICODE
)

target_sources(strmbasd PRIVATE
    ${DSHOW_SRC}
)
