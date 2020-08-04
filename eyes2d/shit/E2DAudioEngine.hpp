#pragma once
#include "E2DGlobal.hpp"
#include "E2DSound.hpp"
#include "XAudio2.h"

namespace Eyes2D {
	namespace Sound {
		class AudioMixerImpl : public AudioMixer {
		private:
			unsigned int m_ID;
			IXAudio2SubmixVoice* m_Mixer;
		public:
			IXAudio2SubmixVoice* GetMixerHandle() { return m_Mixer; };

			unsigned GetID() { return m_ID; };
			bool SetVolume(float v);
			float GetVolume();
		public:
			AudioMixerImpl(unsigned int id, AudioEngine* engine);
			~AudioMixerImpl();
		};

		class EYESDLLAPI AudioEngineImpl : public AudioEngine {
		private:
			struct Pool;
			IXAudio2* m_XAudio;
			IXAudio2MasteringVoice* m_Master;
			Pool* m_Pool;
		public:
			IXAudio2* GetEngineHandle() { return m_XAudio; };
			IXAudio2MasteringVoice* GetMasterHandle() { return m_Master; };

			bool Start();
			void Stop();

			bool SetVolume(float v);
			float GetVolume();

			AudioMixer* GetMixer(unsigned int id);
			AudioPlayer* GetPlayer(unsigned int id);

			unsigned int CreateMixer(AudioMixer** out);
			unsigned int CreatePlayer(fcyStream* stream, AudioPlayer** out);
			bool RemoveMixer(unsigned int id);
			bool RemovePlayer(unsigned int id);
		public:
			AudioEngineImpl();
			~AudioEngineImpl();
		};
	}
}
