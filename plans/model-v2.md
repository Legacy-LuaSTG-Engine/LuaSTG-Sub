# Model v2 —— 3D 模型渲染迁移到 GPU 抽象层

> 目标：将现有 `Model_D3D11` / `Model_Shader_D3D11` 的 3D 模型渲染
> 从"raw D3D11 手写状态"路径迁移到 `IGraphicsPipeline` + `IGraphicsCommandBuffer` 组成的现代 GPU 抽象层，
> 并拆分为 `Model`（数据）与 `ModelRenderer`（渲染）两部分。
>
> 关联审核报告：`reviews/model.md`
> 状态：设计稿 / 待实现

---

## 一、背景与现状

### 架构事实（代码现状）
- **抽象层**（`engine/graphics/core/GraphicsDevice.hpp`、`GraphicsPipeline.hpp`、`GraphicsBuffer.hpp`）已具备：
  - 动态常量缓冲 `IGraphicsBuffer::update/map/unmap`
  - 顶点/索引缓冲 + 逐槽多缓冲绑定
  - 逐阶段纹理/采样器绑定
  - 完整 PSO `GraphicsPipelineState`（输入布局、VS/PS 字节码、光栅/深度模板/混合）
  - GPU 实例化 `drawIndexedInstanced` + `input_rate=instance`
  - `r16_uint / r32_uint` 索引格式
- **两条并存路径**：
  - 2D 渲染器走 `createGraphicsPipeline + bindGraphicsPipeline`（PSO 路径，图元固定 triangle_list，预生成 5 维矩阵 ≈704 个 PSO）
  - `Mesh` / `Model_D3D11` 走 **raw 残留路径**：`ctx = cmd->getNativeHandle()` 拿 raw `ID3D11DeviceContext` 手动设 IA/VS/输入布局，只把 draw 交给命令缓冲
- 模型当前**不参与 PSO**、**不参与批渲染**、**不共享 GPU 缓冲**，且 shader 为运行时 D3DCompiler 动态编译的大字符串排列

### 抽象层缺口（迁移前需确认/补齐）
| 缺口 | 说明 | 建议 |
|------|------|------|
| 绘制期图元拓扑切换 | `primitive_type` 烘焙进 PSO，命令缓冲无 `setPrimitiveTopology` | **必改** |
| 通用管线缓存 | 无 `state→hash→cache` 工具，只有硬编码 5 维矩阵 | **建议加** |
| 模板参考值 | 无 `setStencilRef` | 可选，暂不需要 |

---

## 二、抽象层改造清单（问题一）

### 2.1【必改】`IGraphicsCommandBuffer::setPrimitiveTopology`
模型一个 gltf 场景内不同 primitive 可能混用 `TRIANGLES/STRIP/POINTS/LINES`。
把图元拓扑烘焙进 PSO 会让每图元变化的拓扑逼出"PSO × 拓扑数"的组合爆炸。

```cpp
// core/GraphicsDevice.hpp —— IGraphicsCommandBuffer 新增纯虚成员
virtual void setPrimitiveTopology(GraphicsPrimitiveType topology) = 0;
```
- 与 D3D11 `IASetPrimitiveTopology` 语义一致，也贴合 `Mesh.cpp` 先例
- PSO 里的 `primitive_type` 保留为默认值，或不再参与调用侧判断
- `GraphicsPrimitiveType` 枚举位于 `core/GraphicsPipeline.hpp`
- **注意**：`GraphicsPrimitiveType` 目前只有 `triangle_list / triangle_strip / line_list / line_strip / point_list`，**不含 LINE_LOOP / TRIANGLE_FAN**；此二者在 `IModel` 加载期**转换**为可表达拓扑（见 §四），不进入 `setPrimitiveTopology`

### 2.2【建议】通用管线缓存
2D 预生成 704 个 PSO；模型迁移 PSO 后，属性组合（pos + normal/uv/color 可选 → 8 组合）× 雾 × alpha × 单双面会再扩出几十上百个 PSO。

```cpp
// IGraphicsDevice 新增（或独立 IGraphicsPipelineManager）
virtual bool createGraphicsPipelineCached(
    const GraphicsPipelineState* state, IGraphicsPipeline** out) = 0;
// 内部以 state 的 hash 缓存并返回已存在实例
```
- 2D 与模型共用同一缓存设施，取代硬编码 5 维矩阵

### 2.3【可选】`IGraphicsCommandBuffer::setStencilRef`
模型当前只用默认模板参考值 0，暂不需要。若将来要模板剔除效果再加（D3D11 模板参考值属于输出合并期可变状态，不在 PSO 内）。

