#include "Video.hpp"
#include "Texture2D.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"
#include "core/VideoDecoder.hpp"

namespace luastg::binding {
	std::string_view Video::class_name{ "lstg.Video" };

	struct VideoBinding : Video {
		// meta methods

		// NOLINTBEGIN(*-reserved-identifier)

		static int __gc(lua_State* vm) {
			if (auto const self = as(vm, 1); self->data) {
				self->data->release();
				self->data = nullptr;
			}
			return 0;
		}
		static int __tostring(lua_State* vm) {
			lua::stack_t const ctx(vm);
			[[maybe_unused]] auto const self = as(vm, 1);
			ctx.push_value(class_name);
			return 1;
		}
		static int __eq(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (is(vm, 2)) {
				auto const other = as(vm, 2);
				ctx.push_value(self->data == other->data);
			} else {
				ctx.push_value(false);
			}
			return 1;
		}

		// NOLINTEND(*-reserved-identifier)

		// helper: get video decoder from texture
		static core::IVideoDecoder* getDecoder(core::ITexture2D* texture) {
			if (!texture || !texture->isVideoTexture()) {
				return nullptr;
			}
			return texture->getVideoDecoder();
		}

		// method - video info

		static int getWidth(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getSize().x);
			return 1;
		}
		static int getHeight(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getSize().y);
			return 1;
		}
		static int getDuration(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = getDecoder(self->data)) {
				ctx.push_value(decoder->getDuration());
			} else {
				ctx.push_value(0.0);
			}
			return 1;
		}
		static int getCurrentTime(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = getDecoder(self->data)) {
				ctx.push_value(decoder->getCurrentTime());
			} else {
				ctx.push_value(0.0);
			}
			return 1;
		}
		static int getFPS(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = getDecoder(self->data)) {
				auto interval = decoder->getFrameInterval();
				ctx.push_value(interval > 0.0 ? 1.0 / interval : 0.0);
			} else {
				ctx.push_value(0.0);
			}
			return 1;
		}
		static int isLooping(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = getDecoder(self->data)) {
				ctx.push_value(decoder->isLooping());
			} else {
				ctx.push_value(false);
			}
			return 1;
		}
		static int getTexture(lua_State* vm) {
			auto const self = as(vm, 1);
			auto const texture = Texture2D::create(vm);
			texture->data = self->data;
			if (texture->data) {
				texture->data->retain();
			}
			return 1;
		}

		// method - playback control

		static int seek(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const time = ctx.get_value<double>(2);
			if (auto decoder = getDecoder(self->data)) {
				ctx.push_value(decoder->seek(time));
			} else {
				ctx.push_value(false);
			}
			return 1;
		}
		static int update(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const time = ctx.get_value<double>(2);
			if (auto decoder = getDecoder(self->data)) {
				ctx.push_value(decoder->updateToTime(time));
			} else {
				ctx.push_value(false);
			}
			return 1;
		}
		static int setLooping(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const loop = ctx.get_value<bool>(2);
			if (auto decoder = getDecoder(self->data)) {
				decoder->setLooping(loop);
			}
			return 0;
		}
		static int setLoopRange(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const loop_end = ctx.get_value<double>(2);
			auto const loop_duration = ctx.get_value<double>(3);
			if (auto decoder = getDecoder(self->data)) {
				decoder->setLoopRange(loop_end, loop_duration);
			}
			return 0;
		}
		static int getLoopRange(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = getDecoder(self->data)) {
				double loop_end = 0.0, loop_duration = 0.0;
				decoder->getLoopRange(&loop_end, &loop_duration);
				ctx.push_value(loop_end);
				ctx.push_value(loop_duration);
				return 2;
			}
			ctx.push_value(0.0);
			ctx.push_value(0.0);
			return 2;
		}

		// method - stream info

		static int getVideoStreams(lua_State* vm) {
			auto const self = as(vm, 1);
			if (auto decoder = getDecoder(self->data)) {
				std::vector<core::VideoStreamInfo> list;
				auto cb = [](core::VideoStreamInfo const& info, void* ud) {
					static_cast<std::vector<core::VideoStreamInfo>*>(ud)->push_back(info);
				};
				decoder->getVideoStreams(cb, &list);
				lua_createtable(vm, (int)list.size(), 0);
				for (size_t i = 0; i < list.size(); ++i) {
					lua_createtable(vm, 0, 5);
					lua_pushinteger(vm, (lua_Integer)list[i].index);
					lua_setfield(vm, -2, "index");
					lua_pushinteger(vm, (lua_Integer)list[i].width);
					lua_setfield(vm, -2, "width");
					lua_pushinteger(vm, (lua_Integer)list[i].height);
					lua_setfield(vm, -2, "height");
					lua_pushnumber(vm, list[i].fps);
					lua_setfield(vm, -2, "fps");
					lua_pushnumber(vm, list[i].duration_seconds);
					lua_setfield(vm, -2, "duration");
					lua_rawseti(vm, -2, (int)i + 1);
				}
			} else {
				lua_createtable(vm, 0, 0);
			}
			return 1;
		}
		static int getAudioStreams(lua_State* vm) {
			auto const self = as(vm, 1);
			if (auto decoder = getDecoder(self->data)) {
				std::vector<core::AudioStreamInfo> list;
				auto cb = [](core::AudioStreamInfo const& info, void* ud) {
					static_cast<std::vector<core::AudioStreamInfo>*>(ud)->push_back(info);
				};
				decoder->getAudioStreams(cb, &list);
				lua_createtable(vm, (int)list.size(), 0);
				for (size_t i = 0; i < list.size(); ++i) {
					lua_createtable(vm, 0, 4);
					lua_pushinteger(vm, (lua_Integer)list[i].index);
					lua_setfield(vm, -2, "index");
					lua_pushinteger(vm, (lua_Integer)list[i].channels);
					lua_setfield(vm, -2, "channels");
					lua_pushinteger(vm, (lua_Integer)list[i].sample_rate);
					lua_setfield(vm, -2, "sample_rate");
					lua_pushnumber(vm, list[i].duration_seconds);
					lua_setfield(vm, -2, "duration");
					lua_rawseti(vm, -2, (int)i + 1);
				}
			} else {
				lua_createtable(vm, 0, 0);
			}
			return 1;
		}
		static int getVideoStreamIndex(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = getDecoder(self->data)) {
				ctx.push_value(decoder->getVideoStreamIndex());
			} else {
				ctx.push_value(0);
			}
			return 1;
		}
		static int reopen(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = getDecoder(self->data)) {
				core::VideoOpenOptions opt = decoder->getLastOpenOptions();
				// 读取 options 表
				if (lua_gettop(vm) >= 2 && lua_istable(vm, 2)) {
					lua_getfield(vm, 2, "video_stream");
					if (lua_isnumber(vm, -1)) {
						opt.video_stream_index = static_cast<uint32_t>(lua_tointeger(vm, -1));
					}
					lua_pop(vm, 1);
					
					lua_getfield(vm, 2, "width");
					if (lua_isnumber(vm, -1)) {
						opt.output_width = static_cast<uint32_t>(lua_tointeger(vm, -1));
					}
					lua_pop(vm, 1);
					
					lua_getfield(vm, 2, "height");
					if (lua_isnumber(vm, -1)) {
						opt.output_height = static_cast<uint32_t>(lua_tointeger(vm, -1));
					}
					lua_pop(vm, 1);
					
					lua_getfield(vm, 2, "premultiplied_alpha");
					if (lua_isboolean(vm, -1)) {
						opt.premultiplied_alpha = lua_toboolean(vm, -1);
					}
					lua_pop(vm, 1);
					
					lua_getfield(vm, 2, "looping");
					if (lua_isboolean(vm, -1)) {
						opt.looping = lua_toboolean(vm, -1);
					}
					lua_pop(vm, 1);
					
					lua_getfield(vm, 2, "loop_end");
					if (lua_isnumber(vm, -1)) {
						opt.loop_end = lua_tonumber(vm, -1);
					}
					lua_pop(vm, 1);
					
					lua_getfield(vm, 2, "loop_duration");
					if (lua_isnumber(vm, -1)) {
						opt.loop_duration = lua_tonumber(vm, -1);
					}
					lua_pop(vm, 1);
				}
				ctx.push_value(decoder->reopen(opt));
			} else {
				ctx.push_value(false);
			}
			return 1;
		}

		// static method

		static int create(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const path = ctx.get_value<std::string_view>(1);
			
			core::VideoOpenOptions opt{};
			// 读取 options 表
			if (lua_gettop(vm) >= 2 && lua_istable(vm, 2)) {
				lua_getfield(vm, 2, "video_stream");
				if (lua_isnumber(vm, -1)) {
					opt.video_stream_index = static_cast<uint32_t>(lua_tointeger(vm, -1));
				}
				lua_pop(vm, 1);
				
				lua_getfield(vm, 2, "width");
				if (lua_isnumber(vm, -1)) {
					opt.output_width = static_cast<uint32_t>(lua_tointeger(vm, -1));
				}
				lua_pop(vm, 1);
				
				lua_getfield(vm, 2, "height");
				if (lua_isnumber(vm, -1)) {
					opt.output_height = static_cast<uint32_t>(lua_tointeger(vm, -1));
				}
				lua_pop(vm, 1);
				
				lua_getfield(vm, 2, "premultiplied_alpha");
				if (lua_isboolean(vm, -1)) {
					opt.premultiplied_alpha = lua_toboolean(vm, -1);
				}
				lua_pop(vm, 1);
				
				lua_getfield(vm, 2, "looping");
				if (lua_isboolean(vm, -1)) {
					opt.looping = lua_toboolean(vm, -1);
				}
				lua_pop(vm, 1);
				
				lua_getfield(vm, 2, "loop_end");
				if (lua_isnumber(vm, -1)) {
					opt.loop_end = lua_tonumber(vm, -1);
				}
				lua_pop(vm, 1);
				
				lua_getfield(vm, 2, "loop_duration");
				if (lua_isnumber(vm, -1)) {
					opt.loop_duration = lua_tonumber(vm, -1);
				}
				lua_pop(vm, 1);
			}

			core::SmartReference<core::ITexture2D> texture;
			if (!LAPP.getGraphicsDevice()->createVideoTexture(path, opt, texture.put())) {
				auto const error_message = std::format(
					"create Video from file '{}' failed", path);
				return luaL_error(vm, error_message.c_str());
			}
			auto const self = Video::create(vm);
			self->data = texture.detach();
			return 1;
		}
	};

	bool Video::is(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	Video* Video::as(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<Video>(index);
	}
	Video* Video::create(lua_State* vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<Video>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}
	void Video::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		// video info
		ctx.set_map_value(method_table, "getWidth", &VideoBinding::getWidth);
		ctx.set_map_value(method_table, "getHeight", &VideoBinding::getHeight);
		ctx.set_map_value(method_table, "getDuration", &VideoBinding::getDuration);
		ctx.set_map_value(method_table, "getCurrentTime", &VideoBinding::getCurrentTime);
		ctx.set_map_value(method_table, "getFPS", &VideoBinding::getFPS);
		ctx.set_map_value(method_table, "isLooping", &VideoBinding::isLooping);
		ctx.set_map_value(method_table, "getTexture", &VideoBinding::getTexture);
		// playback control
		ctx.set_map_value(method_table, "seek", &VideoBinding::seek);
		ctx.set_map_value(method_table, "update", &VideoBinding::update);
		ctx.set_map_value(method_table, "setLooping", &VideoBinding::setLooping);
		ctx.set_map_value(method_table, "setLoopRange", &VideoBinding::setLoopRange);
		ctx.set_map_value(method_table, "getLoopRange", &VideoBinding::getLoopRange);
		// stream info
		ctx.set_map_value(method_table, "getVideoStreams", &VideoBinding::getVideoStreams);
		ctx.set_map_value(method_table, "getAudioStreams", &VideoBinding::getAudioStreams);
		ctx.set_map_value(method_table, "getVideoStreamIndex", &VideoBinding::getVideoStreamIndex);
		ctx.set_map_value(method_table, "reopen", &VideoBinding::reopen);
		// static method
		ctx.set_map_value(method_table, "create", &VideoBinding::create);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__gc", &VideoBinding::__gc);
		ctx.set_map_value(metatable, "__tostring", &VideoBinding::__tostring);
		ctx.set_map_value(metatable, "__eq", &VideoBinding::__eq);
		ctx.set_map_value(metatable, "__index", method_table);
	}
}
