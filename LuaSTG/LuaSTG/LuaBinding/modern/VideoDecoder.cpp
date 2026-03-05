#include "VideoDecoder.hpp"
#include "Texture2D.hpp"
#include "VideoBindingHelpers.hpp"
#include "lua/plus.hpp"
#include "AppFrame.h"
#include "core/VideoDecoder.hpp"

namespace luastg::binding {
	const std::string_view VideoDecoder::class_name{ "lstg.VideoDecoder" };

	struct VideoDecoderBinding : VideoDecoder {
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
			ctx.push_value(self->data->getVideoSize().x);
			return 1;
		}
		static int getHeight(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getVideoSize().y);
			return 1;
		}

		static int getDuration(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getDuration());
			return 1;
		}
		static int getCurrentTime(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getCurrentTime());
			return 1;
		}
		static int getFPS(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const interval = self->data->getFrameInterval();
			ctx.push_value(interval > 0.0 ? 1.0 / interval : 0.0);
			return 1;
		}
		static int isLooping(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->isLooping());
			return 1;
		}
		static int getTexture(lua_State* vm) {
			auto const self = as(vm, 1);
			auto const texture = Texture2D::create(vm);
			texture->data = self->data->getTexture();
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
			ctx.push_value(self->data->seek(time));
			return 1;
		}
		static int update(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const time = ctx.get_value<double>(2);
			ctx.push_value(self->data->updateToTime(time));
			return 1;
		}
		static int setLooping(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const loop = ctx.get_value<bool>(2);
			self->data->setLooping(loop);
			return 0;
		}
		static int setLoopRange(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			auto const loop_end = ctx.get_value<double>(2);
			auto const loop_duration = ctx.get_value<double>(3);
			self->data->setLoopRange(loop_end, loop_duration);
			return 0;
		}
		static int getLoopRange(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			double loop_end = 0.0, loop_duration = 0.0;
			self->data->getLoopRange(&loop_end, &loop_duration);
			ctx.push_value(loop_end);
			ctx.push_value(loop_duration);
			return 2;
		}

		// method - stream info

		static int getVideoStreams(lua_State* vm) {
			auto const self = as(vm, 1);
			video::pushVideoStreamsToLua(vm, self->data);
			return 1;
		}
		static int getAudioStreams(lua_State* vm) {
			auto const self = as(vm, 1);
			video::pushAudioStreamsToLua(vm, self->data);
			return 1;
		}
		static int getVideoStreamIndex(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			ctx.push_value(self->data->getVideoStreamIndex());
			return 1;
		}
		static int reopen(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const self = as(vm, 1);
			core::VideoOpenOptions opt = self->data->getLastOpenOptions();
			if (ctx.index_of_top() >= 2) {
				video::parseVideoOptions(vm, 2, opt);
			}
			ctx.push_value(self->data->reopen(opt));
			return 1;
		}

		// static method

		static int create(lua_State* vm) {
			lua::stack_t const ctx(vm);
			auto const path = ctx.get_value<std::string_view>(1);
			
			core::VideoOpenOptions opt{};
			if (ctx.index_of_top() >= 2) {
				video::parseVideoOptions(vm, 2, opt);
			}

			core::SmartReference<core::IVideoDecoder> decoder;
			if (!LAPP.getGraphicsDevice()->createVideoDecoder(decoder.put())) {
				return luaL_error(vm, "create VideoDecoder from file '%s' failed", path.data());
			}

			if (!decoder->open(path)) {
				return luaL_error(vm, "create VideoDecoder from file '%s' failed", path.data());
			}

			auto const self = VideoDecoder::create(vm);
			self->data = decoder.detach();
			return 1;
		}
	};

	bool VideoDecoder::is(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.is_metatable(index, class_name);
	}
	VideoDecoder* VideoDecoder::as(lua_State* vm, int const index) {
		lua::stack_t const ctx(vm);
		return ctx.as_userdata<VideoDecoder>(index);
	}
	VideoDecoder* VideoDecoder::create(lua_State* vm) {
		lua::stack_t const ctx(vm);
		auto const self = ctx.create_userdata<VideoDecoder>();
		auto const self_index = ctx.index_of_top();
		ctx.set_metatable(self_index, class_name);
		self->data = nullptr;
		return self;
	}
	void VideoDecoder::registerClass(lua_State* vm) {
		[[maybe_unused]] lua::stack_balancer_t stack_balancer(vm);
		lua::stack_t const ctx(vm);

		// method

		auto const method_table = ctx.create_module(class_name);
		// video info
		ctx.set_map_value(method_table, "getWidth", &VideoDecoderBinding::getWidth);
		ctx.set_map_value(method_table, "getHeight", &VideoDecoderBinding::getHeight);
		ctx.set_map_value(method_table, "getDuration", &VideoDecoderBinding::getDuration);
		ctx.set_map_value(method_table, "getCurrentTime", &VideoDecoderBinding::getCurrentTime);
		ctx.set_map_value(method_table, "getFPS", &VideoDecoderBinding::getFPS);
		ctx.set_map_value(method_table, "isLooping", &VideoDecoderBinding::isLooping);
		ctx.set_map_value(method_table, "getTexture", &VideoDecoderBinding::getTexture);
		// playback control
		ctx.set_map_value(method_table, "seek", &VideoDecoderBinding::seek);
		ctx.set_map_value(method_table, "update", &VideoDecoderBinding::update);
		ctx.set_map_value(method_table, "setLooping", &VideoDecoderBinding::setLooping);
		ctx.set_map_value(method_table, "setLoopRange", &VideoDecoderBinding::setLoopRange);
		ctx.set_map_value(method_table, "getLoopRange", &VideoDecoderBinding::getLoopRange);
		// stream info
		ctx.set_map_value(method_table, "getVideoStreams", &VideoDecoderBinding::getVideoStreams);
		ctx.set_map_value(method_table, "getAudioStreams", &VideoDecoderBinding::getAudioStreams);
		ctx.set_map_value(method_table, "getVideoStreamIndex", &VideoDecoderBinding::getVideoStreamIndex);
		ctx.set_map_value(method_table, "reopen", &VideoDecoderBinding::reopen);
		// static method
		ctx.set_map_value(method_table, "create", &VideoDecoderBinding::create);

		// metatable

		auto const metatable = ctx.create_metatable(class_name);
		ctx.set_map_value(metatable, "__gc", &VideoDecoderBinding::__gc);
		ctx.set_map_value(metatable, "__tostring", &VideoDecoderBinding::__tostring);
		ctx.set_map_value(metatable, "__eq", &VideoDecoderBinding::__eq);
		ctx.set_map_value(metatable, "__index", method_table);
	}
}
