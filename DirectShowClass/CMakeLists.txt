aux_source_directory(src DSHOW_SRC)

add_library(strmbasd)

target_compile_options(strmbasd PRIVATE
    "/MP"
)

target_compile_definitions(strmbasd PRIVATE
    UNICODE
    _UNICODE
)

target_include_directories(strmbasd PUBLIC
    src
)

target_sources(strmbasd PRIVATE
    ${DSHOW_SRC}
)
