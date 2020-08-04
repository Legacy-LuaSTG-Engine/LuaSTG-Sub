////////////////////////////////////////////////////////////////////////////////
/// @file  f2dRenderDevice.h
/// @brief fancy2D渲染设备接口定义
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dInterface.h"

/// @addtogroup f2d渲染系统
/// @brief fancy2d引擎中的渲染系统，提供对2D图形的渲染支持。
/// @{

// =============================== 纹理包装 ===================================

// 纹理格式：

////////////////////////////////////////////////////////////////////////////////
/// @brief 纹理页面格式
////////////////////////////////////////////////////////////////////////////////
enum F2DSURFACEFMT
{
	F2DSURFACEFMT_UNKNOWN,  ///< @brief 未知格式
	F2DSURFACEFMT_A8R8G8B8  ///< @brief ARGB格式
};

// 纹理接口：

////////////////////////////////////////////////////////////////////////////////
/// @brief 纹理接口
////////////////////////////////////////////////////////////////////////////////
struct f2dTexture :
	public f2dInterface
{
	/// @brief   获得内部纹理对象
	/// @warning 高级方法，若无特殊需要请勿使用
	/// @return  返回IDirect3DTexutreBase9接口
	virtual void* GetHandle()=0;

	virtual fuInt GetDimension()=0;   ///< @brief 返回纹理维数, 用以区分Tex1D,Tex2D,Tex3D
	virtual fBool IsDynamic()=0;      ///< @brief 是否可修改
	virtual fBool IsRenderTarget()=0; ///< @brief 是否是渲染目标
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 二维纹理接口
////////////////////////////////////////////////////////////////////////////////
struct f2dTexture2D :
	public f2dTexture
{
	virtual fuInt GetWidth()=0;    ///< @brief 返回宽度
	virtual fuInt GetHeight()=0;   ///< @brief 返回高度

	/// @brief      锁定纹理
	/// @note       只有动态纹理可以被锁定。锁定操作和解锁操作配对，在未解锁前只能锁定一次。
	/// @param[in]  pLockRect 被锁定的区域，设为NULL则锁定整个纹理
	/// @param[in]  Discard   如果原始数据正在被使用则抛弃原始数据
	/// @param[out] Pitch     行对齐字节
	/// @param[out] pOut      返回的内存指针，自动计算到区域的首字节
	virtual fResult Lock(fcyRect* pLockRect, fBool Discard, fuInt* Pitch, fData* pOut)=0;

	/// @brief 解锁纹理
	virtual fResult Unlock()=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 深度模板缓冲区
////////////////////////////////////////////////////////////////////////////////
struct f2dDepthStencilSurface :
	public f2dInterface
{
	virtual void* GetHandle()=0;   ///< @brief 返回IDirect3DSurface对象，高级方法
	virtual fuInt GetWidth()=0;    ///< @brief 返回宽度
	virtual fuInt GetHeight()=0;   ///< @brief 返回高度
};

// =============================== 效果包装 ===================================

// 效果参数：

////////////////////////////////////////////////////////////////////////////////
/// @brief 效果参数类型
////////////////////////////////////////////////////////////////////////////////
enum F2DEPTYPE
{
	F2DEPTYPE_UNKNOWN,  ///< @brief 未知参数类型
	F2DEPTYPE_VALUE,    ///< @brief 值类型
	F2DEPTYPE_ARRAY,    ///< @brief 数组
	F2DEPTYPE_STRUCT    ///< @brief 结构体
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 效果参数值类型
////////////////////////////////////////////////////////////////////////////////
enum F2DEPVTYPE
{
	F2DEPVTYPE_UNKNOWN,   ///< @brief 未知类型
	F2DEPVTYPE_VOID,      ///< @brief 无类型
	F2DEPVTYPE_BOOL,      ///< @brief Bool型
	F2DEPVTYPE_FLOAT,     ///< @brief Float型
	F2DEPVTYPE_INT,       ///< @brief Int型
	F2DEPVTYPE_VECTOR,    ///< @brief 向量
	F2DEPVTYPE_MATRIX,    ///< @brief 矩阵
	F2DEPVTYPE_TEXTURE1D, ///< @brief 1D纹理
	F2DEPVTYPE_TEXTURE2D, ///< @brief 2D纹理
	F2DEPVTYPE_TEXTURE3D, ///< @brief 3D纹理
	F2DEPVTYPE_STRING     ///< @brief 字符串
};

// 效果参数对象：

////////////////////////////////////////////////////////////////////////////////
/// @brief 效果参数
/// @note  注意一旦Effect对象被释放，该接口将无效。
////////////////////////////////////////////////////////////////////////////////
struct f2dEffectParam
{
	virtual fcStr GetName()=0;                 ///< @brief 返回参数名称
	virtual F2DEPTYPE GetType()=0;             ///< @brief 返回参数类型

	virtual fcStr GetSemantic()=0;                        ///< @brief 获得参数语义
	virtual f2dEffectParam* GetAnnotation(fcStr Name)=0;  ///< @brief 返回注释对象
	virtual f2dEffectParam* GetAnnotation(fuInt Index)=0; ///< @brief 返回注释对象
	virtual fuInt GetAnnotationCount()=0;                 ///< @brief 返回注释对象个数
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 效果参数对象
/// @note  注意一旦Effect对象被释放，该接口将无效。
////////////////////////////////////////////////////////////////////////////////
struct f2dEffectParamValue :
	public f2dEffectParam
{
	virtual F2DEPVTYPE GetValueType()=0;      ///< @brief 获得类型

	virtual fuInt GetRow()=0;                 ///< @brief 获得向量/矩阵行数
	virtual fuInt GetColumn()=0;              ///< @brief 获得向量/矩阵列数

	virtual fBool IsRowFirstMatrix()=0;       ///< @brief 是否为行优先矩阵

	virtual fBool GetBool()=0;                ///< @brief 返回Bool型
	virtual fFloat GetFloat()=0;              ///< @brief 返回Float型
	virtual fInt GetInt()=0;                  ///< @brief 返回Int型
	virtual fcyVec4 GetVector()=0;            ///< @brief 返回向量
	virtual fcyMatrix4 GetMatrix()=0;         ///< @brief 返回矩阵
	virtual f2dTexture* GetTexture()=0;       ///< @brief 获得纹理对象
	virtual fcStr GetString()=0;              ///< @brief 返回字符串数据

	virtual fResult SetBool(fBool Value)=0;   ///< @brief 设置Bool型
	virtual fResult SetFloat(fFloat Value)=0; ///< @brief 设置Float型
	virtual fResult SetInt(fInt Value)=0;     ///< @brief 设置Int型
	virtual fResult SetVector(const fcyVec4& Value)=0;    ///< @brief 设置向量
	virtual fResult SetMatrix(const fcyMatrix4& Value)=0; ///< @brief 设置矩阵
	virtual fResult SetTexture(f2dTexture* pTex)=0;       ///< @brief 设置纹理
	virtual fResult SetString(fcStr Str)=0;               ///< @brief 设置字符串
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 数组类型参数
/// @note  注意一旦Effect对象被释放，该接口将无效。
////////////////////////////////////////////////////////////////////////////////
struct f2dEffectParamArr :
	public f2dEffectParam
{
	/// @brief 返回元素个数
	virtual fuInt GetElementCount()=0;

	/// @brief 通过元素Index获取元素
	virtual f2dEffectParam* GetElement(fuInt Index)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 结构类型参数
/// @note  注意一旦Effect对象被释放，该接口将无效。
////////////////////////////////////////////////////////////////////////////////
struct f2dEffectParamStruct :
	public f2dEffectParam
{
	///< @brief 获得成员数目
	virtual fuInt GetElementCount()=0;

	///< @brief 通过成员名获得成员
	virtual f2dEffectParam* GetElement(fcStr Name)=0;
	///< @brief 通过成员下标获得成员
	virtual f2dEffectParam* GetElement(fuInt Index)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 效果入口函数
/// @note  注意一旦Effect对象被释放，该接口将无效。
////////////////////////////////////////////////////////////////////////////////
struct f2dEffectFunction
{
	virtual fcStr GetName()=0;                            ///< @brief 返回入口名称
	virtual f2dEffectParam* GetAnnotation(fcStr Name)=0;  ///< @brief 返回注释对象
	virtual f2dEffectParam* GetAnnotation(fuInt Index)=0; ///< @brief 返回注释对象
	virtual fuInt GetAnnotationCount()=0;                 ///< @brief 返回注释对象个数
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 效果批次
/// @note  注意一旦Effect对象被释放，该接口将无效。
////////////////////////////////////////////////////////////////////////////////
struct f2dEffectPass
{
	virtual fuInt GetIndex()=0;                           ///< @brief 返回Pass的Index
	virtual fcStr GetName()=0;                            ///< @brief 返回名称
	virtual f2dEffectParam* GetAnnotation(fcStr Name)=0;  ///< @brief 返回注释对象
	virtual f2dEffectParam* GetAnnotation(fuInt Index)=0; ///< @brief 返回注释对象
	virtual fuInt GetAnnotationCount()=0;                 ///< @brief 返回注释对象个数
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 效果技术
////////////////////////////////////////////////////////////////////////////////
struct f2dEffectTechnique
{
	virtual fcStr GetName()=0;                            ///< @brief 返回名称
	virtual f2dEffectParam* GetAnnotation(fcStr Name)=0;  ///< @brief 返回注释对象
	virtual f2dEffectParam* GetAnnotation(fuInt Index)=0; ///< @brief 返回注释对象
	virtual fuInt GetAnnotationCount()=0;                 ///< @brief 返回注释对象个数

	virtual fBool IsValidate()=0;                 ///< @brief 检查设备是否支持该效果
	virtual fuInt GetPassCount()=0;               ///< @brief 返回效果批次个数
	virtual f2dEffectPass* GetPass(fuInt Index)=0;///< @brief 获得渲染批次
	virtual f2dEffectPass* GetPass(fcStr Name)=0; ///< @brief 获得渲染批次
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 效果对象
/// @note  监听器优先级 = 16
////////////////////////////////////////////////////////////////////////////////
struct f2dEffect :
	public f2dInterface
{
	/// @brief   获得ID3DXEffect对象
	/// @warning 高级方法
	virtual void* GetHandle()=0;

	virtual fuInt GetParamCount()=0;                  ///< @brief 获得参数个数
	virtual f2dEffectParam* GetParam(fuInt Index)=0;  ///< @brief 获得参数
	virtual f2dEffectParam* GetParam(fcStr Name)=0;   ///< @brief 获得参数

	virtual fuInt GetTechniqueCount()=0;                     ///< @brief 获得技术个数
	virtual f2dEffectTechnique* GetTechnique(fuInt Index)=0; ///< @brief 获得技术
	virtual f2dEffectTechnique* GetTechnique(fcStr Name)=0;  ///< @brief 获得技术

	virtual fuInt GetFunctionCount()=0;                     ///< @brief 获得函数个数
	virtual f2dEffectFunction* GetFunction(fuInt Index)=0;  ///< @brief 获得函数
	virtual f2dEffectFunction* GetFunction(fcStr Name)=0;   ///< @brief 获得函数
	
	virtual f2dEffectTechnique* GetCurrentTechnique()=0;              ///< @brief 获得当前渲染技术
	virtual fResult SetCurrentTechnique(f2dEffectTechnique* pTech)=0; ///< @brief 设置当前渲染技术

	virtual fuInt GetCurrentPassCount()=0; ///< @brief 返回当前技术的渲染批次个数
};

// ============================= 渲染器包装 ===================================

// 纹理采样选项：

////////////////////////////////////////////////////////////////////////////////
/// @brief 采样状态
////////////////////////////////////////////////////////////////////////////////
enum F2DSAMPLERSTATETYPE {
	F2DSAMPLERSTATE_ADDRESSU    = 1, //纹理U坐标采样方式
	F2DSAMPLERSTATE_ADDRESSV    = 2, //纹理V坐标采样方式

	F2DSAMPLERSTATE_BORDERCOLOR = 4, //纹理界外颜色，默认0x00000000

	F2DSAMPLERSTATE_MAGFILTER   = 5, //放大采样过滤器
	F2DSAMPLERSTATE_MINFILTER   = 6, //缩小采样过滤器
	F2DSAMPLERSTATE_MIPFILTER   = 7, //mipmap过滤器
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 采样寻址
////////////////////////////////////////////////////////////////////////////////
enum F2DTEXTUREADDRESS {
	F2DTEXTUREADDRESS_WRAP       = 1, //重复平铺
	F2DTEXTUREADDRESS_MIRROR     = 2, //镜像重复平铺
	F2DTEXTUREADDRESS_CLAMP      = 3, //限制界外为纹理边缘颜色
	F2DTEXTUREADDRESS_BORDER     = 4, //界外为设置的边缘颜色
	F2DTEXTUREADDRESS_MIRRORONCE = 5, //绕一个轴镜像重复平铺
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 采样过滤器
////////////////////////////////////////////////////////////////////////////////
enum F2DTEXFILTERTYPE {
	F2DTEXFILTER_POINT         = 1, //邻近插值
	F2DTEXFILTER_LINEAR        = 2, //三线性插值
	F2DTEXFILTER_ANISOTROPIC   = 3, //各项异性过滤插值，不适用于MIPFILTER
	F2DTEXFILTER_PYRAMIDALQUAD = 6, //4向柔和采样过滤器，一般不使用
	F2DTEXFILTER_GAUSSIANQUAD  = 7, //4向高斯采样过滤器，一般不使用
};

// 混合选项：

////////////////////////////////////////////////////////////////////////////////
/// @brief 混合运算符
////////////////////////////////////////////////////////////////////////////////
enum F2DBLENDOPERATOR
{
	F2DBLENDOPERATOR_ADD         = 1, //混合结果 = 源像素 + 目标像素
	F2DBLENDOPERATOR_SUBTRACT    = 2, //混合结果 = 源像素 - 目标像素
	F2DBLENDOPERATOR_REVSUBTRACT = 3, //混合结果 = 目标像素 - 源像素
	F2DBLENDOPERATOR_MIN         = 4, //混合结果 = MIN(源像素, 目标像素)
	F2DBLENDOPERATOR_MAX         = 5  //混合结果 = MAX(源像素, 目标像素)
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 混合因素
/// @note  颜色混合公式：Color = SrcColor * SrcBlend + DestColor * DestBlend
////////////////////////////////////////////////////////////////////////////////
enum F2DBLENDFACTOR
{
    F2DBLENDFACTOR_ZERO               = 1,  //混合因素(0, 0, 0, 0)
    F2DBLENDFACTOR_ONE                = 2,  //混合因素(1, 1, 1, 1)
    F2DBLENDFACTOR_SRCCOLOR           = 3,  //混合因素(Rs, Gs, Bs, As)
    F2DBLENDFACTOR_INVSRCCOLOR        = 4,  //混合因素(1 - Rs, 1 - Gs, 1 - Bs, 1 - As)
    F2DBLENDFACTOR_SRCALPHA           = 5,  //混合因素(As, As, As, As)
    F2DBLENDFACTOR_INVSRCALPHA        = 6,  //混合因素(1 - As, 1 - As, 1 - As, 1 - As)
    F2DBLENDFACTOR_DESTALPHA          = 7,  //混合因素(Ad, Ad, Ad, Ad)
    F2DBLENDFACTOR_INVDESTALPHA       = 8,  //混合因素(1 - Ad, 1 - Ad, 1 - Ad, 1 - Ad)
    F2DBLENDFACTOR_DESTCOLOR          = 9,  //混合因素(Rd, Gd, Bd, Ad)
    F2DBLENDFACTOR_INVDESTCOLOR       = 10, //混合因素(1 - Rd, 1 - Gd, 1 - Bd, 1 - Ad)
    F2DBLENDFACTOR_SRCALPHASAT        = 11, //混合因素(f, f, f, 1)，其中f = min(As, 1 - Ad)
	F2DBLENDFACTOR_BLENDFACTOR        = 14, //混合因素(Rf, Gf, Bf, Af)
	F2DBLENDFACTOR_INVBLENDFACTOR     = 15, //混合因素(1 - Rf, 1 - Gf, 1 - Bf, 1 - Af)
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 混合状态
////////////////////////////////////////////////////////////////////////////////
struct f2dBlendState
{
	F2DBLENDFACTOR SrcBlend;        //源混合参数
	F2DBLENDFACTOR DestBlend;       //目的混合参数
	F2DBLENDOPERATOR BlendOp;       //混合运算符
	F2DBLENDFACTOR AlphaSrcBlend;   //Alpha源混合参数，不可使用RGBA选项
	F2DBLENDFACTOR AlphaDestBlend;  //Alpha目的混合参数，不可使用RGBA选项
	F2DBLENDOPERATOR AlphaBlendOp;  //Alpha混合运算符
};

// 渲染器：

////////////////////////////////////////////////////////////////////////////////
/// @brief 渲染器接口
////////////////////////////////////////////////////////////////////////////////
struct f2dGraphics :
	public f2dInterface
{
	virtual fBool IsGraphics3D()=0;  ///< @brief 是否为3D渲染器，否则为2D渲染器

	// === 渲染过程控制 ===
	virtual fBool IsInRender()=0;    ///< @brief 检查是否处于Begin/End区段
	virtual fResult Begin()=0;       ///< @brief 启动渲染器并完成状态同步
	virtual fResult Flush()=0;       ///< @brief 立即递交渲染队列
	virtual fResult End()=0;         ///< @brief 结束渲染过程并递交渲染队列

	// === 渲染状态控制 ===
	virtual const fcyMatrix4& GetWorldTransform()=0;  ///< @brief 返回世界变换矩阵
	virtual const fcyMatrix4& GetViewTransform()=0; ///< @brief 返回观察矩阵
	virtual const fcyMatrix4& GetProjTransform()=0;   ///< @brief 返回投影矩阵

	virtual void SetWorldTransform(const fcyMatrix4& Mat)=0;  ///< @brief 设置世界变换矩阵
	virtual void SetViewTransform(const fcyMatrix4& Mat)=0; ///< @brief 设置观察矩阵
	virtual void SetProjTransform(const fcyMatrix4& Mat)=0;   ///< @brief 设置投影矩阵

	// === 混合状态控制 ===
	virtual const f2dBlendState& GetBlendState()=0;           ///< @brief 返回像素混合参数
	virtual void SetBlendState(const f2dBlendState& State)=0; ///< @brief 设置像素混合参数
};

// 2D渲染器：

////////////////////////////////////////////////////////////////////////////////
/// @brief 2D渲染器混合选项
////////////////////////////////////////////////////////////////////////////////
enum F2DGRAPH2DBLENDTYPE
{
	F2DGRAPH2DBLENDTYPE_DISABLE,          ///< @brief 禁用
	F2DGRAPH2DBLENDTYPE_SELECTCOLOR,      ///< @brief 选择颜色(final=color)
	F2DGRAPH2DBLENDTYPE_SELECTTEXTURE,    ///< @brief 选择纹理(final=texture)
	F2DGRAPH2DBLENDTYPE_ADD,              ///< @brief 颜色相加(final=color+texture)
	F2DGRAPH2DBLENDTYPE_SUBTRACT,         ///< @brief 颜色相减(final=color-texture)
	F2DGRAPH2DBLENDTYPE_MODULATE,         ///< @brief 颜色相乘(final=color*texture)
	F2DGRAPH2DBLENDTYPE_MODULATE2X,       ///< @brief 颜色相乘x2(final=color*texture*2)
	F2DGRAPH2DBLENDTYPE_MODULATE4X,       ///< @brief 颜色相乘x4(final=color*texture*4)
	F2DGRAPH2DBLENDTYPE_ADDSIGNED,        ///< @brief 颜色相加并偏移(final=color+texture-0.5)
	F2DGRAPH2DBLENDTYPE_ADDSIGNED2X,      ///< @brief 颜色相加并偏移x2(final=(color+texture-0.5)x2)
	F2DGRAPH2DBLENDTYPE_ADDSMOOTH,         ///< @brief 平滑相加(final=color+texture-color*texture)
	F2DGRAPH2DBLENDTYPE_ALPHATEXTURE      ///< @brief 颜色透明(final=tex*texalpha+color*(1-texalpha))

};

////////////////////////////////////////////////////////////////////////////////
/// @brief 二维渲染器顶点
////////////////////////////////////////////////////////////////////////////////
struct f2dGraphics2DVertex
{
	fFloat x;      ///< @brief x坐标
	fFloat y;      ///< @brief y坐标
	fFloat z;      ///< @brief z坐标
	fuInt color;   ///< @brief 漫反射颜色
	fFloat u;      ///< @brief 纹理u坐标
	fFloat v;      ///< @brief 纹理v坐标
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 二维渲染器顶点
////////////////////////////////////////////////////////////////////////////////
struct f2dGraphics2DLVertex
{
	fFloat x;      ///< @brief x坐标
	fFloat y;      ///< @brief y坐标
	fFloat z;      ///< @brief z坐标
	fFloat nx;    ///< @brief 法线x
	fFloat ny;    ///< @brief 法线y
	fFloat nz;    ///< @brief 法线z
	fFloat u;      ///< @brief 纹理u坐标
	fFloat v;      ///< @brief 纹理v坐标
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 2D渲染器接口
////////////////////////////////////////////////////////////////////////////////
struct f2dGraphics2D :
	public f2dGraphics
{
	// === 混合选项 ===
	/// @brief 返回颜色混合状态
	virtual F2DGRAPH2DBLENDTYPE GetColorBlendType()=0;

	/// @brief 设置颜色混合状态
	virtual fResult SetColorBlendType(F2DGRAPH2DBLENDTYPE Type)=0;

	// === 2D绘制函数 ===
	/// @brief 绘制矩形
	virtual fResult DrawQuad(
		f2dTexture2D* pTex,
		const f2dGraphics2DVertex& v1, 
		const f2dGraphics2DVertex& v2, 
		const f2dGraphics2DVertex& v3, 
		const f2dGraphics2DVertex& v4,
		fBool bAutoFixCoord = true
		)=0;

	/// @brief 绘制矩形
	virtual fResult DrawQuad(f2dTexture2D* pTex, const f2dGraphics2DVertex* arr, fBool bAutoFixCoord = true)=0;

	/// @brief 原始绘制函数
	virtual fResult DrawRaw(f2dTexture2D* pTex, fuInt VertCount, fuInt IndexCount, const f2dGraphics2DVertex* VertArr, const fuShort* IndexArr, fBool bAutoFixCoord = true)=0;
};

// 3D渲染器：

////////////////////////////////////////////////////////////////////////////////
/// @brief 顶点元素用途
////////////////////////////////////////////////////////////////////////////////
enum F2DVDUSAGE
{
    F2DVDUSAGE_POSITION = 0,    ///< @brief 坐标
    F2DVDUSAGE_BLENDWEIGHT,     ///< @brief 混合权重
    F2DVDUSAGE_BLENDINDICES,    ///< @brief 混合索引
    F2DVDUSAGE_NORMAL,          ///< @brief 法线
    F2DVDUSAGE_PSIZE,           ///< @brief 粒子大小
    F2DVDUSAGE_TEXCOORD,        ///< @brief 纹理坐标
    F2DVDUSAGE_TANGENT,         ///< @brief 切线
    F2DVDUSAGE_BINORMAL,        ///< @brief 次法线
    F2DVDUSAGE_TESSFACTOR,      ///< @brief 细分因子
    F2DVDUSAGE_POSITIONT,       ///< @brief 
    F2DVDUSAGE_COLOR,           ///< @brief 颜色
    F2DVDUSAGE_FOG,             ///< @brief 雾
    F2DVDUSAGE_DEPTH,           ///< @brief 深度
    F2DVDUSAGE_SAMPLE           ///< @brief 采样
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 顶点元素数据类型
////////////////////////////////////////////////////////////////////////////////
enum F2DVDTYPE
{
    F2DVDTYPE_FLOAT1 =  0,  ///< @brief 1*4字节浮点
	                        ///< @note  GPU内扩展到 (value, 0., 0., 1.)
    F2DVDTYPE_FLOAT2 =  1,  ///< @brief 2*4字节浮点
	                        ///< @note  GPU内扩展到 (value, value, 0., 1.)
    F2DVDTYPE_FLOAT3 =  2,  ///< @brief 3*4字节浮点
	                        ///< @note  GPU内扩展到 (value, value, value, 1.)
    F2DVDTYPE_FLOAT4 =  3,  ///< @brief 4*4字节浮点
    F2DVDTYPE_COLOR  =  4,  ///< @brief 4*1字节
	                        ///< @note  GPU内扩展各个分量到[0,1]，排布 (R, G, B, A)

    F2DVDTYPE_UBYTE4 =  5,  ///< @brief 4*2字节无符号短整数
    F2DVDTYPE_SHORT2 =  6,  ///< @brief 2*2字节有符号短整数
					        ///< @note  GPU内扩展到 (value, value, 0., 1.)
    F2DVDTYPE_SHORT4 =  7   ///< @brief 4*2字节有符号短整数
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 顶点元素声明
////////////////////////////////////////////////////////////////////////////////
struct f2dVertexElement
{
	F2DVDTYPE Type;   ///< @brief 类型
	F2DVDUSAGE Usage; ///< @brief 用途
	fByte UsageIndex; ///< @brief 用途索引
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 3D渲染器接口
////////////////////////////////////////////////////////////////////////////////
struct f2dGraphics3D :
	public f2dGraphics
{
	// === FX状态 ===
	/// @brief 返回当前渲染程序
	virtual f2dEffect* GetEffect()=0;

	/// @brief 设置当前渲染程序
	virtual fResult SetEffect(f2dEffect* Effect)=0;

	/// @brief     开始一个渲染遍
	/// @param[in] PassIndex 渲染遍下标
	virtual fResult BeginPass(fuInt PassIndex)=0;

	/// @brief 结束一个渲染遍
	virtual fResult EndPass()=0;

	/// @brief 刷新Effect状态
	/// @note  在pass中改变状态需要手动调用本函数
	virtual fResult CommitState()=0;

	// === PostEffect ===
	/// @brief 为执行PostEffect进行渲染
	virtual fResult RenderPostEffect()=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 图元类型
////////////////////////////////////////////////////////////////////////////////
enum F2DPRIMITIVETYPE 
{
	F2DPT_NULL          = 0,  ///< @brief 无效值
    F2DPT_POINTLIST     = 1,  ///< @brief 点集
    F2DPT_LINELIST      = 2,  ///< @brief 线集
    F2DPT_LINESTRIP     = 3,  ///< @brief 线带集
    F2DPT_TRIANGLELIST  = 4,  ///< @brief 三角形集
    F2DPT_TRIANGLESTRIP = 5,  ///< @brief 三角形带
    F2DPT_TRIANGLEFAN   = 6   ///< @brief 三角形扇
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 网格子集属性
////////////////////////////////////////////////////////////////////////////////
struct f2dMeshSubsetInfo
{
	F2DPRIMITIVETYPE Type;  ///< @brief 图元类型
	fuInt StartIndex;       ///< @brief 开始索引
	fuInt PrimitiveCount;   ///< @brief 图元数量
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 网格
////////////////////////////////////////////////////////////////////////////////
struct f2dMeshData :
	public f2dInterface
{
	/// @brief      锁定顶点数据
	/// @param[in]  StartVert 开始锁定的顶点
	/// @param[in]  VertCount 要锁定的顶点数量，0表示全部顶点
	/// @param[out] pOut      输出的缓冲区指针
	virtual fResult LockVertexData(fuInt StartVert, fuInt VertCount, void** pOut)=0;

	/// @brief 解锁顶点缓冲区
	virtual fResult UnlockVertexData()=0;

	/// @brief      锁定索引数据
	/// @param[in]  StartIndex 开始锁定的索引
	/// @param[in]  IndexCount 要锁定的索引数量，0表示全部索引
	/// @param[out] pOut       输出的缓冲区指针
	virtual fResult LockIndexData(fuInt StartIndex, fuInt IndexCount, void** pOut)=0;
	
	/// @brief 解锁索引缓冲区
	virtual fResult UnlockIndexData()=0;

	/// @brief      返回子集
	/// @param[in]  ID   子集ID
	/// @param[out] pOut 输出的子集数据
	virtual fResult GetSubset(fInt ID, f2dMeshSubsetInfo* pOut)=0;

	/// @brief     设置子集
	/// @param[in] ID             子集ID
	/// @param[in] Type           图元类型
	/// @param[in] StartIndex     开始索引
	/// @param[in] PrimitiveCount 图元数量
	virtual fResult SetSubset(fInt ID, F2DPRIMITIVETYPE Type, fuInt StartIndex, fuInt PrimitiveCount)=0;

	/// @brief 返回子集个数
	virtual fuInt GetSubsetCount()=0;

	/// @brief     渲染子集
	/// @param[in] ID 子集ID
	virtual fResult Render(fInt ID)=0;
};

// ============================= 渲染设备 ===================================

////////////////////////////////////////////////////////////////////////////////
/// @brief 渲染设备抗锯齿等级
////////////////////////////////////////////////////////////////////////////////
enum F2DAALEVEL
{
	F2DAALEVEL_NONE = 0,     ///< @brief 关闭抗锯齿
	F2DAALEVEL_2    = 1,     ///< @brief 2x抗锯齿
	F2DAALEVEL_4    = 2,     ///< @brief 4x抗锯齿
	F2DAALEVEL_8    = 3,     ///< @brief 8x抗锯齿
	F2DAALEVEL_16   = 4      ///< @brief 16x抗锯齿
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 渲染设备事件监听器
/// @note  当设备丢失或者重置时引发该事件
////////////////////////////////////////////////////////////////////////////////
struct f2dRenderDeviceEventListener
{
	/// @brief 设备丢失事件
	virtual void OnRenderDeviceLost(){}

	/// @brief 设备重置事件
	virtual void OnRenderDeviceReset(){}
};

////////////////////////////////////////////////////////////////////////////////
/// @brief 渲染设备
/// @note  负责渲染对象的创建和设备管理。
////////////////////////////////////////////////////////////////////////////////
struct f2dRenderDevice
{
	// --- 渲染器属性 ---
	/// @brief   返回D3D设备指针
	/// @warning 高级方法。如无特殊需要，应当避免使用该方法
	/// @return  返回IDirect3DDevice9接口指针
	virtual void* GetHandle()=0;
	
	/// @brief 返回显卡名称
	virtual fcStr GetDeviceName()=0;

	/// @brief 返回缓冲区宽度
	virtual fuInt GetBufferWidth()=0;

	/// @brief 返回缓冲区高度
	virtual fuInt GetBufferHeight()=0;

	/// @brief 是否窗口化
	virtual fBool IsWindowed()=0;

	/// @brief 获得抗锯齿等级
	virtual F2DAALEVEL GetAALevel()=0;

	/// @brief 检查抗锯齿是否被支持
	/// @param[in] AALevel  要检测的抗锯齿等级
	/// @param[in] Windowed 是否窗口模式
	virtual fBool CheckMultiSample(F2DAALEVEL AALevel, fBool Windowed)=0;

	/// @brief 返回支持的全屏分辨率数量
	virtual fuInt GetSupportResolutionCount()=0;

	/// @brief 枚举支持的分辨率
	virtual fcyVec2 EnumSupportResolution(fuInt Index)=0;

	/// @brief     重设缓冲区
	/// @note      该操作过后需要手动设置Graphics的矩阵和Viewport大小
	/// @param[in] Width    缓冲区宽度
	/// @param[in] Height   缓冲区高度
	/// @param[in] Windowed 窗口化
	/// @param[in] VSync    垂直同步
	/// @param[in] AALevel  抗锯齿等级
	virtual fResult SetBufferSize(fuInt Width, fuInt Height, fBool Windowed, fBool VSync, F2DAALEVEL AALevel)=0;

	// --- 事件监听器 ---
	/// @brief     挂接一个消息监听器
	/// @param[in] Listener 要绑定的监听器
	/// @param[in] Priority 监听器优先级，越大的值越晚调用
	virtual fResult AttachListener(f2dRenderDeviceEventListener* Listener, fInt Priority=0)=0;
	
	/// @brief     移除一个消息监听器
	/// @param[in] Listener 要移除的监听器
	virtual fResult RemoveListener(f2dRenderDeviceEventListener* Listener)=0;

	// --- 资源创建 ---
	/// @brief      从数据流创建纹理
	/// @note       可以选择性创建动态/静态纹理
	/// @param[in]  pStream   数据流，将读取整个数据流
	/// @param[in]  Width     宽度，设为0将使用数据流中图像默认宽度
	/// @param[in]  Height    高度，设为0将使用数据流中图像默认高度
	/// @param[in]  IsDynamic 是否为动态纹理
	/// @param[in]  HasMipmap 创建Mipmap链，用于加快图像渲染，对动态纹理和渲染目标无效。推荐设为true
	/// @param[out] pOut      输出的纹理指针
	virtual fResult CreateTextureFromStream(f2dStream* pStream, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut)=0;
	
	/// @brief      从内存区域创建纹理
	/// @note       可以选择性创建动态/静态纹理
	/// @param[in]  pMemory   内存区域
	/// @param[in]  Size      内存区域大小
	/// @param[in]  Width     宽度，设为0将使用数据流中图像默认宽度
	/// @param[in]  Height    高度，设为0将使用数据流中图像默认高度
	/// @param[in]  IsDynamic 是否为动态纹理
	/// @param[in]  HasMipmap 创建Mipmap链，用于加快图像渲染，对动态纹理和渲染目标无效。推荐设为true
	/// @param[out] pOut      输出的纹理指针
	virtual fResult CreateTextureFromMemory(fcData pMemory, fLen Size, fuInt Width, fuInt Height, fBool IsDynamic, fBool HasMipmap, f2dTexture2D** pOut) = 0;

	/// @brief      创建一个动态纹理
	/// @note       动态纹理之中的数据会在设备丢失之后消失。
	/// @param[in]  Width  宽度
	/// @param[in]  Height 高度
	/// @param[out] pOut   输出的纹理指针
	virtual fResult CreateDynamicTexture(fuInt Width, fuInt Height, f2dTexture2D** pOut)=0;

	/// @brief      创建一个渲染目标
	/// @note       渲染目标用于存放渲染数据，不可锁定
	/// @warning    渲染目标大小应和深度模板缓冲区大小一致，否则将会造成不可预料的结果。
	/// @param[in]  Width      宽度
	/// @param[in]  Height     高度
	/// @param[in]  AutoResize 重置时自动设为屏幕分辨率
	/// @param[out] pOut       输出的纹理指针
	virtual fResult CreateRenderTarget(fuInt Width, fuInt Height, fBool AutoResize, f2dTexture2D** pOut)=0;

	/// @brief      创建一个深度模板缓冲区
	/// @warning    渲染目标大小应和深度模板缓冲区大小一致，否则将会造成不可预料的结果。
	/// @param[in]  Width      宽度
	/// @param[in]  Height     高度
	/// @param[in]  Discard    不保留数据
	/// @param[in]  AutoResize 重置时自动设为屏幕分辨率
	/// @param[out] pOut       输出的缓冲区指针
	virtual fResult CreateDepthStencilSurface(fuInt Width, fuInt Height, fBool Discard, fBool AutoResize, f2dDepthStencilSurface** pOut)=0;

	/// @brief      创建一个2D渲染器
	/// @param[in]  VertexBufferSize 内部顶点缓冲区大小，置为0则默认
	/// @param[in]  IndexBufferSize  内部索引缓冲区大小，置为0则默认
	/// @param[out] pOut             输出指针
	virtual fResult CreateGraphics2D(fuInt VertexBufferSize, fuInt IndexBufferSize, f2dGraphics2D** pOut)=0;

	/// @brief      创建一个3D渲染器（基于fx的）
	/// @param[in]  pDefaultEffect 默认效果，可空
	/// @param[out] pOut           输出的渲染器指针
	virtual fResult CreateGraphics3D(f2dEffect* pDefaultEffect, f2dGraphics3D** pOut)=0;

	/// @brief      创建一个效果
	/// @param[in]  pStream    效果代码(fx文件)，将读取整个流
	/// @param[in]  bAutoState 是否自动设置状态，当被Graphics3D激活时将会自动设置所有关联的渲染参数。
	/// @param[out] pOut       输出的效果
	/// @param[out] pErrOut    错误信息，可空，仅当创建失败时设置
	virtual fResult CreateEffect(f2dStream* pStream, fBool bAutoState, f2dEffect** pOut)=0;

	/// @brief      创建一个网格数据
	/// @param[in]  pVertElement 顶点元素数组
	/// @param[in]  ElementCount 顶点元素个数
	/// @param[in]  VertCount    顶点数量
	/// @param[in]  IndexCount   索引数量
	/// @param[in]  Int32Index   是否使用Int作为索引单位，否则使用Short
	/// @param[out] pOut         输出的模型数据对象
	virtual fResult CreateMeshData(f2dVertexElement* pVertElement, fuInt ElementCount, fuInt VertCount, fuInt IndexCount, fBool Int32Index, f2dMeshData** pOut)=0;

	// --- 绘图状态 ---
	/// @brief     清空Z缓冲和渲染目标
	/// @param[in] BackBufferColor 背景颜色
	/// @param[in] ZValue          Z深度值，无特殊情况应取1.0f
	virtual fResult Clear(const fcyColor& BackBufferColor = 0, fFloat ZValue = 1.0f)=0;

	/// @brief     清空Z缓冲、渲染目标、模板缓冲区
	/// @param[in] BackBufferColor 背景颜色
	/// @param[in] ZValue          Z深度值，无特殊情况应取1.0f
	/// @param[in] StencilValue    模板缓冲区值
	virtual fResult Clear(const fcyColor& BackBufferColor, fFloat ZValue, fuInt StencilValue)=0;

	/// @brief     清空渲染目标
	/// @param[in] BackBufferColor 背景颜色
	virtual fResult ClearColor(const fcyColor& BackBufferColor = 0)=0;

	/// @brief     清空Z缓冲
	/// @param[in] Value Z深度值，无特殊情况应取1.0f
	virtual fResult ClearZBuffer(fFloat Value=1.0f)=0;

	/// @brief     清空模板缓冲
	/// @param[in] StencilValue 模板缓冲值
	virtual fResult ClearStencilBuffer(fuInt StencilValue=0)=0;

	/// @brief   返回目前使用中的渲染目标
	/// @warning 渲染目标会在一轮渲染结束后恢复到默认后台缓冲区
	/// @note    如果为默认后台缓冲区将返回NULL；该函数不增加引用计数
	virtual f2dTexture2D* GetRenderTarget()=0;

	/// @brief 设置渲染目标
	/// @note  若需设置为默认后台缓冲区，请置为NULL
	virtual fResult SetRenderTarget(f2dTexture2D* pTex)=0;

	/// @brief   返回目前使用中的深度模板缓冲区
	/// @warning 深度模板缓冲区会在一轮渲染结束后恢复到默认后台缓冲区
	/// @note    若为默认后台缓冲区返回NULL；该函数不增加引用计数
	virtual f2dDepthStencilSurface* GetDepthStencilSurface()=0;

	/// @brief 设置深度模板缓冲区
	/// @note  若需设置为默认模板缓冲区，请置为NULL
	virtual fResult SetDepthStencilSurface(f2dDepthStencilSurface* pSurface)=0;

	/// @brief 返回目前的裁剪矩形
	virtual fcyRect GetScissorRect()=0;

	/// @brief   设置裁剪矩形
	/// @warning 矩形应当小于RenderTarget大小
	/// @note    重新设置缓冲区大小时该值会被覆盖
	virtual fResult SetScissorRect(const fcyRect& pRect)=0;

	/// @brief 返回目前的Viewport
	virtual fcyRect GetViewport()=0;

	/// @brief     设置目前的Viewport
	/// @param[in] vp 给定的视区
	virtual fResult SetViewport(fcyRect vp)=0;

	/// @brief 返回是否开启Z-buffer测试
	virtual fBool IsZBufferEnabled()=0;

	/// @brief 设置是否开启Z-buffer测试
	virtual fResult SetZBufferEnable(fBool v)=0;

	// --- 高级 ---
	/// @brief     截屏
	/// @note      以PNG形式保存
	/// @param[in] pStream 输出的流，从流的当前位置开始写入
	virtual fResult SaveScreen(f2dStream* pStream)=0;

	/// @brief     保存纹理
	/// @note      以PNG形式保存
	/// @param[in] pStream 输出的流，从流的当前位置开始写入
	/// @param[in] pTex    要保存的纹理
	virtual fResult SaveTexture(f2dStream* pStream, f2dTexture2D* pTex)=0;

	/// @brief     调用UpdateLayeredWindow更新分层窗口
	/// @note      要求主窗口被设置为分层窗口
	/// @param[in] KeyColor 关键色
	/// @param[in] Alpha    透明度
	virtual fResult UpdateScreenToWindow(fcyColor KeyColor, fByte Alpha)=0;

	/// @brief     设置纹理采样时地址取值方式
	/// @param[in] address     取值方式
	/// @param[in] borderColor 界外颜色
	virtual fResult SetTextureAddress(F2DTEXTUREADDRESS address, const fcyColor& borderColor) = 0;

	/// @brief     纹理采样器
	/// @param[in] filter 采样器类型
	virtual fResult SetTextureFilter(F2DTEXFILTERTYPE filter) = 0;
};

/// @}
