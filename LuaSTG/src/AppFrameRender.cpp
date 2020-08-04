#include "AppFrame.h"

namespace LuaSTGPlus {
	void AppFrame::updateGraph2DBlendMode(BlendMode m)
	{
		if (m != m_Graph2DLastBlendMode)
		{
			m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_SRCALPHA;
			m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
			m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
			
			m_Graph2DBlendState.AlphaSrcBlend = F2DBLENDFACTOR_SRCALPHA;
			m_Graph2DBlendState.AlphaDestBlend = F2DBLENDFACTOR_INVSRCALPHA;
			m_Graph2DBlendState.AlphaBlendOp = F2DBLENDOPERATOR_ADD;

			m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_MODULATE;

			switch (m)
			{
			case BlendMode::AddAdd:
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
				break;
			case BlendMode::AddAlpha:
				m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
				break;
			case BlendMode::AddSub:
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_SUBTRACT;
				m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
				break;
			case BlendMode::AddRev:
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_REVSUBTRACT;
				m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
				break;
			case BlendMode::MulAdd:
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				break;
			case BlendMode::MulAlpha:
				break;
			case BlendMode::MulSub:
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_SUBTRACT;
				break;
			case BlendMode::MulRev:
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_REVSUBTRACT;
				break;

			case BlendMode::MulMin:
				m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_MIN;
				break;
			case BlendMode::MulMax:
				m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_MAX;
				break;
			case BlendMode::MulMutiply:
				m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_ZERO;
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_SRCCOLOR;
				break;
			case BlendMode::MulScreen:
				m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCCOLOR;
				break;
			case BlendMode::AddMin:
				m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_MIN;
				m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
				break;
			case BlendMode::AddMax:
				m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_MAX;
				m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
				break;
			case BlendMode::AddMutiply:
				m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_ZERO;
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_SRCCOLOR;
				m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
				break;
			case BlendMode::AddScreen:
				m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCCOLOR;
				m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
				break;

			case BlendMode::AlphaBal:
				m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_INVDESTCOLOR;
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCCOLOR;
				m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ALPHATEXTURE;
				break;
			case BlendMode::One:
				m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ZERO;
				m_Graph2DBlendState.AlphaSrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.AlphaDestBlend = F2DBLENDFACTOR_ZERO;
				break;

			default:
				break;
			}
			m_Graph2DLastBlendMode = m;
			m_Graph2D->SetBlendState(m_Graph2DBlendState);
			m_Graph2D->SetColorBlendType(m_Graph2DColorBlendState);
		}
	}

	void AppFrame::updateGraph3DBlendMode(BlendMode m)
	{
		if (m != m_Graph3DLastBlendMode)
		{
			m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_SRCALPHA;
			switch (m)
			{
			case BlendMode::AddAdd:
			case BlendMode::MulAdd:
				m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
				break;
			case BlendMode::AddSub:
			case BlendMode::MulSub:
				m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
				m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_SUBTRACT;
				break;
			case BlendMode::AddRev:
			case BlendMode::MulRev:
				m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
				m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_REVSUBTRACT;
				break;
			case BlendMode::AddAlpha:
			case BlendMode::MulAlpha:
				m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
				m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
				break;

			case BlendMode::AddMin:
			case BlendMode::MulMin:
				m_Graph3DBlendState.SrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_MIN;
				break;
			case BlendMode::AddMax:
			case BlendMode::MulMax:
				m_Graph3DBlendState.SrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
				m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_MAX;
				break;
			case BlendMode::AddMutiply:
			case BlendMode::MulMutiply:
				m_Graph3DBlendState.SrcBlend = F2DBLENDFACTOR_ZERO;
				m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_SRCCOLOR;
				m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
				break;
			case BlendMode::AddScreen:
			case BlendMode::MulScreen:
				m_Graph3DBlendState.SrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCCOLOR;
				m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
				break;

			case BlendMode::AlphaBal:
				m_Graph3DBlendState.SrcBlend = F2DBLENDFACTOR_INVDESTCOLOR;
				m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCCOLOR;
				m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
				break;
			case BlendMode::One:
				m_Graph2DBlendState.SrcBlend = F2DBLENDFACTOR_ONE;
				m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ZERO;
				m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
				break;

			default:
				m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
				m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
				break;
			}
			m_Graph3DLastBlendMode = m;
			m_Graph3D->SetBlendState(m_Graph3DBlendState);
		}
	}

