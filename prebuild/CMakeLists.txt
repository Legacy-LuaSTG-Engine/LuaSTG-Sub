# ---- zlib

add_library(zlib SHARED IMPORTED GLOBAL)
target_compile_definitions(zlib INTERFACE
    ZLIB_DLL
)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_include_directories(zlib INTERFACE
        amd64/include
    )
    set_target_properties(zlib PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/amd64/lib/zlib.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/amd64/bin/zlib.dll
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_include_directories(zlib INTERFACE
        x86/include
    )
    set_target_properties(zlib PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/x86/lib/zlib.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/x86/bin/zlib.dll
    )
else()
    message(FATAL_ERROR "unknown platform")
endif()

# ---- zip

add_library(zip SHARED IMPORTED GLOBAL)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_include_directories(zip INTERFACE
        amd64/include
    )
    set_target_properties(zip PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/amd64/lib/zip.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/amd64/bin/zip.dll
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_include_directories(zip INTERFACE
        x86/include
    )
    set_target_properties(zip PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/x86/lib/zip.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/x86/bin/zip.dll
    )
else()
    message(FATAL_ERROR "unknown platform")
endif()

# ---- freetype

#add_library(freetype SHARED IMPORTED GLOBAL)
#target_compile_definitions(freetype INTERFACE
#    DLL_IMPORT
#)
#if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#    target_include_directories(freetype INTERFACE
#        amd64/include
#        amd64/include/freetype2
#    )
#    set_target_properties(freetype PROPERTIES
#        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/amd64/lib/freetype.lib
#        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/amd64/bin/freetype.dll
#    )
#elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
#    target_include_directories(freetype INTERFACE
#        x86/include
#        x86/include/freetype2
#    )
#    set_target_properties(freetype PROPERTIES
#        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/x86/lib/freetype.lib
#        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/x86/bin/freetype.dll
#    )
#else()
#    message(FATAL_ERROR "unknown platform")
#endif()

# ---- ogg

#add_library(ogg SHARED IMPORTED GLOBAL)
#if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#    target_include_directories(ogg INTERFACE
#        amd64/include
#    )
#    set_target_properties(ogg PROPERTIES
#        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/amd64/lib/ogg.lib
#        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/amd64/bin/ogg.dll
#    )
#elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
#    target_include_directories(ogg INTERFACE
#        x86/include
#    )
#    set_target_properties(ogg PROPERTIES
#        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/x86/lib/ogg.lib
#        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/x86/bin/ogg.dll
#    )
#else()
#    message(FATAL_ERROR "unknown platform")
#endif()

# vorbis

#add_library(vorbis SHARED IMPORTED GLOBAL)
#if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#    target_include_directories(vorbis INTERFACE
#        amd64/include
#    )
#    set_target_properties(vorbis PROPERTIES
#        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/amd64/lib/vorbis.lib
#        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/amd64/bin/vorbis.dll
#    )
#elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
#    target_include_directories(vorbis INTERFACE
#        x86/include
#    )
#    set_target_properties(vorbis PROPERTIES
#        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/x86/lib/vorbis.lib
#        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/x86/bin/vorbis.dll
#    )
#else()
#    message(FATAL_ERROR "unknown platform")
#endif()

# vorbisfile

#add_library(vorbisfile SHARED IMPORTED GLOBAL)
#if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#    target_include_directories(vorbisfile INTERFACE
#        amd64/include
#    )
#    set_target_properties(vorbisfile PROPERTIES
#        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/amd64/lib/vorbisfile.lib
#        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/amd64/bin/vorbisfile.dll
#    )
#elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
#    target_include_directories(vorbisfile INTERFACE
#        x86/include
#    )
#    set_target_properties(vorbisfile PROPERTIES
#        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/x86/lib/vorbisfile.lib
#        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/x86/bin/vorbisfile.dll
#    )
#else()
#    message(FATAL_ERROR "unknown platform")
#endif()

# ---- luajit

add_library(luajit SHARED IMPORTED GLOBAL)
target_compile_definitions(luajit INTERFACE
    LUA_BUILD_AS_DLL
)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_include_directories(luajit INTERFACE
        amd64/include
    )
    set_target_properties(luajit PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/amd64/lib/lua51.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/amd64/bin/lua51.dll
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_include_directories(luajit INTERFACE
        x86/include
    )
    set_target_properties(luajit PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/x86/lib/lua51.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/x86/bin/lua51.dll
    )
else()
    message(FATAL_ERROR "unknown platform")
endif()
