# wrapper

function(utility_download_package dir url filename sha256)
    # create download cache directory
    set(_DOWNLOAD_CACHE_DIR ${CMAKE_CURRENT_LIST_DIR}/.download)
    file(MAKE_DIRECTORY ${_DOWNLOAD_CACHE_DIR})
    # check package hash
    if(EXISTS ${_DOWNLOAD_CACHE_DIR}/${filename})
        file(SHA256 ${_DOWNLOAD_CACHE_DIR}/${filename} _CURRENT_FILE_SHA256)
        string(TOUPPER ${_CURRENT_FILE_SHA256} _CURRENT_FILE_SHA256)
        string(TOUPPER ${sha256} _FILE_SHA256)
        if(NOT (${_CURRENT_FILE_SHA256} STREQUAL ${_FILE_SHA256}))
            file(REMOVE ${_DOWNLOAD_CACHE_DIR}/${filename})
            message(WARNING "file is outdated/corrupted:" ${_DOWNLOAD_CACHE_DIR}/${filename})
        endif()
        unset(_CURRENT_FILE_SHA256)
        unset(_FILE_SHA256)
    endif()
    # fetch package
    if(NOT EXISTS ${_DOWNLOAD_CACHE_DIR}/${filename})
        # download package
        file(DOWNLOAD
            ${url}
            ${_DOWNLOAD_CACHE_DIR}/${filename}
            EXPECTED_HASH SHA256=${sha256}
            SHOW_PROGRESS
        )
    endif()
    # extract package
    if(EXISTS ${_DOWNLOAD_CACHE_DIR}/${filename})
        file(REMOVE_RECURSE ${CMAKE_CURRENT_LIST_DIR}/${dir})
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${dir})
        file(ARCHIVE_EXTRACT
            INPUT ${_DOWNLOAD_CACHE_DIR}/${filename}
            DESTINATION ${CMAKE_CURRENT_LIST_DIR}/${dir}
        )
    endif()
endfunction()

# Microsoft.XAudio2.Redist

set(XAUDIO2_PACKAGE Microsoft.XAudio2.Redist)

utility_download_package(${XAUDIO2_PACKAGE}
    https://www.nuget.org/api/v2/package/Microsoft.XAudio2.Redist/1.2.11
    microsoft.xaudio2.redist.1.2.11.nupkg
    4552E0B5B59DE0CDBC6C217261C45F5968F7BBF1E8AB5F208E4BCA6FD8FC5780
)

add_library(${XAUDIO2_PACKAGE} SHARED IMPORTED GLOBAL)
target_include_directories(${XAUDIO2_PACKAGE} INTERFACE
    ${XAUDIO2_PACKAGE}/build/native/include
)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set_target_properties(${XAUDIO2_PACKAGE} PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/${XAUDIO2_PACKAGE}/build/native/release/lib/x64/xaudio2_9redist.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/${XAUDIO2_PACKAGE}/build/native/release/bin/x64/xaudio2_9redist.dll
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set_target_properties(${XAUDIO2_PACKAGE} PROPERTIES
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_LIST_DIR}/${XAUDIO2_PACKAGE}/build/native/release/lib/x86/xaudio2_9redist.lib
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/${XAUDIO2_PACKAGE}/build/native/release/bin/x86/xaudio2_9redist.dll
    )
else()
    message(FATAL_ERROR "unknown platform")
endif()

# Microsoft.Windows.ImplementationLibrary

set(WIL_PACKAGE Microsoft.Windows.ImplementationLibrary)

utility_download_package(${WIL_PACKAGE}
    https://www.nuget.org/api/v2/package/Microsoft.Windows.ImplementationLibrary/1.0.230202.1
    microsoft.windows.implementationlibrary.1.0.230202.1.nupkg
    81E65673A427A2079F97EA6436A888A8F5E8E243F438C76B148BF98C7496A15C
)

add_library(${WIL_PACKAGE} INTERFACE IMPORTED GLOBAL)
target_include_directories(${WIL_PACKAGE} INTERFACE
    ${WIL_PACKAGE}/include
)
set(WIL_PACKAGE_NATVIS ${WIL_PACKAGE}/natvis/wil.natvis)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/${WIL_PACKAGE} FILES ${WIL_PACKAGE_NATVIS})
target_sources(${WIL_PACKAGE} INTERFACE
    ${WIL_PACKAGE_NATVIS}
)
