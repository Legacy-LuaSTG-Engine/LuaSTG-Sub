////////////////////////////////////////////////////////////////////////////////
/// @file   f2dRenderer.h
/// @brief  fancy2D引擎渲染器接口定义
/// @note   定义了f2D的渲染器接口
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "f2dInterface.h"

#include "f2dRenderDevice.h"

/// @addtogroup f2d渲染系统
/// @brief fancy2d引擎中的渲染系统，提供对2D图形的渲染支持。
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 精灵翻转选项
////////////////////////////////////////////////////////////////////////////////
enum F2DSPRITEFLIP
{
	F2DSPRITEFLIP_NONE,  ///< @brief 无翻转
	F2DSPRITEFLIP_H,     ///< @brief 水平翻转
	F2DSPRITEFLIP_V,     ///< @brief 垂直翻转
	F2DSPRITEFLIP_HV     ///< @brief 水平垂直翻转
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 精灵
////////////////////////////////////////////////////////////////////////////////
struct f2dSprite : 
	public f2dInterface
{
	/// @brief 获得精灵绑定的纹理
	virtual f2dTexture2D* GetTexture()=0;

	/// @brief     设置一个新的纹理
	/// @param[in] pTex 纹理
	virtual fResult SetTexture(f2dTexture2D* pTex)=0;

	/// @brief 返回对应纹理的矩形
	virtual const fcyRect& GetTexRect()const=0;

	/// @brief 设置对应纹理的矩形
	virtual fResult SetTexRect(const fcyRect& Rect)=0;

	/// @brief 返回热点
	virtual const fcyVec2& GetHotSpot()const=0;

	/// @brief     设置热点
	/// @param[in] Point 热点
	virtual fResult SetHotSpot(const fcyVec2& Point)=0;

	/// @brief 返回翻转方式
	virtual F2DSPRITEFLIP GetFlipType()const=0;
	
	/// @brief     设置翻转方式
	/// @param[in] Type 翻转方式
	virtual fResult SetFlipType(F2DSPRITEFLIP Type)=0;

	/// @brief 返回Z值
	virtual fFloat GetZ()const=0;

	/// @brief     设置Z值
	/// @param[in] Value 新的Z值
	virtual fResult SetZ(fFloat Value)=0;

	/// @brief     返回顶点的颜色
	/// @param[in] Index 顶点索引[0~3]，按照顺时针方向定位
	virtual fcyColor GetColor(fuInt Index)const=0;

	/// @brief     返回顶点的颜色
	/// @param[in] pOut 大小为4的数组
	virtual void GetColor(fcyColor* pOut)const=0;

	/// @brief     设置所有顶点的颜色
	/// @param[in] Color 混合颜色
	virtual fResult SetColor(fcyColor Color)=0;

	/// @brief 设置顶点颜色
	/// @param[in] Index 顶点索引[0~3]，按照顺时针方向定位
	/// @param[in] Color 混合颜色
	virtual fResult SetColor(fuInt Index, fcyColor Color)=0;

	/// @brief     设置所有顶点的颜色
	/// @param[in] pArr 大小为4的数组
	virtual fResult SetColor(fcyColor* pArr)=0;

	/// @brief 绘制精灵
	/// @param[in] pGraph 绘图对象指针
	/// @param[in] Dest   目标矩形
	virtual fResult Draw(f2dGraphics2D* pGraph, const fcyRect& Dest, fBool bAutoFixCoord = true) = 0;

	/// @brief 绘制精灵
	/// @param[in] pGraph 绘图对象指针
	/// @param[in] Dest   目标矩形
	/// @param[in] SubTex 子纹理区域[0~1, 0~1]
	virtual fResult Draw(f2dGraphics2D* pGraph, const fcyRect& Dest, const fcyRect& SubTex)=0;

	/// @brief 绘制精灵
	/// @param[in] pGraph 绘图对象指针
	/// @param[in] v1     坐标1
	/// @param[in] v2     坐标2
	/// @param[in] v3     坐标3
	/// @param[in] v4     坐标4
	virtual fResult Draw(f2dGraphics2D* pGraph, fcyVec3 v1, fcyVec3 v2, fcyVec3 v3, fcyVec3 v4, fBool bAutoFixCoord = true) = 0;

	/// @brief 绘制精灵
	/// @param[in] pGraph 绘图对象指针
	/// @param[in] Center 精灵显示中心
	virtual fResult Draw(f2dGraphics2D* pGraph, const fcyVec2& Center)=0;

	/// @brief 绘制精灵
	/// @param[in] pGraph 绘图对象指针
	/// @param[in] Center 精灵显示中心
	/// @param[in] Scale  缩放
	virtual fResult Draw(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale)=0;

	/// @brief 绘制精灵
	/// @param[in] pGraph 绘图对象指针
	/// @param[in] Center 精灵显示中心
	/// @param[in] Scale  缩放
	/// @param[in] SubTex 子纹理区域[0~1, 0~1]
	virtual fResult Draw(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, const fcyRect& SubTex)=0;

	/// @brief 绘制精灵
	/// @param[in] pGraph   绘图对象指针
	/// @param[in] Center   精灵显示中心
	/// @param[in] Scale    缩放
	/// @param[in] Rotation 顺时针旋转角度，弧度制。
	virtual fResult Draw(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation, fBool bAutoFixCoord = true) = 0;
	
	/// @brief 绘制精灵
	/// @param[in] pGraph   绘图对象指针
	/// @param[in] Center   精灵显示中心
	/// @param[in] Scale    缩放
	/// @param[in] Rotation 顺时针旋转角度，弧度制。
	/// @param[in] SubTex   子纹理区域[0~1, 0~1]
	virtual fResult Draw(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation, const fcyRect& SubTex)=0;

	/// @brief 绘制精灵
	/// @note  该方法适应y轴翻转的情况
	/// @param[in] pGraph 绘图对象指针
	/// @param[in] Center 精灵显示中心
	virtual fResult Draw2(f2dGraphics2D* pGraph, const fcyVec2& Center)=0;

	/// @brief 绘制精灵
	/// @note  该方法适应y轴翻转的情况
	/// @param[in] pGraph 绘图对象指针
	/// @param[in] Center 精灵显示中心
	/// @param[in] Scale  缩放
	virtual fResult Draw2(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale)=0;

	/// @brief 绘制精灵
	/// @note  该方法适应y轴翻转的情况
	/// @param[in] pGraph 绘图对象指针
	/// @param[in] Center 精灵显示中心
	/// @param[in] Scale  缩放
	/// @param[in] SubTex 子纹理区域[0~1, 0~1]
	virtual fResult Draw2(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, const fcyRect& SubTex)=0;

	/// @brief 绘制精灵
	/// @note  该方法适应y轴翻转的情况
	/// @param[in] pGraph   绘图对象指针
	/// @param[in] Center   精灵显示中心
	/// @param[in] Scale    缩放
	/// @param[in] Rotation 顺时针旋转角度，弧度制。
	virtual fResult Draw2(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation, fBool bAutoFixCoord = true) = 0;

	/// @brief 绘制精灵
	/// @note  该方法适应y轴翻转的情况
	/// @param[in] pGraph   绘图对象指针
	/// @param[in] Center   精灵显示中心
	/// @param[in] Scale    缩放
	/// @param[in] Rotation 顺时针旋转角度，弧度制。
	/// @param[in] SubTex   子纹理区域[0~1, 0~1]
	virtual fResult Draw2(f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation, const fcyRect& SubTex)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D简单几何体渲染器
////////////////////////////////////////////////////////////////////////////////
struct f2dGeometryRenderer : 
	public f2dInterface
{
	/// @brief 获得Z值
	virtual fFloat GetZ()=0;

	/// @brief     设置Z值
	/// @param[in] ZValue Z值
	virtual void SetZ(fFloat ZValue)=0;

	/// @brief     获得画笔的颜色
	/// @param[in] Index 索引[0~2]，0=外侧颜色，1=中间颜色，2=内侧颜色
	virtual fcyColor GetPenColor(fuInt Index)=0;

	/// @brief     设置画笔颜色
	/// @param[in] Index 索引[0~2]，0=外侧颜色，1=中间颜色，2=内侧颜色
	/// @param[in] Color 颜色值
	virtual void SetPenColor(fuInt Index, const fcyColor& Color)=0;

	/// @brief 获得画笔大小
	virtual fFloat GetPenSize()=0;

	/// @brief     设置画笔大小
	/// @param[in] Size 画笔大小
	virtual void SetPenSize(fFloat Size)=0;

	/// @brief     画线
	/// @param[in] pGraph 绘图指针
	/// @param[in] Begin  起点
	/// @param[in] End    终点
	virtual fResult DrawLine(f2dGraphics2D* pGraph, const fcyVec2& Begin, const fcyVec2& End)=0;

	/// @brief     绘制矩形
	/// @param[in] pGraph 绘图指针
	/// @param[in] Rect   矩形
	virtual fResult DrawRectangle(f2dGraphics2D* pGraph, const fcyRect& Rect)=0;

	/// @brief     绘制圆
	/// @param[in] pGraph 绘图指针
	/// @param[in] Center 中心
	/// @param[in] R      半径
	/// @param[in] Divide 细分
	virtual fResult DrawCircle(f2dGraphics2D* pGraph, const fcyVec2& Center, float R, fuInt Divide=50)=0;

	/// @brief     填充矩形
	/// @param[in] pGraph    绘图指针
	/// @param[in] Rect      填充区域
	/// @param[in] pColorArr 颜色数组[0~3]，顺时针四个角的颜色
	virtual fResult FillRectangle(f2dGraphics2D* pGraph, const fcyRect& Rect, const fcyColor* pColorArr)=0;

	/// @brief     填充圆
	/// @param[in] pGraph      绘图指针
	/// @param[in] Center      中心
	/// @param[in] R           半径
	/// @param[in] CenterColor 中心颜色
	/// @param[in] BoundColor  边界颜色
	/// @param[in] Divide      细分
	virtual fResult FillCircle(f2dGraphics2D* pGraph, const fcyVec2& Center, float R, const fcyColor& CenterColor, const fcyColor& BoundColor, fuInt Divide=50)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 字形数据
////////////////////////////////////////////////////////////////////////////////
struct f2dGlyphInfo
{
	fcyRect GlyphPos;    ///< @brief 字形在纹理上的uv坐标
	fcyVec2 GlyphSize;   ///< @brief 字形大小
	fcyVec2 BrushPos;    ///< @brief 笔触距离字形左上角坐标
	fcyVec2 Advance;     ///< @brief 笔触的前进量(像素)
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 字体提供器
////////////////////////////////////////////////////////////////////////////////
struct f2dFontProvider :
	public f2dInterface
{
	/// @brief 获得默认行高
	virtual fFloat GetLineHeight()=0;

	/// @brief 获得基线到轮廓线的距离
	virtual fFloat GetAscender()=0;

	/// @brief 与Ascender相反
	virtual fFloat GetDescender()=0;

	/// @brief 返回缓冲纹理
	virtual f2dTexture2D* GetCacheTexture()=0;

	/// @breif     缓存字符串
	/// @note      针对动态产生文字的特定方法
	/// @param[in] String 字符串
	virtual fResult CacheString(fcStrW String)=0;

	/// @brief      请求一个字形
	/// @param[in]  pGraph    正在使用的渲染器，空表示只查询字形数据
	/// @param[in]  Character 请求的字符
	/// @param[out] InfoOut   返回的字形数据
	virtual fResult QueryGlyph(f2dGraphics* pGraph, fCharW Character, f2dGlyphInfo* InfoOut)=0;

	/// @brief      获取可用的字形缓存数量
	virtual fInt GetCacheCount() = 0;

	/// @brief      获取字形缓存贴图大小
	virtual fInt GetCacheTexSize() = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 字体选项
////////////////////////////////////////////////////////////////////////////////
enum F2DFONTFLAG
{
	F2DFONTFLAG_NONE,    ///< @brief 无选项
	F2DFONTFLAG_OUTLINE  ///< @brief [尚未实现] 返回描边
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 字体绘制监听器
////////////////////////////////////////////////////////////////////////////////
struct f2dFontRendererListener
{
	/// @brief          正准备绘制字形
	/// @param[in]      Index     该字符在文本串中的索引
	/// @param[in]      Character 字符
	/// @param[in][out] DrawPos   起始绘制位置
	/// @param[in][out] Adv       前进量
	/// @return         字形是否被绘制，注意无论是否绘制，前进量仍起作用。
	virtual fBool OnGlyphBeginDraw(fuInt Index, fCharW Character, fcyVec2& DrawPos, fcyVec2& Adv) { return true; }

	/// @brief          绘制字形
	/// @param[in][out] pVerts 顶点数组，大小为4
	virtual void OnGlyphCalcuCoord(f2dGraphics2DVertex pVerts[]) { }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 字体渲染器
////////////////////////////////////////////////////////////////////////////////
struct f2dFontRenderer :
	public f2dInterface
{
	/// @brief 返回监听器
	virtual f2dFontRendererListener* GetListener()=0;
	
	/// @brief 设置监听器
	virtual void SetListener(f2dFontRendererListener* pListener)=0;

	/// @brief 返回字形提供者
	virtual f2dFontProvider* GetFontProvider()=0;

	/// @brief     设置字形提供者
	/// @param[in] pProvider 字形提供者，非空
	virtual fResult SetFontProvider(f2dFontProvider* pProvider)=0;

	/// @brief 获得Z值
	virtual fFloat GetZ()=0;

	/// @brief     设置Z值
	/// @param[in] ZValue Z值
	virtual void SetZ(fFloat ZValue)=0;

	/// @brief     获得顶点颜色
	/// @param[in] Index 索引[0~3]
	virtual fcyColor GetColor(fuInt Index)=0;

	/// @brief     设置字体颜色
	/// @param[in] Color 字体颜色
	virtual void SetColor(const fcyColor& Color)=0;

	/// @brief     设置顶点颜色
	/// @param[in] Index 索引[0~3]
	/// @param[in] Color 颜色
	virtual void SetColor(fuInt Index, const fcyColor& Color)=0;

	/// @brief 获得翻转方式
	virtual F2DSPRITEFLIP GetFlipType()=0;

	/// @brief     设置翻转方式
	/// @param[in] Type 翻转方式
	virtual void SetFlipType(F2DSPRITEFLIP Type)=0;

	/// @brief 获取字形缩放
	virtual fcyVec2 GetScale()=0;

	/// @brief 设置字形缩放
	/// @note  该方法将影响MeasureString方法
	virtual void SetScale(fcyVec2 Scale)=0;

	/// @brief     测量一个字符串最终绘制的大小
	/// @param[in] String 字符串
	/// @param[in] bStrictWidth 严格宽度
	virtual fcyRect MeasureString(fcStrW String, bool bStrictWidth=true)=0;

	/// @brief     测量一个字符串最终绘制的宽度
	/// @param[in] String 字符串
	virtual fFloat MeasureStringWidth(fcStrW String)=0;

	/// @brief     绘制文字
	/// @param[in] pGraph   渲染器
	/// @param[in] Text     文字
	/// @param[in] StartPos 绘制起始基准
	virtual fResult DrawTextW(f2dGraphics2D* pGraph, fcStrW Text, const fcyVec2& StartPos)=0;

	/// @brief     绘制文字
	/// @param[in]  pGraph   渲染器
	/// @param[in]  Text     文字
	/// @param[in]  Count    字数，设为-1则为全部文字
	/// @param[in]  StartPos 绘制起始基准
	/// @param[out] PosOut   绘制终止位置，可空
	virtual fResult DrawTextW(f2dGraphics2D* pGraph, fcStrW Text, fuInt Count, const fcyVec2& StartPos, fcyVec2* PosOut)=0;

	/// @brief      绘制文字
	/// @param[in]  pGraph   渲染器
	/// @param[in]  Text     文字
	/// @param[in]  Count    字数，设为-1则为全部文字
	/// @param[in]  Bias     倾斜角，0为不倾斜
	/// @param[in]  StartPos 绘制起始基准
	/// @param[out] PosOut   绘制终止位置，可空
	virtual fResult DrawTextW(f2dGraphics2D* pGraph, fcStrW Text, fuInt Count, fFloat Bias, const fcyVec2& StartPos, fcyVec2* PosOut) = 0;

	/// @brief     绘制文字
	/// @brief      该方法适应y轴向上的情况
	/// @param[in] pGraph   渲染器
	/// @param[in] Text     文字
	/// @param[in] StartPos 绘制起始基准
	virtual fResult DrawTextW2(f2dGraphics2D* pGraph, fcStrW Text, const fcyVec2& StartPos) = 0;

	/// @brief     绘制文字
	/// @brief      该方法适应y轴向上的情况
	/// @param[in]  pGraph   渲染器
	/// @param[in]  Text     文字
	/// @param[in]  Count    字数，设为-1则为全部文字
	/// @param[in]  StartPos 绘制起始基准
	/// @param[out] PosOut   绘制终止位置，可空
	virtual fResult DrawTextW2(f2dGraphics2D* pGraph, fcStrW Text, fuInt Count, const fcyVec2& StartPos, fcyVec2* PosOut) = 0;

	/// @brief      绘制文字
	/// @brief      该方法适应y轴向上的情况
	/// @param[in]  pGraph   渲染器
	/// @param[in]  Text     文字
	/// @param[in]  Count    字数，设为-1则为全部文字
	/// @param[in]  Bias     倾斜角，0为不倾斜
	/// @param[in]  StartPos 绘制起始基准
	/// @param[out] PosOut   绘制终止位置，可空
	virtual fResult DrawTextW2(f2dGraphics2D* pGraph, fcStrW Text, fuInt Count, fFloat Bias, const fcyVec2& StartPos, fcyVec2* PosOut) = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D精灵动画实例
////////////////////////////////////////////////////////////////////////////////
struct f2dSpriteAnimationInstance
{
	fuInt KeyFrameIndex;
	fuInt KeyFrameElapsedTime;
	F2DSPRITEFLIP FlipType;
	fcyColor BlendColor;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D精灵动画
////////////////////////////////////////////////////////////////////////////////
struct f2dSpriteAnimation : 
	public f2dInterface
{
	/// @brief 返回帧对应的精灵
	virtual f2dSprite* GetSprite(fuInt Index)=0;
	/// @brief 返回帧对应的时间
	virtual fInt GetKeyFrameTime(fuInt Index)const=0;
	/// @brief 设置帧对应的精灵
	virtual fResult SetSprite(fuInt Index, f2dSprite* pSprite)=0;
	/// @brief 设置帧对应的时间
	virtual fResult SetKeyFrameTime(fuInt Index, fuInt Time)=0;

	/// @brief 计算总帧数
	virtual fuInt GetFrameCount()const=0;
	/// @brief 返回关键帧个数
	virtual fuInt GetKeyFrameCount()const=0;

	/// @brief 追加关键帧
	virtual fuInt Append(fuInt FrameTime, f2dTexture2D* FrameTex)=0;
	/// @brief 追加关键帧
	virtual fuInt Append(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect)=0;
	/// @brief 追加关键帧
	virtual fuInt Append(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect, const fcyVec2& HotPos)=0;
	/// @brief 分割并加入
	virtual void SplitAndAppend(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount)=0;
	/// @brief 分割并加入
	virtual void SplitAndAppend(fuInt* FrameTimeArr, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount)=0;
	/// @brief 逆序分割并加入
	virtual void ReverseSplitAndAppend(fuInt FrameTime, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount)=0;
	/// @brief 逆序分割并加入
	virtual void ReverseSplitAndAppend(fuInt* FrameTimeArr, f2dTexture2D* FrameTex, const fcyRect& TexRect, fuInt SplitCount)=0;
	/// @brief 清空
	virtual void Clear()=0;

	/// @brief 返回循环起点
	virtual fInt GetLoopStart()const=0;
	/// @brief 设置循环起点
	virtual void SetLoopStart(fInt Index)=0;
	/// @brief 返回循环终点
	virtual fInt GetLoopEnd()const=0;
	/// @brief 设置循环终点
	virtual void SetLoopEnd(fInt Index)=0;
	/// @brief 是否循环
	virtual fBool IsLoop()const=0;
	/// @brief 设置循环
	virtual void SetLoop(fBool bLoop)=0;

	/// @brief     初始化实例
	/// @param[in] Instance 实例对象
	virtual void InitInstance(f2dSpriteAnimationInstance& Instance, bool bResetBlendColor = true, bool bResetFlipType = true)const=0;
	/// @brief     初始化实例到动画结尾
	/// @param[in] Instance 实例对象
	virtual void InitInstanceToEnd(f2dSpriteAnimationInstance& Instance, bool bResetBlendColor = true, bool bResetFlipType = true)const=0;
	/// @brief  单步实例
	/// @return 若动画结束返回false
	virtual fBool StepInstance(f2dSpriteAnimationInstance& Instance)const=0;
	/// @brief  后退一步
	/// @return 若动画结束返回false
	virtual fBool StepbackInstance(f2dSpriteAnimationInstance& Instance)const=0;
	/// @brief  跳到帧
	virtual fResult JumpTo(f2dSpriteAnimationInstance& Instance, fuInt FrameIndex)const=0;
	/// @brief  跳到关键帧
	virtual fResult JumpToKeyFrame(f2dSpriteAnimationInstance& Instance, fuInt FrameIndex)const=0;

	/// @brief 绘制精灵动画
	/// @param[in] Instance 动画实例
	/// @param[in] pGraph   绘图对象指针
	/// @param[in] Dest     目标矩形
	virtual fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyRect& Dest)=0;
	/// @brief     绘制精灵动画
	/// @param[in] Instance 动画实例
	/// @param[in] pGraph   绘图对象指针
	/// @param[in] Dest     目标矩形
	/// @param[in] SubTex   子纹理区域[0~1, 0~1]
	virtual fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyRect& Dest, const fcyRect& SubTex)=0;
	/// @brief     绘制精灵动画
	/// @param[in] Instance 动画实例
	/// @param[in] pGraph   绘图对象指针
	/// @param[in] Center   精灵显示中心
	virtual fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center)=0;
	/// @brief     绘制精灵动画
	/// @param[in] Instance 动画实例
	/// @param[in] pGraph   绘图对象指针
	/// @param[in] Center   精灵显示中心
	/// @param[in] Scale    缩放
	virtual fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale)=0;
	/// @brief     绘制精灵动画
	/// @param[in] Instance 动画实例
	/// @param[in] pGraph   绘图对象指针
	/// @param[in] Center   精灵显示中心
	/// @param[in] Scale    缩放
	/// @param[in] SubTex   子纹理区域[0~1, 0~1]
	virtual fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, const fcyRect& SubTex)=0;
	/// @brief     绘制精灵
	/// @param[in] Instance 动画实例
	/// @param[in] pGraph   绘图对象指针
	/// @param[in] Center   精灵显示中心
	/// @param[in] Scale    缩放
	/// @param[in] Rotation 顺时针旋转角度，弧度制。
	virtual fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation)=0;
	/// @brief     绘制精灵
	/// @param[in] Instance 动画实例
	/// @param[in] pGraph   绘图对象指针
	/// @param[in] Center   精灵显示中心
	/// @param[in] Scale    缩放
	/// @param[in] Rotation 顺时针旋转角度，弧度制。
	/// @param[in] SubTex   子纹理区域[0~1, 0~1]
	virtual fResult DrawInstance(const f2dSpriteAnimationInstance& Instance, f2dGraphics2D* pGraph, const fcyVec2& Center, const fcyVec2& Scale, fFloat Rotation, const fcyRect& SubTex)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 2D粒子池 粒子参数
////////////////////////////////////////////////////////////////////////////////
struct f2dParticleCreationDesc
{
	fcyRect PosRange;      ///< @brief 创建范围偏移
	fcyVec2 VRange;        ///< @brief 初速度范围
	fcyVec2 VAngleRange;   ///< @brief 初速度方向范围
	fcyVec2 ARRange;       ///< @brief 初法向加速度范围
	fcyVec2 ATRange;       ///< @brief 初切向加速度范围
	fcyVec2 InitialAngle;  ///< @brief 初始化偏转角度范围
	fcyVec2 SpinRange;     ///< @brief 自旋速度范围
	fcyVec2 LifeTimeRange; ///< @brief 生存时间范围

	fcyColor StartColor;      ///< @brief 初始颜色
	fcyVec2  StartColorRange; ///< @brief 初始颜色抖动范围
	                          ///< @note  StartColor+=random(StartColorRange.x, StartColorRange.y)
	fcyColor EndColor;        ///< @brief 终止颜色

	fcyVec2 StartScale;       ///< @brief 初始缩放
	fcyVec2 StartScaleRange;  ///< @brief 初始缩放抖动范围
	                          ///< note   计算同上
	fcyVec2 EndScale;         ///< @brief 终止缩放
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 2D粒子池 力场
////////////////////////////////////////////////////////////////////////////////
struct f2dParticleForce
{
	virtual void OnUpdate(fFloat ElapsedTime, fcyVec2& Pos, fcyVec2& V, float& Angle)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D 2D粒子池
////////////////////////////////////////////////////////////////////////////////
struct f2dParticlePool : 
	public f2dInterface
{
	/// @brief 返回随机数种子
	virtual fuInt GetRandomSeed()=0;
	/// @brief     设置随机数种子
	/// @param[in] Seed 种子
	virtual void SetRandomSeed(fuInt Seed)=0;
	/// @brief 制造随机数
	virtual fFloat RandFloat(fFloat Min, fFloat Max)=0;

	/// @brief 返回粒子数量
	virtual fuInt GetCount()=0;

	/// @brief 返回粒子容量
	virtual fuInt GetCapacity()=0;

	/// @brief 清空粒子
	virtual void Clear()=0;

	/// @brief     追加一个力
	virtual fResult AppendForce(f2dParticleForce* pForce)=0;
	
	/// @brief     移除一个力
	/// @note      若有多个相同的对象将移除第一个对象
	virtual bool RemoveForce(f2dParticleForce* pForce)=0;

	/// @brief     清空力场
	virtual void ClearForce()=0;

	/// @brief     发射一组粒子
	/// @param[in] pSprite           发射的精灵
	/// @param[in] Center            发射中心
	/// @param[in] EmittedCountRange 发射数量范围
	/// @param[in] Desc              粒子详细描述
	virtual fResult Emitted(f2dSprite* pSprite, const fcyVec2& Center, const fcyVec2& EmittedCountRange, const f2dParticleCreationDesc& Desc)=0;

	/// @brief     更新粒子状态
	/// @param[in] ElapsedTime 流逝时间
	virtual void Update(fFloat ElapsedTime)=0;
	/// @brief     渲染粒子
	/// @param[in] pGraph 绘图接口
	virtual void Render(f2dGraphics2D* pGraph)=0;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fancy2D渲染器
////////////////////////////////////////////////////////////////////////////////
struct f2dRenderer
{
	virtual f2dRenderDevice* GetDevice()=0;  ///< @brief 获得渲染设备指针

	// === 对象创建 ===
	/// @brief      创建Sprite2D
	/// @param[in]  pTex 绑定的纹理
	/// @param[out] pOut 创建的Sprite
	virtual fResult CreateSprite2D(f2dTexture2D* pTex, f2dSprite** pOut)=0;

	/// @brief      创建Sprite2D
	/// @param[in]  pTex 绑定的纹理
	/// @param[in]  Org  原始纹理矩形
	/// @param[out] pOut 创建的Sprite
	virtual fResult CreateSprite2D(f2dTexture2D* pTex, const fcyRect& Org, f2dSprite** pOut)=0;

	/// @brief      创建Sprite2D
	/// @param[in]  pTex    绑定的纹理
	/// @param[in]  Org     原始纹理矩形
	/// @param[in]  HotSopt 热点
	/// @param[out] pOut    创建的Sprite
	virtual fResult CreateSprite2D(f2dTexture2D* pTex, const fcyRect& Org, const fcyVec2& HotSopt, f2dSprite** pOut)=0;

	/// @brief      创建几何体渲染器
	/// @param[out] pOut 输出的渲染器对象
	virtual fResult CreateGeometryRenderer(f2dGeometryRenderer** pOut)=0;

	/// @brief      创建文字渲染器
	/// @param[in]  pProvider 字体源
	/// @param[out] pOut      输出的字体对象
	virtual fResult CreateFontRenderer(f2dFontProvider* pProvider, f2dFontRenderer** pOut)=0;

	/// @brief      从字体文件加载字体
	/// @param[in]  pStream   字体文件，读取整个文件
	/// @param[in]  FaceIndex 若有多个Face，可以指定索引。0总是有效值。
	/// @param[in]  FontSize  字体大小
	/// @param[in]  BBoxSize  手动指定包围盒大小，专治SB思源系列字体，为0时忽略该参数
	/// @param[in]  Flag      额外选项
	/// @param[out] pOut      返回的字体对象
	virtual fResult CreateFontFromFile(f2dStream* pStream, fuInt FaceIndex, const fcyVec2& FontSize, const fcyVec2& BBoxSize, F2DFONTFLAG Flag, f2dFontProvider** pOut)=0;

	/// @brief      从系统加载字体
	/// @param[in]  FaceName  字体在注册表中的名称
	/// @param[in]  FaceIndex 若有多个Face，可以指定索引。0总是有效值。
	/// @param[in]  FontSize  字体大小
	/// @param[in]  Flag      额外选项
	/// @param[out] pOut      返回的字体对象
	virtual fResult CreateSystemFont(fcStrW FaceName, fuInt FaceIndex, const fcyVec2& FontSize, F2DFONTFLAG Flag, f2dFontProvider** pOut)=0;

	/// @brief      从纹理加载字体
	/// @param[in]  pDefineFile 字体定义文件
	/// @param[in]  pTex        纹理
	/// @param[out] pOut        返回的字体对象
	virtual fResult CreateFontFromTex(f2dStream* pDefineFile, f2dTexture2D* pTex, f2dFontProvider** pOut)=0;
	
	/// @brief      从纹理加载字体
	/// @param[in]  pDefineFile 字体定义
	/// @param[in]  pTex        纹理
	/// @param[out] pOut        返回的字体对象
	virtual fResult CreateFontFromTex(fcStrW pDefineText, f2dTexture2D* pTex, f2dFontProvider** pOut)=0;

	/// @brief      创建精灵动画
	/// @param[out] pOut        返回的对象
	virtual fResult CreateSpriteAnimation(f2dSpriteAnimation** pOut)=0;

	/// @brief      创建粒子池
	/// @param[out] pOut        返回的对象
	virtual fResult CreateParticlePool(f2dParticlePool** pOut)=0;
};

/// @}