### 2.4【IRenderer 层面，必改】显式暴露帧常量
修掉 `reviews/model.md` **P0#3（隐式槽位依赖）**：模型现在靠 2D 渲染器"残留绑定"的 `_vp_matrix_buffer / _camera_pos_buffer / _fog_data_buffer` 才能拿到 VS `b0`、PS `b0/b1`。

```cpp
// IRenderer 新增
virtual IGraphicsBuffer* getViewProjectionConstantBuffer() = 0;
virtual IGraphicsBuffer* getCameraConstantBuffer() = 0;
virtual IGraphicsBuffer* getFogConstantBuffer() = 0;
// 或：virtual bool bindModelFrameConstant(IGraphicsCommandBuffer*) = 0;
```
→ 让 VS `b0=VP`、PS `b0=camera`、PS `b1=fog` 由契约显式绑定。

---

## 三、Model + ModelRenderer 拆分设计（问题二）

### 3.1 可行性
**可行且贴合仓库既有范式**：`Mesh`/`IMeshRenderer`、`Sprite`/`ISpriteRenderer` 已是"数据对象 + 渲染器对象"先例，照此复制 `Model`/`IModelRenderer`。

| 类型 | 职责 | 可变性 |
|------|------|--------|
| `IModel` | 纯资源/数据：加载 gltf；拥有每 primitive 的 GPU 缓冲（pos/normal/uv/color、索引）、材质块（base_color/alpha_mode/double_side）、不可变 `local_matrix`、纹理/采样器引用 | immutable，可多实例共享 |
| `IModelRenderer` | 帧级渲染对象：持有 `IModel*`、实例变换/光照/环境光；`draw(IRenderer*)` 内做 PSO 绑定 + 逐 primitive 常量上传 + 绘制 | 每帧可变 |

附带解决审核报告 P1 **#六（无实例化/无共享）**：`IModel` 跨实例共享 GPU 缓冲 + `drawIndexedInstanced`。

### 3.2 建议接口
```cpp
struct IModel : IReferenceCounted {
    virtual uint32_t getPrimitiveCount() const noexcept = 0;
    virtual bool isReadOnly() const noexcept = 0;
    static bool create(IGraphicsDevice* device, StringView path, IModel** output);
};

struct IModelRenderer : IReferenceCounted {
    virtual void setModel(IModel*) = 0;
    virtual void setTransform(Matrix4F const& world) = 0;                 // 实例世界变换
    virtual void setAmbient(Vector3F const& color, float brightness) = 0; // 原 IModel 上的光设置迁到渲染器
    virtual void setDirectionalLight(Vector3F const& dir, Vector3F const& color, float brightness) = 0;
    virtual void draw(IRenderer* renderer) = 0;
    static bool create(IGraphicsDevice* device, IModelRenderer** output);
};
```
> 光照接口从 `IModel` 挪到 `IModelRenderer`，顺带修复 `reviews/model.md` **P2 #五（光 API 不可达）**——现 `lib_drawModel` 从不调用 `setAmbient / setDirectionalLight`。

---

## 四、图元拓扑支持矩阵（glTF 2.0）

> 来源：`specification/2.0/schema/mesh.primitive.schema.json`（`mode`）与 `Specification.adoc` 网格/图元章节。
> glTF 2.0 `mode` 枚举共 7 种，默认 `4`（TRIANGLES）。

| mode | 名称 | D3D11 原生拓扑 | 抽象层 `GraphicsPrimitiveType` | 支持策略 |
|------|------|----------------|-------------------------------|----------|
| 0 | POINTS | ✅ `POINTLIST` | 有（`point_list`） | 原生支持 |
| 1 | LINES | ✅ `LINELIST` | 有（`line_list`） | 原生支持 |
| 2 | LINE_LOOP | ❌ 无 | 无 | 转换 → LINES + log 警告 |
| 3 | LINE_STRIP | ✅ `LINESTRIP` | 有（`line_strip`） | 原生支持 |
| 4 | TRIANGLES（默认） | ✅ `TRIANGLELIST` | 有（`triangle_list`） | 原生支持（核心） |
| 5 | TRIANGLE_STRIP | ✅ `TRIANGLESTRIP` | 有（`triangle_strip`） | 原生支持 |
| 6 | TRIANGLE_FAN | ❌ 无 | 无 | 转换 → TRIANGLES + log 警告 |

