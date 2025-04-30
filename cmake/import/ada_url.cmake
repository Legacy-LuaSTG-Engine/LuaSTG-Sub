# ada_url

CPMAddPackage(
    NAME ada_url
    VERSION 3.2.4
    URL https://github.com/ada-url/ada/releases/download/v3.2.4/singleheader.zip
    URL_HASH SHA256=BD89FCF57C93E965E6E2488448AB9D1CF8005311808C563B288F921D987E4924
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
