#include "LuaWrapper/LuaWrapper.hpp"
#include "LuaWrapper/lua_luastg_hash.hpp"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using HSVColor = LuaSTGPlus::LuaWrapper::ColorWrapper::HSVColor;

namespace LuaSTGPlus
{
	namespace LuaWrapper
	{
		HSVColor ColorWrapper::RGB2HSV(const fcyColor& rgb)LNOEXCEPT {
			auto maxrgb = std::max(rgb.r, rgb.g);
			maxrgb = std::max(maxrgb, rgb.b);
			auto minrgb = std::min(rgb.r, rgb.g);
			minrgb = std::min(minrgb, rgb.b);

			float h = 0.0f, s = 0.0f, v = 0.0f;
			v = ((float)maxrgb) / 255.0f;
			int mm = (int)maxrgb - (int)minrgb;
			if (maxrgb == 0) {
				s = 0.0f;
			}
			else {
				s = (float)mm / (float)maxrgb;
			}
			if (mm == 0) {
				h = 0.0f;
			}
			else if (rgb.r == maxrgb) {
				h = (float)(rgb.g - rgb.b) / (float)mm / 60.0f;
			}
			else if (rgb.g == maxrgb) {
				h = 120.0f + (float)(rgb.b - rgb.r) / (float)mm / 60.0f;
			}
			else if (rgb.b == maxrgb) {
				h = 240.0f + (float)(rgb.r - rgb.g) / (float)mm / 60.0f;
			}
			if (h < 0.0f) {
				h += 360.0f;
			}
			s *= 100.0f;
			v *= 100.0f;
			return { h, s, v };
		}
		
		fcyColor ColorWrapper::HSV2RGB(const HSVColor& hsv)LNOEXCEPT {
			float h = hsv.hue, s = hsv.saturation, v = hsv.value;
			h -= std::floorf((h / 360.0f)) * 360.0f;
			s /= 100.0f;
			v /= 100.0f;
			fInt d = (fInt)(v * 255.0f);
			if (std::fabsf(s - 0.0f) <= std::numeric_limits<float>::min()) {
				return fcyColor(255, d, d, d);
			}
			int i = (int)std::floorf(h / 60.0f) % 6;
			float f = h / 60.0f - (float)i;
			fInt a = (fInt)std::floorf(d * (1.0f - s));
			fInt b = (fInt)std::floorf(d * (1.0f - s * f));
			fInt c = (fInt)std::floorf(d * (1.0f - s * (1.0f - f)));
			switch (i)
			{
			case 0:
				return fcyColor(255, d, c, a);
			case 1:
				return fcyColor(255, b, d, a);
			case 2:
				return fcyColor(255, a, d, c);
			case 3:
				return fcyColor(255, a, b, d);
			case 4:
				return fcyColor(255, c, a, d);
			case 5:
				return fcyColor(255, d, a, b);
			default:
				return fcyColor(0);
			}
		}
		
