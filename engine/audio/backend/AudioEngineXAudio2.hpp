#pragma once
#include "core/AudioEngine.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include <string>
#include <vector>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <xaudio2redist.h>
#include "win32/base.hpp"

namespace core {
	class AudioEngineXAudio2 final : public implement::ReferenceCounted<IAudioEngine> {
	public:
		// IAudioEngine

		void addEventListener(IAudioEngineEventListener* listener) override;
		void removeEventListener(IAudioEngineEventListener* listener) override;

		[[nodiscard]] bool refreshAudioEndpoint() override;
		[[nodiscard]] uint32_t getAudioEndpointCount() const noexcept override;
		[[nodiscard]] std::string_view getAudioEndpointName(uint32_t index) const noexcept override;

		void setPreferredAudioEndpoint(std::string_view name) override;
		[[nodiscard]] bool setAudioEndpoint(std::string_view name) override;
		[[nodiscard]] std::string_view getCurrentAudioEndpointName() const noexcept override;

		void setVolume(float volume) override;
		[[nodiscard]] float getVolume() const noexcept override;
		void setMixingChannelVolume(AudioMixingChannel channel, float volume) override;
		[[nodiscard]] float getMixingChannelVolume(AudioMixingChannel channel) const noexcept override;

		[[nodiscard]] bool createAudioPlayer(IAudioDecoder* decoder, AudioMixingChannel channel, IAudioPlayer** output_player) override;
		[[nodiscard]] bool createStreamAudioPlayer(IAudioDecoder* decoder, AudioMixingChannel channel, IAudioPlayer** output_player) override;

		// AudioEngineXAudio2

		AudioEngineXAudio2();
		AudioEngineXAudio2(AudioEngineXAudio2 const&) = delete;
		AudioEngineXAudio2(AudioEngineXAudio2&&) = delete;
		~AudioEngineXAudio2() override;

		AudioEngineXAudio2& operator=(AudioEngineXAudio2 const&) = delete;
		AudioEngineXAudio2& operator=(AudioEngineXAudio2&&) = delete;

		bool create();
		void destroy();
		void dispatchOnAudioEndpointCreate();
		void dispatchOnAudioEndpointDestroy();
		IXAudio2* getFactory() const noexcept { return m_endpoint.get(); }
		IXAudio2MasteringVoice* getDirectChannel() const noexcept;
		IXAudio2SubmixVoice* getSoundEffectChannel() const noexcept;
		IXAudio2SubmixVoice* getMusicChannel() const noexcept;
		IXAudio2Voice* getChannel(AudioMixingChannel channel) const noexcept;

	private:
		struct AudioEndpointInfo {
			std::string id;
			std::string name;
		};

		void closeMixingChannels();

		std::vector<IAudioEngineEventListener*> m_listeners;
		std::vector<AudioEndpointInfo> m_endpoints;
		std::string m_current_endpoint;
		std::string m_preferred_endpoint;
		win32::com_ptr<IXAudio2> m_endpoint;
		float m_mixing_channel_volumes[static_cast<size_t>(AudioMixingChannel::count)]{};
		IXAudio2Voice* m_mixing_channels[static_cast<size_t>(AudioMixingChannel::count)]{};
	};
}
