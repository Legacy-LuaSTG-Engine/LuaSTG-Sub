#include <unordered_map>
#include "E2DAudioEngine.hpp"

namespace Eyes2D {
	namespace Sound {
		AudioMixerImpl::AudioMixerImpl(unsigned int id, AudioEngine* engine) {
			m_ID = id;
			m_Mixer = nullptr;

			AudioEngineImpl& real = *((AudioEngineImpl*)engine);

			XAUDIO2_SEND_DESCRIPTOR sendunit;
			sendunit.Flags = 0;
			sendunit.pOutputVoice = real.GetMasterHandle();
			XAUDIO2_VOICE_SENDS sendlist;
			sendlist.SendCount = 1;
			sendlist.pSends = &sendunit;

			HRESULT hr = real.GetEngineHandle()->CreateSubmixVoice(&m_Mixer, 2, 44100, 0, 0, &sendlist, 0);
			if (FAILED(hr))
				throw E2DException(0, hr, L"Eyes2D::Sound::AudioMixerImpl::AudioMixerImpl", L"Failed to create SubmixVoice.");
		}

		AudioMixerImpl::~AudioMixerImpl() {
			if (m_Mixer != nullptr) {
				m_Mixer->SetOutputVoices(nullptr);
				m_Mixer->DestroyVoice();
				m_Mixer = nullptr;
			}
		}

		bool AudioMixerImpl::SetVolume(float v) {
			return (S_OK == m_Mixer->SetVolume(v));
		}

		float AudioMixerImpl::GetVolume() {
			float out;
			m_Mixer->GetVolume(&out);
			return out;
		}

		struct AudioEngineImpl::Pool {
			unsigned int uuidm = 0u;
			unsigned int uuidp = 0u;
			std::unordered_map<unsigned int, AudioMixer*> mixer;
			std::unordered_map<unsigned int, AudioPlayer*> player;
		};

		AudioEngineImpl::AudioEngineImpl() {
			m_XAudio = nullptr;
			m_Master = nullptr;
			m_Pool = nullptr;

			HRESULT hr;
			hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
			if (FAILED(hr))
				throw E2DException(0, hr, L"Eyes2D::XAudio2Impl::XAudio2Impl", L"Failed to initialize COM.");
			hr = XAudio2Create(&m_XAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
			if (FAILED(hr))
				throw E2DException(0, hr, L"Eyes2D::XAudio2Impl::XAudio2Impl", L"Failed to create XAudio2.");
			hr = m_XAudio->CreateMasteringVoice(&m_Master, 2, 44100);
			if (FAILED(hr))
				throw E2DException(0, hr, L"Eyes2D::XAudio2Impl::XAudio2Impl", L"Failed to create MasteringVoice.");
			m_Pool = new AudioEngineImpl::Pool;
		}

		AudioEngineImpl::~AudioEngineImpl() {
			for (auto it = m_Pool->mixer.begin(); it != m_Pool->mixer.end();) {
				AudioMixerImpl* p = (AudioMixerImpl*)it->second;
				it = m_Pool->mixer.erase(it);
				delete p;
			}
			if (m_Pool != nullptr) delete m_Pool;
			if (m_Master != nullptr) m_Master->DestroyVoice();
			if (m_XAudio != nullptr) m_XAudio->Release();
			m_Pool = nullptr;
			m_Master = nullptr;
			m_XAudio = nullptr;
		}

		bool AudioEngineImpl::Start() {
			return (S_OK == m_XAudio->StartEngine());
		}

		void AudioEngineImpl::Stop() {
			m_XAudio->StopEngine();
		}

		bool AudioEngineImpl::SetVolume(float v) {
			return (S_OK == m_Master->SetVolume(v));
		}

		float AudioEngineImpl::GetVolume() {
			float out;
			m_Master->GetVolume(&out);
			return out;
		}

		AudioMixer* AudioEngineImpl::GetMixer(unsigned int id) {
			auto i = m_Pool->mixer.find(id);
			if (i != m_Pool->mixer.end()) {
				return i->second;
			}
			else {
				return nullptr;
			}
		}

		AudioPlayer* AudioEngineImpl::GetPlayer(unsigned int id) {
			auto i = m_Pool->player.find(id);
			if (i != m_Pool->player.end()) {
				return i->second;
			}
			else {
				return nullptr;
			}
		}

		unsigned int AudioEngineImpl::CreateMixer(AudioMixer** out) {
			AudioMixer* mixer = nullptr;
			unsigned int uuid = m_Pool->uuidm;
			try {
				AudioMixerImpl* _mixer = new AudioMixerImpl(uuid, this);
				mixer = _mixer;
			}
			catch (const E2DException& e) {
				*out = nullptr;
				return 0u;
			}
			*out = mixer;
			m_Pool->mixer.emplace(std::make_pair(uuid, mixer));
			m_Pool->uuidm++;
			return uuid;
		}

		unsigned int AudioEngineImpl::CreatePlayer(fcyStream* stream, AudioPlayer** out) {
			try {
			}
			catch (const E2DException& e) {
			}
			return 0u;
		}

		bool AudioEngineImpl::RemoveMixer(unsigned int id) {
			auto i = m_Pool->mixer.find(id);
			if (i != m_Pool->mixer.end()) {
				AudioMixerImpl* m = (AudioMixerImpl*)i->second;
				m_Pool->mixer.erase(i);
				delete m;
				return true;
			}
			else {
				return false;
			}
		}

		bool AudioEngineImpl::RemovePlayer(unsigned int id) {
			return false;
		}
	}
}
