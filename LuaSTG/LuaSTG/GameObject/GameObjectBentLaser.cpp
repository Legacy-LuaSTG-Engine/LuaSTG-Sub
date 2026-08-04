#include "GameObject/GameObjectBentLaser.hpp"
#include "AppFrame.h"
#include <algorithm>

using namespace luastg;

//------------------------------------------------------------------------------

static std::pmr::unsynchronized_pool_resource s_game_object_curve_laser_pool;
#ifndef NDEBUG
static size_t s_game_object_curve_laser_count = 0;
static size_t s_game_object_curve_laser_memory_usage = 0;
#endif

GameObjectBentLaser* GameObjectBentLaser::AllocInstance()
{
#ifndef NDEBUG
	s_game_object_curve_laser_count += 1;
	s_game_object_curve_laser_memory_usage += sizeof(GameObjectBentLaser);
#endif
	//潜在bad_alloc已在luawrapper中处理
	auto* pRet = static_cast<GameObjectBentLaser*>(
		s_game_object_curve_laser_pool.allocate(
			sizeof(GameObjectBentLaser),
			alignof(GameObjectBentLaser)
		)
	);
	new(pRet) GameObjectBentLaser();
	return pRet;
}

void GameObjectBentLaser::FreeInstance(GameObjectBentLaser* p)
{
	p->~GameObjectBentLaser();
	s_game_object_curve_laser_pool.deallocate(p,
		sizeof(GameObjectBentLaser), alignof(GameObjectBentLaser));
#ifndef NDEBUG
	s_game_object_curve_laser_count -= 1;
	s_game_object_curve_laser_memory_usage -= sizeof(GameObjectBentLaser);
#endif
	return;
}

GameObjectBentLaser::GameObjectBentLaser() noexcept
{
}

GameObjectBentLaser::~GameObjectBentLaser() noexcept
{
}

void GameObjectBentLaser::_UpdateNodeVertexExtend(size_t i) noexcept
{
	// 当前节点，先去掉大转角标记
	LaserNode& node = m_Queue[i];
	node.sharp = false;

	// 只有一个节点，也就是自己的情况
	if (m_Queue.size() <= 1)
	{
		// 不渲染
		node.x_dir = 0.0f;
		node.y_dir = 0.0f;
		return;
	}

	// 对首尾的情况特殊处理
	if (i == 0)
	{
		// 是首节点（老节点），利用下一个节点的位置来计算节点朝向
		LaserNode& next = m_Queue[i + 1]; // 下一个节点（相对更新的节点）
		core::Vector2F const vec = next.pos - node.pos;

		// 逆时针旋转 90 度
		core::Vector2F const vecr(vec.y, -vec.x);

		// 归一化向量，保存
		core::Vector2F const vecn = vecr.normalized();
		node.x_dir = vecn.x;
		node.y_dir = vecn.y;
		return;
	}
	else if (i == (m_Queue.size() - 1))
	{
		// 是尾节点（新节点），利用上一个节点的位置来计算节点朝向
		LaserNode& last = m_Queue[i - 1]; // 上一个节点（相对更老的节点）
		core::Vector2F const vec = node.pos - last.pos;

		// 逆时针旋转 90 度
		core::Vector2F const vecr(vec.y, -vec.x);

		// 归一化向量，保存
		core::Vector2F const vecn = vecr.normalized();
		node.x_dir = vecn.x;
		node.y_dir = vecn.y;
		return;
	}

	// 节点在中间，两侧均有节点
	LaserNode& last = m_Queue[i - 1]; // 上一个节点（相对更老的节点）
	LaserNode& next = m_Queue[i + 1]; // 下一个节点（相对更新的节点）

	// 计算向量以及点积
	core::Vector2F const vec1 = node.pos - last.pos;
	core::Vector2F const vec2 = next.pos - node.pos;
	float const dotv = vec1.dot(vec2);

	// 转角小于 90 度的情况
	if (dotv > 0.0f)
	{
		// 将两个向量都逆时针旋转 90 度
		// A = -pi / 2
		// cosA = 0
		// sinA = -1
		// x = X * cosA - Y * sinA = Y
		// y = X * sinA + Y * cosA = -X
		core::Vector2F const vec1r(vec1.y, -vec1.x);
		core::Vector2F const vec2r(vec2.y, -vec2.x);
		
		// 向量求和，得到延展向量
		core::Vector2F const vec = vec1r + vec2r;

		// 归一化向量，保存
		core::Vector2F const vecn = vec.normalized();
		node.x_dir = vecn.x;
		node.y_dir = vecn.y;
		return;
	}

	// 转角大于等于 90 度的情况
	else
	{
		// 向量求和，得到延展向量
		core::Vector2F const vec = -vec1 + vec2;

		// 归一化向量，保存
		core::Vector2F const vecn = vec.normalized();
		node.x_dir = vecn.x;
		node.y_dir = vecn.y;
		return;
	}

	// EX+ 的写法
	/*
	LaserNode& cur = m_Queue[i];
	cur.sharp = false;
	int sz = m_Queue.size() - 1;
	if (sz > 0)
	{
		float curcos = 0.0f, cursin = 0.0f;
		float nextcos = 0.0f, nextsin = 0.0f;
		cursin = sin(cur.rot);
		curcos = cos(cur.rot);
		if (i == 0 || i == sz)
		{
			//头节点或为尾节点，顶点朝向为节点朝向的垂直方向
			cur.x_dir = cursin;
			cur.y_dir = -curcos;
			return; // 不需要继续计算
		}
		else
		{
			//中间节点，计算出下一个节点的朝向
			LaserNode& next = m_Queue[i + 1];
			nextcos = cos(next.rot);
			nextsin = sin(next.rot);
		}
		float d1 = curcos * nextsin - nextcos * cursin; // 行列式，平行四边形的面积
		if (d1 < 0.3 && d1 > -0.3) {
			//处于较小的锐角或较大的钝角时
			float d2 = curcos * nextcos + cursin * nextsin; // 点积
			if (d2 > 0.0f) {
				//锐角，曲线激光比较平直
				if (d1<0.01 && d1>-0.01)
				{
					//平行四边形法则
					cur.x_dir = (cursin + nextsin) / 2;
					cur.y_dir = (-curcos - nextcos) / 2;
					return;//不需要继续计算
				}
			}
			else {
				//钝角，曲线激光弯曲程度较大
				cur.sharp = true;
				//翻转向量
				nextcos = -nextcos;
				nextsin = -nextsin;
				d1 = -d1;
			}
		}
		//处于接近90度的锐角或钝角时
		cur.x_dir = (curcos - nextcos) / d1;
		cur.y_dir = (cursin - nextsin) / d1;
	}
	//*/
}

