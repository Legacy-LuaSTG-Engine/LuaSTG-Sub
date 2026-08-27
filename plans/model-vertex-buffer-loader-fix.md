# Model 顶点缓冲加载器修复 —— getBufferFromAccessor 健壮化与 spec 合规

> 目标：修复 `engine/graphics/core/Graphics/Model_D3D11.cpp` 中顶点属性加载
> `getBufferFromAccessor`（行 819-881）及其 4 个调用点（POSITION/NORMAL/COLOR_0/TEXCOORD_0）
> 的越界崩溃、无边界校验、normalized 整型格式错位等缺陷，
> 对齐 glTF 2.0 规范（`C:\Users\Alpha\Documents\glTF\specification\2.0`）的
> Data Access / Data Alignment / Sparse Accessors 章节。
>
> 关联文档：`reviews/model.md`（P0#七.1）、`plans/model-v2.md`（M3 顶点越界待办、§5.3/§5.9）
> 状态：设计稿 / 待实现

---

## 一、背景

### 目标函数
```cpp
// Model_D3D11.cpp:819
static bool getBufferFromAccessor(
    tinygltf::Model const& model,
    tinygltf::Accessor const& accessor,
    uint8_t*& output,              // 指向 tight-packed 顶点数据
    size_t& total_size_in_bytes,
    std::vector<uint8_t>& intermediate_buffer
);
```
- **tight 分支**（`byteStride==0`）：输出 = 指向 `buffer.data()` 内部的零拷贝指针
- **strided 分支**（`byteStride>0`）：按 `byteStride` 去交错，逐元素拷贝进 `intermediate_buffer`
- 调用方 `processNode` 对 4 个语义分别建独立 `D3D11_BUFFER_DESC` 顶点缓冲

### 对标实现：索引路径 `read_gltf_index_accessor`
已达标（`return false` + 越界/span 三层校验 + 忽略索引 byteStride），顶点路径应完全对齐。

---

## 二、问题清单与修复方案（对应审核结论）

> 优先级依据：导致崩溃/数据错乱的风险，以及 spec 强制约束。

### P0-1 越界解引用不返回（崩溃）
**现状**：`accessor.bufferView` / `buffer_view.buffer` 越界只有 `Logger::error`，随后仍
`model.bufferViews[accessor.bufferView]` / `model.buffers[buffer_view.buffer]` 解引用。
**依据**：`accessor.schema.json.bufferView` 为 `glTFid` 索引引用。
**修复**：两处 `Logger::error` 后补 `return false`（对齐索引路径第一步）。

### P0-2 无 buffer/bufferView 范围校验（越界读）
**现状**：两分支均无任何边界检查；tight 分支直接返回裸指针。
**依据**：`bufferView.schema.json`（`byteOffset`/`byteLength`）+ `Specification.adoc:1224-1228`
（accessor MUST fit its bufferView）与 `buffer` 内 `byteOffset+byteLength≤buffer.size`。
**修复**：新增三层前置校验，任一失败 `return false`：
1. `buffer_view.byteOffset + buffer_view.byteLength ≤ buffer.data.size()`
2. tight：`accessor.byteOffset + elem_size×count ≤ buffer_view.byteLength`
3. strided：`buffer_view.byteOffset + accessor.byteOffset + byteStride×(count-1) + elem_size ≤ buffer.data.size()`

### P1-1 normalized 整型顶点属性格式错位
**现状**：原样拷贝原始字节；输入布局/着色器固定 `R32G32B32_FLOAT` 等。
**依据**：`accessor.schema.json.normalized`（无符号→[0,1]，有符号→[−1,1]；禁止用于 FLOAT/UNSIGNED_INT）。
**决策**：**本次仅修复加载器正确性，不做 GPU 格式映射**（那属 v2 §5.9 的 Model/ModelRenderer 迁移范畴）。
本次对 `TEXCOORD_n` / `COLOR_n` 且 `normalized==true` 的整型 accessor：
- **CPU 展开为 float** 写入中间缓冲（POSITION/NORMAL 恒 float，无需处理）
- 否则维持字节拷贝
- 展开逻辑收口在加载期，避免触碰现有输入布局/PSO
> ⚠️ 与 v2 §5.9「直接映射 unorm 格式」策略不同——这是**用于旧路径的过渡实现**；
> v2 迁移时以 unorm 硬件归一化取代。此处不做 GPU 端改动，见 §六 说明。