		void ColorWrapper::Register(lua_State* L)LNOEXCEPT {
			struct Function {
#define _GETUDATA(i) static_cast<fcyColor*>(luaL_checkudata(L, (i), LUASTG_LUA_TYPENAME_COLOR));
#define GETUDATA(p, i) fcyColor* (p) = _GETUDATA(i);
#define GETUDATAHSV(p, i) HSVColor* (p) = (HSVColor*)((fcyColor*)luaL_checkudata(L, (i), LUASTG_LUA_TYPENAME_COLOR) + 1);
				static int ARGB(lua_State* L)LNOEXCEPT
				{
					auto args = lua_gettop(L);
					GETUDATA(p, 1);
					GETUDATAHSV(q, 1);
					switch (args)
					{
					case 1:
						lua_pushinteger(L, p->a);
						lua_pushinteger(L, p->r);
						lua_pushinteger(L, p->g);
						lua_pushinteger(L, p->b);
						return 4;
					case 2:
						p->argb = (fuInt)luaL_checknumber(L, 2);
						*q = RGB2HSV(*p);
						return 0;
					case 5:
						p->a = (fByte)std::clamp<fInt>(luaL_checkinteger(L, 2), 0, 255);
						p->r = (fByte)std::clamp<fInt>(luaL_checkinteger(L, 3), 0, 255);
						p->g = (fByte)std::clamp<fInt>(luaL_checkinteger(L, 4), 0, 255);
						p->b = (fByte)std::clamp<fInt>(luaL_checkinteger(L, 5), 0, 255);
						*q = RGB2HSV(*p);
						return 0;
					default:
						return luaL_error(L, "Invalid args.");
					}
				}
				static int AHSV(lua_State* L) {
					auto args = lua_gettop(L);
					GETUDATA(p, 1);
					GETUDATAHSV(q, 1);
					switch (args)
					{
					case 1:
						lua_pushnumber(L, ((lua_Number)p->a / 255.0) * 100.0);
						lua_pushnumber(L, q->hue);
						lua_pushnumber(L, q->saturation);
						lua_pushnumber(L, q->value);
						return 3;
					case 5:
						q->hue			= (float)std::clamp(luaL_checknumber(L, 3), 0.0, 100.0);
						q->saturation	= (float)std::clamp(luaL_checknumber(L, 4), 0.0, 100.0);
						q->value		= (float)std::clamp(luaL_checknumber(L, 5), 0.0, 100.0);
						*p = HSV2RGB(*q);
						p->a = (fByte)(std::clamp(luaL_checknumber(L, 2) / 100.0, 0.0, 1.0) * 255.0);
						return 0;
					default:
						return luaL_error(L, "Invalid args.");
					}
				}
				
				static int Meta_Index(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					GETUDATAHSV(q, 1);
					const char* key = luaL_checkstring(L, 2);
					switch (LuaSTG::MapColorMember(key))
					{
					case LuaSTG::ColorMember::m_a:
						lua_pushinteger(L, (lua_Integer)p->a);
						break;
					case LuaSTG::ColorMember::m_r:
						lua_pushinteger(L, (lua_Integer)p->r);
						break;
					case LuaSTG::ColorMember::m_g:
						lua_pushinteger(L, (lua_Integer)p->g);
						break;
					case LuaSTG::ColorMember::m_b:
						lua_pushinteger(L, (lua_Integer)p->b);
						break;
					case LuaSTG::ColorMember::m_argb:
						lua_pushnumber(L, (lua_Number)p->argb);
						break;
					case LuaSTG::ColorMember::m_h:
						lua_pushnumber(L, (lua_Number)q->hue);
						break;
					case LuaSTG::ColorMember::m_s:
						lua_pushnumber(L, (lua_Number)q->saturation);
						break;
					case LuaSTG::ColorMember::m_v:
						lua_pushnumber(L, (lua_Number)q->value);
						break;
					case LuaSTG::ColorMember::f_ARGB:
						lua_pushcfunction(L, ARGB);
						break;
					case LuaSTG::ColorMember::f_AHSV:
						lua_pushcfunction(L, AHSV);
						break;
					default:
						return luaL_error(L, "Invalid index key.");
					}
					return 1;
				}
				static int Meta_NewIndex(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					GETUDATAHSV(q, 1);
					fByte olda = p->a;
					const char* key = luaL_checkstring(L, 2);
					switch (LuaSTG::MapColorMember(key))
					{
					case LuaSTG::ColorMember::m_a:
						p->a = (fByte)std::clamp<fInt>(luaL_checkinteger(L, 3), 0, 255);
						break;
					case LuaSTG::ColorMember::m_r:
						p->r = (fByte)std::clamp<fInt>(luaL_checkinteger(L, 3), 0, 255);
						*q = RGB2HSV(*p);
						break;
					case LuaSTG::ColorMember::m_g:
						p->g = (fByte)std::clamp<fInt>(luaL_checkinteger(L, 3), 0, 255);
						*q = RGB2HSV(*p);
						break;
					case LuaSTG::ColorMember::m_b:
						p->b = (fByte)std::clamp<fInt>(luaL_checkinteger(L, 3), 0, 255);
						*q = RGB2HSV(*p);
						break;
					case LuaSTG::ColorMember::m_argb:
						p->argb = (fuInt)luaL_checknumber(L, 3);
						*q = RGB2HSV(*p);
						break;
					case LuaSTG::ColorMember::m_h:
						q->hue = (float)luaL_checknumber(L, 3); // any angle
						*p = HSV2RGB(*q);
						p->a = olda;
						break;
					case LuaSTG::ColorMember::m_s:
						q->saturation = (float)std::clamp(luaL_checknumber(L, 3), 0.0, 100.0);
						*p = HSV2RGB(*q);
						p->a = olda;
						break;
					case LuaSTG::ColorMember::m_v:
						q->value = (float)std::clamp(luaL_checknumber(L, 3), 0.0, 100.0);
						*p = HSV2RGB(*q);
						p->a = olda;
						break;
					default:
						return luaL_error(L, "Invalid index key.");
					}
					return 0;
				}
				static int Meta_Eq(lua_State* L)LNOEXCEPT
				{
					GETUDATA(pA, 1);
					GETUDATA(pB, 2);
					lua_pushboolean(L, pA->argb == pB->argb);
					return 1;
				}
				static int Meta_Add(lua_State* L)LNOEXCEPT
				{
					if (lua_isnumber(L, 1)) {  // arg1为数字，则arg2必为lstgColor
						lua_Number tFactor = luaL_checknumber(L, 1);
						GETUDATA(p, 2);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)(tFactor + (lua_Number)p->a), 0, 255),
							std::clamp((fInt)(tFactor + (lua_Number)p->r), 0, 255),
							std::clamp((fInt)(tFactor + (lua_Number)p->g), 0, 255),
							std::clamp((fInt)(tFactor + (lua_Number)p->b), 0, 255)
						));
					}
					else if (lua_isnumber(L, 2)) { // arg2为数字，则arg1必为lstgColor
						lua_Number tFactor = luaL_checknumber(L, 2);
						GETUDATA(p, 1);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)((lua_Number)p->a + tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->r + tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->g + tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->b + tFactor), 0, 255)
						));
					}
					else {
						GETUDATA(pA, 1);
						GETUDATA(pB, 2);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)pA->a + (fInt)pB->a, 0, 255),
							std::clamp((fInt)pA->r + (fInt)pB->r, 0, 255),
							std::clamp((fInt)pA->g + (fInt)pB->g, 0, 255),
							std::clamp((fInt)pA->b + (fInt)pB->b, 0, 255)
						));
					}
					return 1;
				}
				static int Meta_Sub(lua_State* L)LNOEXCEPT
				{
					if (lua_isnumber(L, 1)) {  // arg1为数字，则arg2必为lstgColor
						lua_Number tFactor = luaL_checknumber(L, 1);
						GETUDATA(p, 2);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)(tFactor - (lua_Number)p->a), 0, 255),
							std::clamp((fInt)(tFactor - (lua_Number)p->r), 0, 255),
							std::clamp((fInt)(tFactor - (lua_Number)p->g), 0, 255),
							std::clamp((fInt)(tFactor - (lua_Number)p->b), 0, 255)
						));
					}
					else if (lua_isnumber(L, 2)) { // arg2为数字，则arg1必为lstgColor
						lua_Number tFactor = luaL_checknumber(L, 2);
						GETUDATA(p, 1);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)((lua_Number)p->a - tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->r - tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->g - tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->b - tFactor), 0, 255)
						));
					}
					else {
						GETUDATA(pA, 1);
						GETUDATA(pB, 2);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)pA->a - (fInt)pB->a, 0, 255),
							std::clamp((fInt)pA->r - (fInt)pB->r, 0, 255),
							std::clamp((fInt)pA->g - (fInt)pB->g, 0, 255),
							std::clamp((fInt)pA->b - (fInt)pB->b, 0, 255)
						));
					}
					return 1;
				}
				static int Meta_Mul(lua_State* L)LNOEXCEPT
				{
					if (lua_isnumber(L, 1)) {  // arg1为数字，则arg2必为lstgColor
						lua_Number tFactor = luaL_checknumber(L, 1);
						GETUDATA(p, 2);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)(tFactor * (lua_Number)p->a), 0, 255),
							std::clamp((fInt)(tFactor * (lua_Number)p->r), 0, 255),
							std::clamp((fInt)(tFactor * (lua_Number)p->g), 0, 255),
							std::clamp((fInt)(tFactor * (lua_Number)p->b), 0, 255)
						));
					}
					else if (lua_isnumber(L, 2)) { // arg2为数字，则arg1必为lstgColor
						lua_Number tFactor = luaL_checknumber(L, 2);
						GETUDATA(p, 1);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)((lua_Number)p->a * tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->r * tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->g * tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->b * tFactor), 0, 255)
						));
					}
					else {
						GETUDATA(pA, 1);
						GETUDATA(pB, 2);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)pA->a * (fInt)pB->a, 0, 255),
							std::clamp((fInt)pA->r * (fInt)pB->r, 0, 255),
							std::clamp((fInt)pA->g * (fInt)pB->g, 0, 255),
							std::clamp((fInt)pA->b * (fInt)pB->b, 0, 255)
						));
					}
					return 1;
				}
				static int Meta_Div(lua_State* L)LNOEXCEPT
				{
					if (lua_isnumber(L, 1)) {  // arg1为数字，则arg2必为lstgColor
						lua_Number tFactor = luaL_checknumber(L, 1);
						GETUDATA(p, 2);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)(tFactor / (lua_Number)p->a), 0, 255),
							std::clamp((fInt)(tFactor / (lua_Number)p->r), 0, 255),
							std::clamp((fInt)(tFactor / (lua_Number)p->g), 0, 255),
							std::clamp((fInt)(tFactor / (lua_Number)p->b), 0, 255)
						));
					}
					else if (lua_isnumber(L, 2)) { // arg2为数字，则arg1必为lstgColor
						lua_Number tFactor = luaL_checknumber(L, 2);
						GETUDATA(p, 1);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)((lua_Number)p->a / tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->r / tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->g / tFactor), 0, 255),
							std::clamp((fInt)((lua_Number)p->b / tFactor), 0, 255)
						));
					}
					else {
						GETUDATA(pA, 1);
						GETUDATA(pB, 2);
						ColorWrapper::CreateAndPush(L, fcyColor(
							std::clamp((fInt)((lua_Number)pA->a / (lua_Number)pB->a), 0, 255),
							std::clamp((fInt)((lua_Number)pA->r / (lua_Number)pB->r), 0, 255),
							std::clamp((fInt)((lua_Number)pA->g / (lua_Number)pB->g), 0, 255),
							std::clamp((fInt)((lua_Number)pA->b / (lua_Number)pB->b), 0, 255)
						));
					}
					return 1;
				}
				static int Meta_ToString(lua_State* L)LNOEXCEPT
				{
					GETUDATA(p, 1);
					lua_pushfstring(L, "lstg.Color(%d, %d, %d, %d)", p->a, p->r, p->g, p->b);
					return 1;
				}
