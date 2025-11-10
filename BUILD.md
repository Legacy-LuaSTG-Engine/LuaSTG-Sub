# Build Projects

It is recommended to compile the project with CMake preset and workflow.

## Environment Requirement

* Visual Studio 2022 (17.14)  
* Windows SDK 10.0.26100  
* CMake 3.31  

## x86 64-bit

CMake configuration:  

```shell
cmake --preset vs2022-amd64
```

Build (Debug or Release profile): 

```shell
cmake --build --preset windows-amd64-debug
cmake --build --preset windows-amd64-release
```

Release (configuration and build Release profile):  

```shell
cmake --workflow --preset windows-amd64-release
```

## x86 32-bit (Deprecated)

> WARNING: Support for x86 32-bit version is deprecated  

CMake configuration:  

```shell
cmake --preset vs2022-x86
```

Build (Debug or Release profile): 

```shell
cmake --build --preset windows-x86-debug
cmake --build --preset windows-x86-release
```

Release (configuration and build Release profile):  

```shell
cmake --workflow --preset windows-x86-release
```

## arm 64-bit (TODO)

> TODO: Waiting for me to get a Windows on arm laptop  

## Start Developing

Next, you can use Visual Studio 2022 to open the sln solution file and start developing.

Note: Please compile the LuaSTG target at least once, otherwise the header files of dependency libraries such as libflac, libogg, libvorbis, zlib ng, minizip ng, etc. do not exist.

## Steam API Support  

If you need to use the Steam API, please follow these steps:  

1. Download the Steamworks SDK from the official Steamworks website  
    1. Open the website: https://partner.steamgames.com/doc/sdk  
    2. Login to your Steamworks account  
    3. Download the latest version of the Steamworks SDK (as of November 11, 2025, the latest version is `1.62`): https://partner.steamgames.com/downloads/list  
2. Extract the Steamworks SDK  
    1. After downloading, you will receive a compressed file named `steamworks_sdk_xyy.zip`, where `xyy` represents the version number (as of November 11, 2025, the latest version number is `162`)  
    2. Open the ZIP archive using any software that supports `zip` files; you will see an `sdk` folder inside  
    3. Extract the `sdk` folder from the archive into directory `external/steam_api/SteamworksSDK`  
3. Enable Steam API in CMake  
    1. Set the CMake option `LUASTG_STEAM_API_ENABLE` to `TRUE`  
    2. Set the CMake option `LUASTG_STEAM_API_APP_ID` to your game’s AppID  
    3. To prevent players from launching the `exe` directly by double-clicking (which may cause Steam API initialization to fail), set the CMake option `LUASTG_STEAM_API_FORCE_LAUNCH_BY_STEAM` to `TRUE`  
4. Re-run CMake configuration (CMake Configure)  

---

# 编译项目

建议通过 CMake 预配（preset）和工作流（workflow）编译项目。

## 编译环境

* Visual Studio 2022 (17.14)  
* Windows SDK 10.0.26100  
* CMake 3.31  

## x86 64 位版本

CMake 配置：  

```shell
cmake --preset vs2022-amd64
```

编译：  

```shell
cmake --build --preset windows-amd64-debug
cmake --build --preset windows-amd64-release
```

一键发行（CMake 配置、编译发行版）：  

```shell
cmake --workflow --preset windows-amd64-release
```

## x86 32 位版本（已弃用）

> 警告：x86 32 位版本即将停止支持

CMake 配置：  

```shell
cmake --preset vs2022-x86
```

编译（调试版或发行版）：  

```shell
cmake --build --preset windows-x86-debug
cmake --build --preset windows-x86-release
```

一键发行（CMake 配置、编译发行版）：  

```shell
cmake --workflow --preset windows-x86-release
```

## arm64 版本（未完成）  

> 未完成：等我买一台 Windows on arm 笔记本  

## 开始编写代码

接下来你就可以用 Visual Studio 2022 打开 sln 解决方案开始开发。

注意：请编译 LuaSTG 项目至少一次，否则 libflac、libogg、libvorbis、zlib-ng、minizip-ng 等依赖库的头文件不存在。

## Steam API 支持  

如果你需要使用 Steam API，请遵循以下步骤：  

1. 从 Steamworks 官网下载 Steamworks SDK  
    1. 打开网站：https://partner.steamgames.com/doc/sdk  
    2. 登录你的 Steamworks 账号  
    3. 下载最新版本的 Steamworks SDK（截至 2025 年 11 月 11 日，最新版本为 `1.62`）：https://partner.steamgames.com/downloads/list  
2. 解压 Steamworks SDK  
    1. 下载完成后会得到 `steamworks_sdk_xyy.zip` 压缩包，其中 `xyy` 是版本号（截至 2025 年 11 月 11 日，最新版本号为 `162`）  
    2. 用任意支持 `zip` 压缩包的软件打开压缩包，可以看到压缩包中有一个 `sdk` 文件夹  
    3. 将压缩包中的 `sdk` 文件夹解压到 `external/steam_api/SteamworksSDK` 文件夹中  
3. 在 CMake 中启用 Steam API  
    1. 将 CMake 选项 `LUASTG_STEAM_API_ENABLE` 设置为 `TRUE`  
    2. 将 CMake 选项 `LUASTG_STEAM_API_APP_ID` 修改为你的 AppID
    3. 为了避免玩家直接双击 `exe` 启动造成 Steam API 初始化失败，可以将 CMake 选项 `LUASTG_STEAM_API_FORCE_LAUNCH_BY_STEAM` 设置为 `TRUE`  
4. 重新运行一次 CMake 配置（CMake Configure）  
