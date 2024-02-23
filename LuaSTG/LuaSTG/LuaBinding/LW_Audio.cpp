#include "LuaBinding/LuaWrapper.hpp"
#include "LuaBinding/lua_utility.hpp"
#include "AppFrame.h"

// 微软我日你仙人
#ifdef PlaySound
#undef PlaySound
#endif

void LuaSTGPlus::LuaWrapper::AudioWrapper::Register(lua_State* L)noexcept
{
	struct Wrapper
	{
		static int ListAudioDevice(lua_State* L)
		{
			if (!LAPP.GetAppModel()->getAudioDevice())
			{
				return luaL_error(L, "engine not initialized");
			}
			lua::stack_t S(L);
			auto const refresh = S.get_value<bool>(1);
			uint32_t const count = LAPP.GetAppModel()->getAudioDevice()->getAudioDeviceCount(refresh);
			S.create_array(count);
			for (uint32_t i = 0; i < count; i += 1)
			{
				auto const name = LAPP.GetAppModel()->getAudioDevice()->getAudioDeviceName(i);
				S.set_array_value_zero_base<std::string_view>(i, name);
			}
			return 1;
		}
		static int ChangeAudioDevice(lua_State* L)
		{
			if (!LAPP.GetAppModel()->getAudioDevice())
			{
				return luaL_error(L, "engine not initialized");
			}
			lua::stack_t S(L);
			auto const name = S.get_value<std::string_view>(1);
			auto const result = LAPP.GetAppModel()->getAudioDevice()->setTargetAudioDevice(name);
			S.push_value<bool>(result);
			return 1;
		}
		static int GetCurrentAudioDeviceName(lua_State* L)
		{
			if (!LAPP.GetAppModel()->getAudioDevice())
			{
				return luaL_error(L, "engine not initialized");
			}
			lua::stack_t S(L);
			auto const result = LAPP.GetAppModel()->getAudioDevice()->getCurrentAudioDeviceName();
			S.push_value<std::string_view>(result);
			return 1;
		}

		static int PlaySound(lua_State* L)noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceSoundEffect> p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Play((float)luaL_checknumber(L, 2), (float)luaL_optnumber(L, 3, 0.0));
			return 0;
		}
		static int StopSound(lua_State* L)noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceSoundEffect> p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Stop();
			return 0;
		}
		static int PauseSound(lua_State* L)noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceSoundEffect> p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Pause();
			return 0;
		}
		static int ResumeSound(lua_State* L)noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceSoundEffect> p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Resume();
			return 0;
		}
		static int GetSoundState(lua_State* L)noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceSoundEffect> p = LRES.FindSound(s);
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
		static int SetSEVolume(lua_State* L)noexcept
		{
			float v = static_cast<float>(luaL_checknumber(L, 1));
			LAPP.SetSEVolume(v);
			return 0;
		}
		static int GetSEVolume(lua_State* L)
		{
			lua_pushnumber(L, LAPP.GetSEVolume());
			return 1;
		}
		static int SetSESpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			float speed = (float)luaL_checknumber(L, 2);
			Core::ScopeObject<IResourceSoundEffect> p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			if (!p->SetSpeed(speed))
				return luaL_error(L, "Can't set sound('%s') playing speed.", s);
			return 0;
		}
		static int GetSESpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceSoundEffect> p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			lua_pushnumber(L, p->GetSpeed());
			return 1;
		}
		static int UpdateSound(lua_State*)noexcept
		{
			// 否决的方法
			return 0;
		}

		static int PlayMusic(lua_State* L)noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Play((float)luaL_optnumber(L, 2, 1.), luaL_optnumber(L, 3, 0.));
			return 0;
		}
		static int StopMusic(lua_State* L)noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Stop();
			return 0;
		}
		static int PauseMusic(lua_State* L)noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Pause();
			return 0;
		}
		static int ResumeMusic(lua_State* L)noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Resume();
			return 0;
		}
		static int GetMusicState(lua_State* L)noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(s);
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
		static int GetMusicFFT(lua_State* L)noexcept
		{
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->GetAudioPlayer()->updateFFT();
			size_t sz = p->GetAudioPlayer()->getFFTSize();
			float* fdata = p->GetAudioPlayer()->getFFT();
			if (!lua_istable(L, 2))
			{
				lua_createtable(L, (int)sz, 0);
			}
			for (int i = 0; i < (int)sz; i += 1)
			{
				lua_pushnumber(L, (lua_Number)fdata[i]);
				lua_rawseti(L, 2, i + 1);
			}
			return 1;
		}
		static int SetMusicLoopRange(lua_State* L) {
			lua::stack_t S(L);

			auto const music_name = S.get_value<std::string_view>(1);
			Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(music_name.data());
			if (!p)
				return luaL_error(L, "music '%s' not found.", music_name.data());

			// 没有第二个参数，禁用循环

			if (S.index_of_top().value <= 1 || S.is_nil(2))
			{
				MusicRoopRange range{};
				range.type = MusicRoopRangeType::Disable;
				p->SetLoopRange(range);
				return 0;
			}

			lua::stack_index_t a_range(2);
			MusicRoopRange range{};
			if (!S.is_table(a_range))
				return luaL_error(L, "invalid parameter #2, required table");

			// 按采样

			if (S.has_map_value(a_range, "start_in_samples") && S.has_map_value(a_range, "end_in_samples"))
			{
				range.type = MusicRoopRangeType::StartPointAndEndPoint;
				range.unit = MusicRoopRangeUnit::Sample;
				range.start_in_samples = S.get_map_value<uint32_t>(a_range, "start_in_samples");
				range.end_in_samples = S.get_map_value<uint32_t>(a_range, "end_in_samples");
			}
			else if(S.has_map_value(a_range, "start_in_samples") && S.has_map_value(a_range, "length_in_samples"))
			{
				range.type = MusicRoopRangeType::StartPointAndLength;
				range.unit = MusicRoopRangeUnit::Sample;
				range.start_in_samples = S.get_map_value<uint32_t>(a_range, "start_in_samples");
				range.length_in_samples = S.get_map_value<uint32_t>(a_range, "length_in_samples");
			}
			else if (S.has_map_value(a_range, "length_in_samples") && S.has_map_value(a_range, "end_in_samples"))
			{
				range.type = MusicRoopRangeType::LengthAndEndPoint;
				range.unit = MusicRoopRangeUnit::Sample;
				range.length_in_samples = S.get_map_value<uint32_t>(a_range, "length_in_samples");
				range.end_in_samples = S.get_map_value<uint32_t>(a_range, "end_in_samples");
			}
			else if (S.has_map_value(a_range, "start_in_samples"))
			{
				range.type = MusicRoopRangeType::StartPointToEnd;
				range.unit = MusicRoopRangeUnit::Sample;
				range.start_in_samples = S.get_map_value<uint32_t>(a_range, "start_in_samples");
			}
			else if (S.has_map_value(a_range, "end_in_samples"))
			{
				range.type = MusicRoopRangeType::StartToEndPoint;
				range.unit = MusicRoopRangeUnit::Sample;
				range.end_in_samples = S.get_map_value<uint32_t>(a_range, "end_in_samples");
			}

			// 按秒

			else if (S.has_map_value(a_range, "start_in_seconds") && S.has_map_value(a_range, "end_in_seconds"))
			{
				range.type = MusicRoopRangeType::StartPointAndEndPoint;
				range.unit = MusicRoopRangeUnit::Second;
				range.start_in_seconds = S.get_map_value<double>(a_range, "start_in_seconds");
				range.end_in_seconds = S.get_map_value<double>(a_range, "end_in_seconds");
			}
			else if (S.has_map_value(a_range, "start_in_seconds") && S.has_map_value(a_range, "length_in_seconds"))
			{
				range.type = MusicRoopRangeType::StartPointAndLength;
				range.unit = MusicRoopRangeUnit::Second;
				range.start_in_seconds = S.get_map_value<double>(a_range, "start_in_seconds");
				range.length_in_seconds = S.get_map_value<double>(a_range, "length_in_seconds");
			}
			else if (S.has_map_value(a_range, "length_in_seconds") && S.has_map_value(a_range, "end_in_seconds"))
			{
				range.type = MusicRoopRangeType::LengthAndEndPoint;
				range.unit = MusicRoopRangeUnit::Second;
				range.length_in_seconds = S.get_map_value<double>(a_range, "length_in_seconds");
				range.end_in_seconds = S.get_map_value<double>(a_range, "end_in_seconds");
			}
			else if (S.has_map_value(a_range, "start_in_seconds"))
			{
				range.type = MusicRoopRangeType::StartPointToEnd;
				range.unit = MusicRoopRangeUnit::Second;
				range.start_in_seconds = S.get_map_value<double>(a_range, "start_in_seconds");
			}
			else if (S.has_map_value(a_range, "end_in_seconds"))
			{
				range.type = MusicRoopRangeType::StartToEndPoint;
				range.unit = MusicRoopRangeUnit::Second;
				range.end_in_seconds = S.get_map_value<double>(a_range, "end_in_seconds");
			}

			// 全曲循环

			else
			{
				range.type = MusicRoopRangeType::All;
			}

			p->SetLoopRange(range);
			return 0;
		}
		static int SetBGMVolume(lua_State* L)noexcept
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
				Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(s);
				if (!p)
					return luaL_error(L, "music '%s' not found.", s);
				p->SetVolume(x);
			}
			return 0;
		}
		static int GetBGMVolume(lua_State* L)noexcept
		{
			if (lua_gettop(L) == 0)
			{
				lua_pushnumber(L, LAPP.GetBGMVolume());
			}
			else if (lua_gettop(L) == 1)
			{
				const char* s = luaL_checkstring(L, 1);
				Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(s);
				if (!p)
					return luaL_error(L, "music '%s' not found.", s);
				lua_pushnumber(L, p->GetVolume());
			}
			return 1;
		}
		static int SetBGMSpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			float speed = (float)luaL_checknumber(L, 2);
			Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			if (!p->SetSpeed(speed))
				return luaL_error(L, "Can't set music('%s') playing speed.", s);
			return 0;
		}
		static int GetBGMSpeed(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			lua_pushnumber(L, p->GetSpeed());
			return 1;
		}
		static int SetBGMLoop(lua_State* L) {
			const char* s = luaL_checkstring(L, 1);
			bool loop = lua_toboolean(L, 2);
			Core::ScopeObject<IResourceMusic> p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->SetLoop(loop);
			return 0;
		}
	};

	luaL_Reg const lib[] = {
		{ "ListAudioDevice", &Wrapper::ListAudioDevice },
		{ "ChangeAudioDevice", &Wrapper::ChangeAudioDevice },
		{ "GetCurrentAudioDeviceName", &Wrapper::GetCurrentAudioDeviceName },

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
		{ "SetMusicLoopRange", &Wrapper::SetMusicLoopRange },
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
