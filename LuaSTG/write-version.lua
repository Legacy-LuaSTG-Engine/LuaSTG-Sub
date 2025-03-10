
local version = {
    major = 0,
    minor = 21,
    patch = 101,
}

local utf8_bom = "\xEF\xBB\xBF"

--------------------------------------------------------------------------------

-- 原始的 rc 文件
--[[
// Microsoft Visual C++ generated resource script.
//
#include "resource.h"

#define APSTUDIO_READONLY_SYMBOLS
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 2 resource.
//
#include "winres.h"

/////////////////////////////////////////////////////////////////////////////
#undef APSTUDIO_READONLY_SYMBOLS

/////////////////////////////////////////////////////////////////////////////
// resources

#if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_CHS)
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US

#ifdef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// TEXTINCLUDE
//

1 TEXTINCLUDE 
BEGIN
    "resource.h\0"
END

2 TEXTINCLUDE 
BEGIN
    "#include ""winres.h""\r\n"
    "\0"
END

3 TEXTINCLUDE 
BEGIN
    "\r\n"
    "\0"
END

#endif    // APSTUDIO_INVOKED


/////////////////////////////////////////////////////////////////////////////
//
// Version
//

VS_VERSION_INFO VERSIONINFO
    FILEVERSION 1,0,0,0
    PRODUCTVERSION 1,0,0,0
    FILEFLAGSMASK 0x3fL
#ifdef _DEBUG
    FILEFLAGS 0x1L
#else
    FILEFLAGS 0x0L
#endif
    FILEOS 0x40004L
    FILETYPE 0x1L
    FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "000004b0"
        BEGIN
            VALUE "CompanyName", "璀境石"
            VALUE "FileDescription", "LuaSTG Sub"
            VALUE "FileVersion", "1.0.0.0"
            VALUE "InternalName", "LuaSTG Sub"
            VALUE "LegalCopyright", "Copyright 2020-2025 璀境石"
            VALUE "OriginalFilename", "LuaSTGSub.exe"
            VALUE "ProductName", "LuaSTG Sub"
            VALUE "ProductVersion", "1.0.0.0"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x0, 1200
    END
END


/////////////////////////////////////////////////////////////////////////////
//
// Icon
//

// Icon with lowest ID value placed first to ensure application icon
// remains consistent on all systems.
IDI_APPICON             ICON                    "app.ico"

#endif    // resources
/////////////////////////////////////////////////////////////////////////////



#ifndef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 3 resource.
//


/////////////////////////////////////////////////////////////////////////////
#endif    // not APSTUDIO_INVOKED
]]

local rc_fmt = [[// Microsoft Visual C++ generated resource script.
//
#include "resource.h"

#define APSTUDIO_READONLY_SYMBOLS
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 2 resource.
//
#include "winres.h"

/////////////////////////////////////////////////////////////////////////////
#undef APSTUDIO_READONLY_SYMBOLS

/////////////////////////////////////////////////////////////////////////////
// resources

#if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_CHS)
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US

#ifdef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// TEXTINCLUDE
//

1 TEXTINCLUDE 
BEGIN
    "resource.h\0"
END

2 TEXTINCLUDE 
BEGIN
    "#include ""winres.h""\r\n"
    "\0"
END

3 TEXTINCLUDE 
BEGIN
    "\r\n"
    "\0"
END

#endif    // APSTUDIO_INVOKED


/////////////////////////////////////////////////////////////////////////////
//
// Version
//

VS_VERSION_INFO VERSIONINFO
    FILEVERSION %s
    PRODUCTVERSION %s
    FILEFLAGSMASK 0x3fL
#ifdef _DEBUG
    FILEFLAGS 0x1L
#else
    FILEFLAGS 0x0L
#endif
    FILEOS 0x40004L
    FILETYPE 0x1L
    FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "000004b0"
        BEGIN
            VALUE "CompanyName", "璀境石"
            VALUE "FileDescription", "LuaSTG Sub"
            VALUE "FileVersion", "%s"
            VALUE "InternalName", "LuaSTG Sub"
            VALUE "LegalCopyright", "Copyright 2020-2025 璀境石"
            VALUE "OriginalFilename", "LuaSTGSub.exe"
            VALUE "ProductName", "LuaSTG Sub"
            VALUE "ProductVersion", "%s"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x0, 1200
    END
END


/////////////////////////////////////////////////////////////////////////////
//
// Icon
//

// Icon with lowest ID value placed first to ensure application icon
// remains consistent on all systems.
IDI_APPICON             ICON                    "app.ico"

#endif    // resources
/////////////////////////////////////////////////////////////////////////////



#ifndef APSTUDIO_INVOKED
/////////////////////////////////////////////////////////////////////////////
//
// Generated from the TEXTINCLUDE 3 resource.
//


/////////////////////////////////////////////////////////////////////////////
#endif    // not APSTUDIO_INVOKED
]]

local function write_rc()
    local v_str_1 = string.format("%d,%d,%d,0", version.major, version.minor, version.patch)
    local v_str_2 = string.format("%d.%d.%d.0", version.major, version.minor, version.patch)
    local rc_str = string.format(rc_fmt, v_str_1, v_str_1, v_str_2, v_str_2)
    ---@type file*
    local f = assert(io.open("LuaSTG/Custom/resource.rc", "wb"))
    f:write(utf8_bom)
    f:write(rc_str)
    f:close()
end

--------------------------------------------------------------------------------