### P1-2 sparse accessor 未应用
**现状**：直接读基础 `buffer.data()`，不应用 `accessor.sparse` 替换。
**依据**：`Specification.adoc:1120-1150`（sparse：bufferView 未定义时基准为全零 + sparse 增量）。
**决策**：**本次不实现 sparse 解码**（弹幕基准资源极少用 sparse）。改为：
- 检测 `accessor.sparse.isSparse` 为真 → `Logger::warn`（说明降级处理），仍按非 sparse 读取基础区域
- 明确文档化限制；v2 迁移时评估是否需要完整 sparse 支持
> 理由：sparse 涉及子 bufferView 解析 + 基准全零构造，改动面大且当前无资产触发；
> 优先保证 P0 安全。

### P2-1 strided 合法性校验
**现状**：无 `byteStride` 合法性检查。
**依据**：`Specification.adoc:1171`（byteStride MUST 为组件大小倍数）。
**修复**：strided 分支前置校验 `byteStride % componentSize == 0`，否则 `return false`。

### P2-2 （记录）矩阵列对齐 padding 未处理
**现状**：mat2/mat3 列首 4 字节 padding 未处理。
**依据**：`Specification.adoc:1187-1200`。
**决策**：顶点属性不涉及矩阵 type，**本次不处理**，仅记录为限制。

---

## 三、函数级重构设计

### 3.1 新增静态辅助：范围校验
```cpp
// 前置校验：三层边界，全过才继续（对齐 read_gltf_index_accessor 范式）
static bool validate_accessor_range(
    tinygltf::Model const& model,
    tinygltf::Accessor const& accessor,
    std::string const& tag,
    size_t& out_component_size,      // 组件字节数
    size_t& out_element_size,         // 元素字节数 = comp × numComp
    tinygltf::BufferView const** out_view,
    tinygltf::Buffer const** out_buffer
) {
    // 1) bufferView 索引在界（越界 return false + logger）
    // 2) buffer 索引在界（同）
    // 3) componentSize / numComponents 合法（<0 → return false）
    // 4) bufferView.byteOffset + byteLength <= buffer.data.size()
    // 5) byteStride == 0（紧排）:
    //        accessor.byteOffset + elem_size*count <= bufferView.byteLength
    //    byteStride > 0:
    //        byteStride % comp==0
    //        bufferView.byteOffset + accessor.byteOffset + byteStride*(count-1) + elem_size <= buffer.data.size()
    return true;
}
```

### 3.2 重写 `getBufferFromAccessor`（保留签名）
```cpp
static bool getBufferFromAccessor(model, accessor, output, total_size, intermediate_buffer) {
    size_t comp_size, elem_size;
    tinygltf::BufferView const* view; tinygltf::Buffer const* buf;
    if (!validate_accessor_range(model, accessor, tag, comp_size, elem_size, &view, &buf))
        return false;

    bool const has_stride = view->byteStride > 0;
    uint8_t const* src = buf->data.data() + view->byteOffset + accessor.byteOffset;
    size_t const count = (size_t)accessor.count;

    if (/* normalized 整型 TEXCOORD/COLOR 且需要展开 */ ...) {
        // 见 §3.3 展开路径：生成 float 中间缓冲
        intermediate_buffer.resize(count * 4 * N);
        逐元素 normalize 整型 → float 写入；
        output = intermediate_buffer.data();
        total_size = intermediate_buffer.size();
        return true;
    }

    if (!has_stride) {
        output = const_cast<uint8_t*>(src);          // 零拷贝直指（已通过边界校验）
        total_size = elem_size * count;
        return true;
    }

    intermediate_buffer.resize(elem_size * count);   // 去交错 tight-packed
    for (i in 0..count) {
        std::memcpy(intermediate_buffer.data() + elem_size*i, src + byteStride*i, elem_size);
    }
    total_size = intermediate_buffer.size();
    output = intermediate_buffer.data();
    return true;
}
```

