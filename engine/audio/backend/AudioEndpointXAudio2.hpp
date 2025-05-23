#pragma once
#include "core/AudioEndpoint.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include <string>
#include <vector>
#include <winrt/base.h>
#include <xaudio2redist.h>

namespace core {
	class AudioEndpointXAudio2 final : public implement::ReferenceCounted<IAudioEndpoint> {
	public:
		// IAudioEndpoint

		void addEventListener(IAudioEndpointEventListener* listener) override;
		void removeEventListener(IAudioEndpointEventListener* listener) override;

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
		[[nodiscard]] bool createLoopAudioPlayer(IAudioDecoder* decoder, AudioMixingChannel channel, IAudioPlayer** output_player) override;
		[[nodiscard]] bool createStreamAudioPlayer(IAudioDecoder* decoder, AudioMixingChannel channel, IAudioPlayer** output_player) override;

		// AudioEndpointXAudio2

		AudioEndpointXAudio2();
		AudioEndpointXAudio2(AudioEndpointXAudio2 const&) = delete;
		AudioEndpointXAudio2(AudioEndpointXAudio2&&) = delete;
		~AudioEndpointXAudio2() override;

		AudioEndpointXAudio2& operator=(AudioEndpointXAudio2 const&) = delete;
		AudioEndpointXAudio2& operator=(AudioEndpointXAudio2&&) = delete;

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

		std::vector<IAudioEndpointEventListener*> m_listeners;
		std::vector<AudioEndpointInfo> m_endpoints;
		std::string m_current_endpoint;
		std::string m_preferred_endpoint;
		winrt::com_ptr<IXAudio2> m_endpoint;
		float m_mixing_channel_volumes[static_cast<size_t>(AudioMixingChannel::count)]{};
		IXAudio2Voice* m_mixing_channels[static_cast<size_t>(AudioMixingChannel::count)]{};
	};
}
