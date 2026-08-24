# 3D 模型（glTF）渲染实现 —— 架构审核报告

> 审核对象：`engine/graphics/core/Graphics/Model_D3D11.{hpp,cpp}` 与 `Model_Shader_D3D11.cpp`
> 视角：游戏引擎架构设计师
> 状态：梳理性审阅 / 待重构

相关代码：
- 渲染器接入：`engine/graphics/core/Graphics/Renderer_D3D11.{hpp,cpp}`（`createModel` / `drawModel`）
- Lua 绑定：`LuaSTG/LuaSTG/LuaBinding/LW_Renderer.cpp`（`lib_drawModel` → `lstg.RenderModel`）
- 资源层：`LuaSTG/LuaSTG/GameResource/Implement/ResourceModelImpl.{hpp,cpp}`
- 公共抽象：`engine/graphics/core/Graphics/Renderer.hpp`（`IModel`、`IRenderer`）

---

## 一、实现概述

### 类划分
| 组件 | 职责 |
|------|------|
| `ModelSharedComponent_D3D11` | 所有模型**共享**的资源：默认纹理/采样器、shader 变体、常量缓冲、渲染状态、input layout |
| `Model_D3D11` | **单个模型**：加载 tinygltf、构建 `model_block`（每 primitive 一组 GPU 缓冲）、运行期变换（scale/rotation/translation）、绘制 |
| `Sunshine` | 光照结构：环境光 + 方向光 + 位置（结构体专用于着色器常量上传） |

### 加载流程（`Model_D3D11::createResources`）
1. 通过自定义 `FileSystemWrapper`（回接到引擎虚拟文件系统）让 tinygltf 读文件
2. `.gltf` → `LoadASCIIFromFile`，否则（`.glb`）→ `LoadBinaryFromFile`
3. `createImage`：每张纹理建 D3D11 纹理 + SRV，并 `GenerateMips` 生成 mipmap
4. `createSampler`：gltf filter/wrap → D3D11 映射（**但最后 filter 被强行覆盖为各向异性**）
5. `createModelBlock` → `processNode`：**递归遍历场景节点树**，每 primitive 建一个 `ModelBlock`：
   - `getBufferFromAccessor` 从 gltf buffer 拷出 POSITION / NORMAL / TEXCOORD_0 / COLOR_0，各建**独立顶点缓冲**（分离式布局，stride 固定 3/3/2/3 float）
   - 索引缓冲（支持 8/16/32 位，1 字节自动扩到 2 字节），`DXGI_FORMAT_R16/R32_UINT`
   - `local_matrix`：TRS 累积（矩阵栈 `mTRS_stack`），末尾乘 `Scaling(1,1,-1)` 做**右手→左手系**转换；`local_matrix_normal` 为其逆转置（法线矩阵）
   - 材质信息：`base_color`、Alpha 模式（MASK/BLEND）、`doubleSided`
   - 图元拓扑映射（支持 TRIANGLES/STRIP/POINTS/LINES；LINE_LOOP/FAN 为 `assert(false)`）

### 绘制流程（`Model_D3D11::draw(fog)`）
按 alpha 模式**三趟提交**：不透明 → Alpha Mask → Alpha Blend（当前实际走 **ScreenDoor** 算法）。
每 block：
1. 选 input layout（有无顶点色 `input_layout` / `input_layout_vc`）
2. 按 有无纹理 × 有无顶点色 × fog × alpha 模式 从庞大的 shader 矩阵里选 VS/PS
3. 上传 `cbo_mlw`（local-world + 逆转置）、`cbo_alpha`、`cbo_light`
4. 绑 4 个顶点缓冲 + 索引缓冲，`drawIndexed` / `draw`
5. 结束时 `clear_state` 做部分解绑

### 着色器（`Model_Shader_D3D11.cpp` 内嵌 HLSL）
- 单一大字符串 + 运行时 D3DCompiler 动态编译（`vs_4_0` / `ps_4_0`）
- VS：`VS_Main`（无顶点色）/ `VS_Main_VertexColor`
- PS：以 `PS_Main` 为核心，按宏 `FOG_ENABLE + FOG_LINEAR/EXP/EXP2`、有无纹理、有无顶点色、Alpha 模式、ScreenDoor 等出**大量变体**
- 光照：`ApplySimpleLight` = `ambient*ambient.a + sunshine_color*sunshine_color.a*max(0,dot(N,-dir))`（仅 Lambert 漫反射，无高光，相关高光代码已注释）
- 伽马：纹理 `pow(rgb,2.2)`（sRGB→线性），运算后 `pow(1/2.2)`（线性→sRGB）
- 常量槽：VS `b0=ProjectionMatrix, b1=LocalWorld+(Normal)`；PS `b0=camera, b1=fog, b2=alpha, b3=light`

---

## 二、问题清单（按严重度分组）

