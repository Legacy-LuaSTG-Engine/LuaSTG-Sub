# 图元拓扑抽象改造 —— setPrimitiveTopology（提前落地项）

> 目标：把「图元拓扑」拆成**基本类型（点/线/三角形）**与**具体子类型（list/strip）**两个正交维度，
> 为 `IGraphicsCommandBuffer` 增加运行时子类型切换 `setPrimitiveTopology`，并作为 `plans/model-v2.md` 的前置改造**提前落地**。
>
> 关联：`plans/model-v2.md`（§2.1、§5.7、§5.9.4、§六、§7、§八、§九）、`reviews/model.md`
> 状态：设计稿 / 待实现
> 审批结论：**采用**（评估见对话记录/§四 收益与代价）

---

## 一、动机

模型一个 glTF 场景内不同 primitive 可能混用 `TRIANGLES / TRIANGLE_STRIP / POINTS / LINES`。
若把具体拓扑烘焙进 PSO，会让「PSO × 具体拓扑数」组合爆炸；但完全动态（子类型 + 基本类型都随意切）又为未来
D3D12 / Vulkan 埋下返工——两者要求**基本类型在创建期固定**。

故采用分层模型：
- **基本类型**（point / line / triangle）→ 创建管线状态（PSO）时配置，**固定不可变**；
- **具体子类型**（list / strip）→ 运行期经 `setPrimitiveTopology` **动态切换**（同一基本类型内）。

这与 **D3D12**（PSO 内 `D3D12_PRIMITIVE_TOPOLOGY_TYPE` = POINT/LINE/TRIANGLE/PATCH +
命令列表 `IASetPrimitiveTopology` 细分）和 **Vulkan 1.3 动态拓扑**（仅同类别内切换）一致，属「向现代 API 对齐」。

---

## 二、设计

### 2.1 枚举：基本类型（新增）

```cpp
// core/GraphicsPipeline.hpp
// 基本类型：与 glTF mode / D3D12 PRIMITIVE_TOPOLOGY_TYPE 对齐
enum class GraphicsPrimitiveTopologyType : uint8_t {
    points    = 0,
    lines     = 1,
    triangles = 2,
};

// 现有具体子类型枚举（保持不变）
enum class GraphicsPrimitiveType : uint8_t {
    point_list, line_list, line_strip, triangle_list, triangle_strip,
};
```

> glTF 的 `LINE_LOOP / TRIANGLE_FAN` 本枚举不存在——在模型加载期转换为 `line_list / triangle_list`（见 `model-v2.md` §四），
> 转换后归属基本类型 lines/triangles，**不进入 `setPrimitiveTopology`**。

### 2.2 子类型 → 基本类型映射

```cpp
constexpr GraphicsPrimitiveTopologyType toTopologyType(GraphicsPrimitiveType t) noexcept {
    switch (t) {
        case GraphicsPrimitiveType::point_list:      return GraphicsPrimitiveTopologyType::points;
        case GraphicsPrimitiveType::line_list:
        case GraphicsPrimitiveType::line_strip:      return GraphicsPrimitiveTopologyType::lines;
        case GraphicsPrimitiveType::triangle_list:
        case GraphicsPrimitiveType::triangle_strip:  return GraphicsPrimitiveType::triangles;
        default:                                     return GraphicsPrimitiveType::triangles; // 安全默认
    }
}
```

### 2.3 PSO 结构：烘焙基本类型（替代烘焙具体子类型）

```cpp
// core/GraphicsPipeline.hpp —— GraphicsPipelineState 新增字段
GraphicsPrimitiveTopologyType primitive_topology_type
    = GraphicsPrimitiveTopologyType::triangles;   // 默认 triangles，向后兼容 2D（固定 triangle_list）
```

- 现有 `primitive_type`（具体子类型）**不再参与 PSO**（或保留但恒为默认、不参与判断）。
- **2D 渲染器**现固定 `triangle_list` → 基本类型恒 `triangles`，**默认值直接生效，零改动**兼容。

### 2.4 命令缓冲：运行时子类型切换

```cpp
// core/GraphicsDevice.hpp —— IGraphicsCommandBuffer 新增纯虚成员
virtual void setPrimitiveTopology(GraphicsPrimitiveType topology) = 0;
```

- 仅接受**具体子类型**；基本类型不由此传入（由 PSO 决定）。
- 语义与 D3D11 `IASetPrimitiveTopology` 一致，贴合 `Mesh.cpp` 先例。

---

## 三、D3D11 后端实现要点

### 3.1 PSO 创建
- `GraphicsPipelineState.primitive_topology_type` → 存入后端管线对象；用其驱动后续校验。
- D3D11 的 `CreateInputLayout` / shader 绑定**无**基本类型字段——基本类型只作**约定 + 校验**，不由驱动强制。

