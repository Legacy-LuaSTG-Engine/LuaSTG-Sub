#include "LuaBinding/LuaWrapper.hpp"
#include "LuaBinding/lua_luastg_hash.hpp"
#include <DirectXMath.h>

namespace LuaSTGPlus::LuaWrapper
{
	inline Core::Color4B HSV2RGB(float const hue, float const saturation, float const value, float const alpha)
	{
		DirectX::XMFLOAT4 const vec(hue * 0.01f, saturation * 0.01f, value * 0.01f, alpha * 0.01f);
		DirectX::XMFLOAT4 vec2{};
		DirectX::XMStoreFloat4(&vec2, DirectX::XMColorHSVToRGB(DirectX::XMLoadFloat4(&vec)));
		return Core::Color4B(
			(uint8_t)(vec2.x * 255.0f),
			(uint8_t)(vec2.y * 255.0f),
			(uint8_t)(vec2.z * 255.0f),
			(uint8_t)(vec2.w * 255.0f)
		);
	}

	// In this case it returns a standard vector of floats, wxyz = ahsv respectively
	inline Core::Vector4F RGB2HSV(uint8_t const red, uint8_t const green, uint8_t const blue, uint8_t const alpha)
	{
		DirectX::XMFLOAT4 const vec(red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f);
		DirectX::XMFLOAT4 vec2{};
		DirectX::XMStoreFloat4(&vec2, DirectX::XMColorRGBToHSV(DirectX::XMLoadFloat4(&vec)));
		return Core::Vector4(
			vec2.x * 100.0f,
			vec2.y * 100.0f,
			vec2.z * 100.0f,
			vec2.w * 100.0f
		);
	}

	std::string_view const ColorWrapper::ClassID = "lstg.Color";

	Core::Color4B* ColorWrapper::Cast(lua_State* L, int idx)
	{
		return static_cast<Core::Color4B*>(luaL_checkudata(L, idx, ClassID.data()));
	}

	constexpr lua_Number const _1_255 = 1.0 / 255.0;