### 结论
- **5 种原生支持**：POINTS / LINES / LINE_STRIP / TRIANGLES / TRIANGLE_STRIP —— D3D11 原生拓扑直接映射，零成本，抽象层 `GraphicsPrimitiveType` 均已具备。
- **2 种需转换 + log 警告**：LINE_LOOP、TRIANGLE_FAN —— D3D11 **和** 抽象层 `GraphicsPrimitiveType` **都无对应值**，必须转换为可表达的拓扑。
  - **LINE_LOOP**：展开索引，追加首段闭合环（末顶点→首顶点），按 LINES 提交。
  - **TRIANGLE_FAN**：以首顶点为扇心展开，按 TRIANGLES 提交（**需保持绕序/面朝向**）。
  - **策略确定**：二者一律做转换并 `Logger::warn` 记录（说明发生转换、文件/primitive、耗时影响），**不报错拒绝、不静默跳过**。这样既保证不被丢弃，也避免隐藏问题。

### 顶点数校验（spec 强制）
| 拓扑 | 顶点限制 |
|------|---------|
| 点 | 非 0 |
| 线 / line loop / line strip | ≥ 2 |
| 三角形 / triangle strip / triangle fan | ≥ 3 |
| lines | 被 2 整除 |
| triangles | 被 3 整除 |

### 实现落点
- 原生 5 种：走 `bindGraphicsPipeline` + `setPrimitiveTopology` 直接映射。
- 非原生 2 种（LINE_LOOP / TRIANGLE_FAN）：在 `IModel` 加载阶段**强制转换**——fan 保绕序、loop 闭合，生成新的索引缓冲后按 LINES/TRIANGLES 提交，并 **`Logger::warn`** 说明发生了拓扑转换（文件、primitive、转换前后索引数）。**不报错拒绝，也不静默跳过**。

---

## 五、顶点输入配置（glTF 2.0）

> 来源：`Specification.adoc` 的 Buffers/Accessors/Geometry/Meshes 章节 + `schema/accessor.schema.json`、`schema/mesh.primitive.schema.json`。
> 目标：梳理 `IModel` 加载层需要支持的顶点输入全部配置，作为输入布局/PSO 缓存设计的依据。

### 5.1 语义槽位（`primitive.attributes`）
`attributes` 是一个键值映射：**语义名 → accessor 索引**。

Spec 定义的标准语义：

| 语义 | Accessor 类型 | 组件类型 | 说明 |
|------|--------------|---------|------|
| `POSITION` | VEC3 | float | 单位无关 XYZ 位置（**必含 min/max**） |
| `NORMAL` | VEC3 | float | 单位化法线 |
| `TANGENT` | VEC4 | float | XYZW，W=±1 表示切空间手性 |
| `TEXCOORD_n` | VEC2 | float / ubyte* / ushort* | ST 纹理坐标 |
| `COLOR_n` | VEC3 / VEC4 | float / ubyte* / ushort* | 顶点色线性倍率（VEC3 时 alpha 视为 1.0） |
| `JOINTS_n` | VEC4 | ubyte / ushort | 骨架关节索引 |
| `WEIGHTS_n` | VEC4 | float / ubyte* / ushort* | 骨骼权重 |

(\* = `normalized` 整型)

- 应用自定义语义必须以 `_` 开头（如 `_TEMPERATURE`），**不得**用 unsigned int 组件。
- 索引起始 0 且连续（`TEXCOORD_0, TEXCOORD_1, ...`），无前导零（`TEXCOORD_01` 非法）。
- 实现应至少支持 **2 套 TEXCOORD、1 套 COLOR、1 套 JOINTS/WEIGHTS**。

**映射到当前 Model 输入布局**：

| 语义 | 槽位 | 当前格式 |
|------|------|---------|
| `POSITION` | 0 | `R32G32B32_FLOAT` |
| `NORMAL` | 1 | `R32G32B32_FLOAT` |
| `TEXCOORD_0` | 2 | `R32G32_FLOAT` |
| `COLOR_0` | 3 | `R32G32B32_FLOAT` |

> 上表为**当前（旧 Model_D3D11）基线**；v2 起槽位不变，但各属性格式由 accessor 的 `componentType × type × normalized` **动态推导**（见 §5.9.1），不再固定为上述值。
> v2 最小实现只需 `POSITION + TEXCOORD_0（+可选 COLOR_0）`（见 §5.8），上面 spec 「至少 2 套 TEXCOORD」为一般实现建议，超出最小范围。

### 5.2 数据类型（`componentType` × `type`）
组件类型：

