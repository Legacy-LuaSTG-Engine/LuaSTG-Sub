////////////////////////////////////////////////////////////////////////////////
/// @file  f2dGraphics2DImpl.h
/// @brief fancy2D 二维图元渲染器实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "GraphicDevice11/f2dGraphicsImpl.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 二维渲染器实现
////////////////////////////////////////////////////////////////////////////////
class f2dGraphics2DImpl :
	public f2dGraphicsImpl<f2dGraphics2D>,
	public f2dRenderDeviceEventListener
{
	friend class f2dRenderDeviceImpl;
protected:
	static const fuInt FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;  // 顶点标识
	struct RenderCommand
	{
		f2dTexture2D* pTex;  // 渲染用的纹理，可空
		fuInt VertCount;     // 绘制耗费的顶点数
		fuInt IndexCount;    // 绘制耗费的索引数
	};
protected:
	IDirect3DVertexBuffer9* m_pVB;
	IDirect3DIndexBuffer9* m_pIB;
	
	fuInt m_VBMaxCount;   // 顶点缓冲最大容量
	fuInt m_IBMaxCount;   // 索引缓冲最大容量
	fuInt m_VBUsedCount;  // 顶点缓冲区已用空间
	fuInt m_IBUsedCount;  // 索引缓冲区已用空间
	fuInt m_VBAlloced;    // 待flush
	fuInt m_IBAlloced;    // 待flush

	f2dGraphics2DVertex* m_pVBData;
	fuShort* m_pIBData;

	// 指令列表
	std::vector<RenderCommand> m_Commands;

	// 当前混合模式
	F2DGRAPH2DBLENDTYPE m_ColorBlendType;
private:
	// 快速小数转整数
	// 已废弃
	static inline long ftol_ieee_M(float x) 
	{ 
		static const float magic_f = (3<<21);
		static const long magic_i = 0x4ac00000;
		float ftmp=x+magic_f;
		return  (*((long*)&ftmp)-magic_i) >> 1; 
	}
	// 坐标修复
	void inline coordFix(float& x)
	{
		x = x - 0.5f;
	}
	// 执行所有绘图指令
	void flush(bool Discard=false);
	// 分配内存
	bool alloc(fuInt VertCountToAlloc, fuInt IndexCountToAlloc, f2dGraphics2DVertex** pVertexOut, fuShort** pIndexOut);
	// 追加命令并进行合并
	void pushCommand(f2dTexture2D* pTex, fuInt VertCount, fuInt IndexCount);
	
	// 翻译并设置混合模式
	fResult setColorBlendType(F2DGRAPH2DBLENDTYPE Type);
protected: // 设备丢失处理
	void OnRenderDeviceLost();
	void OnRenderDeviceReset();
public: // 接口实现
	fBool IsGraphics3D() { return false; }

	fResult Begin();
	fResult Flush();
	fResult End();

	F2DGRAPH2DBLENDTYPE GetColorBlendType();
	fResult SetColorBlendType(F2DGRAPH2DBLENDTYPE Type);

	fResult DrawQuad(
		f2dTexture2D* pTex,
		const f2dGraphics2DVertex& v1, 
		const f2dGraphics2DVertex& v2, 
		const f2dGraphics2DVertex& v3, 
		const f2dGraphics2DVertex& v4,
		fBool bAutoFixCoord = true
		);
	fResult DrawQuad(f2dTexture2D* pTex, const f2dGraphics2DVertex* arr, fBool bAutoFixCoord = true);
	fResult DrawRaw(f2dTexture2D* pTex, fuInt VertCount, fuInt IndexCount, const f2dGraphics2DVertex* VertArr, const fuShort* IndexArr, fBool bAutoFixCoord = true);
public:
	f2dGraphics2DImpl(f2dRenderDeviceImpl* pParent, fuInt VertexBufferCount, fuInt IndexBufferCount);
	~f2dGraphics2DImpl();
};
