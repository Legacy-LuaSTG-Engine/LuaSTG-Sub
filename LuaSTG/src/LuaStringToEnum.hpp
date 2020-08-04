#pragma once
#include "lua.hpp"
#include "ResourceMgr.h"

namespace Xrysnow {
	// GameObject 属性

	enum class GameObjectProperty
	{
		// object
		STATUS = 0,
		CLASS  = 1,
		// user
		TIMER  = 2,
		// position
		X  = 3,
		Y  = 4,
		Z  = 5, //
		DX = 6,
		DY = 7,
		DZ = 8, //
		// movement
		AX,
		AY,
		AZ, //
		AG,
		VX,
		VY,
		VZ, //
		MAXV,
		MAXVX,
		MAXVY,
		MAXVZ, //
		// render
		RES, //
		IMG,
		ANI,
		ROT,
		OMEGA,
		LAYER,
		HIDE,
		NAVI,
		HSCALE,
		VSCALE,
		ZSCALE, //
		_BLEND,
		_COLOR,
		_A,
		_R,
		_G,
		_B,
		// collision
		GROUP,
		BOUND,
		COLLI,
		A,
		B,
		RECT,
		COLLIDER,
		// ex+
		PAUSE,
		RESOLVEMOVE,
		IGNORESUPERPAUSE,
		VANGLE,
		VSPEED,
		WORLD,

		_KEY_NOT_FOUND = -1
	};

	//必须是lua的TString类型，直接以原始字符串作为参数是不能正常工作的
	GameObjectProperty GameObjectPropertyHash(const char* key);

	//index是参数在栈上的索引
	GameObjectProperty GameObjectPropertyHash(lua_State* L, int index);

	//index是参数在栈上的索引，str是返回的字符串
	GameObjectProperty GameObjectPropertyHash(lua_State* L, int index, const char** str);

	// BlendMode 枚举

	//字符串转混合模式枚举
	LuaSTGPlus::BlendMode BlendModeHash(lua_State* L, int index);

	// Color 包装器

	enum class ColorWrapperProperty {
		m_a = 0,
		m_r,
		m_g,
		m_b,
		m_argb,
		m_h,
		m_s,
		m_v,
		f_ARGB,
		f_AHSV,

		_KEY_NOT_FOUND = -1,
	};

	ColorWrapperProperty ColorWrapperPropertyHash(lua_State* L, int index);

	// 初始化方法

	//初始化所有hash表
	void InitStringToEnumHash(lua_State* L);
}