### 3.3 normalized 整型 → float 展开（过渡实现）
适用范围：语义为 `TEXCOORD_n` / `COLOR_n`，`accessor.normalized==true`，整型组件。
```cpp
// 有符号整型 → [-1,1]：除 2^(n-1)-1；无符号整型 → [0,1]：除 2^n-1
// 用户需感知 count×numComp 个 float 输出
```
- TEXCOORD VEC2 → 每元素 2 float
- COLOR VEC3 → 3 float（alpha 视 1.0，见 note）；VEC4 → 4 float
> **注意**：COLOR VEC3 时，现有输入布局/着色器按 VEC4（R32G32B32A32）读取的路径会错位，
> 本次对 VEC3 展开为 3 float，若下游布局为 VEC4 需在调用点补 alpha——**本次明确不在旧布局上强撑 VEC4**，
> 仅保证"数据正确展开 + 记录日志"，布局适配交由 v2 迁移。

### 3.4 调用点适配（processNode 4 处）
- 4 处调用签名不变；传入 `std::string const&` prim_tag 供日志统一
- 每处赋值 `total_size_in_bytes = ASCII buffer 恒定（memcpy 直出）`（保持现有 ByteWidth/UINT 计算）
- 无其他结构改动

---

## 四、修复后行为矩阵

| 输入条件 | 修复前 | 修复后 |
|----------|--------|--------|
| `accessor.bufferView` 越界 | error + 解引用崩溃 | error + `return false`（进程级安全终止加载） |
| `buffer_view.buffer` 越界 | error + 解引用崩溃 | error + `return false` |
| bufferView 超出 buffer | 越界读/崩溃 | `return false` |
| accessor 超出 bufferView（tight） | 越界读 | `return false` |
| strided 超 buffer | 越界读 | `return false` |
| `byteStride % compSize != 0` | 错位数据 | `return false` |
| TEXCOORD/COLOR normalized 整型 | 字节错位 | CPU 展开为 float |
| sparse accessor | 数据读错（静默） | `Logger::warn` 降级 + 非 sparse 读基础区 |
| 常规 float 属性 | ✓ 正常 | ✓ 不变 |

---

## 五、验证与测试

### 5.1 单测/负例（新增）
构造不合法模型，断言 `getBufferFromAccessor` 返回 false 且不崩溃：
1. `accessor.bufferView = 1000`（超出）
2. `bufferView.buffer = 999`（超出）
3. `bufferView.byteOffset+byteLength > buffer.size`
4. tight accessor `byteOffset + span > view.byteLength`
5. `byteStride` 非组件大小倍数
6. strided 越出 buffer

### 5.2 正例回归
- 现有 float 顶点模型加载/渲染无回归（POSITION/NORMAL/UV 字节级对比 LoadBinaryFromFile 前后）
- 手工构造 `TEXCOORD_0` ubyte/ushort normalized 模型，验证展开为 float 后 UV 正确

### 5.3 覆盖
单元测试尽量在加载层无 GPU 依赖处跑；`CreateBuffer` 前即返回，可同进程断言。

---

## 六、与 v2 迁移的关系（避免重复建设）

| 本次修复 | v2（plans/model-v2.md） | 协同 |
|----------|------------------------|------|
| 越界/范围校验（P0） | M3 已列入待办 | 本次实现，v2 直接搬入 `IModel` |
| normalized 整型 CPU→float | §5.9 unorm 硬件归一化 | **策略不同**：本次为旧路径过渡，仅为不渲染错；v2 迁移时删 CPU 展开，改 unorm 格式 |
| sparse 降级 warn | 未规划 | 本次记录限制；v2 评估是否完整实现 |
| 矩阵 padding | 不涉及 | 两者均不处理，记录限制 |

