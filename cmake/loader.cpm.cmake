# CPM.cmake
# 建议给系统或用户设置一个环境变量 CPM_SOURCE_CACHE 指向一个目录
# 可以让多个项目共享包缓存

function(CPMInitialize)
    # SPDX-License-Identifier: MIT
    #
    # SPDX-FileCopyrightText: Copyright (c) 2019-2023 Lars Melchior and contributors

    set(CPM_DOWNLOAD_VERSION 0.40.0)
    set(CPM_HASH_SUM "7B354F3A5976C4626C876850C93944E52C83EC59A159AE5DE5BE7983F0E17A2A")

    if(CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    elseif(DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    else()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    endif()

    # Expand relative path. This is important if the provided path contains a tilde (~)
    get_filename_component(CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE)

    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION} EXPECTED_HASH SHA256=${CPM_HASH_SUM}
    )

    include(${CPM_DOWNLOAD_LOCATION})
endfunction()