### 严重度分类
- **P0** 正确性 / 稳定性
- **P1** 架构 / 可维护性
- **P2** 功能完整性 / 性能
- **P3** 整理型

### 一、分层违规：平台特定代码放进抽象层（P1）

`engine/graphics` 既成约定：`core/*` 放后端无关抽象，`d3d11/*` 放具体实现。
但模型实现是 `core/Graphics/Model_D3D11.{hpp,cpp}` —— **D3D11 专用代码长在抽象层目录**，`.cpp` 直接 `#include "d3d11/GraphicsDevice.hpp"` 并 new `ID3D11Buffer`。

- 抽象层因此泄漏了平台；未来加 D3D12/Vulkan 后端时模型子系统必须重写，无法复用任何抽象实现（`IModel` 只是 `Renderer.hpp` 里的薄接口）。
- 对比 `core/Graphics/Mesh.hpp`（抽象）+ `core/Graphics/Direct3D11/Mesh.*`（实现），也不够干净，但至少拆了抽象/实现。

> 建议：`Model_D3D11` 作为 `d3d11/` 中 `IModel` 的后端实现，抽象层只保留 `IModel`。

---

### 二、绕过渲染器的管线抽象，形成"两套并行后端"（P1）

现代 2D 路径（`Renderer_D3D11`）用 `IGraphicsPipeline`（PSO）+ 5 维管线缓存矩阵预组合状态；
而 `Model_D3D11::draw()` 是**完全手写 raw D3D11 immediate state**：逐条 `RSSetState` / `OMSetDepthStencilState` / `OMSetBlendState` / `VSSetShader` / `PSSetShader`，不经过 PSO、不参与状态缓存、不用 `IGraphicsCommandBuffer::bindGraphicsPipeline`。

后果：
- 同一渲染器内存在两套互不相通的状态绑定与清理代码，维护/行为分歧风险加倍。
- 状态组合（深度×混合×光栅×shader）无共享、无去重，逐 block 最坏情况状态切换。

> 建议：模型路径构造 `IGraphicsPipeline`（把现状的"状态+shader 组合"沉淀为 PSO），并经通用 command buffer 提交，与 2D 共享状态管理。

---

### 三、对 Renderer 常量缓冲的隐式槽位依赖（P0，最微妙）

模型 shader 声明槽位：VS `b0=ViewProjection, b1=LocalWorld`；PS `b0=Camera, b1=fog, b2=alpha, b3=light`。

但 `Model_D3D11::draw()`：
- **从不绑定 VS `b0`** —— 只 `VSSetConstantBuffers(1,1,{cbo_mlw})` 设 `b1`
- **从不绑定 PS `b0`/`b1`** —— 只 `PSSetConstantBuffers(2,2,{cbo_alpha,cbo_light})` 设 `b2/b3`

它依赖注释里"by Renderer at register(b0)"——即 **2D Renderer 之前 `uploadCameraState` / `uploadFogState` 顺手绑在 `b0/b1` 的缓冲残留**。也就是说 3D 模型能否正确显示，取决于一次**不成文的副作用**：必须先经过 2D 渲染器上传相机/VP/雾。

> 违背"显式契约优于隐式全局状态"原则，是典型模块间隐藏耦合。
> 建议：`Model::draw(IRenderer*)` 显式获取并绑定相机/VP/fog（或由 Renderer 统一上传一次再进入模型循环）；槽位常量收敛到单一来源 `d3d11/SlotConstants.hpp`，消除两套 shader 各自用字面量写 register 的重复约定。

---

### 四、Shader 系统问题（P2）

1. **两套独立 HLSL + 各自运行时编译**：3D（`Model_Shader_D3D11.cpp`）与 2D（`Renderer_Shader_D3D11.cpp`）都是内嵌大字符串 + D3DCompiler 动态编译 + 宏展开，无共享 HLSL；`SlotConstants` 语义改一处需同步两处。
2. **排列爆炸**：`ModelSharedComponent_D3D11` 堆了 `shader_pixel[_alpha][_inv_alpha][_nt][_vc][_sd]` × FogState，draw 用**多层 if/else** 挑选。光模型就动态编译二三十个变体，每次启动重复编译，无法跨进程缓存。
3. **大量死代码**：`shader_pixel_inv_alpha*` 整族编译了却从未在 `draw()` 排程；真正 `set_alpha_mode_blend` 也被跳过。

> 建议：用 GPU 分支 / 减少宏维度收敛变体；删除死函数；预编译（.cso）并统一 shader 仓库、共享公共 HLSL。

---

### 五、"glTF 2.0" 名不副实 / 光照不可达 / PBR 缺席（P2）

