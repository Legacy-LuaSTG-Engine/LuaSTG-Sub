# 图元拓扑方案决策记录 —— 放弃动态切换，拓扑在创建管线时固定

> 状态：**方案撤销**（2026-08-27 决策变更，由"设计稿/待实现"改为决策记录）
> 撤销对象：本文档初版设计「基本类型（point/line/triangle）烘焙 PSO + 子类型（list/strip）经
> `setPrimitiveTopology` 运行期动态切换」
> 关联：`plans/model-v2.md`（§2.1、§四、§5.7、§5.9.4、§六、§七、§八、§九）与 `reviews/model.md`
> 已按本决策同步修订。

---

## 一、初版方案回顾（已撤销）

初版曾计划（见 git 历史 `93dc0add` 及之前版本）：

- 新增 `GraphicsPrimitiveTopologyType`（points/lines/triangles）枚举与 `toTopologyType` 映射；
- `GraphicsPipelineState` 增 `primitive_topology_type` 字段，PSO 只烘焙**基本类型**；
- `IGraphicsCommandBuffer` 新增 `setPrimitiveTopology(GraphicsPrimitiveType)`，**子类型**运行期切换；
- D3D11 后端自校验（debug assert + release warn）兜底基本类型/子类型错配。

动机是同时规避「PSO × 具体拓扑」组合爆炸，并向 D3D12（`PRIMITIVE_TOPOLOGY_TYPE` 创建期固定 +
命令列表细分）与 Vulkan 1.3 动态拓扑对齐。

## 二、放弃理由

1. **Metal 不支持运行期拓扑切换**：`MTLRenderPipelineDescriptor.primitiveTopology` 在管线对象
   创建期固定，命令编码层没有等价切换 API。跨后端抽象不能建立在 Metal 缺失的能力上。
2. **Vulkan 侧生态覆盖率不乐观**：dynamic state 已是 Vulkan 1.3 的核心功能，但存量设备/驱动对
   Vulkan 1.3 的支持参差，未达 1.3 时还需回退依赖 dynamic state 扩展；抽象层以其为
   设计基线不可持续。
3. **收益本就有限**：实际 glTF 资产同一模型内混用 list/strip 少见，初版试图避免的
   「PSO × 具体拓扑」组合爆炸规模可控（见 §三末）。

## 三、替代方案（采纳）：完整拓扑固定于 PSO

- 拓扑 = **完整** `GraphicsPrimitiveType`（triangle_list / triangle_strip / line_list / line_strip /
  point_list 共 5 值），在 `GraphicsPipelineState` 创建时固定，**不再新增任何运行期切换 API**。
- **与现有代码一致，抽象层零改动**：`d3d11/GraphicsPipeline.cpp` 已在管线创建时
  `toPrimitiveType(create_info.primitive_type)` 烘焙，`apply()` 绑定时 `IASetPrimitiveTopology`。
- 初版新增项全部撤销：`GraphicsPrimitiveTopologyType`、`toTopologyType`、
  `GraphicsPipelineState::primitive_topology_type`、`IGraphicsCommandBuffer::setPrimitiveTopology`、
  D3D11 基本类型校验——均不实现。
- **管线缓存键含完整拓扑**：`属性签名 × 拓扑(≤5) × fog × alpha × 单双面`
  （对应修正 `model-v2.md` §5.7 / §5.9.4；组合上界 ×5，但模型通常单拓扑、2D 恒
  triangle_list，实际命中数很小，惰性缓存足以消化）。
- 拓扑不同的 primitive 经**切换 PSO** 提交；模型内混排拓扑（少见）可按拓扑分组排序，
  减少 PSO 绑定次数。
- glTF `LINE_LOOP / TRIANGLE_FAN` 仍在**加载期强制转换**为 LINES / TRIANGLES +
  `Logger::warn`（与动态切换无关，维持 `model-v2.md` §四 原决策）。

## 四、里程碑（修订）

- [x] ~~M-PT1..M-PT5：枚举 / PSO 字段 / `setPrimitiveTopology` / 校验 / 构建验证~~ —— **全部撤销**，
      现抽象层既有形态即最终设计，零改动
- [x] M-PT6：`model-v2.md` 相关章节（§2.1、§四、§5.7、§5.9.4、§六、§七、§八、§九）同步修订为
      「拓扑固定于 PSO 创建」
- [ ] model-v2 M2 联调时验证：模型路径不依赖任何动态拓扑 API，拓扑变化一律走 `bindGraphicsPipeline`