### 3.2 绑定与校验（软约束 = 自校验）
D3D11 的 `IASetPrimitiveTopology` **不会校验基本类型**，即使不匹配也能画（只是结果错）。因此：
- `bindGraphicsPipeline(pso)` 时记录该管线基本类型 `bound_type`；
- `setPrimitiveTopology(t)` 时校验 `toTopologyType(t) == bound_type`；
- **不匹配策略**（需定死，三选一）：
  - debug 构建 `assert(false)`；
  - release 构建 `Logger::warn` 记录一次，并**忽略/钳制到匹配**（避免静默画错）；
  - 或按最近绑定的 PSO 基本类型**自动纠正**子类型。
  > 建议：debug assert + release warn，不自动纠正（让调用方显式负责）。

### 3.3 与既有状态的协调
- PSO 内不再有具体子类型烘焙；`setPrimitiveTopology` 成为与深度/混合等并列的 **IA 期可变状态**，两条路径（2D PSO 路径 / 模型路径）共用同一实现。

---

## 四、收益与代价

### 收益
| 项 | 说明 |
|----|------|
| 消解「两处真理」 | 基本类型归 PSO、子类型归 IA——`model-v2.md` §九 该风险自然消除 |
| 子类型切换廉价 | `list↔strip` 一条 `IASetPrimitiveTopology`，不触发 PSO 更换，避免「PSO×具体拓扑」爆炸 |
| 后端可移植 | 与 D3D12 / Vulkan 1.3 静态基本类型约束一致，未来加后端零返工 |
| 类别错配早捕获 | 借校验把 D3D11 静默错误转为断言/告警 |
| 向后兼容 | 默认基本类型 triangles → 2D 固定 triangle_list 不受影响 |

### 代价 / 需处理
| 项 | 说明 |
|----|------|
| **缓存键重引入基本类型** | `model-v2.md` §5.7/§5.9.4 原「拓扑不进缓存键」需**回改为「基本类型进、子类型不进」**，键 = `属性签名 × 基本类型(≤3) × fog × alpha × 单双面`（对上轮结论的设计性回改） |
| 失去跨基本类型切换灵活 | 同 PSO 内 triangle↔line 不再允许；调试线框场景需并置两个 PSO；gltf 基本类型按 primitive 固定，**basecolor 全三角场景实际无影响** |
| 软约束需自校验 | D3D11 驱动不强制，需按 §3.2 的断言/告警策略自行把关 |
| 绘制循环按基本类型分组 | `model-v2.md`  §7 步骤6a 需区分「子类型变→set」「基本类型变→换 PSO」，模型内混排 triangle/line（少见）需分组排序以减少 PSO 切换 |

---

## 五、配套 / 新增 API 汇总
| # | API / 字段 | 类型 | 状态 |
|---|-----------|------|------|
| 1 | `GraphicsPrimitiveTopologyType`（points/lines/triangles） | 枚举 | 新增 |
| 2 | `toTopologyType(GraphicsPrimitiveType)` | 映射 | 新增 |
| 3 | `GraphicsPipelineState::primitive_topology_type`（默认 triangles） | PSO 字段 | 新增 |
| 4 | `IGraphicsCommandBuffer::setPrimitiveTopology(GraphicsPrimitiveType)` | 命令缓冲 | 新增 |
| 5 | 后端绑定 + 基本类型校验 + 不匹配策略 | D3D11 实现 | 新增 |

---

## 六、迁移步骤（里程碑）
- [ ] M-PT1：新增 `GraphicsPrimitiveTopologyType` 枚举 + `toTopologyType` 映射
- [ ] M-PT2：`GraphicsPipelineState` 增 `primitive_topology_type` 字段（默认 triangles）；确认 2D 创建路径零改动
- [ ] M-PT3：`IGraphicsCommandBuffer::setPrimitiveTopology(GraphicsPrimitiveType)` + D3D11 实现
- [ ] M-PT4：D3D11 `bindGraphicsPipeline` 记录基本类型 + `setPrimitiveTopology` 校验 + 不匹配策略（assert/warn）
- [ ] M-PT5：增量构建验证（`windows-vs2026-amd64-release`）；2D 画面不回归
- [ ] M-PT6：联调 `model-v2.md` M1/M2 时改走子类型 set + 基本类型烘焙 PSO；更新其 §5.7/§5.9.4 缓存键

---

## 七、风险与注意事项
- **缓存键语义回改**：`model-v2.md` §5.7/§5.9.4 曾写「拓扑不进缓存键」，本计划将之改为「**基本类型进键、子类型不进**」——落地时同步修正，避免前后矛盾。
- **软约束依赖调用方**：D3D11 不自校验；若调用方绑定 PSO 后设了不匹配子类型，release 下由 warn 兜底。
- **兼容默认值**：`primitive_topology_type` 默认 triangles，2D 与 Mesh 路径不显式设置也不破坏。
- **PSO 缓存 hash**：若基本类型加入 `GraphicsPipelineState`，缓存键须对字段值（而非指针）哈希。