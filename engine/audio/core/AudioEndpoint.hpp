#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/AudioDecoder.hpp"
#include "core/AudioPlayer.hpp"

namespace core {
	enum class AudioMixingChannel : uint8_t {
		direct = 0,
		sound_effect = 1,
		music = 2,

		count = 3,
	};

	struct CORE_NO_VIRTUAL_TABLE IAudioEndpointEventListener {
		virtual void onAudioEndpointCreate() = 0;
		virtual void onAudioEndpointDestroy() = 0;
	};

	struct CORE_NO_VIRTUAL_TABLE IAudioEndpoint : IReferenceCounted {
		virtual void addEventListener(IAudioEndpointEventListener* listener) = 0;
		virtual void removeEventListener(IAudioEndpointEventListener* listener) = 0;

		[[nodiscard]] virtual bool refreshAudioEndpoint() = 0;
		[[nodiscard]] virtual uint32_t getAudioEndpointCount() const noexcept = 0;
		[[nodiscard]] virtual std::string_view getAudioEndpointName(uint32_t index) const noexcept = 0;

		virtual void setPreferredAudioEndpoint(std::string_view name) = 0;
		[[nodiscard]] virtual bool setAudioEndpoint(std::string_view name) = 0;
		[[nodiscard]] virtual std::string_view getCurrentAudioEndpointName() const noexcept = 0;

		virtual void setVolume(float volume) = 0;
		[[nodiscard]] virtual float getVolume() const noexcept = 0;
		virtual void setMixingChannelVolume(AudioMixingChannel channel, float volume) = 0;
		[[nodiscard]] virtual float getMixingChannelVolume(AudioMixingChannel channel) const noexcept = 0;

		[[nodiscard]] virtual bool createAudioPlayer(IAudioDecoder* decoder, AudioMixingChannel channel, IAudioPlayer** output_player) = 0;
		[[nodiscard]] virtual bool createStreamAudioPlayer(IAudioDecoder* decoder, AudioMixingChannel channel, IAudioPlayer** output_player) = 0;

		[[nodiscard]] static bool create(IAudioEndpoint** output_endpoint);
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IAudioEndpoint
	template<> constexpr InterfaceId getInterfaceId<IAudioEndpoint>() { return UUID::parse("323bb2d0-e6d4-5811-bc7c-0c9cdf676dee"); }
}