| 值 | 类型 | 有无符号 | 位 |
|----|------|---------|----|
| 5120 | signed byte | 有 | 8 |
| 5121 | unsigned byte | 无 | 8 |
| 5122 | signed short | 有 | 16 |
| 5123 | unsigned short | 无 | 16 |
| 5125 | unsigned int | 无 | 32 |
| 5126 | float | — | 32 |

（**32 位有符号整数不支持**）

`type`：SCALAR(1) / VEC2(2) / VEC3(3) / VEC4(4) / MAT2(4) / MAT3(9) / MAT4(16)。
元素字节 = 组件字节 × 组件数。二进制统一**小端**字节序（GLB BIN chunk）。

### 5.3 `normalized` 标志
- `true` → 整型访问时归一化：
  - **无符号 → [0,1]**（除以 `2^n−1`：ubyte/255、ushort/65535）
  - **有符号 → [−1,1]**（除以 `2^(n−1)−1`）
- **禁止对 FLOAT / UNSIGNED_INT 置 true**
- 对 `min/max` 无影响
- 适用于 TEXCOORD / COLOR / WEIGHTS（morph 的 COLOR/TEXCOORD 允许有符号变体）

**对引擎的关键影响**：`TEXCOORD_0` / `COLOR_0` 允许 normalized 整型（ubyte/ushort）。当前 `Model_D3D11` 用固定 float 格式读缓冲，遇整型源会格式错乱。v2 需二选一：
- (a) **加载期展开为 float** —— 简单、兼容现有输入布局；
- (b) 用对应 DXGI 格式（`R8G8B8A8_UNORM` 等）由 GPU 硬件归一化 —— 省内存/带宽，但输入布局随组件类型变化 → PSO 变体增多。

### 5.4 布局 / 交错（`bufferView.byteStride`）
- 属性 accessor 用 `byteStride` 定义顶点间距；**未定义 → 紧密排列**（stride = 元素大小）。
- **2+ 个属性共享同一 bufferView → byteStride 必定义**（交错布局）。
- `byteStride` 必须是组件类型大小的倍数；建议每顶点 **4 字节对齐**（`byteStride`/`byteOffset` 为 4 的倍数）。
- `accessor.byteOffset` 定位于 bufferView 内。

### 5.5 索引 accessor
- 类型 `SCALAR` + **无符号整型**组件（5121 / 5123 / 5125）。
- 仅索引图元使用；`count` = 索引数。
- 索引**不得**含组件类型的最大值（255 / 65535 / 4294967295）—— 该值触发 primitive restart。
- 无索引时：属性 `count` = 待渲染顶点数（隐式索引 `[0..count)`）；有索引时：索引值 `< 属性 count`。

### 5.6 属性一致性 / 缺失处理
- 同一 primitive 所有属性 accessor 的 `count` **必须一致**。
- 缺 `POSITION` → 实现应**跳过**该 primitive（除非扩展提供位置）。
- 缺 `NORMAL` → 必须计算平面法线（basecolor 最简模式可忽略，需文档化该豁免）。

### 5.7 对管线缓存 / 输入布局的本质影响
- **输入布局的格式由 accessor 的（componentType × type × normalized）共同决定，无法静态唯一**。
  → PSO 需按 **「属性签名」**（存在哪些语义 + 各自格式）缓存；缓存键 = `属性签名 × fog × alpha × 单双面`。
  - ⚠️ **拓扑不进入 PSO 缓存键**：拓扑经 `setPrimitiveTopology` 在 PSO 绑定后**动态**设置（见 §2.1 / 四 / §七），不烘焙进 PSO，因此也不进缓存键——避免 §2.1 所述的 "PSO × 拓扑数" 组合爆炸。
- 索引格式由索引 accessor 组件类型决定：ubyte→扩 ushort / ushort→`r16_uint` / uint→`r32_uint`。

### 5.8 覆盖性排查结论（GPU 抽象层 VertexInput vs glTF 2.0）

**范围收敛**：本项目只需**网格 + basecolor** 基础渲染，即保证 `POSITION` + `TEXCOORD_0`（+ 可选 `COLOR_0`）可渲染即可，法线/切线/蒙皮/多 texcoord 集合非必需。

**格式覆盖矩阵**：