-- 原始的 manifest 文件
--[[
<?xml version="1.0" encoding="utf-8"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0" xmlns:asmv3="urn:schemas-microsoft-com:asm.v3">
    <assemblyIdentity type="win32" name="ChuiKingShek.LuaSTG.Sub" version="1.0.0.0"/>
    <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
        <security>
            <requestedPrivileges>
                <requestedExecutionLevel level="asInvoker" uiAccess="false"/>
            </requestedPrivileges>
        </security>
    </trustInfo>
    <dependency>
        <dependentAssembly>
            <assemblyIdentity
                type="win32"
                name="Microsoft.Windows.Common-Controls"
                version="6.0.0.0"
                processorArchitecture="*"
                publicKeyToken="6595b64144ccf1df"
                language="*"
            />
        </dependentAssembly>
    </dependency>
    <compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1">
        <application>
            <!-- Windows 10/11 -->
            <supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"/>
            <!-- Windows 8.1 -->
            <supportedOS Id="{1f676c76-80e1-4239-95bb-83d0f6d0da78}"/>
            <!-- Windows 8 -->
            <supportedOS Id="{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}"/>
            <!-- Windows 7 -->
            <supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}"/>
        </application>
    </compatibility>
    <asmv3:application>
        <asmv3:windowsSettings>
            <dpiAware xmlns="http://schemas.microsoft.com/SMI/2005/WindowsSettings">true/pm</dpiAware>
            <dpiAwareness xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">PerMonitorV2, PerMonitor</dpiAwareness>
            <longPathAware xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">true</longPathAware>
            <activeCodePage xmlns="http://schemas.microsoft.com/SMI/2019/WindowsSettings">UTF-8</activeCodePage>
        </asmv3:windowsSettings>
    </asmv3:application>
</assembly>
]]

local manifest_fmt = [[<?xml version="1.0" encoding="utf-8"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0" xmlns:asmv3="urn:schemas-microsoft-com:asm.v3">
    <assemblyIdentity type="win32" name="ChuiKingShek.LuaSTG.Sub" version="%s"/>
    <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
        <security>
            <requestedPrivileges>
                <requestedExecutionLevel level="asInvoker" uiAccess="false"/>
            </requestedPrivileges>
        </security>
    </trustInfo>
    <dependency>
        <dependentAssembly>
            <assemblyIdentity
                type="win32"
                name="Microsoft.Windows.Common-Controls"
                version="6.0.0.0"
                processorArchitecture="*"
                publicKeyToken="6595b64144ccf1df"
                language="*"
            />
        </dependentAssembly>
    </dependency>
    <compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1">
        <application>
            <!-- Windows 10/11 -->
            <supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"/>
            <!-- Windows 8.1 -->
            <supportedOS Id="{1f676c76-80e1-4239-95bb-83d0f6d0da78}"/>
            <!-- Windows 8 -->
            <supportedOS Id="{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}"/>
            <!-- Windows 7 -->
            <supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}"/>
        </application>
    </compatibility>
    <asmv3:application>
        <asmv3:windowsSettings>
            <dpiAware xmlns="http://schemas.microsoft.com/SMI/2005/WindowsSettings">true/pm</dpiAware>
            <dpiAwareness xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">PerMonitorV2, PerMonitor</dpiAwareness>
            <longPathAware xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">true</longPathAware>
            <activeCodePage xmlns="http://schemas.microsoft.com/SMI/2019/WindowsSettings">UTF-8</activeCodePage>
        </asmv3:windowsSettings>
    </asmv3:application>
</assembly>
]]

local function write_manifest()
    local v_str_2 = string.format("%d.%d.%d.0", version.major, version.minor, version.patch)
    local manifest_str = string.format(manifest_fmt, v_str_2)
    ---@type file*
    local f = assert(io.open("LuaSTG/LuaSTG.manifest", "wb"))
    f:write(manifest_str)
    f:close()
end

--------------------------------------------------------------------------------

-- 原始的 LConfig.h 文件
--[[
#pragma once

#define LUASTG_NAME          "LuaSTG"
#define LUASTG_BRANCH        "Sub"
#define LUASTG_VERSION_NAME  "v0.0.1"
#define LUASTG_VERSION_MAJOR 0
#define LUASTG_VERSION_MINOR 0
#define LUASTG_VERSION_PATCH 1

#define LUASTG_INFO LUASTG_NAME " " LUASTG_BRANCH " " LUASTG_VERSION_NAME

// WARNING: see write-version.lua

//#define LuaSTG_enable_GameObjectManager_Debug 1

//#define LDEVVERSION 1
]]

local h_fmt = [[#pragma once

#define LUASTG_NAME          "LuaSTG"
#define LUASTG_BRANCH        "Sub"
#define LUASTG_VERSION_NAME  "v%d.%d.%d"
#define LUASTG_VERSION_MAJOR %d
#define LUASTG_VERSION_MINOR %d
#define LUASTG_VERSION_PATCH %d

#define LUASTG_INFO LUASTG_NAME " " LUASTG_BRANCH " " LUASTG_VERSION_NAME

// WARNING: see write-version.lua

//#define LuaSTG_enable_GameObjectManager_Debug 1

//#define LDEVVERSION 1
]]

local function write_h()
    local h_str = string.format(h_fmt,
        version.major, version.minor, version.patch,
        version.major, version.minor, version.patch)
    ---@type file*
    local f = assert(io.open("LuaSTG/LConfig.h", "wb"))
    --f:write(utf8_bom) -- 大概不需要？
    f:write(h_str)
    f:close()
end

--------------------------------------------------------------------------------

if arg then
    -- 假设 arg 这个全局变量是独立 lua 解释器特有的，如果它存在，说明这是命令行环境
    write_rc()
    write_manifest()
    write_h()
    print("[I] write 3 files")
end

return version
