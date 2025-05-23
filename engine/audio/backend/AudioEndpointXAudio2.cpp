#include "backend/AudioEndpointXAudio2.hpp"
#include "core/Configuration.hpp"
#include "core/Logger.hpp"
#include <ranges>

namespace core {
	void AudioEndpointXAudio2::addEventListener(IAudioEndpointEventListener* const listener) {
		for (auto const v : m_listeners) {
			if (v == listener) {
				return;
			}
		}
		m_listeners.push_back(listener);
	}
	void AudioEndpointXAudio2::removeEventListener(IAudioEndpointEventListener* const listener) {
		for (auto& v : m_listeners) {
			if (v == listener) {
				v = nullptr;
			}
		}
	}

	uint32_t AudioEndpointXAudio2::getAudioEndpointCount() const noexcept {
		return static_cast<uint32_t>(m_endpoints.size());
	}
	std::string_view AudioEndpointXAudio2::getAudioEndpointName(uint32_t const index) const noexcept {
		if (index >= m_endpoints.size()) {
			return "";
		}
		return m_endpoints[index].name;
	}

	void AudioEndpointXAudio2::setPreferredAudioEndpoint(std::string_view const name) {
		m_preferred_endpoint = name;
	}
	bool AudioEndpointXAudio2::setAudioEndpoint(std::string_view const name) {
		destroy();
		setPreferredAudioEndpoint(name);
		return create();
	}
	std::string_view AudioEndpointXAudio2::getCurrentAudioEndpointName() const noexcept {
		return m_current_endpoint;
	}

	void AudioEndpointXAudio2::setVolume(float const volume) {
		setMixingChannelVolume(AudioMixingChannel::direct, volume);
	}
	float AudioEndpointXAudio2::getVolume() const noexcept {
		return getMixingChannelVolume(AudioMixingChannel::direct);
	}
	void AudioEndpointXAudio2::setMixingChannelVolume(AudioMixingChannel const channel, float const volume) {
		m_mixing_channel_volumes[static_cast<size_t>(channel)] = std::clamp(volume, 0.0f, 1.0f);
		if (auto const voice = m_mixing_channels[static_cast<size_t>(channel)]; voice != nullptr) {
			try {
				winrt::check_hresult(voice->SetVolume(volume));
			}
			catch (winrt::hresult_error const& e) {
				Logger::error("[core] set voice volume failed <winrt::hresult_error> {}", winrt::to_string(e.message()));
			}
			catch (std::exception const& e) {
				Logger::error("[core] set voice volume failed <std::exception> {}", e.what());
			}
		}
	}
	float AudioEndpointXAudio2::getMixingChannelVolume(AudioMixingChannel const channel) const noexcept {
		return m_mixing_channel_volumes[static_cast<size_t>(channel)];
	}

	bool AudioEndpointXAudio2::createAudioPlayer(IAudioDecoder* decoder, AudioMixingChannel channel, IAudioPlayer** output_player) {
		return false;
	}
	bool AudioEndpointXAudio2::createLoopAudioPlayer(IAudioDecoder* decoder, AudioMixingChannel channel, IAudioPlayer** output_player) {
		return false;
	}
	bool AudioEndpointXAudio2::createStreamAudioPlayer(IAudioDecoder* decoder, AudioMixingChannel channel, IAudioPlayer** output_player) {
		return false;
	}

	AudioEndpointXAudio2::AudioEndpointXAudio2() {
		for (auto& volume : m_mixing_channel_volumes) {
			volume = 1.0f;
		}
		auto const& config = ConfigurationLoader::getInstance().getAudioSystem();
		m_mixing_channel_volumes[static_cast<size_t>(AudioMixingChannel::sound_effect)] = config.getSoundEffectVolume();
		m_mixing_channel_volumes[static_cast<size_t>(AudioMixingChannel::music)] = config.getMusicVolume();
	}
	AudioEndpointXAudio2::~AudioEndpointXAudio2() {
		destroy();
	}

