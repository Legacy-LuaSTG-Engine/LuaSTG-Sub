# sqlite3

CPMAddPackage(
    NAME sqlite3
    VERSION 3.47.0
    URL https://sqlite.org/2024/sqlite-amalgamation-3470000.zip
    URL_HASH SHA256=2842FDDBB1CC33F66C7DA998A57535F14A6BFEE159676A07BB4BF3E59375D93E
    DOWNLOAD_ONLY YES
)

if (sqlite3_ADDED)
    set(sqlite3_root ${sqlite3_SOURCE_DIR})

    add_library(sqlite3 STATIC)
    if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(sqlite3 PRIVATE "/utf-8")
    endif ()
    target_compile_features(sqlite3 PRIVATE c_std_17)
    target_include_directories(sqlite3 PUBLIC ${sqlite3_root})
    target_sources(sqlite3 PRIVATE ${sqlite3_root}/sqlite3.h ${sqlite3_root}/sqlite3.c)
    set_target_properties(sqlite3 PROPERTIES OUTPUT_NAME "libsqlite3")

    add_executable(sqlite3_cli)
    if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(sqlite3_cli PRIVATE "/utf-8")
    endif ()
    target_compile_features(sqlite3_cli PRIVATE c_std_17)
    target_sources(sqlite3_cli PRIVATE ${sqlite3_root}/shell.c)
    target_link_libraries(sqlite3_cli PRIVATE sqlite3)
    set_target_properties(sqlite3_cli PROPERTIES OUTPUT_NAME "sqlite3")

    add_custom_command(TARGET sqlite3_cli POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${CMAKE_BINARY_DIR}/bin
        COMMAND ${CMAKE_COMMAND} ARGS -E copy $<TARGET_FILE:sqlite3_cli> ${CMAKE_BINARY_DIR}/bin
    )

    set_target_properties(sqlite3 PROPERTIES FOLDER external)
    set_target_properties(sqlite3_cli PROPERTIES FOLDER external)
endif ()
