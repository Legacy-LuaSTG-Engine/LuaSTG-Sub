#include "backend/AudioEngineXAudio2.hpp"
#include "core/Configuration.hpp"
#include "core/Logger.hpp"
#include "backend/AudioPlayerXAudio2.hpp"
#include "backend/StreamAudioPlayerXAudio2.hpp"
#include "utf8.hpp"
#include <ranges>

using std::string_view_literals::operator ""sv;

namespace core {
	void AudioEngineXAudio2::addEventListener(IAudioEngineEventListener* const listener) {
		for (auto const v : m_listeners) {
			if (v == listener) {
				return;
			}
		}
		m_listeners.push_back(listener);
	}
	void AudioEngineXAudio2::removeEventListener(IAudioEngineEventListener* const listener) {
		for (auto& v : m_listeners) {
			if (v == listener) {
				v = nullptr;
			}
		}
	}

	uint32_t AudioEngineXAudio2::getAudioEndpointCount() const noexcept {
		return static_cast<uint32_t>(m_endpoints.size());
	}
	std::string_view AudioEngineXAudio2::getAudioEndpointName(uint32_t const index) const noexcept {
		if (index >= m_endpoints.size()) {
			return "";
		}
		return m_endpoints[index].name;
	}

	void AudioEngineXAudio2::setPreferredAudioEndpoint(std::string_view const name) {
		m_preferred_endpoint = name;
	}
	bool AudioEngineXAudio2::setAudioEndpoint(std::string_view const name) {
		destroy();
		setPreferredAudioEndpoint(name);
		return create();
	}
	std::string_view AudioEngineXAudio2::getCurrentAudioEndpointName() const noexcept {
		return m_current_endpoint;
	}

	void AudioEngineXAudio2::setVolume(float const volume) {
		setMixingChannelVolume(AudioMixingChannel::direct, volume);
	}
	float AudioEngineXAudio2::getVolume() const noexcept {
		return getMixingChannelVolume(AudioMixingChannel::direct);
	}
	void AudioEngineXAudio2::setMixingChannelVolume(AudioMixingChannel const channel, float const volume) {
		m_mixing_channel_volumes[static_cast<size_t>(channel)] = std::clamp(volume, 0.0f, 1.0f);
		if (auto const voice = m_mixing_channels[static_cast<size_t>(channel)]; voice != nullptr) {
			win32::check_hresult(voice->SetVolume(volume), "IXAudio2Voice::SetVolume"sv);
		}
	}
	float AudioEngineXAudio2::getMixingChannelVolume(AudioMixingChannel const channel) const noexcept {
		return m_mixing_channel_volumes[static_cast<size_t>(channel)];
	}

	bool AudioEngineXAudio2::createAudioPlayer(IAudioDecoder* const decoder, AudioMixingChannel const channel, IAudioPlayer** const output_player) {
		try {
			SmartReference<AudioPlayerXAudio2> player;
			player.attach(new AudioPlayerXAudio2);
			if (!player->create(this, channel, decoder)) {
				return false;
			}
			*output_player = player.detach();
			return true;
		}
		catch (std::exception const& e) {
			Logger::error("[core] create AudioPlayerXAudio2 failed: {}", e.what());
			return false;
		}
	}
	bool AudioEngineXAudio2::createStreamAudioPlayer(IAudioDecoder* const decoder, AudioMixingChannel const channel, IAudioPlayer** const output_player) {
		try {
			SmartReference<StreamAudioPlayerXAudio2> player;
			player.attach(new StreamAudioPlayerXAudio2);
			if (!player->create(this, channel, decoder)) {
				return false;
			}
			*output_player = player.detach();
			return true;
		}
		catch (std::exception const& e) {
			Logger::error("[core] create StreamAudioPlayerXAudio2 failed: {}", e.what());
			return false;
		}
	}

	AudioEngineXAudio2::AudioEngineXAudio2() {
		for (auto& volume : m_mixing_channel_volumes) {
			volume = 1.0f;
		}
		auto const& config = ConfigurationLoader::getInstance().getAudioSystem();
		m_preferred_endpoint = config.getPreferredEndpointName();
		m_mixing_channel_volumes[static_cast<size_t>(AudioMixingChannel::sound_effect)] = config.getSoundEffectVolume();
		m_mixing_channel_volumes[static_cast<size_t>(AudioMixingChannel::music)] = config.getMusicVolume();
	}
	AudioEngineXAudio2::~AudioEngineXAudio2() {
		destroy();
	}

	bool AudioEngineXAudio2::create() {
		try {
			// com

			if (auto const hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED); hr != S_OK && hr != S_FALSE) {
				win32::check_hresult(hr, "CoInitializeEx"sv);
				return false;
			}

			// endpoint

			if (!win32::check_hresult_as_boolean(XAudio2Create(m_endpoint.put()), "XAudio2Create"sv)) {
				return false;
			}

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
				auto const endpoint_id_wide = utf8::to_wstring(endpoint_id);
				if (!win32::check_hresult_as_boolean(
					m_endpoint->CreateMasteringVoice(
						&direct,
						XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0U,
						endpoint_id_wide.c_str()),
					"IXAudio2::CreateMasteringVoice"sv
				)) {
					return false;
				}
			}
			else {
				if (!win32::check_hresult_as_boolean(m_endpoint->CreateMasteringVoice(&direct), "IXAudio2::CreateMasteringVoice"sv)) {
					return false;
				}
			}
			m_mixing_channels[static_cast<size_t>(AudioMixingChannel::direct)] = direct;

			// fixed, 2 channel, 44100hz sample rate

			XAUDIO2_VOICE_DETAILS direct_info{};
			direct->GetVoiceDetails(&direct_info);