	bool AudioEndpointXAudio2::create() {
		try {
			// endpoint

			winrt::check_hresult(XAudio2Create(m_endpoint.put()));

		#ifndef NDEBUG
			XAUDIO2_DEBUG_CONFIGURATION debug_config{};
			debug_config.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_INFO | XAUDIO2_LOG_DETAIL;
			debug_config.BreakMask = XAUDIO2_LOG_ERRORS;
			debug_config.LogThreadID = TRUE;
			debug_config.LogTiming = TRUE;
			m_endpoint->SetDebugConfiguration(&debug_config);
		#endif

			// select endpoint

			std::string_view endpoint_id;
			std::string_view endpoint_name;
			for (auto const& [id, name] : m_endpoints) {
				if (name == m_preferred_endpoint) {
					endpoint_id = id;
					endpoint_name = name;
					break;
				}
			}

			IXAudio2MasteringVoice* direct{};
			if (!endpoint_id.empty()) {
				auto const endpoint_id_wide = winrt::to_hstring(endpoint_id);
				winrt::check_hresult(m_endpoint->CreateMasteringVoice(
					&direct,
					XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0U,
					endpoint_id_wide.c_str())
				);
			}
			else {
				winrt::check_hresult(m_endpoint->CreateMasteringVoice(&direct));
			}
			m_mixing_channels[static_cast<size_t>(AudioMixingChannel::direct)] = direct;

			// fixed, 2 channel, 44100hz sample rate

			XAUDIO2_VOICE_DETAILS direct_info{};
			direct->GetVoiceDetails(&direct_info);

			IXAudio2SubmixVoice* sound_effect{};
			winrt::check_hresult(m_endpoint->CreateSubmixVoice(&sound_effect, direct_info.InputChannels, direct_info.InputSampleRate));
			m_mixing_channels[static_cast<size_t>(AudioMixingChannel::sound_effect)] = sound_effect;

			IXAudio2SubmixVoice* music{};
			winrt::check_hresult(m_endpoint->CreateSubmixVoice(&music, direct_info.InputChannels, direct_info.InputSampleRate));
			m_mixing_channels[static_cast<size_t>(AudioMixingChannel::music)] = music;

			// build graph

			XAUDIO2_SEND_DESCRIPTOR voice_send_master = {};
			voice_send_master.pOutputVoice = direct;
			XAUDIO2_VOICE_SENDS voice_send_list{};
			voice_send_list.SendCount = 1;
			voice_send_list.pSends = &voice_send_master;

			winrt::check_hresult(sound_effect->SetOutputVoices(&voice_send_list));
			winrt::check_hresult(music->SetOutputVoices(&voice_send_list));

			// update volume
			winrt::check_hresult(direct->SetVolume(m_mixing_channel_volumes[static_cast<size_t>(AudioMixingChannel::direct)]));
			winrt::check_hresult(sound_effect->SetVolume(m_mixing_channel_volumes[static_cast<size_t>(AudioMixingChannel::sound_effect)]));
			winrt::check_hresult(music->SetVolume(m_mixing_channel_volumes[static_cast<size_t>(AudioMixingChannel::music)]));

			m_current_endpoint = endpoint_name;
			dispatchOnAudioEndpointCreate();
			return true;
		}
		catch (winrt::hresult_error const& e) {
			Logger::error("[core] create audio endpoint failed <winrt::hresult_error> {}", winrt::to_string(e.message()));
		}
		catch (std::exception const& e) {
			Logger::error("[core] create audio endpoint failed <std::exception> {}", e.what());
		}
		return false;
	}
	void AudioEndpointXAudio2::destroy() {
		dispatchOnAudioEndpointDestroy();
		m_current_endpoint.clear();
		closeMixingChannels();
		m_endpoint = nullptr;
	}
	void AudioEndpointXAudio2::dispatchOnAudioEndpointCreate() {
		bool has_nullptr{};
		for (auto const listener : m_listeners) {
			if (listener == nullptr) {
				has_nullptr = true;
				continue;
			}
			listener->onAudioEndpointCreate();
		}
		if (!has_nullptr) {
			return;
		}
		std::vector<IAudioEndpointEventListener*> listeners;
		listeners.reserve(m_listeners.size());
		for (auto const listener : m_listeners) {
			if (listener == nullptr) {
				continue;
			}
			listeners.push_back(listener);
		}
		std::swap(m_listeners, listeners);
	}
	void AudioEndpointXAudio2::dispatchOnAudioEndpointDestroy() {
		bool has_nullptr{};
		for (auto const listener : m_listeners) {
			if (listener == nullptr) {
				has_nullptr = true;
				continue;
			}
			listener->onAudioEndpointDestroy();
		}
		if (!has_nullptr) {
			return;
		}
		std::vector<IAudioEndpointEventListener*> listeners;
		listeners.reserve(m_listeners.size());
		for (auto const listener : m_listeners) {
			if (listener == nullptr) {
				continue;
			}
			listeners.push_back(listener);
		}
		std::swap(m_listeners, listeners);
	}

	IXAudio2MasteringVoice* AudioEndpointXAudio2::getDirectChannel() const noexcept {
		return static_cast<IXAudio2MasteringVoice*>(m_mixing_channels[static_cast<size_t>(AudioMixingChannel::direct)]);
	}
	IXAudio2SubmixVoice* AudioEndpointXAudio2::getSoundEffectChannel() const noexcept {
		return static_cast<IXAudio2SubmixVoice*>(m_mixing_channels[static_cast<size_t>(AudioMixingChannel::sound_effect)]);
	}
	IXAudio2SubmixVoice* AudioEndpointXAudio2::getMusicChannel() const noexcept {
		return static_cast<IXAudio2SubmixVoice*>(m_mixing_channels[static_cast<size_t>(AudioMixingChannel::music)]);
	}
	IXAudio2Voice* AudioEndpointXAudio2::getChannel(AudioMixingChannel channel) const noexcept {
		return m_mixing_channels[static_cast<size_t>(channel)];
	}

	void AudioEndpointXAudio2::closeMixingChannels() {
		for (auto& channel : m_mixing_channels | std::views::reverse) {
			if (channel == nullptr) {
				continue;
			}
			channel->DestroyVoice();
			channel = nullptr;
		}
	}
}