| 需求 | glTF 语义 / 类型 | 抽象层 `GraphicsFormat` | `toFormat → DXGI` | 覆盖 |
|------|------------------|------------------------|-------------------|------|
| 顶点位置 | POSITION VEC3 float | `r32_g32_b32_float` | `R32G32B32_FLOAT` | ✅ |
| 纹理坐标 (float) | TEXCOORD_0 VEC2 float | `r32_g32_float` | `R32G32_FLOAT` | ✅ |
| 纹理坐标 (ubyte norm) | TEXCOORD_0 VEC2 ubyte | `r8_g8_unorm` | `R8G8_UNORM` | ✅ |
| 纹理坐标 (ushort norm) | TEXCOORD_0 VEC2 ushort | `r16_g16_unorm` | `R16G16_UNORM` | ✅ |
| 顶点色 (VEC3 float) | COLOR_0 VEC3 float | `r32_g32_b32_float` | `R32G32B32_FLOAT` | ✅ |
| 顶点色 (VEC4 float) | COLOR_0 VEC4 float | `r32_g32_b32_a32_float` | `R32G32B32A32_FLOAT` | ✅ |
| 顶点色 (ubyte norm) | COLOR_0 VEC4 ubyte | `r8_g8_b8_a8_unorm` | `R8G8B8A8_UNORM` | ✅ |
| 顶点色 (ushort norm) | COLOR_0 VEC4 ushort | `r16_g16_b16_a16_unorm` | `R16G16B16A16_UNORM` | ✅ |
| （可选）法线 | NORMAL VEC3 float | `r32_g32_b32_float` | `R32G32B32_FLOAT` | ✅ |
| 索引 ubyte | ubyte | 加载期扩→ushort | `r16_uint`（buffer stride 2） | ✅ |
| 索引 ushort | ushort | `r16_uint` | `R16_UINT` | ✅ |
| 索引 uint | uint | `r32_uint` | `R32_UINT` | ✅ |

**结构覆盖**：
- 逐槽 stride：由 **buffer 自身 `getBufferStride()`（创建时设定）** 提供，多属性独立 buffer 均可 ✅
- 交错布局：同 bufferView 多属性交错 `byteStride` → 单个 `IGraphicsBuffer` + 多 element 共享 `buffer_slot` + `offset` 偏移 ✅（`InputSlot` + `AlignedByteOffset` 支持）
- 多 UV/color 集合：`semantic_index`（`TEXCOORD_0/1...`）支持 ✅
- 实例数据：`input_rate=instance` + `instance_step_rate` 支持 ✅（最小需求暂用不到）

**结论**：`GraphicsVertexInputState` / `GraphicsFormat` **完全覆盖** glTF 2.0 最小（mesh + basecolor）渲染需求，**无需改动**。
  - `GraphicsFormat` 已含 `r8_g8_unorm / r16_g16_unorm / r8_g8_b8_a8_unorm / r16_g16_b16_a16_unorm`，可直接由 GPU 硬件归一化整型 TEXCOORD/COLOR（免加载期展开）。
  - 所需的逐槽 stride 与结构（独立/交错、多槽、instance）均已具备。

**剩下的都是加载层的职责（非抽象层）**：
  1. 每属性 `IGraphicsBuffer` 的 stride 须按元素大小（紧密）或 `bufferView.byteStride`（交错）设定。
  2. 索引 ubyte 需扩展为 ushort（`r16`），并排除 primitive-restart 最大值。
  3. normalized 整型策略已定（见 §5.9）：**直接映射 unorm 格式**；无对应 N 通道（VEC3 normalized）**扩展 VEC4 补 alpha=1.0**，不转 float。

### 5.9 加载层设计（normalized 整型直接使用 GPU 格式）

**决策**：`TEXCOORD_n` / `COLOR_n` 的 normalized 整型**直接映射到对应的 unorm GPU 格式**，由 GPU 硬件完成 `[0,1]` 归一化；**不做 CPU 展开为 float**。
当不存在匹配的 N 通道 unorm 格式时（VEC3 normalized），**扩展为 VEC4 unorm（补 alpha=1.0）**，**不转 float**——float 需 12 字节/元素，而 VEC4 unorm 仅 4（ubyte）/8（ushort）字节，避免体积过度膨胀。

#### 5.9.1 格式选择表（加载期由 accessor 推导）