- **只读 `baseColorFactor` + `baseColorTexture`**；`metallicRoughness`、`normalTexture`、`occlusionTexture`、`emissiveTexture` 全部忽略。光照为手写**环境光 + 单一方向漫反射（Lambert）**（`Sunshine`），无高光（相关代码注释掉）。本质是"能解 gltf 容器的贴图网格渲染器"，非 PBR 渲染器。
- **`IModel::setAmbient` / `setDirectionalLight` 是死 API**：Lua 绑定 `lib_drawModel` 只调 `setScaling` / `setRotation` / `setPosition` / `drawModel`，**从不调用**这两个光设置方法。默认光照恒定全白环境光，脚本无法调整灯光。接口承诺了能力，实际不可达。

> 建议：要么在 Lua 绑定补齐光照接口并支持多方向光/高光；要么从 `IModel` 删掉这两个虚拟方法，避免虚假 API 面。

---

### 六、无实例化 / 无资源共享（与弹幕游戏需求相悖）（P1）

弹幕引擎常需**大量复用同一模型的敌机**，但：
- 每个 `Model_D3D11` 独立 `LoadBinaryFromFile` 并从磁盘重建**全套顶点/法线/UV/索引缓冲**（属性各自独立，非交织），兄弟实例零共享。
- **完全不用 GPU instancing**，尽管 `IGraphicsCommandBuffer` 已支持 `drawIndexedInstanced`。

> 建议：同一 gltf 的 GPU 缓冲**共享**（引用计数），绘制走 `drawIndexedInstanced`；这是内存问题更是弹幕场景帧率瓶颈。

---

### 七、正确性 / 健壮性 Bug（P0）

1. `getBufferFromAccessor`：**越界只 `Logger::error` 却不 `return false`**，随后仍 `model.bufferViews[...]` / `model.buffers[...]` 越界解引用 —— 坏文件可直接崩溃。
2. 索引缓冲对 `bufferView.byteStride > 0` 是**空桩**（`std::ignore = nullptr;`）—— 交错索引数据静默出错。
3. `alphaMode: BLEND` 被**静默替换成 ScreenDoor**（`set_alpha_mode_blend` 注释掉、改调 screen door）——真实透明混合丢失，带屏幕门闪烁伪影。
4. `createSampler` 把 gltf 指定的 filter **强行覆盖为各向异性**（`// TODO: better?`）——最近邻 / 周期性纹理期望被破坏。
5. 图像在 shader 里手动 `pow(,2.2)` 做 gamma，而非用 sRGB 格式 + 硬件采样。

---

### 八、其他架构权衡（P3）

- **状态泄漏 / 清理不完整**：`clear_state` 只 unbind VB/IB、VS `b1`、PS 采样器/SRV/`b2/b3`；深度、混合、光栅、输入布局、VS/PS 本体**留着不还原**，靠 2D 之后每帧重新覆盖兜底 —— 脆弱。
- **坐标系耦合**：右手→左手转换 `Scaling(1,1,-1)` 与法线逆转置**硬编码在节点遍历 `processNode`** 里，把"文件格式约定"与"引擎内部约定"搅在一起。
- **同步阻塞加载**：`createResources` 创建线程同步读盘；无异步 / 流式加载。
- **`SetStoreOriginalJSONForExtrasAndExtensions(true)` 却从未读回** —— extension 支持是"预留但未实现"。
- **无裁剪 / LOD / 动画 / skin** —— 单默认场景、无骨骼动画。对弹幕引擎或可接受，但应作为**明确文档化限制**，而非无条件声称"3D 模型支持"。

---

## 三、优先级与建议动作

| 优先级 | 项 | 建议动作 |
|--------|----|---------|
| P0 | 隐式常量槽依赖（#三） | 固定显式绑定相机/VP/fog |
| P0 | 越界崩溃（#七.1） | `getBufferFromAccessor` 加边界校验并正确返回 |
| P1 | 绕过 PSO 的分叉后端（#二） | 模型路径并入 `IGraphicsPipeline` 缓存 |
| P1 | 实例化 / 共享缺失（#六） | buffer 共享 + `drawIndexedInstanced` |
| P2 | PBR / 光照 API 名不副实（#五） | 补 Lua 绑定，或删接口 |
| P2 | shader 排列 / 死代码（#四） | 收敛变体、删死代码、预编译 |
| P3 | 分层 / 坐标系 / 同步加载（#一 | #八） | 随重构整理 |

---

## 四、待办跟踪

- [ ] P0：模型绘制显式绑定相机/VP/fog 常量缓冲，去除对 2D 渲染器的隐式残留绑定依赖
- [ ] P0：`getBufferFromAccessor` 对 bufferView / buffer 越界返回 false，杜绝坏文件崩溃
- [ ] P1：模型路径重构到 `IGraphicsPipeline`，复用 PSO 状态缓存
- [ ] P1：同模型 GPU 缓冲共享 + instancing
- [ ] P2：决策：补齐模型的真实照明 / 或从 `IModel` 移除不可达的光设置接口
- [ ] P2：收敛 shader 变体矩阵、清理死代码（`inv_alpha` 族、`set_alpha_mode_blend`）
- [ ] P3：`Model_D3D11` 移到 `d3d11/`，抽象层只留 `IModel`