	void ColorWrapper::Register(lua_State* L) noexcept
	{
		struct Function
		{
		#define GETUDATA(p, i) Core::Color4B* (p) = Cast(L, i);

			static int ARGB(lua_State* L) noexcept
			{
				GETUDATA(p, 1);
				int const argc = lua_gettop(L);
				if (argc == 1)
				{
					lua_pushinteger(L, (lua_Integer)p->a);
					lua_pushinteger(L, (lua_Integer)p->r);
					lua_pushinteger(L, (lua_Integer)p->g);
					lua_pushinteger(L, (lua_Integer)p->b);
					return 4;
				}
				else if (argc == 2)
				{
					p->color((uint32_t)luaL_checknumber(L, 2));
					return 0;
				}
				else if (argc == 5)
				{
					p->a = (uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 2), 0, 255);
					p->r = (uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 3), 0, 255);
					p->g = (uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 4), 0, 255);
					p->b = (uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 5), 0, 255);
					return 0;
				}
				else
				{
					return luaL_error(L, "Invalid args.");
				}
			}
			static int AHSV(lua_State* L)
			{
				GETUDATA(p, 1);
				int const argc = lua_gettop(L);
				if (argc == 1)
				{
					Core::Vector4F const hsva = RGB2HSV(p->r, p->g, p->b, p->a);
					lua_pushnumber(L, (lua_Number)hsva.w); // alpha
					lua_pushnumber(L, (lua_Number)hsva.x); // hue
					lua_pushnumber(L, (lua_Number)hsva.y); // saturation
					lua_pushnumber(L, (lua_Number)hsva.z); // value
					return 4;
				}
				else if (argc == 5)
				{
					float const alpha = (float)std::clamp(luaL_checknumber(L, 2), 0.0, 100.0);
					float const hue = (float)std::clamp(luaL_checknumber(L, 3), 0.0, 100.0);
					float const saturation = (float)std::clamp(luaL_checknumber(L, 4), 0.0, 100.0);
					float const value = (float)std::clamp(luaL_checknumber(L, 5), 0.0, 100.0);
					*p = HSV2RGB(hue, saturation, value, alpha);
					return 0;
				}
				else
				{
					return luaL_error(L, "Invalid args.");
				}
			}

			static int Meta_Index(lua_State* L) noexcept
			{
				GETUDATA(p, 1);
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
				case LuaSTG::ColorMember::m_h:
					lua_pushnumber(L, (lua_Number)RGB2HSV(p->r, p->g, p->b, p->a).x);
					break;
				case LuaSTG::ColorMember::m_s:
					lua_pushnumber(L, (lua_Number)RGB2HSV(p->r, p->g, p->b, p->a).y);
					break;
				case LuaSTG::ColorMember::m_v:
					lua_pushnumber(L, (lua_Number)RGB2HSV(p->r, p->g, p->b, p->a).z);
					break;
				case LuaSTG::ColorMember::m_argb:
					lua_pushnumber(L, (lua_Number)p->color());
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
			static int Meta_NewIndex(lua_State* L) noexcept
			{
				GETUDATA(p, 1);
				const char* key = luaL_checkstring(L, 2);
				switch (LuaSTG::MapColorMember(key))
				{
				case LuaSTG::ColorMember::m_a:
					p->a = (uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 3), 0, 255);
					break;
				case LuaSTG::ColorMember::m_r:
					p->r = (uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 3), 0, 255);
					break;
				case LuaSTG::ColorMember::m_g:
					p->g = (uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 3), 0, 255);
					break;
				case LuaSTG::ColorMember::m_b:
					p->b = (uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 3), 0, 255);
					break;
				case LuaSTG::ColorMember::m_h:
					{
						Core::Vector4F hsva = RGB2HSV(p->r, p->g, p->b, p->a);
						hsva.x = (float)std::clamp(luaL_checknumber(L, 3), 0.0, 100.0);
						*p = HSV2RGB(hsva.x, hsva.y, hsva.z, hsva.w);
					}
					break;
				case LuaSTG::ColorMember::m_s:
					{
						Core::Vector4F hsva = RGB2HSV(p->r, p->g, p->b, p->a);
						hsva.y = (float)std::clamp(luaL_checknumber(L, 3), 0.0, 100.0);
						*p = HSV2RGB(hsva.x, hsva.y, hsva.z, hsva.w);
					}
					break;
				case LuaSTG::ColorMember::m_v:
					{
						Core::Vector4F hsva = RGB2HSV(p->r, p->g, p->b, p->a);
						hsva.z = (float)std::clamp(luaL_checknumber(L, 3), 0.0, 100.0);
						*p = HSV2RGB(hsva.x, hsva.y, hsva.z, hsva.w);
					}
					break;
				case LuaSTG::ColorMember::m_argb:
					p->color((uint32_t)luaL_checknumber(L, 3));
					break;
				default:
					return luaL_error(L, "Invalid index key.");
				}
				return 0;
			}
			static int Meta_Eq(lua_State* L) noexcept
			{
				GETUDATA(pA, 1);
				GETUDATA(pB, 2);
				lua_pushboolean(L, *pA == *pB);
				return 1;
			}
			static int Meta_Add(lua_State* L) noexcept
			{
				if (lua_isnumber(L, 1))
				{
					lua_Number const v = luaL_checknumber(L, 1);
					GETUDATA(p, 2);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp(v + (lua_Number)p->r, 0.0, 255.0),
						(uint8_t)std::clamp(v + (lua_Number)p->g, 0.0, 255.0),
						(uint8_t)std::clamp(v + (lua_Number)p->b, 0.0, 255.0),
						(uint8_t)std::clamp(v + (lua_Number)p->a, 0.0, 255.0)
					));
				}
				else if (lua_isnumber(L, 2))
				{
					lua_Number const v = luaL_checknumber(L, 2);
					GETUDATA(p, 1);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp((lua_Number)p->r + v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->g + v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->b + v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->a + v, 0.0, 255.0)
					));
				}
				else
				{
					GETUDATA(pA, 1);
					GETUDATA(pB, 2);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp((lua_Number)pA->r + (lua_Number)pB->r, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)pA->g + (lua_Number)pB->g, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)pA->b + (lua_Number)pB->b, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)pA->a + (lua_Number)pB->a, 0.0, 255.0)
					));
				}
				return 1;
			}
			static int Meta_Sub(lua_State* L) noexcept
			{
				if (lua_isnumber(L, 1))
				{
					lua_Number const v = luaL_checknumber(L, 1);
					GETUDATA(p, 2);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp(v - (lua_Number)p->r, 0.0, 255.0),
						(uint8_t)std::clamp(v - (lua_Number)p->g, 0.0, 255.0),
						(uint8_t)std::clamp(v - (lua_Number)p->b, 0.0, 255.0),
						(uint8_t)std::clamp(v - (lua_Number)p->a, 0.0, 255.0)
					));
				}
				else if (lua_isnumber(L, 2))
				{
					lua_Number const v = luaL_checknumber(L, 2);
					GETUDATA(p, 1);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp((lua_Number)p->r - v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->g - v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->b - v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->a - v, 0.0, 255.0)
					));
				}
				else
				{
					GETUDATA(pA, 1);
					GETUDATA(pB, 2);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp((lua_Number)pA->r - (lua_Number)pB->r, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)pA->g - (lua_Number)pB->g, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)pA->b - (lua_Number)pB->b, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)pA->a - (lua_Number)pB->a, 0.0, 255.0)
					));
				}
				return 1;
			}
			static int Meta_Mul(lua_State* L) noexcept
			{
				if (lua_isnumber(L, 1))
				{
					lua_Number const v = luaL_checknumber(L, 1);
					GETUDATA(p, 2);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp(v * (lua_Number)p->r, 0.0, 255.0),
						(uint8_t)std::clamp(v * (lua_Number)p->g, 0.0, 255.0),
						(uint8_t)std::clamp(v * (lua_Number)p->b, 0.0, 255.0),
						(uint8_t)std::clamp(v * (lua_Number)p->a, 0.0, 255.0)
					));
				}
				else if (lua_isnumber(L, 2))
				{
					lua_Number const v = luaL_checknumber(L, 2);
					GETUDATA(p, 1);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp((lua_Number)p->r * v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->g * v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->b * v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->a * v, 0.0, 255.0)
					));
				}
				else
				{
					GETUDATA(pA, 1);
					GETUDATA(pB, 2);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp(255.0 * (((lua_Number)pA->r * _1_255) * ((lua_Number)pB->r * _1_255)), 0.0, 255.0),
						(uint8_t)std::clamp(255.0 * (((lua_Number)pA->g * _1_255) * ((lua_Number)pB->g * _1_255)), 0.0, 255.0),
						(uint8_t)std::clamp(255.0 * (((lua_Number)pA->b * _1_255) * ((lua_Number)pB->b * _1_255)), 0.0, 255.0),
						(uint8_t)std::clamp(255.0 * (((lua_Number)pA->a * _1_255) * ((lua_Number)pB->a * _1_255)), 0.0, 255.0)
					));
				}
				return 1;
			}
			static int Meta_Div(lua_State* L) noexcept
			{
				if (lua_isnumber(L, 1))
				{
					lua_Number const v = luaL_checknumber(L, 1);
					GETUDATA(p, 2);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp(v / (lua_Number)p->r, 0.0, 255.0),
						(uint8_t)std::clamp(v / (lua_Number)p->g, 0.0, 255.0),
						(uint8_t)std::clamp(v / (lua_Number)p->b, 0.0, 255.0),
						(uint8_t)std::clamp(v / (lua_Number)p->a, 0.0, 255.0)
					));
				}
				else if (lua_isnumber(L, 2))
				{
					lua_Number const v = luaL_checknumber(L, 2);
					GETUDATA(p, 1);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp((lua_Number)p->r / v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->g / v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->b / v, 0.0, 255.0),
						(uint8_t)std::clamp((lua_Number)p->a / v, 0.0, 255.0)
					));
				}
				else
				{
					GETUDATA(pA, 1);
					GETUDATA(pB, 2);
					ColorWrapper::CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp(255.0 * (((lua_Number)pA->r * _1_255) / ((lua_Number)pB->r * _1_255)), 0.0, 255.0),
						(uint8_t)std::clamp(255.0 * (((lua_Number)pA->g * _1_255) / ((lua_Number)pB->g * _1_255)), 0.0, 255.0),
						(uint8_t)std::clamp(255.0 * (((lua_Number)pA->b * _1_255) / ((lua_Number)pB->b * _1_255)), 0.0, 255.0),
						(uint8_t)std::clamp(255.0 * (((lua_Number)pA->a * _1_255) / ((lua_Number)pB->a * _1_255)), 0.0, 255.0)
					));
				}
				return 1;
			}
			static int Meta_ToString(lua_State* L) noexcept
			{
				GETUDATA(p, 1);
				lua_pushfstring(L, "lstg.Color(%d, %d, %d, %d)", (int)p->a, (int)p->r, (int)p->g, (int)p->b);
				return 1;
			}

			static int Color(lua_State* L) noexcept
			{
				if (lua_gettop(L) == 1) {
					CreateAndPush(L, Core::Color4B((uint32_t)luaL_checknumber(L, 1)));
				}
				else {
					CreateAndPush(L, Core::Color4B(
						(uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 2), 0, 255),
						(uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 3), 0, 255),
						(uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 4), 0, 255),
						(uint8_t)std::clamp<lua_Integer>(luaL_checkinteger(L, 1), 0, 255) // 这个才是 a 通道
					));
				}
				return 1;
			}
			static int HSVColor(lua_State* L) noexcept
			{
				float const alpha = (float)std::clamp(luaL_checknumber(L, 1), 0.0, 100.0);
				float const hue = (float)std::clamp(luaL_checknumber(L, 2), 0.0, 100.0);
				float const saturation = (float)std::clamp(luaL_checknumber(L, 3), 0.0, 100.0);
				float const value = (float)std::clamp(luaL_checknumber(L, 4), 0.0, 100.0);
				CreateAndPush(L, HSV2RGB(hue, saturation, value, alpha));
				return 1;
			}

		#undef GETUDATA
		};

		luaL_Reg tMethods[] = {
			{ "ARGB", &Function::ARGB },
			{ "AHSV", &Function::AHSV },
			{ NULL, NULL }
		};

		luaL_Reg tMetaTable[] = {
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

		luaL_Reg lib[] = {
			{ "Color", &Function::Color },
			{ "HSVColor", &Function::HSVColor },
			{ NULL, NULL }
		};

		luaL_register(L, LUASTG_LUA_LIBNAME, lib);
		RegisterClassIntoTable2(L, ".Color", tMethods, ClassID.data(), tMetaTable);
		lua_pop(L, 1);
	}

	void ColorWrapper::CreateAndPush(lua_State* L, Core::Color4B const& color)
	{
		Core::Color4B* p = static_cast<Core::Color4B*>(lua_newuserdata(L, sizeof(Core::Color4B))); // udata
		p->color(color.color());
		luaL_getmetatable(L, ClassID.data()); // udata mt
		lua_setmetatable(L, -2); // udata
	}
}