			IXAudio2SubmixVoice* sound_effect{};
			if (!win32::check_hresult_as_boolean(
				m_endpoint->CreateSubmixVoice(&sound_effect, 2, direct_info.InputSampleRate),
				"IXAudio2::CreateSubmixVoice"sv
			)) {
				return false;
			}
			m_mixing_channels[static_cast<size_t>(AudioMixingChannel::sound_effect)] = sound_effect;

			IXAudio2SubmixVoice* music{};
			if (!win32::check_hresult_as_boolean(
				m_endpoint->CreateSubmixVoice(&music, 2, direct_info.InputSampleRate),
				"IXAudio2::CreateSubmixVoice"sv
			)) {
				return false;
			}
			m_mixing_channels[static_cast<size_t>(AudioMixingChannel::music)] = music;

			// build graph

			XAUDIO2_SEND_DESCRIPTOR voice_send_master = {};
			voice_send_master.pOutputVoice = direct;
			XAUDIO2_VOICE_SENDS voice_send_list{};
			voice_send_list.SendCount = 1;
			voice_send_list.pSends = &voice_send_master;

			if (!win32::check_hresult_as_boolean(sound_effect->SetOutputVoices(&voice_send_list), "IXAudio2SubmixVoice::SetOutputVoices"sv)) {
				return false;
			}
			if (!win32::check_hresult_as_boolean(music->SetOutputVoices(&voice_send_list), "IXAudio2SubmixVoice::SetOutputVoices"sv)) {
				return false;
			}

			// update volume
			if (!win32::check_hresult_as_boolean(direct->SetVolume(getMixingChannelVolume(AudioMixingChannel::direct)), "IXAudio2MasteringVoice::SetOutputVoices"sv)) {
				return false;
			}
			if (!win32::check_hresult_as_boolean(sound_effect->SetVolume(getMixingChannelVolume(AudioMixingChannel::sound_effect)), "IXAudio2SubmixVoice::SetVolume"sv)) {
				return false;
			}
			if (!win32::check_hresult_as_boolean(music->SetVolume(getMixingChannelVolume(AudioMixingChannel::music)), "IXAudio2SubmixVoice::SetVolume"sv)) {
				return false;
			}

			m_current_endpoint = endpoint_name;
			dispatchOnAudioEndpointCreate();
			return true;
		}
		catch (std::exception const& e) {
			Logger::error("[core] create audio endpoint failed <std::exception> {}", e.what());
		}
		return false;
	}
	void AudioEngineXAudio2::destroy() {
		dispatchOnAudioEndpointDestroy();
		m_current_endpoint.clear();
		closeMixingChannels();
		m_endpoint = nullptr;
	}
	void AudioEngineXAudio2::dispatchOnAudioEndpointCreate() {
		bool has_nullptr{};
		for (auto const listener : m_listeners) {
			if (listener == nullptr) {
				has_nullptr = true;
				continue;
			}
			listener->onAudioEngineCreate();
		}
		if (!has_nullptr) {
			return;
		}
		std::vector<IAudioEngineEventListener*> listeners;
		listeners.reserve(m_listeners.size());
		for (auto const listener : m_listeners) {
			if (listener == nullptr) {
				continue;
			}
			listeners.push_back(listener);
		}
		std::swap(m_listeners, listeners);
	}
	void AudioEngineXAudio2::dispatchOnAudioEndpointDestroy() {
		bool has_nullptr{};
		for (auto const listener : m_listeners) {
			if (listener == nullptr) {
				has_nullptr = true;
				continue;
			}
			listener->onAudioEngineDestroy();
		}
		if (!has_nullptr) {
			return;
		}
		std::vector<IAudioEngineEventListener*> listeners;
		listeners.reserve(m_listeners.size());
		for (auto const listener : m_listeners) {
			if (listener == nullptr) {
				continue;
			}
			listeners.push_back(listener);
		}
		std::swap(m_listeners, listeners);
	}

	IXAudio2MasteringVoice* AudioEngineXAudio2::getDirectChannel() const noexcept {
		return static_cast<IXAudio2MasteringVoice*>(m_mixing_channels[static_cast<size_t>(AudioMixingChannel::direct)]);
	}
	IXAudio2SubmixVoice* AudioEngineXAudio2::getSoundEffectChannel() const noexcept {
		return static_cast<IXAudio2SubmixVoice*>(m_mixing_channels[static_cast<size_t>(AudioMixingChannel::sound_effect)]);
	}
	IXAudio2SubmixVoice* AudioEngineXAudio2::getMusicChannel() const noexcept {
		return static_cast<IXAudio2SubmixVoice*>(m_mixing_channels[static_cast<size_t>(AudioMixingChannel::music)]);
	}
	IXAudio2Voice* AudioEngineXAudio2::getChannel(AudioMixingChannel channel) const noexcept {
		return m_mixing_channels[static_cast<size_t>(channel)];
	}

	void AudioEngineXAudio2::closeMixingChannels() {
		for (auto& channel : m_mixing_channels | std::views::reverse) {
			if (channel == nullptr) {
				continue;
			}
			channel->DestroyVoice();
			channel = nullptr;
		}
	}

	bool IAudioEngine::create(IAudioEngine** const output_endpoint) {
		if (output_endpoint == nullptr) {
			assert(false);
			return false;
		}
		SmartReference<AudioEngineXAudio2> endpoint;
		endpoint.attach(new AudioEngineXAudio2);
		if (!endpoint->create()) {
			Logger::warn("[core] AudioEngineXAudio2::create failed");
		}
		*output_endpoint = endpoint.detach();
		return true;
	}
}
