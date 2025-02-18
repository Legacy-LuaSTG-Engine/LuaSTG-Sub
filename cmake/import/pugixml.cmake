# pugixml

CPMAddPackage(
    NAME pugixml
    VERSION 1.15
    GITHUB_REPOSITORY zeux/pugixml
    DOWNLOAD_ONLY YES
)

if(pugixml_ADDED)
    # pugixml 提供的 CMake 支持有点脏，我们自己来
    add_library(pugixml STATIC)
    target_include_directories(pugixml PUBLIC
        ${pugixml_SOURCE_DIR}/src
    )
    set(pugixml_src
        ${pugixml_SOURCE_DIR}/src/pugiconfig.hpp
        ${pugixml_SOURCE_DIR}/src/pugixml.hpp
        ${pugixml_SOURCE_DIR}/src/pugixml.cpp
    )
    source_group(TREE ${pugixml_SOURCE_DIR} FILES ${pugixml_src})
    set(pugixml_natvis
        ${pugixml_SOURCE_DIR}/scripts/natvis/pugixml.natvis
    )
    source_group(TREE ${pugixml_SOURCE_DIR}/scripts FILES ${pugixml_natvis})
    target_sources(pugixml PUBLIC
        ${pugixml_src}
        ${pugixml_natvis}
    )
    set_target_properties(pugixml PROPERTIES FOLDER external)
endif()
