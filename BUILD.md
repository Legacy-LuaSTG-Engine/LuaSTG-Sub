# 编译项目

建议通过 CMake 预配（preset）和工作流（workflow）编译项目。

## x86 32 位版本

> 注意：x86 32 位版本即将停止支持

### 仅配置

```
cmake vs2022-x86
```

### 仅编译

```
cmake --build --preset windows-x86-debug
cmake --build --preset windows-x86-release
```

### 编译发行版

```
cmake --workflow --preset windows-x86-release
```

## x86 64 位版本

### 仅配置

```
cmake vs2022-amd64
```

### 仅编译

```
cmake --build --preset windows-amd64-debug
cmake --build --preset windows-amd64-release
```

### 编译发行版

```
cmake --workflow --preset windows-amd64-release
```