void GameObjectBentLaser::_UpdateAllNode() noexcept
{
	// 无论如何都重置长度
	m_fLength = 0.0f;

	// 检查节点数量
	size_t const node_count = m_Queue.size();
	if (node_count == 0)
	{
		return;
	}
	else if (node_count == 1)
	{
		LaserNode& node = m_Queue[0];
		//node.rot = 0.0f; // 保留不变
		node.dis = 0.0f;
		node.x_dir = 0.0f;
		node.y_dir = 0.0f;
		node.sharp = false;
		return;
	}

	// 更新所有节点间的距离和节点的朝向以及总长度，注意，尾部才是新节点，头部是最早的老节点
	m_Queue[0].dis = 0.0f; // 已经没有上一个节点了，它就是最老的那个
	for (size_t i = 0; i < (node_count - 1); i += 1)
	{
		LaserNode& last = m_Queue[i];     // 靠近头部（老节点）
		LaserNode& next = m_Queue[i + 1]; // 靠近尾部（新节点）
		core::Vector2F const vec = next.pos - last.pos;
		float const len = vec.length();
		//if (len > std::numeric_limits<float>::min())
		//{
		//	next.rot = vec.CalcuAngle(); // 相对上一个节点的角度就是自身的朝向
		//}
		//else
		//{
		//	next.rot = last.rot; // 特殊情况，距离太小计算出来的朝向无意义，直接用上一个节点的朝向
		//}
		// 节点间距离，与上一个节点的距离
		next.dis = len;
		// 总长度
		if (last.active && next.active)
		{
			m_fLength += len; // TODO: 可能会影响渲染
		}
	}
	//m_Queue[0].rot = m_Queue[1].rot; // 让最老的节点的朝向也与下一个节点的一致，这里这么做的原因是，所有的点的位置都被修改了，因此它的朝向可能已经过时

	// 更新所有节点的延展向量
	for (size_t i = 0u; i < node_count; i += 1)
	{
		_UpdateNodeVertexExtend(i);
	}
}

void GameObjectBentLaser::_PopHead() noexcept
{
	if (m_Queue.size() > 1)
	{
		LaserNode const& last = m_Queue.popHead(); // 最老的节点
		if (m_Queue.size() > 1)
		{
			LaserNode& next = m_Queue.head(); // 下一个节点
			// 如果最后两个节点都是激活的，根据节点间的距离减少曲线激光总长度
			if (last.active && next.active)
			{
				m_fLength -= next.dis; // 到上一个节点的距离
			}
			// 让 next 成为最老的节点
			next.dis = 0.0f; // 已经没有上一个节点
		}
	}
}

//------------------------------------------------------------------------------

int GameObjectBentLaser::GetSize() noexcept
{
	return (int)m_Queue.size();
}

GameObjectBentLaser::LaserNode* GameObjectBentLaser::GetNode(size_t i) noexcept
{
	if (m_Queue.size() > 0u)
	{
		if (i >= 0u && i < m_Queue.size())
		{
			return &(m_Queue[i]);
		}
	}
	return nullptr;
}

void GameObjectBentLaser::GetEnvelope(float& height, float& base, float& rate, float& power) noexcept
{
	height = m_fEnvelopeHeight;
	base = m_fEnvelopeBase;
	rate = m_fEnvelopeRate;
	power = m_fEnvelopePower;
}

void GameObjectBentLaser::SetEnvelope(float height, float base, float rate, float power) noexcept
{
	m_fEnvelopeHeight = height;
	m_fEnvelopeBase = std::clamp(base, 0.0f, 1.0f);
	m_fEnvelopeRate = rate;
	m_fEnvelopePower = 0.4f * std::floorf(power / 0.4f); // 不要问，问就是魔法数字
}

bool GameObjectBentLaser::Update(size_t id, int length, float width, bool active) noexcept
{
	GameObject* p = LPOOL.GetPooledObject(id);
	if (!p)
	{
		spdlog::error("[luastg] [GameObjectBentLaser::Update] 无效的lstg.GameObject");
		return false;
	}
	return Update((float)p->x, (float)p->y, (float)p->rot, length, width, active);
}

