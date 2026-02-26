#include "Video.hpp"
#include "Texture2D.hpp"
#include "VideoBindingHelpers.hpp"
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
		
		template<typename Func>
		static int withDecoder(lua_State* vm, Func&& func) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = video::getDecoderFromTexture(self->data)) {
				return func(ctx, decoder);
			}
			ctx.push_value(0.0);
			return 1;
		}
		
		static int getDuration(lua_State* vm) {
			return withDecoder(vm, [](auto& ctx, auto decoder) {
				ctx.push_value(decoder->getDuration());
				return 1;
			});
		}
		static int getCurrentTime(lua_State* vm) {
			return withDecoder(vm, [](auto& ctx, auto decoder) {
				ctx.push_value(decoder->getCurrentTime());
				return 1;
			});
		}
		static int getFPS(lua_State* vm) {
			return withDecoder(vm, [](auto& ctx, auto decoder) {
				auto interval = decoder->getFrameInterval();
				ctx.push_value(interval > 0.0 ? 1.0 / interval : 0.0);
				return 1;
			});
		}
		static int isLooping(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = video::getDecoderFromTexture(self->data)) {
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
			if (auto decoder = video::getDecoderFromTexture(self->data)) {
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
			if (auto decoder = video::getDecoderFromTexture(self->data)) {
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
			if (auto decoder = video::getDecoderFromTexture(self->data)) {
				decoder->setLooping(loop);
			}
			return 0;
		}
		static int setLoopRange(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const loop_end = ctx.get_value<double>(2);
			auto const loop_duration = ctx.get_value<double>(3);
			if (auto decoder = video::getDecoderFromTexture(self->data)) {
				decoder->setLoopRange(loop_end, loop_duration);
			}
			return 0;
		}
		static int getLoopRange(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = video::getDecoderFromTexture(self->data)) {
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
			if (auto decoder = video::getDecoderFromTexture(self->data)) {
				video::pushVideoStreamsToLua(vm, decoder);
			} else {
				lua_createtable(vm, 0, 0);
			}
			return 1;
		}
		static int getAudioStreams(lua_State* vm) {
			auto const self = as(vm, 1);
			if (auto decoder = video::getDecoderFromTexture(self->data)) {
				video::pushAudioStreamsToLua(vm, decoder);
			} else {
				lua_createtable(vm, 0, 0);
			}
			return 1;
		}
		static int getVideoStreamIndex(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = video::getDecoderFromTexture(self->data)) {
				ctx.push_value(decoder->getVideoStreamIndex());
			} else {
				ctx.push_value(0);
			}
			return 1;
		}
		static int reopen(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			if (auto decoder = video::getDecoderFromTexture(self->data)) {
				core::VideoOpenOptions opt = decoder->getLastOpenOptions();
				if (lua_gettop(vm) >= 2) {
					video::parseVideoOptions(vm, 2, opt);
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
			if (lua_gettop(vm) >= 2) {
				video::parseVideoOptions(vm, 2, opt);
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
