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

## 五、配套 / 新增 API 汇总

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

## 六、数据流设计

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

## 七、迁移步骤（里程碑）

### M0：准备工作
- [ ] 确认 `IModel / IModelRenderer` 接口 UUID（`getInterfaceId` 特化）
- [ ] 建立 `d3d11/shader/model/` 目录，从 `Model_Shader_D3D11.cpp` 提取 HLSL → 预编译 `.cso`

### M1：抽象层改造
- [ ] `IGraphicsCommandBuffer::setPrimitiveTopology` + d3d11 实现
- [ ] `IGraphicsDevice::createGraphicsPipelineCached` + 缓存实现（或 `IGraphicsPipelineManager`）
- [ ] `IRenderer` 暴露帧常量 + `withRawDraw` 批作用域（可选）

### M2：拆分 Model / ModelRenderer
- [ ] `IModel` 实现：从 `Model_D3D11` 迁移加载 + 数据持有（缓冲、材质、local_matrix、纹理）
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

## 八、风险与注意事项
- **拓扑切换语义**：`setPrimitiveTopology` 与既有 PSO 烘焙 `primitive_type` 可能产生"两处真理"；需明确 PSO 内 `primitive_type` 不再参与判断或保持默认，避免二义性。
- **PSO 缓存 hash**：`GraphicsPipelineState` 含指针数组（缓冲/元素列表），缓存键需对**解引用后的内容**哈希，而非指针值。
- **兼容壳**：M2 移除旧类前，先确认 `Renderer_D3D11.hpp` / Lua 绑定无遗漏引用。
- **批作用域**：模型绘制必须与 2D 批隔离（`withRawDraw` 包装 endBatch/beginBatch），避免状态泄漏。
- **扇/环模拟的绕序**：triangle fan 展开需保持三角形绕序（CCW），否则面朝向 / 剔除错乱；line loop 闭合需处理首尾重复。