bool GameObjectBentLaser::Update(float x, float y, float rot, int length, float width, bool active) noexcept
{
	std::ignore = rot; // 不再使用到

	if (length <= 1)
	{
		spdlog::error("[luastg] [GameObjectBentLaser::Update] 无效的参数length={}", length);
		return false;
	}

	// ！循环队列的头部是最早创建的，尾部才是最新放入的！

	// 准备插入的新节点
	LaserNode node{};
	node.pos.x = x;
	node.pos.y = y;
	node.half_width = width * 0.5f;
	node.active = active;

	// 变化几乎可以忽略不计，我们可以直接修改最新的节点
	if (!m_Queue.empty() && (node.pos - m_Queue.tail().pos).length() <= std::numeric_limits<float>::min())
	{
		// 移除多余的节点，保证长度在 length 范围内
		while (m_Queue.size() >= (size_t)length)
		{
			_PopHead();
		}

		// 修改最新的节点
		if (m_Queue.size() >= 2)
		{
			LaserNode& prev = m_Queue[m_Queue.size() - 2];
			LaserNode& last = m_Queue[m_Queue.size() - 1];
			// 修改坐标
			last.pos = node.pos;
			//last.half_width = node.half_width; // 保留宽度
			// 修改到上一个节点的距离和重新计算总长度
			m_fLength -= last.dis;
			float const len_ = (last.pos - prev.pos).length();
			last.dis = len_;
			m_fLength += len_;
			// 修改激活状态
			//last.active = node.active; // 保留激活状态
			// 更新节点
			_UpdateNodeVertexExtend(m_Queue.size() - 1);
			_UpdateNodeVertexExtend(m_Queue.size() - 2);
		}
		else
		{
			LaserNode& last = m_Queue[m_Queue.size() - 1];
			// 修改坐标和宽度
			last.pos = node.pos;
			last.half_width = node.half_width;
			// 到上一个节点的距离和总长度直接归 0
			last.dis = 0.0f;
			m_fLength = 0.0f;
			// 修改激活状态
			last.active = node.active;
			// 不更新节点，等节点数量超过 1 再更新
		}
		// 重新应用最新节点的活动状态（如熄灭状态）
		if (!m_Queue.empty())
		{
			m_Queue.tail().active = active;
		}
		// 重新应用切割区：断口随激光本体移动（最后应用，切割区内的节点保持不活动）
		_ApplyCutRanges();
		return true;
	}

	// 否则，插入新节点
	else
	{
		// 移除多余的节点，保证长度在 length 范围内，并空出一个位置插入节点
		while (m_Queue.full() || m_Queue.size() >= (size_t)length)
		{
			_PopHead();
		}

		// 插入新节点
		if (m_Queue.size() > 0)
		{
			LaserNode& last = m_Queue[m_Queue.size() - 1];
			core::Vector2F const vec_ = node.pos - last.pos;
			// 计算到上一个节点的距离和重新计算总长度
			float const len_ = vec_.length();
			node.dis = len_;
			if (active && last.active)
			{
				m_fLength += len_;
			}
			// 【即将废弃】计算朝向
			//node.rot = vec_.CalcuAngle();
			// 插入并更新节点
			m_Queue.pushTail(node);
			_UpdateNodeVertexExtend(m_Queue.size() - 1);
			_UpdateNodeVertexExtend(m_Queue.size() - 2);
		}
		else
		{
			// 到上一个节点的距离和总长度直接归 0
			node.dis = 0.0f;
			m_fLength = 0.0f;
			// 【即将废弃】计算朝向
			//core::Vector2F const vec_((float)p->vy, (float)p->vx);
			//if (vec_.Length() > std::numeric_limits<float>::min())
			//{
			//	node.rot = vec_.CalcuAngle(); // 使用速度方向作为节点朝向
			//}
			//else
			//{
			//	node.rot = (float)p->rot; // 使用游戏对象朝向作为节点朝向
			//}
			// 插入但不更新节点，等节点数量超过 1 再更新
			m_Queue.pushTail(node);
		}
		// 重新应用最新节点的活动状态（如熄灭状态）
		if (!m_Queue.empty())
		{
			m_Queue.tail().active = active;
		}
		// 重新应用切割区：断口随激光本体移动（最后应用，切割区内的节点保持不活动）
		_ApplyCutRanges();
		return true;
	}
}

void GameObjectBentLaser::SetAllWidth(float width)  noexcept
{
	for (size_t i = 0; i < m_Queue.size(); i += 1)
	{
		m_Queue[i].half_width = width / 2.0f;
	}
}

size_t GameObjectBentLaser::CutByPoint(float x, float y, float radius, std::vector<core::Vector2F>* out_cut_positions) noexcept
{
	// 切断指定半径内的节点，被切断的节点不再参与渲染和碰撞
	// 切断以切割区（队列空间索引区间）记录，断口锚定激光尾部，随激光本体移动而移动
	size_t cut_count = 0;
	float const r2 = radius * radius;
	size_t const size = m_Queue.size();
	size_t cut_start = size;
	size_t cut_end = 0;
	for (size_t i = 0; i < size; i += 1)
	{
		LaserNode& node = m_Queue[i];
		if (!node.active)
		{
			continue;
		}
		float const dx = node.pos.x - x;
		float const dy = node.pos.y - y;
		if ((dx * dx + dy * dy) <= r2)
		{
			node.active = false;
			cut_count += 1;
			if (out_cut_positions)
			{
				out_cut_positions->emplace_back(node.pos);
			}
			cut_start = std::min(cut_start, i);
			cut_end = std::max(cut_end, i + 1);
		}
	}
	if (cut_count > 0)
	{
		// 先应用已有切割区（含尾部锚定位移），再添加新切割区（新切割区在当前队列空间，不需要位移）
		_ApplyCutRanges();
		_MergeCutRange(cut_start, cut_end, radius * 2.0f);
		_ApplyCutRanges();
	}
	return cut_count;
}

