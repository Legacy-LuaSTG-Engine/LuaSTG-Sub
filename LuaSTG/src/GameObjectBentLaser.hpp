#pragma once
#include "Global.h"
#include "CirularQueue.hpp"
#include "GameObject.hpp"

#define LGOBJ_MAXLASERNODE 512  // 曲线激光最大节点数

namespace LuaSTGPlus
{
	class GameObjectBentLaser
	{
	public:
		static GameObjectBentLaser* AllocInstance();
		static void FreeInstance(GameObjectBentLaser* p);
		struct LaserNode {
			fcyVec2 pos;			//节点位置
			float half_width = 0.0f;//半宽
			float rot = 0.0f;		//节点朝向
			float dis = 0.0f;		//到上一个节点的距离
			float x_dir = 0.0f;		//顶点向量x分量
			float y_dir = 0.0f;		//顶点向量y分量
			bool active = true;		//节点活动状况
			bool sharp = false;		//相对上一个节点的朝向成钝角
		};
	private:
		CirularQueue<LaserNode, LGOBJ_MAXLASERNODE> m_Queue;
		float m_fLength = 0.0f; // 记录激光长度
	private:
		float m_fEnvelopeHeight = 0.0f;
		float m_fEnvelopeBase = 1.0f;
		float m_fEnvelopeRate = 0.0f;
		float m_fEnvelopePower = 0.0f;
		// https://www.desmos.com/calculator/i6r2pw90xw
		inline float _GetEnvelope(float t) {
			float ret = m_fEnvelopeHeight + (m_fEnvelopeBase * (1.0f - m_fEnvelopeRate * std::powf(2.0f * (t - 0.5f), m_fEnvelopePower)));
			return (std::max)(0.0f, ret);
		}
		void _UpdateNode(size_t i)LNOEXCEPT; // 计算节点的渲染顶点
		void _UpdateAllNode()LNOEXCEPT; // 重新计算所有节点的朝向和距离
		void _PopHead()LNOEXCEPT; // 弹出头部节点，较早的节点
	public:
		// 读取
		int GetSize()LNOEXCEPT; // 获取节点数量
		LaserNode* GetNode(size_t i)LNOEXCEPT; // 获取节点，并非长期有效
		float GetLength()LNOEXCEPT { return m_fLength; } // 获取曲线激光长度
		void GetEnvelope(float& height, float& base, float& rate, float& power)LNOEXCEPT; // 碰撞包络
		// 更新
		bool Update(size_t id, int length, float width, bool active)LNOEXCEPT; // 根据新的位置更新节点
		void SetAllWidth(float width)LNOEXCEPT; // 更改所有节点的碰撞和渲染宽度
		// 渲染
		bool Render(const char* tex_name, BlendMode blend, fcyColor c, float tex_left, float tex_top, float tex_width, float tex_height, float scale)LNOEXCEPT;
		void RenderCollider(fcyColor fillColor)LNOEXCEPT;
		// 碰撞检测
		void SetEnvelope(float height, float base, float rate, float power)LNOEXCEPT; // 设置碰撞包络
		bool BoundCheck()LNOEXCEPT; // 检查是否离开边界
		bool CollisionCheck(float x, float y, float rot, float a, float b, bool rect)LNOEXCEPT; // 碰撞检测
		// 即将被废弃
		bool UpdateByNode(size_t id, int node, int length, float width, bool active)LNOEXCEPT; // 对某个节点开启或关闭并更新
		bool UpdatePositionByList(lua_State* L, int length, float width, int index, bool revert)LNOEXCEPT; // 更改所有节点的坐标并更新
		bool CollisionCheckW(float x, float y, float rot, float a, float b, bool rect, float width)LNOEXCEPT;
		int SampleL(lua_State* L, float length)LNOEXCEPT;
		int SampleT(lua_State* L, float delay) LNOEXCEPT;
	protected:
		GameObjectBentLaser()LNOEXCEPT;
		~GameObjectBentLaser()LNOEXCEPT;
	};
}
