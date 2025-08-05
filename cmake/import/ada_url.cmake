# ada_url

CPMAddPackage(
    NAME ada_url
    VERSION 3.2.7
    URL https://github.com/ada-url/ada/releases/download/v3.2.7/singleheader.zip
    URL_HASH SHA256=464B59BE356C2CB176E3582D1EBF3D353C076AE5821A299E87B9AE8F691CDDBE
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