| 语义（type） | componentType | normalized | → `GraphicsFormat` | 字节/elem | 备注 |
|--------------|---------------|-----------|--------------------|-----------|------|
| POSITION（VEC3） | 5126 float | — | `r32_g32_b32_float` | 12 | 必含 min/max |
| NORMAL（VEC3） | 5126 float | — | `r32_g32_b32_float` | 12 | 可选 |
| TEXCOORD_n（VEC2） | 5126 float | — | `r32_g32_float` | 8 | |
| TEXCOORD_n（VEC2） | 5121 ubyte | true | `r8_g8_unorm` | 2 | 直接 GPU 归一化 |
| TEXCOORD_n（VEC2） | 5123 ushort | true | `r16_g16_unorm` | 4 | 直接 GPU 归一化 |
| COLOR_n（VEC3） | 5126 float | — | `r32_g32_b32_float` | 12 | |
| COLOR_n（VEC4） | 5126 float | — | `r32_g32_b32_a32_float` | 16 | |
| COLOR_n（VEC4） | 5121 ubyte | true | `r8_g8_b8_a8_unorm` | 4 | 直接 GPU 归一化 |
| COLOR_n（VEC4） | 5123 ushort | true | `r16_g16_b16_a16_unorm` | 8 | 直接 GPU 归一化 |
| COLOR_n（VEC3） | 5121 ubyte | true | `r8_g8_b8_a8_unorm` | 3→4 | 扩展 VEC4，补 alpha=1.0 |
| COLOR_n（VEC3） | 5123 ushort | true | `r16_g16_b16_a16_unorm` | 6→8 | 扩展 VEC4，补 alpha=1.0 |

> **VEC3 normalized 处理**：DXGI 无 3 通道 unorm 格式（`R8G8B8_UNORM` 不存在）。当 `COLOR_n` 为 `VEC3` + normalized 整型时，**按 VEC4 扩展**：拷贝 RGB，末尾补 alpha=1.0（ubyte→`0xFF`，ushort→`0xFFFF`），映射为 `r8_g8_b8_a8_unorm` / `r16_g16_b16_a16_unorm`（4/8 字节）。**不转 float**（12 字节）以免体积膨胀。加载时 `Logger::info` 记录扩展。basecolor 常见 `COLOR_0` 为 VEC4，直接命中，此路径为少见例。

#### 5.9.2 加载数据模型（IModel 内部）

```cpp
// 每个属性一个 GPU 缓冲（去交错、紧密排列）；normalized 整型原样上传
struct ModelAttribute {
    std::string semantic;            // "POSITION" / "NORMAL" / "TEXCOORD_0" / "COLOR_0"...
    core::GraphicsFormat format;     // 由 §5.9.1 推导
    uint32_t buffer_slot;            // 0..3
    uint32_t byte_offset;            // 独立布局时为 0
    SmartReference<IGraphicsBuffer> buffer;  // 本属性缓冲
};

struct ModelPrimitive {
    // 顶点输入
    std::vector<ModelAttribute> attributes;   // 存在哪些属性、各槽位格式
    SmartReference<IGraphicsBuffer> index_buffer;
    core::GraphicsFormat index_format;        // r16_uint / r32_uint
    uint32_t index_count;
    bool has_index;                 // false → draw() 非索引
    core::GraphicsPrimitiveType topology;     // LOOP/FAN 已转换
    // 材质（basecolor）
    DirectX::XMFLOAT4 base_color;
    bool double_sided;
    bool alpha_blend;
    bool alpha_mask;
    float alpha_cutoff;
    SmartReference<ITexture2D> image;         // baseColorTexture SRV
    SmartReference<IGraphicsSampler> sampler;
    // 变换
    DirectX::XMFLOAT4X4 local_matrix;
    DirectX::XMFLOAT4X4 local_matrix_normal;
    // 管线缓存键
    uint64_t attribute_signature;
};
```

#### 5.9.3 加载流程（每 primitive）
1. **拓扑**：`mode` → `GraphicsPrimitiveType`；LINE_LOOP / TRIANGLE_FAN 走 §四索引展开 + `Logger::warn`。
2. **解析属性**：遍历 `primitive.attributes`（POSITION 必在；NORMAL/TEXCOORD_0/COLOR_0 可选），按 §5.9.1 推导各属性 `format`。
3. **去交错**：用 accessor 的 `byteStride` 把数据拆为**每属性紧密排列**的独立缓冲（复用现有 `getBufferFromAccessor` 思路，补越界校验）；
   - `createVertexBuffer(elem_bytes * count, elem_bytes, ...)`（stride = 元素大小）。
   - normalized 整型：**原始字节原样上传**（GPU 按 unorm 格式归一化）。
   - VEC3 normalized：每元素拷贝 RGB，末尾补 alpha=1.0（ubyte→`0xFF`，ushort→`0xFFFF`）**扩展为 VEC4** 后按 `r8_g8_b8_a8_unorm` / `r16_g16_b16_a16_unorm` 上传（不做 float 展开，避免体积膨胀）。
