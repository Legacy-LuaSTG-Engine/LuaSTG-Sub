# Microsoft.Windows.ImplementationLibrary

CPMAddPackage(
    NAME WIL
    VERSION 1.0.250325.1
    GITHUB_REPOSITORY microsoft/wil
    # 你妈的解压出来的的路径这么长，动不动就超出 Windows 路径长度限制
    #URL https://www.nuget.org/api/v2/package/Microsoft.Windows.ImplementationLibrary/1.0.231216.1
    #URL_HASH SHA256=5557B5C82D00DB3BD539AA886058475BE552EE09B538CEF87CBCAFF440BAF5E8
    DOWNLOAD_ONLY YES
)

if(WIL_ADDED)
    set(WIL_PACKAGE_NAME Microsoft.Windows.ImplementationLibrary)
    set(WIL_PACKAGE_PATH ${WIL_SOURCE_DIR})
    add_library(${WIL_PACKAGE_NAME} INTERFACE)
    target_include_directories(${WIL_PACKAGE_NAME} INTERFACE
        ${WIL_PACKAGE_PATH}/include
    )
    set(WIL_PACKAGE_NATVIS ${WIL_PACKAGE_PATH}/natvis/wil.natvis)
    source_group(TREE ${WIL_PACKAGE_PATH} FILES ${WIL_PACKAGE_NATVIS})
    target_sources(${WIL_PACKAGE_NAME} INTERFACE
        ${WIL_PACKAGE_NATVIS}
    )
    unset(WIL_PACKAGE_NATVIS)
    unset(WIL_PACKAGE_NAME)
    unset(WIL_PACKAGE_PATH)
else()
    message(FATAL_ERROR "Microsoft.Windows.ImplementationLibrary is requied")
endif()