#undef _GETUDATA
#undef GETUDATA
#undef GETUDATAHSV
			};

			luaL_Reg tMethods[] =
			{
				{ "ARGB", &Function::ARGB },
				{ "AHSV", &Function::AHSV },
				{ NULL, NULL }
			};

			luaL_Reg tMetaTable[] =
			{
				{ "__index", &Function::Meta_Index },
				{ "__newindex", &Function::Meta_NewIndex },
				{ "__eq", &Function::Meta_Eq },
				{ "__add", &Function::Meta_Add },
				{ "__sub", &Function::Meta_Sub },
				{ "__mul", &Function::Meta_Mul },
				{ "__div", &Function::Meta_Div },
				{ "__tostring", &Function::Meta_ToString },
				{ NULL, NULL }
			};

			RegisterClassIntoTable2(L, ".Color", tMethods, LUASTG_LUA_TYPENAME_COLOR, tMetaTable);
		}

		void ColorWrapper::CreateAndPush(lua_State* L, const fcyColor& color) {
			fcyColor* p = static_cast<fcyColor*>(lua_newuserdata(L, sizeof(fcyColor) + sizeof(HSVColor))); // udata
			new(p) fcyColor(color);
			HSVColor* q = (HSVColor*)((fcyColor*)p + 1);
			new(q) HSVColor;
			*q = RGB2HSV(color);
			luaL_getmetatable(L, LUASTG_LUA_TYPENAME_COLOR); // udata mt
			lua_setmetatable(L, -2); // udata
		}
	}
}