void GameObjectBentLaser::_MergeCutRange(size_t start, size_t end, float bridge_distance) noexcept
{
	// 与最近的已有切割区做世界距离桥接：同一消除过程（如 cleaner 的一次穿过）造成的断口应连成一条带
	size_t const size = m_Queue.size();
	auto gap_length = [&](size_t a, size_t b) -> float {
		if (a >= size || b >= size)
		{
			return std::numeric_limits<float>::max();
		}
		return (m_Queue[b].pos - m_Queue[a].pos).length();
	};
	for (CutRange const& r : m_CutRanges)
	{
		if (r.end <= start)
		{
			// 已有切割区在当前切割区之前
			size_t const a = r.end > 0 ? r.end - 1 : 0;
			if (gap_length(a, start) <= bridge_distance)
			{
				start = std::min(start, r.start);
				end = std::max(end, r.end);
			}
		}
		else if (r.start >= end)
		{
			// 已有切割区在当前切割区之后
			size_t const b = end < size ? end : (size > 0 ? size - 1 : 0);
			if (gap_length(b, r.start) <= bridge_distance)
			{
				start = std::min(start, r.start);
				end = std::max(end, r.end);
			}
		}
	}

	m_CutRanges.push_back(CutRange{ start, end });
	std::sort(m_CutRanges.begin(), m_CutRanges.end(),
		[](CutRange const& a, CutRange const& b) { return a.start < b.start; });
	std::vector<CutRange> merged;
	for (CutRange const& r : m_CutRanges)
	{
		if (!merged.empty() && r.start <= merged.back().end)
		{
			merged.back().end = std::max(merged.back().end, r.end);
		}
		else
		{
			merged.push_back(r);
		}
	}
	m_CutRanges = std::move(merged);
}

void GameObjectBentLaser::_ApplyCutRanges() noexcept
{
	size_t const size = m_Queue.size();
	if (m_CutRanges.empty())
	{
		m_CutRangesLastSize = size;
		return;
	}
	// 断口锚定激光尾部：队列增长（激光生成阶段只插入不弹出）或滑动（弹出+插入）时，
	// 切割区保持与尾部的距离不变，断口随激光本体前进
	if (size != m_CutRangesLastSize)
	{
		ptrdiff_t const shift = (ptrdiff_t)size - (ptrdiff_t)m_CutRangesLastSize;
		for (CutRange& r : m_CutRanges)
		{
			r.start = (size_t)std::max<ptrdiff_t>(0, (ptrdiff_t)r.start + shift);
			r.end = (size_t)std::max<ptrdiff_t>(0, (ptrdiff_t)r.end + shift);
		}
		m_CutRangesLastSize = size;
	}
	// 恢复所有节点为活动状态，只保留切割区覆盖的节点为不活动
	// 切割区位于队列空间的固定索引区间（锚定激光尾部），随队列滑动（激光前进/后退）而移动
	for (size_t i = 0; i < size; i += 1)
	{
		m_Queue[i].active = true;
	}
	for (CutRange const& r : m_CutRanges)
	{
		size_t const a = std::min(r.start, size);
		size_t const b = std::min(r.end, size);
		for (size_t i = a; i < b; i += 1)
		{
			m_Queue[i].active = false;
		}
	}
	// 移除已完全弹出队列的切割区（激光前进越过断口后自动愈合）
	std::erase_if(m_CutRanges, [size](CutRange const& r) { return r.start >= size; });
}

size_t GameObjectBentLaser::GetActiveNodeCount() noexcept
{
	size_t count = 0;
	for (size_t i = 0; i < m_Queue.size(); i += 1)
	{
		if (m_Queue[i].active)
		{
			count += 1;
		}
	}
	return count;
}

