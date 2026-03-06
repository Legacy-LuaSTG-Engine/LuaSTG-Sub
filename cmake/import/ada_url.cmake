# ada_url

CPMAddPackage(
    NAME ada_url
    VERSION 3.3.0
    URL https://github.com/ada-url/ada/releases/download/v3.3.0/singleheader.zip
    URL_HASH SHA256=127994BE3CE79A0416FD5BCDBB4B10CEE84839C4EC8251E9C52AA8E6C6E16762
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
