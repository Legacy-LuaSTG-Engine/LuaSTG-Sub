# Build Projects

It is recommended to compile the project with CMake preset and workflow.

## Environment Requirement

* Visual Studio 2022 (17.10)  
* Windows SDK 10.0.26100.0  
* CMake 3.30  

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

---

# 编译项目

建议通过 CMake 预配（preset）和工作流（workflow）编译项目。

## 编译环境

* Visual Studio 2022 (17.10)  
* Windows SDK 10.0.26100.0  
* CMake 3.30  

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
