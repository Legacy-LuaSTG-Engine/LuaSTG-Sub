# 编译项目

建议通过 CMake 工作流（workflow）编译项目。

**编译 x86 32 位版本：**

> 注意：x86 32 位版本即将停止支持

```
cmake --workflow --preset windows-x86-release
```

**编译 x86 64 位版本：**

```
cmake --workflow --preset windows-amd64-release
```