4. **索引**：无 `indices` → `has_index=false`，`index_count` = 属性 `count`；有则按组件类型 ushort→`r16_uint` / uint→`r32_uint` / ubyte→扩 ushort（排除 255）。
5. **构建 `ModelPrimitive`**：材质块（base_color / alpha_mode / double_sided / 纹理）赋值；`local_matrix` = TRS 累积 + 右手→左手术式（见 `reviews/model.md` §一 / §八）。
6. **计算 `attribute_signature`**（见 5.9.4）。

#### 5.9.4 属性签名（PSO 缓存键的一部分）
- 签名由「存在哪些属性槽位 + 各自 `GraphicsFormat`」哈希得到：
```cpp
// 用槽位(0..3) + format 构造确定性签名，供 createGraphicsPipelineCached 复用
uint64_t attr_sig = FNV1a(slot0_format) ^ FNV1a(slot1_format) ...;
```
- PSO 缓存键 = `attribute_signature × fog × alpha_mode × double_sided`（见 §5.7；拓扑经 `setPrimitiveTopology` 动态设置，**不进缓存键**）。
- 纹理有无 / 顶点色有无天然编码在签名中（未声明该槽位即缺该属性）。

#### 5.9.5 缺省/降级（basecolor 语义）
- 无 `TEXCOORD_0` → shader 走 NoBaseTexture 变体，用 `base_color` 纯色。
- 无 `COLOR_0` → 无顶点色变体（顶点色恒白）。
- 无 `POSITION` → 跳过该 primitive + `Logger::warn`。
- 无 `NORMAL` → basecolor 最简模式**豁免**（不计算法线，不做光照）；如需光照再补平面法线。
- ⚠️ **basecolor 最简模式下 `IModelRenderer::setAmbient / setDirectionalLight` 为占位接口**（§3.2 因修复 `reviews` P2#五 移入渲染器，但最小实现暂不用，仅存储供将来启用）；开启光照才需要 NORMAL 与法线矩阵、PS `b3` 上传。

---

## 六、配套 / 新增 API 汇总

### GPU 抽象层
| # | API | 类型 | 状态 |
|---|-----|------|------|
| 1 | `IGraphicsCommandBuffer::setPrimitiveTopology(GraphicsPrimitiveType)` | 必改 | 新增 |
| 2 | `IGraphicsDevice::createGraphicsPipelineCached(state, out)` 或 `IGraphicsPipelineManager` | 建议 | 新增 |
| 3 | `IGraphicsCommandBuffer::setStencilRef(uint8_t)` | 可选 | 暂缓 |

### IRenderer 层面
| # | API | 类型 | 状态 |
|---|-----|------|------|
| 4 | `getViewProjectionConstantBuffer / getCameraConstantBuffer / getFogConstantBuffer` | 必改 | 新增 |
| 5 | 非批绘制批作用域入口 `IRenderer::withRawDraw(std::function<void(IGraphicsCommandBuffer*)>)` | 建议 | 新增（解决状态泄漏 P3） |

```cpp
// 形如：withRawDraw(fn) 内部 endBatch() → fn(cmd) → beginBatch()，
// 由 Renderer 负责恢复 2D 批状态
IRenderer::withRawDraw(fn);
```

### 着色器配套（非 API）
| # | 项 | 状态 |
|---|-----|------|
| 6 | 模型 shader 预编译为 `.cso`/字节数组（仿 `d3d11/shader/mesh/vertex_shader_*.h`），以 `GraphicsShaderByteCode` 喂给 PSO，保留 fog 变体 | 必做（解决 P0#4 / P2#7） |
| 7 | 删除运行时 D3DCompiler 动态编译 + 死代码（`inv_alpha` 族、`set_alpha_mode_blend`） | 随重构 |

### 无需新增（已支持）
- 实例化 `drawIndexedInstanced` + instance 输入率
- 动态常量缓冲 `update/map/unmap`
- 逐阶段纹理/采样器绑定
- 多顶点缓冲 + 逐槽 stride + 可选属性（8 组合 PSO）
- `r16_uint / r32_uint` 索引格式

---

## 七、数据流设计

