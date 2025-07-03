# ada_url

CPMAddPackage(
    NAME ada_url
    VERSION 3.2.5
    URL https://github.com/ada-url/ada/releases/download/v3.2.5/singleheader.zip
    URL_HASH SHA256=245D7CC7CEE51C68542E3B251AA0608D875DA679CD1C466E49E5E761149F9C00
    DOWNLOAD_ONLY YES
)

if(ada_url_ADDED)
    add_library(ada_url STATIC)
    luastg_target_common_options(ada_url)
    target_include_directories(ada_url PUBLIC
        ${ada_url_SOURCE_DIR}
    )
    target_sources(ada_url PRIVATE
        ${ada_url_SOURCE_DIR}/ada.h
        ${ada_url_SOURCE_DIR}/ada.cpp
        ${ada_url_SOURCE_DIR}/ada_c.h
    )
    set_target_properties(ada_url PROPERTIES FOLDER external)
endif()