bool GameObjectBentLaser::Render(const char* tex_name, BlendMode blend, core::Color4B c, float tex_left, float tex_top, float tex_width, float tex_height, float scale) noexcept
{
	using namespace core;
	using namespace core::Graphics;

	// 忽略只有一个节点的情况
	if (m_Queue.size() <= 1)
		return true;

	// 首先拿到纹理
	core::SmartReference<IResourceTexture> pTex = LRES.FindTexture(tex_name);
	if (!pTex)
	{
		spdlog::error("[luastg] [GameObjectBentLaser::Render] 找不到纹理'{}'", tex_name);
		return false;
	}

	// 设置纹理、混合模式等
	const auto renderer = LAPP.getRenderer2D();
	LAPP.updateGraph2DBlendMode(blend);
	renderer->setTexture(pTex->GetTexture());

	// 把连续的活动节点切分为若干段，每段作为一条独立的条带渲染
	// 段内节点数少于 2 的段不参与渲染（与碰撞规则保持一致）
	size_t const node_count = m_Queue.size();
	size_t seg_count = 0;
	size_t active_count = 0;
	for (size_t i = 0; i < node_count;)
	{
		if (!m_Queue[i].active)
		{
			i += 1;
			continue;
		}
		size_t const seg_start = i;
		while (i < node_count && m_Queue[i].active)
		{
			i += 1;
		}
		if ((i - seg_start) >= 2)
		{
			seg_count += 1;
			active_count += (i - seg_start);
		}
	}

	// 没有可渲染的段
	if (seg_count == 0)
		return true;

	// 分配顶点和索引
	// 顶点总共需要：活动节点数 * 2
	// 索引总共需要：(活动节点数 - 段数) * 3 * 2
	// 两个节点之间组成一个四边形
	// 注意：从显卡映射的缓冲区，只能写入，禁止读取
	IRenderer::DrawVertex* p_vertex = nullptr;
	// 注意：从显卡映射的缓冲区，只能写入，禁止读取
	IRenderer::DrawIndex* p_index = nullptr;
	uint16_t index_offset = 0;
	if (!renderer->drawRequest(
		(uint16_t)(active_count * 2),
		(uint16_t)((active_count - seg_count) * 6),
		&p_vertex,
		&p_index,
		&index_offset)) return false; // 分配空间失败了

	// 归一化 uv 坐标
	float const u_scale = 1.0f / (float)pTex->GetTexture()->getSize().x;
	float const v_scale = 1.0f / (float)pTex->GetTexture()->getSize().y;
	float const v_top = tex_top * v_scale;
	float const v_bottom = (tex_top + tex_height) * v_scale;

	// 注意：从显卡映射的缓冲区，只能写入，禁止读取
	IRenderer::DrawVertex* p_vert = p_vertex;
	IRenderer::DrawIndex* p_vidx = p_index;
	uint32_t const vertex_color = c.color();
	uint16_t vertex_offset = 0;

	// 逐段填充顶点和索引，从老节点到新节点
	// 0---2---4---6
	// |\  |\  |\  |
	// | \ | \ | \ |
	// |  \|  \|  \|
	// 1---3---5---7
	for (size_t i = 0; i < node_count;)
	{
		if (!m_Queue[i].active)
		{
			i += 1;
			continue;
		}
		size_t const seg_start = i;
		while (i < node_count && m_Queue[i].active)
		{
			i += 1;
		}
		size_t const seg_len = i - seg_start;
		if (seg_len < 2)
		{
			continue; // 单节点段不渲染
		}

		// 段内总长度（段首节点的 dis 是到段外节点的距离，不参与段内长度）
		float seg_length = 0.0f;
		for (size_t j = seg_start + 1; j < i; j += 1)
		{
			seg_length += m_Queue[j].dis;
		}

		// 段内 uv 从 0 到 1，每段拥有独立的头/身体/尾渐变
		float local_length = 0.0f;
		bool flip = false;
		for (size_t j = seg_start; j < i; j += 1)
		{
			LaserNode& node = m_Queue[j];

			// 拐成钝角，需要翻转一下延展方向
			if (node.sharp)
			{
				flip = !flip;
			}

			// 计算段内累计长度
			if (j > seg_start)
			{
				local_length += node.dis;
			}

			// 计算 u 坐标（像素坐标）
			float tex_u = tex_left;
			if (seg_length > std::numeric_limits<float>::min())
			{
				tex_u = tex_left + (local_length / seg_length) * tex_width;
			}

			// 计算延展向量，逆时针垂直于节点朝向
			float pos_x = node.x_dir * scale * node.half_width;
			float pos_y = node.y_dir * scale * node.half_width;
			if (flip)
			{
				pos_x = -pos_x;
				pos_y = -pos_y;
			}

			// 填充顶点，顶点沿着节点向两侧延展
			const IRenderer::DrawVertex vert2[2]{
				IRenderer::DrawVertex(
					node.pos.x - pos_x,
					node.pos.y - pos_y,
					0.5f,
					tex_u * u_scale,
					v_top,
					vertex_color
				),
				IRenderer::DrawVertex(
					node.pos.x + pos_x,
					node.pos.y + pos_y,
					0.5f,
					tex_u * u_scale,
					v_bottom,
					vertex_color
				),
			};
			std::memcpy(p_vert, vert2, sizeof(vert2)); // 尽可能使用内存复制，避免出现意外的读取

			// 已使用 2 个顶点，接下来不要再修改这些顶点
			p_vert += 2;
		}

		// 填充段内索引
		// 0 0-->2 2 2-->4 4 4-->6
		// |\ \  | |\ \  | |\ \  |
		// | \ \ | | \ \ | | \ \ |
		// |  \ \| |  \ \| |  \ \|
		// 1<--3 3 3<--5 5 5<--7 7
		// 注意：从显卡映射的缓冲区，只能写入，禁止读取
		// 段内第 j 个四边形连接顶点 [vertex_offset + j*2, vertex_offset + j*2 + 3]
		// （本段第 j、j+1 个节点），其中 vertex_offset 是本段首个顶点的全局索引
		for (size_t j = 0; j < (seg_len - 1); j += 1)
		{
			uint16_t const quad_offset = static_cast<uint16_t>(vertex_offset + j * 2);

			// 0-2-3
			p_vidx[0] = index_offset + quad_offset; // + 0
			p_vidx[1] = index_offset + quad_offset + 2;
			p_vidx[2] = index_offset + quad_offset + 3;

			// 3-1-0
			p_vidx[3] = index_offset + quad_offset + 3;
			p_vidx[4] = index_offset + quad_offset + 1;
			p_vidx[5] = index_offset + quad_offset; // + 0

			// 已使用 6 个索引，接下来不要再修改这些索引
			p_vidx += 6;
		}
		// 本段实际写入了 seg_len 个节点，即 seg_len * 2 个顶点
		vertex_offset += static_cast<uint16_t>(seg_len * 2);
	}

	return true;
}