### 每帧绘制流程（IModelRenderer::draw 内部）
```
1. 从 IRenderer 显式取 VP/camera/fog 常量缓冲 → 绑定 VS b0、PS b0、PS b1
2. 计算实例总变换 T = local_matrix[prim] * instance_world；算式不变
3. 上传 light / alpha / base_color（动态常量缓冲 update -> PS b2/b3）
4. 用 createGraphicsPipelineCached 取 PSO（属性组合 × fog × alpha × 单双面）
5. cmd->bindGraphicsPipeline(pso)
6. 逐 primitive：
   a. 若拓扑变化 → cmd->setPrimitiveTopology(...)
   b. 绑顶点/索引缓冲（可选属性对应槽位）
   c. 绑纹理 SRV + 采样器
   d. 上传/绑定本 prim 的 VS b1（local * world）
   e. 绑 PS b2/b3（base_color + alpha、light）
   f. drawIndexed / draw（或 drawIndexedInstanced 批量实例）
7. 结束
```

### 常量槽位约定（与 `d3d11/SlotConstants.hpp` 对齐，收敛单一来源）
```
VS b0  view-projection          (Renderer，显式绑定）
VS b1  local-world + normal-local-world  (本 prim，ModelRenderer)
PS b0  camera-pos / camera-look  (Renderer，显式绑定)
PS b1  fog                        (Renderer，显式绑定)
PS b2  base_color + alpha         (ModelRenderer)
PS b3  light (ambient/pos/dir/color)  (ModelRenderer)
```

---

## 八、迁移步骤（里程碑）

### M0：准备工作
- [ ] 确认 `IModel / IModelRenderer` 接口 UUID（`getInterfaceId` 特化）
- [ ] 建立 `d3d11/shader/model/` 目录，从 `Model_Shader_D3D11.cpp` 提取 HLSL → 预编译 `.cso`

### M1：抽象层改造
- [ ] `IGraphicsCommandBuffer::setPrimitiveTopology` + d3d11 实现
- [ ] `IGraphicsDevice::createGraphicsPipelineCached` + 缓存实现（或 `IGraphicsPipelineManager`）
- [ ] `IRenderer` 暴露帧常量 + `withRawDraw` 批作用域（可选）

### M2：拆分 Model / ModelRenderer
- [ ] `IModel` 实现：从 `Model_D3D11` 迁移加载 + 数据持有（缓冲、材质、local_matrix、纹理）
- [ ] 顶点输入：按 §5.9 处理 —— normalized 整型 TEXCOORD/COLOR **直接映射 unorm 格式**（VEC4 命中直接路径）；VEC3 normalized 少见例**扩展 VEC4 补 alpha=1.0**（ubyte→`r8_g8_b8_a8_unorm`，ushort→`r16_g16_b16_a16_unorm`），**非 float 展开**；索引 ubyte→ushort 扩展 / r16 / r32；若保留交错则用单缓冲多槽位+offset（缺省去交错）
- [ ] 加载期转换 LINE_LOOP / TRIANGLE_FAN（索引展开 + `Logger::warn`），移除 `assert(false)`（见 §四）
- [ ] `IModelRenderer` 实现：迁移绘制循环到 PSO 路径
- [ ] 移除 `Model_D3D11`（或保留一段兼容壳）
- [ ] `Renderer_D3D11::createModel / drawModel` 改为委托给新接口

### M3：接通 / 精简
- [ ] `lib_drawModel` 增加可选光照参数，接通 `setAmbient / setDirectionalLight`
- [ ] 删除运行时 shader 编译 + 死代码（`inv_alpha` 族、`set_alpha_mode_blend`）
- [ ] alphaMode BLEND 恢复真实透明混合（移除 screen-door 强替换）
- [ ] `getBufferFromAccessor` 越界校验返回 false（P0）

### M4：收益项（可选）
- [ ] `IModel` GPU 缓冲跨实例共享 + `drawIndexedInstanced`

---

## 九、风险与注意事项
- **拓扑切换语义**：`setPrimitiveTopology` 与既有 PSO 烘焙 `primitive_type` 可能产生"两处真理"；需明确 PSO 内 `primitive_type` 不再参与判断或保持默认，避免二义性。
- **PSO 缓存 hash**：`GraphicsPipelineState` 含指针数组（缓冲/元素列表），缓存键需对**解引用后的内容**哈希，而非指针值。
- **兼容壳**：M2 移除旧类前，先确认 `Renderer_D3D11.hpp` / Lua 绑定无遗漏引用。
- **批作用域**：模型绘制必须与 2D 批隔离（`withRawDraw` 包装 endBatch/beginBatch），避免状态泄漏。
- **扇/环模拟的绕序**：triangle fan 展开需保持三角形绕序（CCW），否则面朝向 / 剔除错乱；line loop 闭合需处理首尾重复。