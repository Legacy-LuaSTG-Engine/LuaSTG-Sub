#include "LuaBinding/PostEffectShader.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"
#include "LuaBinding/LuaWrapperMisc.hpp"

namespace luastg::binding
{
	namespace
	{
		constexpr std::string_view const ClassID("lstg.PostEffectShader");
		struct Wrapper
		{
			core::Graphics::IPostEffectShader* shader;
		};
	}

	void PostEffectShader::Register(lua_State* L)
	{
		struct Class
		{
			static int setFloat(lua_State* L)
			{
				lua::stack_t S(L);
				auto* self = Cast(L, 1);
				auto const name = S.get_value<std::string_view>(2);
				auto const value = S.get_value<float>(3);
				bool const result = self->setFloat(name, value);
				S.push_value<bool>(result);
				return 1;
			}
			static int setFloat2(lua_State* L)
			{
				lua::stack_t S(L);
				auto* self = Cast(L, 1);
				auto const name = S.get_value<std::string_view>(2);
				auto const x = S.get_value<float>(3);
				auto const y = S.get_value<float>(4);
				bool const result = self->setFloat2(name, { x, y });
				S.push_value<bool>(result);
				return 1;
			}
			static int setFloat3(lua_State* L)
			{
				lua::stack_t S(L);
				auto* self = Cast(L, 1);
				auto const name = S.get_value<std::string_view>(2);
				auto const x = S.get_value<float>(3);
				auto const y = S.get_value<float>(4);
				auto const z = S.get_value<float>(5);
				bool const result = self->setFloat3(name, { x, y, z });
				S.push_value<bool>(result);
				return 1;
			}
			static int setFloat4(lua_State* L)
			{
				lua::stack_t S(L);
				auto* self = Cast(L, 1);
				auto const name = S.get_value<std::string_view>(2);
				auto const x = S.get_value<float>(3);
				auto const y = S.get_value<float>(4);
				auto const z = S.get_value<float>(5);
				auto const w = S.get_value<float>(6);
				bool const result = self->setFloat4(name, { x, y, z, w });
				S.push_value<bool>(result);
				return 1;
			}
			static int setTexture(lua_State* L)
			{
				lua::stack_t S(L);
				auto* self = Cast(L, 1);
				auto const name = S.get_value<std::string_view>(2);
				auto const resource_name = S.get_value<std::string_view>(3);
				
				core::SmartReference<luastg::IResourceTexture> p = LRES.FindTexture(resource_name.data());
				if (!p)
				{
					return luaL_error(L, "can't find texture '%s'", resource_name.data());
				}

				bool const result = self->setTexture2D(name, p->GetTexture());
				S.push_value<bool>(result);
				return 1;
			}

			static int __tostring(lua_State* L)
			{
				lua::stack_t S(L);
				auto* self = Cast(L, 1);
				std::ignore = self;
				S.push_value<std::string_view>(ClassID);
				return 1;
			}
			static int __gc(lua_State* L)
			{
				Wrapper* self = (Wrapper*)luaL_checkudata(L, 1, ClassID.data());
				if (self->shader)
				{
					self->shader->release();
					self->shader = nullptr;
				}
				return 0;
			}

			static int CreatePostEffectShader(lua_State* L)
			{
				lua::stack_t S(L);
				auto const file_path = S.get_value<std::string_view>(1);
				core::SmartReference<core::Graphics::IPostEffectShader> shader;
				if (!LAPP.getRenderer2D()->createPostEffectShader(file_path, shader.put()))
				{
					return luaL_error(L, "lstg.CreatePostEffectShader failed, see log file for more detail");
				}
				Create(L, shader.get());
				return 1;
			}
		};

		luaL_Reg const lib[] = {
			{ "setFloat", &Class::setFloat },
			{ "setFloat2", &Class::setFloat2 },
			{ "setFloat3", &Class::setFloat3 },
			{ "setFloat4", &Class::setFloat4 },
			{ "setTexture", &Class::setTexture },
			{ NULL, NULL },
		};

		luaL_Reg const mt[] = {
			{ "__tostring", &Class::__tostring },
			{ "__gc", &Class::__gc },
			{ NULL, NULL },
		};

		luaL_Reg const fun[] = {
			{ "CreatePostEffectShader", &Class::CreatePostEffectShader },
			{ NULL, NULL },
		};

		luaL_register(L, "lstg", fun); // ??? lstg
		luastg::RegisterClassIntoTable(L, ".PostEffectShader", lib, ClassID.data(), mt);
		lua_pop(L, 1);
	}
	void PostEffectShader::Create(lua_State* L, core::Graphics::IPostEffectShader* p_shader)
	{
		assert(p_shader);
		Wrapper* self = (Wrapper*)lua_newuserdata(L, sizeof(Wrapper));
		self->shader = p_shader;
		if (self->shader)
		{
			self->shader->retain();
		}
		luaL_getmetatable(L, ClassID.data());
		lua_setmetatable(L, -2);
	}
	core::Graphics::IPostEffectShader* PostEffectShader::Cast(lua_State* L, int idx)
	{
		Wrapper* self = (Wrapper*)luaL_checkudata(L, idx, ClassID.data());
		return self->shader;
	}
}