void GameObjectBentLaser::RenderCollider(core::Color4B fillColor) noexcept
{
	// 忽略只有一个节点的情况
	if (m_Queue.size() <= 1)
		return;

	LAPP.DebugSetGeometryRenderState();

	GameObject testObjA;
	testObjA.Reset();
	testObjA.rot = 0.0f;
	testObjA.rect = false;

	float const _1_nc = 1.0f / (float)(m_Queue.size() - 1u);
	for (size_t i = 0; i < m_Queue.size(); ++i)
	{
		LaserNode& n = m_Queue[i];
		if (!n.active) continue;
		// 单节点段不参与碰撞（与渲染规则保持一致）
		if (!((i > 0u && m_Queue[i - 1u].active) || ((i + 1u) < m_Queue.size() && m_Queue[i + 1u].active))) continue;
		/*
		if (i > 0) {
			LaserNode& last = m_Queue[i - 1];
			if (!last.active) {
				float df = n.dis;
				if (df > n.half_width) {
					core::Vector2F c = (last.pos + n.pos) * 0.5;
					testObjA.x = c.x;
					testObjA.y = c.y;
					testObjA.rect = true;
					testObjA.rot = n.rot;
					testObjA.a = df / 2;
					testObjA.b = n.half_width;
					testObjA.UpdateCollisionCircleRadius();
					if (GameObject::isIntersect(&testObjA, &testObjB))
						return true;

				}
			}
		}
		//*/
		testObjA.x = n.pos.x;
		testObjA.y = n.pos.y;
		float const envelope_ = _GetEnvelope((float)i * _1_nc);
		testObjA.a = testObjA.b = n.half_width * envelope_; //n.half_width;
		LAPP.DebugDrawCircle((float)testObjA.x, (float)testObjA.y, (float)testObjA.a, fillColor);
	}
}

bool GameObjectBentLaser::CollisionCheck(float x, float y, float rot, float a, float b, bool rect) noexcept
{
	// 忽略只有一个节点的情况
	if (m_Queue.size() <= 1)
		return false;

	GameObject testObjA;
	testObjA.Reset();
	testObjA.rot = 0.;
	testObjA.rect = false;

	GameObject testObjB;
	testObjB.Reset();
	testObjB.x = x;
	testObjB.y = y;
	testObjB.rot = rot;
	testObjB.a = a;
	testObjB.b = b;
	testObjB.rect = rect;
	testObjB.UpdateCollisionCircleRadius();
	size_t sn = m_Queue.size();
	for (size_t i = 0; i < sn; ++i)
	{
		LaserNode& n = m_Queue[i];
		if (!n.active)continue;
		// 单节点段不参与碰撞（与渲染规则保持一致）
		if (!((i > 0u && m_Queue[i - 1u].active) || ((i + 1u) < sn && m_Queue[i + 1u].active)))continue;
		/*
		if (i > 0) {
			LaserNode& last = m_Queue[i - 1];
			if (!last.active) {
				float df = n.dis;
				if (df > n.half_width) {
					core::Vector2F c = (last.pos + n.pos) * 0.5;
					testObjA.x = c.x;
					testObjA.y = c.y;
					testObjA.rect = true;
					testObjA.rot = n.rot;
					testObjA.a = df / 2;
					testObjA.b = n.half_width;
					testObjA.UpdateCollisionCircleRadius();
					if (GameObject::isIntersect(&testObjA, &testObjB))
						return true;

				}
			}
		}
		//*/
		testObjA.x = n.pos.x;
		testObjA.y = n.pos.y;
		testObjA.a = testObjA.b = n.half_width * _GetEnvelope((float)i / (float)(sn - 1u)); //n.half_width;
		testObjA.rect = false;
		testObjA.UpdateCollisionCircleRadius();
		if (GameObject::isIntersect(&testObjA, &testObjB))
			return true;
	}
	return false;
}

bool GameObjectBentLaser::CollisionCheckW(float x, float y, float rot, float a, float b, bool rect, float width) noexcept
{
	// 忽略只有一个节点的情况
	if (m_Queue.size() <= 1)
		return false;
	
	width = width / 2;
	GameObject testObjA;
	testObjA.Reset();
	testObjA.rot = 0.;
	testObjA.rect = false;

	GameObject testObjB;
	testObjB.Reset();
	testObjB.x = x;
	testObjB.y = y;
	testObjB.rot = rot;
	testObjB.a = a;
	testObjB.b = b;
	testObjB.rect = rect;
	testObjB.UpdateCollisionCircleRadius();
	size_t sn = m_Queue.size();
	for (size_t i = 0; i < sn; ++i)
	{
		LaserNode& n = m_Queue[i];
		if (!n.active)continue;
		// 单节点段不参与碰撞（与渲染规则保持一致）
		if (!((i > 0u && m_Queue[i - 1u].active) || ((i + 1u) < sn && m_Queue[i + 1u].active)))continue;
		/*
		if (i > 0) {
			LaserNode& last = m_Queue[i - 1];
			if (!last.active) {
				float df = n.dis;
				if (df > width) {
					core::Vector2F c = (last.pos + n.pos) * 0.5;
					testObjA.x = c.x;
					testObjA.y = c.y;
					testObjA.rect = true;
					testObjA.rot = n.rot;
					testObjA.a = df / 2;
					testObjA.b = width;
					testObjA.UpdateCollisionCircleRadius();
					if (GameObject::isIntersect(&testObjA, &testObjB))
						return true;

				}
			}
		}
		//*/
		testObjA.x = n.pos.x;
		testObjA.y = n.pos.y;
		testObjA.a = testObjA.b = width;
		testObjA.rect = false;
		testObjA.UpdateCollisionCircleRadius();
		if (GameObject::isIntersect(&testObjA, &testObjB))
			return true;
	}
	return false;
}

