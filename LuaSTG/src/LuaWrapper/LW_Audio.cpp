#include "LuaWrapper\LuaWrapper.hpp"
#include "AppFrame.h"
#include "LConfig.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef PlaySound
#undef PlaySound
#endif

void LuaSTGPlus::LuaWrapper::AudioWrapper::Register(lua_State* L) LNOEXCEPT
{
	struct Wrapper
	{
		static int PlaySound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Play((float)luaL_checknumber(L, 2), (float)luaL_optnumber(L, 3, 0.0));
			return 0;
		}
		static int StopSound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Stop();
			return 0;
		}
		static int PauseSound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Pause();
			return 0;
		}
		static int ResumeSound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Resume();
			return 0;
		}
		static int GetSoundState(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			if (p->IsPlaying())
				lua_pushstring(L, "playing");
			else if (p->IsStopped())
				lua_pushstring(L, "stopped");
			else
				lua_pushstring(L, "paused");
			return 1;
		}
		static int SetSEVolume(lua_State* L)LNOEXCEPT
		{
			float v = static_cast<float>(luaL_checknumber(L, 1));
			LAPP.SetSEVolume(v);
			return 0;
		}
		static int GetSEVolume(lua_State* L)
		{
			lua_pushnumber(L, LAPP.GetSoundSys()->GetSoundEffectChannelVolume());
			return 1;
		}
		static int SetSESpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			float speed = luaL_checknumber(L, 2);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			if (!p->SetSpeed(speed))
				return luaL_error(L, "Can't set sound('%s') playing speed.", s);
			return 0;
		}
		static int GetSESpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			lua_pushnumber(L, p->GetSpeed());
			return 1;
		}
		static int UpdateSound(lua_State* L)LNOEXCEPT
		{
			// 否决的方法
			return 0;
		}

		static int PlayMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Play((float)luaL_optnumber(L, 2, 1.), luaL_optnumber(L, 3, 0.));
			return 0;
		}
		static int StopMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Stop();
			return 0;
		}
		static int PauseMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Pause();
			return 0;
		}
		static int ResumeMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Resume();
			return 0;
		}
		static int GetMusicState(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			if (p->IsPlaying())
				lua_pushstring(L, "playing");
			else if (p->IsPaused())
				lua_pushstring(L, "paused");
			//else if (p->IsStopped())
				//lua_pushstring(L, "stopped");
			else
				lua_pushstring(L, "stopped");
			//lua_pushstring(L, "paused");
			return 1;
		}
		static int GetMusicFFT(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->GetAudioSource()->UpdateFFT();
			size_t sz = p->GetAudioSource()->GetFFTSize();
			float* fdata = p->GetAudioSource()->GetFFTData();
			if (!lua_istable(L, 2))
			{
				lua_createtable(L, (int)sz, 0);
			}
			for (int i = 0; i < sz; i += 1)
			{
				lua_pushnumber(L, (lua_Number)fdata[i]);
				lua_rawseti(L, 2, i + 1);
			}
			return 1;
		}
		static int SetBGMVolume(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) <= 1)
			{
				float x = static_cast<float>(luaL_checknumber(L, 1));
				LAPP.SetBGMVolume(x);
			}
			else
			{
				const char* s = luaL_checkstring(L, 1);
				float x = static_cast<float>(luaL_checknumber(L, 2));
				ResMusic* p = LRES.FindMusic(s);
				if (!p)
					return luaL_error(L, "music '%s' not found.", s);
				p->SetVolume(x);
			}
			return 0;
		}
		static int GetBGMVolume(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) == 0)
			{
				lua_pushnumber(L, LAPP.GetSoundSys()->GetMusicChannelVolume());
			}
			else if (lua_gettop(L) == 1)
			{
				const char* s = luaL_checkstring(L, 1);
				ResMusic* p = LRES.FindMusic(s);
				if (!p)
					return luaL_error(L, "music '%s' not found.", s);
				lua_pushnumber(L, p->GetVolume());
			}
			return 1;
		}
		static int SetBGMSpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			float speed = luaL_checknumber(L, 2);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			if (!p->SetSpeed(speed))
				return luaL_error(L, "Can't set music('%s') playing speed.", s);
			return 0;
		}
		static int GetBGMSpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			lua_pushnumber(L, p->GetSpeed());
			return 1;
		}
		static int SetBGMLoop(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			bool loop = lua_toboolean(L, 2);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->SetLoop(loop);
			return 0;
		}
	};

	luaL_Reg const lib[] = {
		{ "PlaySound", &Wrapper::PlaySound },
		{ "StopSound", &Wrapper::StopSound },
		{ "PauseSound", &Wrapper::PauseSound },
		{ "ResumeSound", &Wrapper::ResumeSound },
		{ "GetSoundState", &Wrapper::GetSoundState },
		{ "SetSEVolume", &Wrapper::SetSEVolume },
		{ "GetSEVolume", &Wrapper::GetSEVolume },
		{ "SetSESpeed", &Wrapper::SetSESpeed },
		{ "GetSESpeed", &Wrapper::GetSESpeed },
		{ "UpdateSound", &Wrapper::UpdateSound },

		{ "PlayMusic", &Wrapper::PlayMusic },
		{ "StopMusic", &Wrapper::StopMusic },
		{ "PauseMusic", &Wrapper::PauseMusic },
		{ "ResumeMusic", &Wrapper::ResumeMusic },
		{ "GetMusicState", &Wrapper::GetMusicState },
		{ "GetMusicFFT", &Wrapper::GetMusicFFT },
		{ "SetBGMVolume", &Wrapper::SetBGMVolume },
		{ "GetBGMVolume", &Wrapper::GetBGMVolume },
		{ "SetBGMSpeed", &Wrapper::SetBGMSpeed },
		{ "GetBGMSpeed", &Wrapper::GetBGMSpeed },
		{ "SetBGMLoop", &Wrapper::SetBGMLoop },
		{ NULL, NULL },
	};

	luaL_Reg const lib_empty[] = {
		{ NULL, NULL },
	};

	luaL_register(L, LUASTG_LUA_LIBNAME, lib);          // ??? lstg
	luaL_register(L, LUASTG_LUA_LIBNAME ".Audio", lib); // ??? lstg lstg.Audio
	lua_setfield(L, -1, "Audio");                       // ??? lstg
	lua_pop(L, 1);                                      // ???
}
