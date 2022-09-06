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

# download xaudio2.redist nuget package

utility_download_package(XAudio2Redist
    https://www.nuget.org/api/v2/package/Microsoft.XAudio2.Redist/1.2.9
    microsoft.xaudio2.redist.1.2.9.nupkg
    A02332CB8D4096C29430BE0FDB6A079E8F4A29781623AE362A811FD5DC015BB5
)
