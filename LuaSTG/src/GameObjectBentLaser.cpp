#include "GameObjectBentLaser.hpp"
#include "AppFrame.h"

using namespace std;
using namespace LuaSTGPlus;

//======================================

static fcyMemPool<sizeof(GameObjectBentLaser)> s_GameObjectBentLaserPool(1024);

GameObjectBentLaser* GameObjectBentLaser::AllocInstance()
{
	//潜在bad_alloc已在luawrapper中处理
	GameObjectBentLaser* pRet = new(s_GameObjectBentLaserPool.Alloc()) GameObjectBentLaser();
	return pRet;
}

void GameObjectBentLaser::FreeInstance(GameObjectBentLaser* p)
{
	p->~GameObjectBentLaser();
	s_GameObjectBentLaserPool.Free(p);
}

GameObjectBentLaser::GameObjectBentLaser()LNOEXCEPT
{
}

GameObjectBentLaser::~GameObjectBentLaser()LNOEXCEPT
{
}

//======================================

void GameObjectBentLaser::_UpdateNode(size_t i)LNOEXCEPT {
	LaserNode& cur = m_Queue[i];
	cur.sharp = false;
	int sz = m_Queue.Size() - 1;
	if (sz > 0) {
		float curcos = 0.0f, cursin = 0.0f;
		float nextcos = 0.0f, nextsin = 0.0f;
		cursin = sin(cur.rot);
		curcos = cos(cur.rot);
		if (i == 0 || i == sz) {
			//头节点或为尾节点，顶点朝向为节点朝向的垂直方向
			cur.x_dir = cursin;
			cur.y_dir = -curcos;
			return;//不需要继续计算
		}
		else {
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
}

void GameObjectBentLaser::_UpdateAllNode()LNOEXCEPT {
	//注意，尾部才是最后插入的，头部是最早插入的
	m_fLength = 0.0f;
	size_t sz = m_Queue.Size();
	if (sz > 0u) {
		{ LaserNode& head = m_Queue[0u]; head.dis = 0.0f; }//队列头的节点一定与上一个节点的距离是0，因为不存在上一个节点
		for (size_t i = (sz - 1u); i > 0u; i--) {
			LaserNode& cur = m_Queue[i]; //靠近尾部
			LaserNode& last = m_Queue[i - 1u]; //靠近头部
			fcyVec2 vec = cur.pos - last.pos;
			fFloat len = vec.Length();
			cur.dis = len;
			if (cur.active && last.active) {
				m_fLength += len;
			}
		}
		for (size_t i = 0u; i < (sz - 1u); i++) {
			LaserNode& cur = m_Queue[i];//靠近头部
			LaserNode& next = m_Queue[i + 1u];//靠近尾部
			fcyVec2 dpos = next.pos - cur.pos;
			fFloat len = dpos.Length();
			if (len > (std::numeric_limits<fFloat>::min)()) {
				next.rot = dpos.CalcuAngle();
			}
			else {
				next.rot = cur.rot;
			}
			if (i == 0u) {
				cur.rot = next.rot;//只有一个节点
			}
		}
		for (size_t i = 0u; i < sz; i++) {
			_UpdateNode(i);
		}
	}
}

void GameObjectBentLaser::_PopHead()LNOEXCEPT {
	if (m_Queue.Size() > 1) {
		LaserNode tLastPop;
		m_Queue.Pop(tLastPop);
		if (!m_Queue.IsEmpty()) {
			LaserNode& tFront = m_Queue.Front();
			// 如果最后两个节点都是激活的，根据节点间的距离减少曲线激光总长度
			if (tLastPop.active && tFront.active) {
				m_fLength -= tFront.dis; //到上一个节点的距离
			}
			tFront.dis = 0; //没有上一个节点
		}
	}
}

int GameObjectBentLaser::GetSize()LNOEXCEPT {
	return (int)m_Queue.Size();
}

GameObjectBentLaser::LaserNode* GameObjectBentLaser::GetNode(size_t i)LNOEXCEPT {
	if (m_Queue.Size() > 0u) {
		if (i >= 0u && i < m_Queue.Size()) {
			return &(m_Queue[i]);
		}
	}
	return nullptr;
}

void GameObjectBentLaser::GetEnvelope(float& height, float& base, float& rate, float& power)LNOEXCEPT {
	height = m_fEnvelopeHeight;
	base = m_fEnvelopeBase;
	rate = m_fEnvelopeRate;
	power = m_fEnvelopePower;
}

bool GameObjectBentLaser::Update(size_t id, int length, float width, bool active)LNOEXCEPT
{
	GameObject* p = LPOOL.GetPooledObject(id);
	if (!p) {
		LERROR("lstg.BentLaser: 无效的lstg.GameObject");
		return false;
	}
	if (length <= 1) {
		LERROR("lstg.BentLaser: 无效的参数length");
		return false;
	}

	// ！循环队列的头部是最早创建的，尾部才是最新放入的！

	// 检查是否有必要更新节点
	LaserNode tNode;
	fcyVec2 dpos;
	fFloat len;
	bool lactive;
	{
		tNode.pos.Set((float)p->x, (float)p->y);
		tNode.half_width = width / 2.f;
		tNode.active = active;
		LaserNode& tNodeLast = m_Queue.Back();
		dpos = tNode.pos - tNodeLast.pos;
		len = dpos.Length();
		lactive = tNodeLast.active;
		if (len <= (std::numeric_limits<fFloat>::min)()) {
			// 仍然需要更新节点数量
			// 移除多余的节点，保证长度在length范围内
			while (m_Queue.Size() >= (size_t)length) {
				_PopHead();
			}
			return true; // 变化几乎可以忽略不计，不插入该节点
		}
	}
	
	// 移除多余的节点，保证长度在length - 1范围内
	while (m_Queue.IsFull() || m_Queue.Size() >= (size_t)length) {
		_PopHead();
	}

	// 计算
	if (m_Queue.Size() > 0) {
		tNode.dis = len;//距离
		tNode.rot = dpos.CalcuAngle();//计算朝向
		if (active && lactive) {
			m_fLength += len;// 增加总长度
		}
	}
	else {
		fcyVec2 speed((fFloat)p->vy, (fFloat)p->vx);
		if (speed.Length() > (std::numeric_limits<fFloat>::min)()) {
			tNode.rot = speed.CalcuAngle();//使用速度方向作为节点朝向
		}
		else {
			tNode.rot = (float)p->rot;//使用朝向作为节点朝向
		}
		tNode.dis = 0.0f;
	}
	m_Queue.Push(tNode); // 插入
	if (m_Queue.Size() > 1u) {
		_UpdateNode(m_Queue.Size() - 1u);
		_UpdateNode(m_Queue.Size() - 2u);
	}
	return true;
}

void GameObjectBentLaser::SetAllWidth(float width) LNOEXCEPT
{
	for (size_t i = 0u; i < m_Queue.Size(); i++)
	{
		m_Queue[i].half_width = width / 2.0f;
	}
}

bool GameObjectBentLaser::Render(const char* tex_name, BlendMode blend, fcyColor c, float tex_left, float tex_top, float tex_width, float tex_height, float scale)LNOEXCEPT
{
	// 忽略只有一个节点的情况
	if (m_Queue.Size() <= 1)
		return true;

	fcyRefPointer<ResTexture> pTex = LRES.FindTexture(tex_name);
	if (!pTex)
	{
		LERROR("lstgBentLaserData: 找不到纹理资源'%m'", tex_name);
		return false;
	}

	f2dGraphics2DVertex renderVertex[4] = {
		{ 0, 0, 0.5f, c.argb, 0, tex_top },
		{ 0, 0, 0.5f, c.argb, 0, tex_top },
		{ 0, 0, 0.5f, c.argb, 0, tex_top + tex_height },
		{ 0, 0, 0.5f, c.argb, 0, tex_top + tex_height }
	};
	fuInt org_c = c.argb;
	c.a = 0;
	fuInt trans_c = c.argb;

	float tVecLength = 0;
	bool flip = false;
	for (size_t i = 0; i < m_Queue.Size() - 1; ++i)
	{
		LaserNode& cur = m_Queue[i];
		LaserNode& next = m_Queue[i + 1];

		if (!cur.active || !next.active) {
			continue;
		}

		if (cur.sharp) {
			flip = !flip;
		}

		// 计算延展方向1
		float expX1 = cur.x_dir * scale * cur.half_width;
		float expY1 = cur.y_dir * scale * cur.half_width;
		if (flip) {
			expX1 = -expX1;
			expY1 = -expY1;
		}
		// 计算U坐标1
		float u = tex_left + tVecLength / m_fLength * tex_width;
		renderVertex[0].x = cur.pos.x + expX1;
		renderVertex[0].y = cur.pos.y + expY1;
		renderVertex[0].u = u;
		renderVertex[0].color = cur.active ? org_c : trans_c;
		renderVertex[3].x = cur.pos.x - expX1;
		renderVertex[3].y = cur.pos.y - expY1;
		renderVertex[3].u = u;
		renderVertex[3].color = cur.active ? org_c : trans_c;

		// 计算延展方向2
		float expX2 = next.x_dir * scale * cur.half_width;
		float expY2 = next.y_dir * scale * cur.half_width;
		if (flip) {
			expX2 = -expX2;
			expY2 = -expY2;
		}
		// 计算U坐标2
		float lenOffsetA = next.dis;
		tVecLength += lenOffsetA;
		u = tex_left + tVecLength / m_fLength * tex_width;
		renderVertex[1].x = next.pos.x + expX2;
		renderVertex[1].y = next.pos.y + expY2;
		renderVertex[1].u = u;
		renderVertex[1].color = next.active ? org_c : trans_c;
		renderVertex[2].x = next.pos.x - expX2;
		renderVertex[2].y = next.pos.y - expY2;
		renderVertex[2].u = u;
		renderVertex[2].color = next.active ? org_c : trans_c;

		if (!LAPP.RenderTexture(pTex, blend, renderVertex))
			return false;
	}
	return true;
}

void GameObjectBentLaser::RenderCollider(fcyColor fillColor)LNOEXCEPT {
	// 忽略只有一个节点的情况
	int sn = m_Queue.Size();
	if (sn <= 1)
		return;

	LAPP.GetRenderDev()->ClearZBuffer();

	fcyRefPointer<f2dGeometryRenderer> grender = LAPP.GetGeometryRenderer();
	fcyRefPointer<f2dGraphics2D> graph = LAPP.GetGraphics2D();

	f2dBlendState stState = graph->GetBlendState();
	f2dBlendState stStateClone = stState;
	stStateClone.SrcBlend = F2DBLENDFACTOR_SRCALPHA;
	stStateClone.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
	stStateClone.BlendOp = F2DBLENDOPERATOR_ADD;
	graph->SetBlendState(stStateClone);
	F2DGRAPH2DBLENDTYPE txState = graph->GetColorBlendType();
	graph->SetColorBlendType(F2DGRAPH2DBLENDTYPE_ADD);//修复反色混合模式的时候会出现颜色异常的问题

	GameObject testObjA;
	testObjA.Reset();

	for (size_t i = 0; i < sn; ++i)
	{
		LaserNode& n = m_Queue[i];
		if (!n.active)
			continue;
		if (i > 0) {
			LaserNode& last = m_Queue[i - 1];
			if (!last.active) {
				float df = n.dis;
				if (df > n.half_width) {
					//计算部分
					fcyVec2 c = (last.pos + n.pos) * 0.5;
					testObjA.x = c.x;
					testObjA.y = c.y;
					testObjA.rect = true;
					testObjA.rot = n.rot;
					testObjA.a = df / 2;
					testObjA.b = n.half_width;
					//testObjA.UpdateCollisionCirclrRadius()
					//渲染部分
					{
						fcyVec2 tHalfSize(testObjA.a, testObjA.b);
						// 计算出矩形的4个顶点
						f2dGraphics2DVertex tFinalPos[4] =
						{
							{ -tHalfSize.x, -tHalfSize.y, 0.5f, fillColor.argb, 0.0f, 0.0f },
							{ tHalfSize.x, -tHalfSize.y, 0.5f, fillColor.argb, 0.0f, 1.0f },
							{ tHalfSize.x, tHalfSize.y, 0.5f, fillColor.argb, 1.0f, 1.0f },
							{ -tHalfSize.x, tHalfSize.y, 0.5f, fillColor.argb, 1.0f, 0.0f }
						};
						float tSin = std::sinf(testObjA.rot);
						float tCos = std::cosf(testObjA.rot);
						// 变换
						for (int i = 0; i < 4; i++)
						{
							fFloat tx = tFinalPos[i].x * tCos - tFinalPos[i].y * tSin;
							fFloat ty = tFinalPos[i].x * tSin + tFinalPos[i].y * tCos;
							tFinalPos[i].x = tx + testObjA.x;
							tFinalPos[i].y = ty + testObjA.y;
						}
						graph->DrawQuad(nullptr, tFinalPos);
					}
				}
			}
		}
		
		//计算
		testObjA.x = n.pos.x;
		testObjA.y = n.pos.y;
		testObjA.rect = false;
		testObjA.a
			= testObjA.b
			= n.half_width * _GetEnvelope((float)i / (float)(sn - 1u));
		//testObjA.UpdateCollisionCirclrRadius();
		//渲染
		grender->FillCircle(graph, fcyVec2((float)testObjA.x, (float)testObjA.y), (float)testObjA.a, fillColor, fillColor,
			testObjA.a < 10.0 ? 4 : (testObjA.a < 20.0 ? 8 : 16));
	}

	graph->SetBlendState(stState);
	graph->SetColorBlendType(txState);
}

void GameObjectBentLaser::SetEnvelope(float height, float base, float rate, float power)LNOEXCEPT {
	m_fEnvelopeHeight = height;
	m_fEnvelopeBase = std::clamp(base, 0.0f, 1.0f);
	m_fEnvelopeRate = rate;
	m_fEnvelopePower = 0.4f * std::floorf(power / 0.4f); // 不要问，问就是魔法数字
}

bool GameObjectBentLaser::CollisionCheck(float x, float y, float rot, float a, float b, bool rect)LNOEXCEPT
{
	// 忽略只有一个节点的情况
	if (m_Queue.Size() <= 1)
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
	testObjB.UpdateCollisionCirclrRadius();
	int sn = m_Queue.Size();
	for (size_t i = 0; i < sn; ++i)
	{
		LaserNode& n = m_Queue[i];
		if (!n.active)continue;
		/*
		if (i > 0) {
			LaserNode& last = m_Queue[i - 1];
			if (!last.active) {
				float df = n.dis;
				if (df > n.half_width) {
					fcyVec2 c = (last.pos + n.pos) * 0.5;
					testObjA.x = c.x;
					testObjA.y = c.y;
					testObjA.rect = true;
					testObjA.rot = n.rot;
					testObjA.a = df / 2;
					testObjA.b = n.half_width;
					testObjA.UpdateCollisionCirclrRadius();
					if (LuaSTGPlus::CollisionCheck(&testObjA, &testObjB))
						return true;

				}
			}
		}
		//*/
		testObjA.x = n.pos.x;
		testObjA.y = n.pos.y;
		testObjA.a = testObjA.b = n.half_width * _GetEnvelope((float)i / (float)(sn - 1u)); //n.half_width;
		testObjA.rect = false;
		testObjA.UpdateCollisionCirclrRadius();
		if (LuaSTGPlus::CollisionCheck(&testObjA, &testObjB))
			return true;
	}
	return false;
}

bool GameObjectBentLaser::CollisionCheckW(float x, float y, float rot, float a, float b, bool rect, float width)LNOEXCEPT
{
	// 忽略只有一个节点的情况
	if (m_Queue.Size() <= 1)
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
	testObjB.UpdateCollisionCirclrRadius();
	int sn = m_Queue.Size();
	for (size_t i = 0; i < sn; ++i)
	{
		LaserNode& n = m_Queue[i];
		if (!n.active)continue;
		/*
		if (i > 0) {
			LaserNode& last = m_Queue[i - 1];
			if (!last.active) {
				float df = n.dis;
				if (df > width) {
					fcyVec2 c = (last.pos + n.pos) * 0.5;
					testObjA.x = c.x;
					testObjA.y = c.y;
					testObjA.rect = true;
					testObjA.rot = n.rot;
					testObjA.a = df / 2;
					testObjA.b = width;
					testObjA.UpdateCollisionCirclrRadius();
					if (LuaSTGPlus::CollisionCheck(&testObjA, &testObjB))
						return true;

				}
			}
		}
		//*/
		testObjA.x = n.pos.x;
		testObjA.y = n.pos.y;
		testObjA.a = testObjA.b = width;
		testObjA.rect = false;
		testObjA.UpdateCollisionCirclrRadius();
		if (LuaSTGPlus::CollisionCheck(&testObjA, &testObjB))
			return true;
	}
	return false;
}

bool GameObjectBentLaser::BoundCheck()LNOEXCEPT
{
	fcyRect tBound = LPOOL.GetBound();
	for (size_t i = 0u; i < m_Queue.Size(); i++)
	{
		LaserNode& n = m_Queue[i];
		if (n.pos.x >= tBound.a.x &&
			n.pos.x <= tBound.b.x &&
			n.pos.y <= tBound.a.y &&
			n.pos.y >= tBound.b.y)
			return true;
	}
	return false;
}

bool GameObjectBentLaser::UpdateByNode(size_t id, int node, int length, float width, bool active)LNOEXCEPT
{
	GameObject* p = LPOOL.GetPooledObject(id);
	if (!p) {
		LERROR("lstg.BentLaser: 无效的lstg.GameObject");
		return false;
	}
	if (length <= 1) {
		LERROR("lstg.BentLaser: 无效的参数length");
		return false;
	}

	// 对索引取余
	if (node < 0) {
		node = m_Queue.Size() + node;
	}

	// 添加新节点
	if (node < m_Queue.Size() && node >= 0)
	{
		LaserNode& tNode = m_Queue[node];
		tNode.active = active;
		_UpdateAllNode();
	}

	return true;
}

bool GameObjectBentLaser::UpdatePositionByList(lua_State * L, int length, float width, int index, bool revert) LNOEXCEPT// ... t(list) //lua index从1开始
{
	// ... t(list)
	int push_count = 0;//以插入头的节点数量 

	for (int i = 0; i < length; i++)
	{
		//获得x,y
		lua_rawgeti(L, -1, i + 1);// ... t(list) t(object)
		lua_pushstring(L, "x");// ... t(list) t(object) 'x'
		lua_gettable(L, -2);// ... t(list) t(object) x
		float x = luaL_optnumber(L, -1, 0.0);
		lua_pop(L, 1);
		lua_pushstring(L, "y");// ... t(list) t(object) 'y'
		lua_gettable(L, -2);// ... t(list) t(object) y
		float y = luaL_optnumber(L, -1, 0.0);// ... t(list) t(object) y
		lua_pop(L, 2);// ... t(list)

		//得到index
		//顶点处在队列前边
		int cindex = push_count + index - 1 + (revert ? -i : i);
		if (cindex < 0) {
			int j = cindex;
			LaserNode np;
			np.active = false;
			while (j > 0) {
				m_Queue.Push(np);
				j--;
				push_count++;
			}
		}

		int size = m_Queue.Size();
		//顶点处在队列后边
		if (cindex >= size) {
			int j = cindex - size + 1;
			LaserNode np;
			np.active = false;
			while (j > 0) {
				m_Queue.PushBack(np);
				j--;
			}
		}
		size = m_Queue.Size();
		//设置顶点
		LaserNode* tNode = &m_Queue[size - cindex - 1];
		tNode->active = true;
		tNode->half_width = width / 2;
		tNode->pos.Set(x, y);
	}
	_UpdateAllNode();
	return true;
}

int GameObjectBentLaser::SampleL(lua_State * L, float length) LNOEXCEPT
{
	//插入一个数组
	lua_newtable(L); //... t(list)
	// 忽略没有节点的情况
	if (m_Queue.Size() <= 1)
		return true;

	float fLeft = 0;// 剩余长度
	int count = 0;

	float tVecLength = 0;
	for (size_t i = m_Queue.Size() - 1; i > 0; --i)
	{
		LaserNode& cur = m_Queue[i];
		LaserNode& next = m_Queue[i - 1];

		fcyVec2 vn = cur.pos;
		fcyVec2 offsetA = next.pos - cur.pos;
		float lenOffsetA = offsetA.Length();
		fcyVec2 expandVec = offsetA.GetNormalize();
		float angle = expandVec.CalcuAngle() * LRAD2DEGREE + 180;
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

int GameObjectBentLaser::SampleT(lua_State * L, float delay) LNOEXCEPT
{
	//插入一个数组
	lua_newtable(L); //... t(list)
	// 忽略没有节点的情况
	if (m_Queue.Size() <= 1)
		return true;

	float fLeft = 0;// 剩余长度
	int count = 0;

	float tVecLength = 0;
	for (size_t i = m_Queue.Size() - 1; i > 0; --i)
	{
		LaserNode& cur = m_Queue[i];
		LaserNode& next = m_Queue[i - 1];


		fcyVec2 vn = cur.pos;
		fcyVec2 offsetA = next.pos - cur.pos;
		float lenOffsetA = offsetA.Length();
		float angle = offsetA.CalcuAngle() * LRAD2DEGREE + 180;
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