bool GameObjectBentLaser::BoundCheck() noexcept
{
	auto& manager = LPOOL;
	for (size_t i = 0u; i < m_Queue.size(); i++) {
		LaserNode& n = m_Queue[i];
		if (manager.isPointInBound(n.pos.x, n.pos.y)) {
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------

bool GameObjectBentLaser::UpdateByNode(size_t id, int node, int length, float width, bool active) noexcept
{
	std::ignore = width; // TODO: ESC，你这个宽度怎么没有用上???

	GameObject* p = LPOOL.GetPooledObject(id);
	if (!p) {
		spdlog::error("[luastg] [GameObjectBentLaser::UpdateByNode] 无效的lstg.GameObject");
		return false;
	}
	if (length <= 1) {
		spdlog::error("[luastg] [GameObjectBentLaser::UpdateByNode] 无效的参数length={}", length);
		return false;
	}

	// 对索引取余
	if (node < 0) {
		node = m_Queue.size() + node;
	}

	// 添加新节点
	if (node < (int)m_Queue.size() && node >= 0)
	{
		LaserNode& tNode = m_Queue[node];
		tNode.active = active;
		_UpdateAllNode();
	}

	return true;
}

bool GameObjectBentLaser::UpdatePositionByList(lua_State * L, int length, float width, int index, bool revert) noexcept// ... t(list) //lua index从1开始
{
	// ... t(list)
	int push_count = 0;//以插入头的节点数量 

	for (int i = 0; i < length; i++)
	{
		//获得x,y
		lua_rawgeti(L, -1, i + 1);// ... t(list) t(object)
		lua_pushstring(L, "x");// ... t(list) t(object) 'x'
		lua_gettable(L, -2);// ... t(list) t(object) x
		float x = (float)luaL_optnumber(L, -1, 0.0);
		lua_pop(L, 1);
		lua_pushstring(L, "y");// ... t(list) t(object) 'y'
		lua_gettable(L, -2);// ... t(list) t(object) y
		float y = (float)luaL_optnumber(L, -1, 0.0);// ... t(list) t(object) y
		lua_pop(L, 2);// ... t(list)

		//得到index
		//顶点处在队列前边
		int cindex = push_count + index - 1 + (revert ? -i : i);
		if (cindex < 0) {
			int j = cindex;
			LaserNode np;
			np.active = false;
			while (j > 0) {
				m_Queue.pushTail(np);
				j--;
				push_count++;
			}
		}

		int size = m_Queue.size();
		//顶点处在队列后边
		if (cindex >= size) {
			int j = cindex - size + 1;
			LaserNode np;
			np.active = false;
			while (j > 0) {
				m_Queue.pushHead(np);
				j--;
			}
		}
		size = m_Queue.size();
		//设置顶点
		LaserNode* tNode = &m_Queue[size - cindex - 1];
		tNode->active = true;
		tNode->half_width = width / 2;
		tNode->pos = core::Vector2F(x, y);
	}
	_UpdateAllNode();
	return true;
}

int GameObjectBentLaser::SampleL(lua_State * L, float length) noexcept
{
	//插入一个数组
	lua_newtable(L); //... t(list)
	// 忽略没有节点的情况
	if (m_Queue.size() <= 1)
		return true;

	float fLeft = 0;// 剩余长度
	int count = 0;

	//float tVecLength = 0;
	for (size_t i = m_Queue.size() - 1; i > 0; --i)
	{
		LaserNode& cur = m_Queue[i];
		LaserNode& next = m_Queue[i - 1];

		core::Vector2F vn = cur.pos;
		core::Vector2F offsetA = next.pos - cur.pos;
		float lenOffsetA = offsetA.length();
		core::Vector2F expandVec = offsetA.normalized();
		float angle = expandVec.angle() * L_RAD_TO_DEG_F + 180.0f;
		while (fLeft - lenOffsetA <= 0) {
			vn = expandVec * fLeft + cur.pos;
			lua_newtable(L); //... t(list) t(object)
			lua_pushnumber(L, vn.x); //... t(list) t(object) <x>
			lua_setfield(L, -2, "x");//... t(list) t(object)
			lua_pushnumber(L, vn.y); //... t(list) t(object) <y>
			lua_setfield(L, -2, "y");//... t(list) t(object)
			lua_pushnumber(L, angle); //... t(list) t(object) <angle>
			lua_setfield(L, -2, "rot");//... t(list) t(object)
			count++;
			lua_rawseti(L, -2, count);//... t(list)
			fLeft = fLeft + length;
		}
		fLeft = fLeft - lenOffsetA;
	}
	return true;
}

int GameObjectBentLaser::SampleT(lua_State * L, float delay) noexcept
{
	//插入一个数组
	lua_newtable(L); //... t(list)
	// 忽略没有节点的情况
	if (m_Queue.size() <= 1)
		return true;

	float fLeft = 0;// 剩余长度
	int count = 0;

	//float tVecLength = 0;
	for (size_t i = m_Queue.size() - 1; i > 0; --i)
	{
		LaserNode& cur = m_Queue[i];
		LaserNode& next = m_Queue[i - 1];


		core::Vector2F vn = cur.pos;
		core::Vector2F offsetA = next.pos - cur.pos;
		//float lenOffsetA = offsetA.length();
		float angle = offsetA.angle() * L_RAD_TO_DEG_F + 180.0f;
		while (fLeft - 1 <= 0) {
			vn = offsetA * fLeft + cur.pos;
			lua_newtable(L); //... t(list) t(object)
			lua_pushnumber(L, vn.x); //... t(list) t(object) <x>
			lua_setfield(L, -2, "x");//... t(list) t(object)
			lua_pushnumber(L, vn.y); //... t(list) t(object) <y>
			lua_setfield(L, -2, "y");//... t(list) t(object)
			lua_pushnumber(L, angle); //... t(list) t(object) <angle>
			lua_setfield(L, -2, "rot");//... t(list) t(object)
			count++;
			lua_rawseti(L, -2, count);//... t(list)
			fLeft = fLeft + delay;
		}
		fLeft = fLeft - 1;
	}
	return true;
}

//------------------------------------------------------------------------------

inline bool luaL_isnumber(lua_State* L, int idx) { return LUA_TNUMBER == lua_type(L, idx); }

int GameObjectBentLaser::api_UpdateSingleNode(lua_State* L)
{
	// self, index, x, y, width
	assert(lua_gettop(L) == 5);

	// 检查参数
	size_t const node_index = (size_t)(luaL_checkinteger(L, 2) - 1); // 索引从 1 开始
	if (node_index >= m_Queue.size())
	{
		return luaL_error(L, "invalid parameter #1, node index out of bounds");
	}
	float const x = (float)luaL_checknumber(L, 3);
	float const y = (float)luaL_checknumber(L, 4);
	float const half_width = (float)(luaL_checknumber(L, 5) * 0.5);

	// 修改节点
	LaserNode& node = m_Queue[node_index];
	m_fLength -= node.dis; // 先更新一次总长度，把这个节点抹掉
	node.pos.x = x;
	node.pos.y = y;
	node.half_width = half_width;
	node.active = true;
	if (node_index > 0)
	{
		LaserNode& last = m_Queue[node_index - 1];
		float const len_ = (node.pos - last.pos).length();
		node.dis = len_;
		m_fLength += len_; // 现在重新加回来
	}
	else
	{
		node.dis = 0.0f; // 没有上一个节点
	}

	// 更新修改的节点和相邻的节点
	_UpdateNodeVertexExtend(node_index);
	if (m_Queue.size() > 1)
	{
		if (node_index > 0) _UpdateNodeVertexExtend(node_index - 1);
		if (node_index < (m_Queue.size() - 1)) _UpdateNodeVertexExtend(node_index + 1);
	}

	return 0;
}

int GameObjectBentLaser::api_GetActiveNodes(lua_State* L)
{
	// 返回活动节点的位置表，供 Lua 在 kill 时生成奖励
	// 奖励只在实际存在的（活动）节点上生成，无论通过何种方式消灭，奖励数量只取决于激光当前状态
	lua_newtable(L); // t
	int count = 0;
	for (size_t i = 0; i < m_Queue.size(); i += 1)
	{
		LaserNode const& node = m_Queue[i];
		if (!node.active)
		{
			continue;
		}
		count += 1;
		lua_newtable(L); // t t(node)
		lua_pushnumber(L, node.pos.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, node.pos.y);
		lua_setfield(L, -2, "y");
		lua_rawseti(L, -2, count); // t
	}
	return 1;
}

int GameObjectBentLaser::api_UpdateAllNodeByList(lua_State* L)
{
	// self, size, x[], y[], width?
	assert(lua_gettop(L) == 5);

	// 检查参数
	if (!lua_istable(L, 3))
	{
		return luaL_error(L, "invalid parameter #2, required number list");
	}
	if (!lua_istable(L, 4))
	{
		return luaL_error(L, "invalid parameter #3, required number list");
	}
	bool read_width = false;
	float half_width = 0.0f;
	if (!lua_istable(L, 5))
	{
		read_width = true;
		half_width = (float)(0.5 * luaL_checknumber(L, 5));
	}

	// 重新分配空间
	size_t const node_count = (size_t)luaL_checkinteger(L, 2);
	if (node_count > m_Queue.capacity())
	{
		return luaL_error(L, "invalid parameter #1, number of nodes should <= %d", (int)m_Queue.capacity());
	}
	m_Queue.placementResize(node_count);

	// 设置所有节点的坐标和宽度
	if (read_width)
	{
		// 宽度是固定值
		for (size_t i = 0; i < node_count; i += 1)
		{
			LaserNode& node = m_Queue[i];
			int const luai = (int)i + 1;
			lua_rawgeti(L, 3, luai); // self, size, x[], y[], width, x
			lua_rawgeti(L, 4, luai); // self, size, x[], y[], width, x, y
			if (!luaL_isnumber(L, 6)) return luaL_error(L, "invalid number at [%d] in parameter #2", luai);
			if (!luaL_isnumber(L, 7)) return luaL_error(L, "invalid number at [%d] in parameter #3", luai);
			node.pos.x = (float)lua_tonumber(L, 6);
			node.pos.y = (float)lua_tonumber(L, 7);
			lua_pop(L, 2);           // self, size, x[], y[], width
			node.half_width = half_width;
			node.active = true;
		}
	}
	else
	{
		// 宽度是列表
		for (size_t i = 0; i < node_count; i += 1)
		{
			LaserNode& node = m_Queue[i];
			int const luai = (int)i + 1;
			lua_rawgeti(L, 3, luai); // self, size, x[], y[], width[], x
			lua_rawgeti(L, 4, luai); // self, size, x[], y[], width[], x, y
			lua_rawgeti(L, 5, luai); // self, size, x[], y[], width[], x, y, width
			if (!luaL_isnumber(L, 6)) return luaL_error(L, "invalid number at [%d] in parameter #2", luai);
			if (!luaL_isnumber(L, 7)) return luaL_error(L, "invalid number at [%d] in parameter #3", luai);
			if (!luaL_isnumber(L, 8)) return luaL_error(L, "invalid number at [%d] in parameter #4", luai);
			node.pos.x = (float)lua_tonumber(L, 6);
			node.pos.y = (float)lua_tonumber(L, 7);
			node.half_width = (float)(0.5 * lua_tonumber(L, 8));
			lua_pop(L, 3);           // self, size, x[], y[], width[]
			node.active = true;
		}
	}
		
	// 更新所有节点
	_UpdateAllNode();

	return 0;
}
