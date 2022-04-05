#include "Core/FileManager.hpp"
#include "LuaWrapper/LuaWrapper.hpp"
#include "AppFrame.h"

void LuaSTGPlus::LuaWrapper::ResourceMgrWrapper::Register(lua_State* L) noexcept
{
	struct Wrapper
	{
		static int SetResLoadInfo(lua_State* L) noexcept {
			ResourceMgr::SetResourceLoadingLog((bool)lua_toboolean(L, 1));
			return 0;
		}
		static int SetResourceStatus(lua_State* L) noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			if (strcmp(s, "global") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::Global);
			else if (strcmp(s, "stage") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::Stage);
			else if (strcmp(s, "none") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::None);
			else
				return luaL_error(L, "invalid argument #1 for 'SetResourceStatus', requires 'stage', 'global' or 'none'.");
			return 0;
		}
		static int GetResourceStatus(lua_State* L) noexcept
		{
			switch (LRES.GetActivedPoolType()) {
			case ResourcePoolType::Global:
				lua_pushstring(L, "global");
				break;
			case ResourcePoolType::Stage:
				lua_pushstring(L, "stage");
				break;
			case ResourcePoolType::None:
				lua_pushstring(L, "none");
				break;
			default:
				return luaL_error(L, "can't get resource pool status at this time.");
			}
			return 1;
		}
		static int LoadTexture(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			if (!pActivedPool->LoadTexture(name, path, lua_toboolean(L, 3) == 0 ? false : true))
				return luaL_error(L, "can't load texture from file '%s'.", path);
			return 0;
		}
		static int LoadSprite(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* texname = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->CreateSprite(
				name,
				texname,
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4),
				luaL_checknumber(L, 5),
				luaL_checknumber(L, 6),
				luaL_optnumber(L, 7, 0.),
				luaL_optnumber(L, 8, 0.),
				lua_toboolean(L, 9) == 0 ? false : true
			))
			{
				return luaL_error(L, "load image failed (name='%s', tex='%s').", name, texname);
			}
			return 0;
		}
		static int LoadAnimation(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* texname = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			
			if (!pActivedPool->CreateAnimation(
				name,
				texname,
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4),
				luaL_checknumber(L, 5),
				luaL_checknumber(L, 6),
				luaL_checkinteger(L, 7),
				luaL_checkinteger(L, 8),
				luaL_checkinteger(L, 9),
				luaL_optnumber(L, 10, 0.0f),
				luaL_optnumber(L, 11, 0.0f),
				lua_toboolean(L, 12) == 0 ? false : true
			))
			{
				return luaL_error(L, "load animation failed (name='%s', tex='%s').", name, texname);
			}

			return 0;
		}
		static int LoadPS(lua_State* L) noexcept
		{
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			
			const char* name = luaL_checkstring(L, 1);
			const char* img_name = luaL_checkstring(L, 3);
			if (lua_type(L, 2) == LUA_TTABLE) {
				ResParticle::ParticleInfo info;
				bool ret = TranslateTableToParticleInfo(L, 2, info);
				if (!ret) return luaL_error(L, "load particle failed (name='%s', define=?, img='%s').", name, img_name);
				if (!pActivedPool->LoadParticle(
					name,
					info,
					img_name,
					luaL_optnumber(L, 4, 0.0f),
					luaL_optnumber(L, 5, 0.0f),
					lua_toboolean(L, 6) == 0 ? false : true
				))
				{
					return luaL_error(L, "load particle failed (name='%s', define=table, img='%s').", name, img_name);
				}
				return 0;
			}
			else {
				const char* path = luaL_checkstring(L, 2);

				if (!pActivedPool->LoadParticle(
					name,
					path,
					img_name,
					luaL_optnumber(L, 4, 0.0f),
					luaL_optnumber(L, 5, 0.0f),
					lua_toboolean(L, 6) == 0 ? false : true
				))
				{
					return luaL_error(L, "load particle failed (name='%s', file='%s', img='%s').", name, path, img_name);
				}
				return 0;
			}
		}
		static int LoadSound(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadSoundEffect(name, path))
				return luaL_error(L, "load sound failed (name=%s, path=%s)", name, path);
			return 0;
		}
		static int LoadMusic(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			double loop_end = luaL_checknumber(L, 3);
			double loop_duration = luaL_checknumber(L, 4);
			double loop_start = std::max(0., loop_end - loop_duration);

			if (!pActivedPool->LoadMusic(
				name,
				path,
				loop_start,
				loop_end
				))
			{
				return luaL_error(L, "load music failed (name=%s, path=%s, loop=%f~%f)", name, path, loop_start, loop_end);
			}
			return 0;
		}
		static int LoadFont(lua_State* L) noexcept
		{
			bool bSucceed = false;
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (lua_gettop(L) == 2)
			{
				// HGE字体 mipmap=true
				bSucceed = pActivedPool->LoadSpriteFont(name, path);
			}
			else
			{
				if (lua_isboolean(L, 3))
				{
					// HGE字体 mipmap=user_defined
					bSucceed = pActivedPool->LoadSpriteFont(name, path, lua_toboolean(L, 3) == 0 ? false : true);
				}
				else
				{
					// fancy2d字体
					const char* texpath = luaL_checkstring(L, 3);
					if (lua_gettop(L) == 4)
						bSucceed = pActivedPool->LoadSpriteFont(name, path, texpath, lua_toboolean(L, 4) == 0 ? false : true);
					else
						bSucceed = pActivedPool->LoadSpriteFont(name, path, texpath);
				}
			}

			if (!bSucceed)
				return luaL_error(L, "can't load font from file '%s'.", path);
			return 0;
		}
		static int LoadTTF(lua_State* L) noexcept
		{
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool) {
				return luaL_error(L, "can't load resource at this time.");
			}

			const char* name = luaL_checkstring(L, 1);
			int t = lua_type(L, 2);
			if (LUA_TSTRING == t) {
				const char* path = luaL_checkstring(L, 2);
				bool result = pActivedPool->LoadTTFFont(name, path, (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), (float)luaL_optnumber(L, 5, 0.0), (float)luaL_optnumber(L, 6, 0.0));
				lua_pushboolean(L, result);
			}
			else {
				using sw = LuaSTGPlus::LuaWrapper::IO::StreamWrapper::Wrapper;
				sw* data = (sw*)luaL_checkudata(L, 2, LUASTG_LUA_TYPENAME_IO_STREAM);
				bool result = pActivedPool->LoadTTFFont(name, data->handle, (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), (float)luaL_optnumber(L, 5, 0.0), (float)luaL_optnumber(L, 6, 0.0));
				lua_pushboolean(L, result);
			}
			return 1;
		}
		static int LoadTrueTypeFont(lua_State* L) noexcept
		{
			// 先检查有没有资源池
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool) {
				return luaL_error(L, "can't load resource at this time.");
			}
			
			// 第一个参数，资源名
			const char* name = luaL_checkstring(L, 1);
			
			// 第二个参数，字体加载配置
			f2dFontProviderParam param;
			{
				param.font_bbox = fcyVec2(0.0f, 0.0f);
				param.glyph_count = 1024;
				param.texture_size = 2048;
			}
			if (lua_istable(L, 2))
			{
				lua_getfield(L, 2, "glyph_bbox_x");
				if (lua_type(L, -1) == LUA_TNUMBER) {
					param.font_bbox.x = (fFloat)luaL_checknumber(L, -1);
				}
				lua_pop(L, 1);
				
				lua_getfield(L, 2, "glyph_bbox_y");
				if (lua_type(L, -1) == LUA_TNUMBER) {
					param.font_bbox.y = (fFloat)luaL_checknumber(L, -1);
				}
				lua_pop(L, 1);
				
				lua_getfield(L, 2, "glyph_count");
				if (lua_type(L, -1) == LUA_TNUMBER) {
					param.glyph_count = (fuInt)std::max<fInt>(0, luaL_checkinteger(L, -1));
				}
				lua_pop(L, 1);
				
				lua_getfield(L, 2, "texture_size");
				if (lua_type(L, -1) == LUA_TNUMBER) {
					param.texture_size = (fuInt)std::max<fInt>(0, luaL_checkinteger(L, -1));
				}
				lua_pop(L, 1);
			}
			
			// 第三个参数，字体
			std::vector<f2dTrueTypeFontParam> fonts;
			std::vector<fcyRefPointer<fcyStream>> streams;
			if (lua_istable(L, 3))
			{
				using sw = LuaSTGPlus::LuaWrapper::IO::StreamWrapper::Wrapper;
				int cnt = (int)lua_objlen(L, 3);
												// name param fonts
				for (int i = 1; i <= cnt; i += 1) {
					f2dTrueTypeFontParam font;
					{
						font.font_file = nullptr; // 不会用上
						font.font_source = nullptr;
						font.font_face = 0;
						font.font_size = fcyVec2(0.0f, 0.0f);
					}
					
					lua_pushinteger(L, i);		// name param fonts i
					lua_gettable(L, 3);			// name param fonts font
					
					lua_getfield(L, -1, "face_index"); // name param fonts font ?
					if (lua_type(L, -1) == LUA_TNUMBER) // name param fonts font v
					{
						font.font_face = (fInt)luaL_checkinteger(L, -1);
					}
					lua_pop(L, 1);				// name param fonts font
					
					lua_getfield(L, -1, "width"); // name param fonts font ?
					if (lua_type(L, -1) == LUA_TNUMBER) // name param fonts font v
					{
						font.font_size.x = (fFloat)luaL_checknumber(L, -1);
					}
					lua_pop(L, 1);				// name param fonts font
					
					lua_getfield(L, -1, "height"); // name param fonts font ?
					if (lua_type(L, -1) == LUA_TNUMBER) // name param fonts font v
					{
						font.font_size.y = (fFloat)luaL_checknumber(L, -1);
					}
					lua_pop(L, 1);				// name param fonts font
					
					lua_getfield(L, -1, "source"); // name param fonts font ?
					if (lua_type(L, -1) == LUA_TUSERDATA) // name param fonts font v
					{
						sw* data = (sw*)luaL_checkudata(L, -1, LUASTG_LUA_TYPENAME_IO_STREAM);
						font.font_source = (fcyMemStream*)data->handle;
					}
					lua_pop(L, 1);				// name param fonts font
					
					lua_getfield(L, -1, "file"); // name param fonts font ?
					if (lua_type(L, -1) == LUA_TSTRING) // name param fonts font v
					{
						const char* filename = luaL_checkstring(L, -1);
						bool loaded = false;
						// 先从文件加载试试看
						try {
							std::wstring wfilename = fcyStringHelper::MultiByteToWideChar(filename);
							try {
								fcyFileStream* stream = new fcyFileStream(wfilename.c_str(), false);
								
								streams.push_back(fcyRefPointer<fcyStream>());
								streams.back().DirectSet(stream);
								loaded = true;
								
								font.font_source = nullptr; // 这个不要
								font.font_file = (f2dStream*)stream;
							}
							catch (...) {}
						}
						catch (...) {}
						// 没有……那只能从FMGR加载了
						if (!loaded) {
							fcyMemStream* stream = nullptr;
							if (!GFileManager().loadEx(filename, &stream)) {
								streams.push_back(fcyRefPointer<fcyStream>());
								streams.back().DirectSet(stream);
								loaded = true;
								
								font.font_source = stream;
							}
						}
					}
					lua_pop(L, 1);				// name param fonts font
					
					lua_pop(L, 1);				// name param fonts
					fonts.emplace_back(std::move(font));
				}
			}
			
			bool result = pActivedPool->LoadTrueTypeFont(name, param, fonts.data(), fonts.size());
			lua_pushboolean(L, result);
			
			return 1;
		}
		static int LoadFX(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (lua_gettop(L) < 3)
			{
				if (!pActivedPool->LoadFX(name, path))
					return luaL_error(L, "load fx failed (name=%s, path=%s)", name, path);
			}
			else
			{
				if (!pActivedPool->LoadFX(name, path, lua_toboolean(L, 3)))
					return luaL_error(L, "load fx failed (name=%s, path=%s)", name, path);
			}
			return 0;
		}
		static int LoadModel(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			const char* model_path = luaL_checkstring(L, 2);
			
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			if (!pActivedPool->LoadModel(
				name,
				model_path))
			{
				return luaL_error(L, "load model failed (name='%s', model='%s').", name, model_path);
			}
			return 0;
		}
		static int CreateRenderTarget(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			
			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			
			if (lua_gettop(L) >= 2)
			{
				const int width = (int)luaL_checkinteger(L, 2);
				const int height = (int)luaL_checkinteger(L, 3);
				if (width < 1 || height < 1)
					return luaL_error(L, "invalid render target size (%dx%d).", width, height);
				if (!pActivedPool->CreateRenderTarget(name, width, height))
					return luaL_error(L, "can't create render target with name '%s'.", name);
			}
			else
			{
				if (!pActivedPool->CreateRenderTarget(name))
					return luaL_error(L, "can't create render target with name '%s'.", name);
			}
			
			return 0;
		}
		static int IsRenderTarget(lua_State* L) noexcept
		{
			ResTexture* p = LRES.FindTexture(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "render target '%s' not found.", luaL_checkstring(L, 1));
			lua_pushboolean(L, p->IsRenderTarget());
			return 1;
		}
		static int SetTexturePreMulAlphaState(lua_State* L) noexcept
		{
			ResTexture* p = LRES.FindTexture(luaL_checkstring(L, 1));
			if (p)
			{
				p->GetTexture()->SetPremultipliedAlpha(lua_toboolean(L, 2));
				return 0;
			}
			return luaL_error(L, "texture '%s' not found.", luaL_checkstring(L, 1));
		}
		static int GetTextureSize(lua_State* L) noexcept
		{
			const char* name = luaL_checkstring(L, 1);
			fcyVec2 size;
			if (!LRES.GetTextureSize(name, size))
				return luaL_error(L, "texture '%s' not found.", name);
			lua_pushnumber(L, size.x);
			lua_pushnumber(L, size.y);
			return 2;
		}
		static int RemoveResource(lua_State* L) noexcept
		{
			ResourcePoolType t;
			const char* s = luaL_checkstring(L, 1);
			if (strcmp(s, "global") == 0)
				t = ResourcePoolType::Global;
			else if (strcmp(s, "stage") == 0)
				t = ResourcePoolType::Stage;
			else if (strcmp(s, "none") != 0)
				t = ResourcePoolType::None;
			else
				return luaL_error(L, "invalid argument #1 for 'RemoveResource', requires 'stage', 'global' or 'none'.");

			if (lua_gettop(L) == 1)
			{
				switch (t)
				{
				case ResourcePoolType::Stage:
					LRES.GetResourcePool(ResourcePoolType::Stage)->Clear();
					break;
				case ResourcePoolType::Global:
					LRES.GetResourcePool(ResourcePoolType::Global)->Clear();
					break;
				default:
					break;
				}
			}
			else
			{
				ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 2));
				const char* tResourceName = luaL_checkstring(L, 3);

				switch (t)
				{
				case ResourcePoolType::Stage:
					LRES.GetResourcePool(ResourcePoolType::Stage)->RemoveResource(tResourceType, tResourceName);
					break;
				case ResourcePoolType::Global:
					LRES.GetResourcePool(ResourcePoolType::Global)->RemoveResource(tResourceType, tResourceName);
					break;
				default:
					break;
				}
			}
			
			return 0;
		}
		static int CheckRes(lua_State* L) noexcept
		{
			ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 1));
			const char* tResourceName = luaL_checkstring(L, 2);
			// 先在全局池中寻找再到关卡池中找
			if (LRES.GetResourcePool(ResourcePoolType::Global)->CheckResourceExists(tResourceType, tResourceName))
				lua_pushstring(L, "global");
			else if (LRES.GetResourcePool(ResourcePoolType::Stage)->CheckResourceExists(tResourceType, tResourceName))
				lua_pushstring(L, "stage");
			else
				lua_pushnil(L);
			return 1;
		}
		static int EnumRes(lua_State* L) noexcept
		{
			ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 1));
			LRES.GetResourcePool(ResourcePoolType::Global)->ExportResourceList(L, tResourceType);
			LRES.GetResourcePool(ResourcePoolType::Stage)->ExportResourceList(L, tResourceType);
			return 2;
		}

		static int SetImageScale(lua_State* L) noexcept
		{
			if (lua_gettop(L) <= 1)
			{
				float x = static_cast<float>(luaL_checknumber(L, 1));
				if (x == 0.f)
					return luaL_error(L, "invalid argument #1 for 'SetImageScale'.");
				LRES.SetGlobalImageScaleFactor(x);
			}
			else
			{
				ResSprite* p = LRES.FindSprite(luaL_checkstring(L, 1));
				if (!p)
					return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
				float x = (float)luaL_checknumber(L, 2);
				p->GetSprite()->SetScale(x);
			}
			return 0;
		}
		static int GetImageScale(lua_State* L) noexcept
		{
			if (lua_gettop(L) <= 0)
			{
				lua_Number ret = LRES.GetGlobalImageScaleFactor();
				lua_pushnumber(L, ret);
				return 1;
			}
			else
			{
				ResSprite* p = LRES.FindSprite(luaL_checkstring(L, 1));
				if (!p)
					return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
				lua_pushnumber(L, p->GetSprite()->GetScale());
				return 1;
			}
		}
		static int SetImageState(lua_State* L) noexcept
		{
			ResSprite* p = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3)
				p->GetSprite()->SetColor(*static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR)));
			else if (lua_gettop(L) == 6)
			{
				fcyColor tColors[] = {
					*static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 4, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 5, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 6, LUASTG_LUA_TYPENAME_COLOR))
				};
				p->GetSprite()->SetColor(tColors);
			}
			return 0;
		}
		static int SetImageCenter(lua_State* L) noexcept
		{
			ResSprite* p = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
			p->GetSprite()->SetHotSpot(fcyVec2(
				static_cast<float>(luaL_checknumber(L, 2) + p->GetSprite()->GetTexRect().a.x),
				static_cast<float>(luaL_checknumber(L, 3) + p->GetSprite()->GetTexRect().a.y)));
			return 0;
		}

		static int SetAnimationScale(lua_State* L) noexcept
		{
			ResAnimation* p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));
			float x = (float)luaL_checknumber(L, 2);
			for (size_t i = 0; i < p->GetCount(); ++i)
				p->GetSprite(i)->SetScale(x);
			return 0;
		}
		static int GetAnimationScale(lua_State* L) noexcept
		{
			ResAnimation* p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));
			lua_pushnumber(L, p->GetSprite(0)->GetScale());
			return 1;
		}
		static int SetAnimationState(lua_State* L) noexcept
		{
			ResAnimation* p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3)
			{
				fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR));
				for (size_t i = 0; i < p->GetCount(); ++i)
					p->GetSprite(i)->SetColor(c);
			}
			else if (lua_gettop(L) == 6)
			{
				fcyColor tColors[] = {
					*static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 4, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 5, LUASTG_LUA_TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 6, LUASTG_LUA_TYPENAME_COLOR))
				};
				for (size_t i = 0; i < p->GetCount(); ++i)
					p->GetSprite(i)->SetColor(tColors);
			}
			return 0;
		}
		static int SetAnimationCenter(lua_State* L) noexcept
		{
			ResAnimation* p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));
			for (size_t i = 0; i < p->GetCount(); ++i)
			{
				p->GetSprite(i)->SetHotSpot(fcyVec2(
					static_cast<float>(luaL_checknumber(L, 2) + p->GetSprite(i)->GetTexRect().a.x),
					static_cast<float>(luaL_checknumber(L, 3) + p->GetSprite(i)->GetTexRect().a.y)));
			}
			return 0;
		}

		static int SetFontState(lua_State* L) noexcept
		{
			ResFont* p = LRES.FindSpriteFont(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "sprite font '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3)
			{
				fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, 3, LUASTG_LUA_TYPENAME_COLOR));
				p->SetBlendColor(c);
			}
			return 0;
		}

		static int CacheTTFString(lua_State* L) {
			size_t len = 0;
			const char* str = luaL_checklstring(L, 2, &len);
			LRES.CacheTTFFontString(luaL_checkstring(L, 1), str, len);
			return 0;
		}
	};

	luaL_Reg const lib[] = {
		{ "SetResLoadInfo", &Wrapper::SetResLoadInfo },
		{ "SetResourceStatus", &Wrapper::SetResourceStatus },
		{ "GetResourceStatus", &Wrapper::GetResourceStatus },
		{ "LoadTexture", &Wrapper::LoadTexture },
		{ "LoadImage", &Wrapper::LoadSprite },
		{ "LoadAnimation", &Wrapper::LoadAnimation },
		{ "LoadPS", &Wrapper::LoadPS },
		{ "LoadSound", &Wrapper::LoadSound },
		{ "LoadMusic", &Wrapper::LoadMusic },
		{ "LoadFont", &Wrapper::LoadFont },
		{ "LoadTTF", &Wrapper::LoadTTF },
		{ "LoadTrueTypeFont", &Wrapper::LoadTrueTypeFont },
		{ "LoadFX", &Wrapper::LoadFX },
		{ "LoadModel", &Wrapper::LoadModel },
		{ "CreateRenderTarget", &Wrapper::CreateRenderTarget },
		{ "IsRenderTarget", &Wrapper::IsRenderTarget },
		{ "SetTexturePreMulAlphaState", &Wrapper::SetTexturePreMulAlphaState },
		{ "GetTextureSize", &Wrapper::GetTextureSize },
		{ "RemoveResource", &Wrapper::RemoveResource },
		{ "CheckRes", &Wrapper::CheckRes },
		{ "EnumRes", &Wrapper::EnumRes },

		{ "SetImageScale", &Wrapper::SetImageScale },
		{ "GetImageScale", &Wrapper::GetImageScale },
		{ "SetImageState", &Wrapper::SetImageState },
		{ "SetImageCenter", &Wrapper::SetImageCenter },

		{ "SetAnimationScale", &Wrapper::SetAnimationScale },
		{ "GetAnimationScale", &Wrapper::GetAnimationScale },
		{ "SetAnimationState", &Wrapper::SetAnimationState },
		{ "SetAnimationCenter", &Wrapper::SetAnimationCenter },

		{ "SetFontState", &Wrapper::SetFontState },

		{ "CacheTTFString", &Wrapper::CacheTTFString },
		{ NULL, NULL },
	};

	luaL_Reg const lib_empty[] = {
		{ NULL, NULL },
	};

	luaL_register(L, LUASTG_LUA_LIBNAME, lib);                    // ??? lstg
	luaL_register(L, LUASTG_LUA_LIBNAME ".ResourceManager", lib); // ??? lstg lstg.ResourceManager
	lua_setfield(L, -1, "ResourceManager");                       // ??? lstg
	lua_pop(L, 1);                                                // ???
}
