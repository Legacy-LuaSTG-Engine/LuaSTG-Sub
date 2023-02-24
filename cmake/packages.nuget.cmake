# Microsoft.XAudio2.Redist

CPMAddPackage(
    NAME Microsoft.XAudio2.Redist
    VERSION 1.2.11
    URL https://www.nuget.org/api/v2/package/Microsoft.XAudio2.Redist/1.2.11
    URL_HASH SHA256=4552E0B5B59DE0CDBC6C217261C45F5968F7BBF1E8AB5F208E4BCA6FD8FC5780
)

if(Microsoft.XAudio2.Redist_ADDED)
    set(XAUDIO2_PACKAGE_NAME Microsoft.XAudio2.Redist)
    set(XAUDIO2_PACKAGE_PATH ${Microsoft.XAudio2.Redist_SOURCE_DIR})
    add_library(${XAUDIO2_PACKAGE_NAME} SHARED IMPORTED GLOBAL)
    target_include_directories(${XAUDIO2_PACKAGE_NAME} INTERFACE
        ${XAUDIO2_PACKAGE_PATH}/build/native/include
    )
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set_target_properties(${XAUDIO2_PACKAGE_NAME} PROPERTIES
            IMPORTED_IMPLIB   ${XAUDIO2_PACKAGE_PATH}/build/native/release/lib/x64/xaudio2_9redist.lib
            IMPORTED_LOCATION ${XAUDIO2_PACKAGE_PATH}/build/native/release/bin/x64/xaudio2_9redist.dll
        )
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set_target_properties(${XAUDIO2_PACKAGE_NAME} PROPERTIES
            IMPORTED_IMPLIB   ${XAUDIO2_PACKAGE_PATH}/build/native/release/lib/x86/xaudio2_9redist.lib
            IMPORTED_LOCATION ${XAUDIO2_PACKAGE_PATH}/build/native/release/bin/x86/xaudio2_9redist.dll
        )
    else()
        message(FATAL_ERROR "unknown platform")
    endif()
    unset(XAUDIO2_PACKAGE_NAME)
    unset(XAUDIO2_PACKAGE_PATH)
endif()

# Microsoft.Windows.ImplementationLibrary

CPMAddPackage(
    NAME Microsoft.Windows.ImplementationLibrary
    VERSION 1.0.230202.1
    URL https://www.nuget.org/api/v2/package/Microsoft.Windows.ImplementationLibrary/1.0.230202.1
    URL_HASH SHA256=81E65673A427A2079F97EA6436A888A8F5E8E243F438C76B148BF98C7496A15C
)

if(Microsoft.Windows.ImplementationLibrary_ADDED)
    set(WIL_PACKAGE_NAME Microsoft.Windows.ImplementationLibrary)
    set(WIL_PACKAGE_PATH ${Microsoft.Windows.ImplementationLibrary_SOURCE_DIR})
    add_library(${WIL_PACKAGE_NAME} INTERFACE IMPORTED GLOBAL)
    target_include_directories(${WIL_PACKAGE_NAME} INTERFACE
        ${WIL_PACKAGE_PATH}/include
    )
    set(WIL_PACKAGE_NATVIS ${WIL_PACKAGE_PATH}/natvis/wil.natvis)
    source_group(TREE ${WIL_PACKAGE_PATH} FILES ${WIL_PACKAGE_NATVIS})
    target_sources(${WIL_PACKAGE_NAME} INTERFACE
        ${WIL_PACKAGE_NATVIS}
    )
    unset(WIL_PACKAGE_NAME)
    unset(WIL_PACKAGE_PATH)
endif()