> ⚠️ **明确边界**：本次修复目标 = **加载器健壮性 + 现有路径不崩不错**，
> 不对输入布局/PSO/GPU 格式做任何改动。normalized 展开的 GPU 端（unorm）放在 v2 §5.9 完成，
> 届时旧 CPU 展开路径整体移除。

---

## 七、里程碑

### M0：边界校验（P0-1/P0-2/P2-1）✅ 已完成
- [x] 新增 `validate_accessor_range`（三层校验 + byteStride 合法性）
- [x] `getBufferFromAccessor` 前置调用，失败 `return false`
- [x] 移除两处"error 后仍解引用"路径

M0 实现说明：
- 新函数 `validate_accessor_range`（`Model_D3D11.cpp`，置于 `getBufferFromAccessor` 上方），输出
  `base/component_size/element_size/has_stride/byte_stride/data_offset` 供读取端使用
- `getBufferFromAccessor` 签名不变；前置校验失败即 `return false`，tight/strided 逻辑沿用原语义
- 保留 `// M1 预留` 占位注释标记 normalized / sparse 接入点

### M1：normalized 整型展开（P1-1）✅ 已完成
- [x] `getBufferFromAccessor` 识别 `normalized && 整型 && (TEXCOORD|COLOR)` → float 展开
- [x] 未命中条件的语义维持字节拷贝
- [x] `Logger::info` 记录发生展开的属性

M1 实现说明：
- `getBufferFromAccessor` 签名增加 `std::string_view semantic` 参数（承载 COLOR/TEXCOORD 语义以决定是否展开，
  用 `starts_with` 判前缀）；4 个调用点（POSITION/NORMAL/COLOR_0/TEXCOORD_0）传入对应字符串字面量
- 新增静态函数 `expand_normalized_to_float`：无符号 → 除 2^n-1（[0,1]），有符号 → 除 2^(n-1)-1（[-1,1]），
  处理 BYTE(5120)/UBYTE(5121)/SHORT(5122)/USHORT(5123)；COLOR_0 保持 VEC3->3/VEC4->4 float（不做 VEC3->VEC4 强拄，见 §六 边界）
- 未命中（float 或非 COLOR/TEXCOORD）走原始字节路径不变
- 构建验证：`Core.Graphics.lib` 编译通过

### M2：sparse 降级（P1-2）✅ 已完成
- [x] 检测 `accessor.sparse.isSparse` → `Logger::warn` 降级

M2 实现说明：
- `getBufferFromAccessor` 在边界校验通过后、正常读取前，检测 `accessor.sparse.isSparse`
- 命中则 `Logger::warn`（含 accessor 索引、semantic、sparse count），随后仍按基础（非 sparse）密读
- 不实现 sparse 替换（弹幕基准资源极少用，改动面大），限定为降级警告 + 文档化限制
- 构建验证：`Core.Graphics.lib` 编译通过

### M3：测试与回归
- [ ] §5.1 负例单测（6 类）
- [ ] §5.2 正例回归 + normalized 手工模型
- [ ] 本地既有模型 LoadBinaryFromFile 渲染冒烟

---

## 八、风险与注意
- **签名不变**：不破坏 4 个调用点与外部（渲染/Lua）契约。
- **`intermediate_buffer` 生命周期**：输出指针在调用点局部作用域内即 `CreateBuffer` 拷贝，安全，维持现状。
- **normalized 展开为 float 是过渡**：v2 迁移务必移除并改 unorm，避免长期保留两套语义。
- **COLOR VEC3 布局错位**：本次不做 GPU/输入布局修补，仅在日志注明；真实有 VEC3 normalized 资产时需在 v2 处理。
- **零拷贝直指受生命周期约束**：`output` 指向 `model` 内部 buffer，调用方须保证 `model` 存活至 CreateBuffer 完成（现状满足）。