	static f2dGraphics2DVertex vcache[2048];
	static fuShort             icache[8192];
	
	bool AppFrame::RenderSector(const char* name, fcyRect uv, bool tran, BlendMode blend, fcyColor color1, fcyColor color2,
		fcyVec2 pos, float rot, float exp, float r1, float r2, int div)
	{
		// 检查渲染器
		if (m_GraphType != GraphicsType::Graph2D)
		{
			LERROR("RenderTexture: 只有2D渲染器可以执行该方法");
			return false;
		}
		// 计算顶点
		if (div < 1) {
			return true; // 不需要绘制
		}
		int total = div + 1;
		int total_vertex = total * 2;
		int total_index = div * 6;
		if (total_vertex > 2048 || total_index > 8192) {
			LERROR("RenderSector: 顶点数量过多");
			return false;
		}
		// 寻找纹理
		fcyRefPointer<ResTexture> p = m_ResourceMgr.FindTexture(name);
		if (!p)
		{
			LERROR("RenderTexture: 找不到纹理资源'%m'", name);
			return false;
		}
		// 更新混合模式
		updateGraph2DBlendMode(blend);
		// 计算顶点
		float rotv = rot, rotd = -exp / (float)div, cosv = 0.0f, sinv = 0.0f;
		float texw = (float)p->GetTexture()->GetWidth(), texh = (float)p->GetTexture()->GetHeight();
		float uvcu = uv.a.x, uvdu = (uv.b.x - uv.a.x) / (float)div;
		float uvcv = uv.a.y, uvdv = (uv.b.y - uv.a.y) / (float)div;
		for (int i = 0; i <= div; i++) {
			f2dGraphics2DVertex& vt1 = vcache[i * 2]; // 内圈顶点
			f2dGraphics2DVertex& vt2 = vcache[i * 2 + 1]; // 外圈顶点
			// XY
			cosv = cosf(rotv); sinv = sinf(rotv);
			rotv += rotd;
			vt1.x = pos.x + cosv * r1; vt1.y = pos.y + sinv * r1;
			vt2.x = pos.x + cosv * r2; vt2.y = pos.y + sinv * r2;
			// Z
			vt1.z = 0.5f; vt2.z = 0.5f;
			// UV
			if (tran) {
				vt1.u = vt2.u = uvcu / texw;
				vt1.v = uv.b.y / texh; vt2.v = uv.a.y / texh;
				uvcu += uvdu;
			}
			else {
				vt1.u = uv.a.x / texw; vt2.u = uv.b.x / texw;
				vt1.v = vt2.v = uvcv / texh;
				uvcv += uvdv;
			}
			// 颜色
			vt1.color = color1.argb; vt2.color = color2.argb;
		}
		// 计算索引
		fuShort indv = 0;
		for (int i = 0; i < div; i++) {
			// 第一个三角形
			icache[i * 6 + 0] = indv + 1;
			icache[i * 6 + 1] = indv + 3;
			icache[i * 6 + 2] = indv + 2;
			// 第二个三角形
			icache[i * 6 + 3] = indv + 1;
			icache[i * 6 + 4] = indv + 2;
			icache[i * 6 + 5] = indv + 0;
			// 下一批
			indv += 2;
		}
		// 绘制
		auto ret = m_Graph2D->DrawRaw(p->GetTexture(), total_vertex, total_index, vcache, icache, false);
		return (FCYERR_OK == ret);
	}

	bool AppFrame::RenderAnnulus(const char* name, fcyRect uv, bool tran, BlendMode blend, fcyColor color1, fcyColor color2,
		fcyVec2 pos, float rot, float r1, float r2, int div, int rep)
	{
		bool ret = true;
		float rotv = rot;
		float persector = (2.0f * (float)LPI_FULL) / (float)rep;
		for (int i = 0; i < rep; i++) {
			ret = ret && RenderSector(name, uv, tran, blend, color1, color2, pos, rotv, persector, r1, r2, div);
			rotv += persector;
		}
		return true;
	}
};
