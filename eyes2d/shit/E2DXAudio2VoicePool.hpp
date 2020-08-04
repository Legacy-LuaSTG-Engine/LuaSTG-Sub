#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "E2DXAudio2Impl.hpp"

namespace Eyes2D {
	namespace Sound {
		class XAudio2VoicePool {
		private:
			IXAudio2* m_XAudio;                                              //只是持有
			IXAudio2MasteringVoice* m_MasteringVoice;                        //只是持有
			std::unordered_map<std::string, IXAudio2SubmixVoice*> m_MixVoice;//混音音轨
			int m_TotalVoiceSpace;                                           //可变，音频池，典型值为64
			int m_NextAllocSpace;                                            //下一次分配的ID数量，典型值为4
			std::vector<int> m_FreeVoiceID;                                  //空闲的音频ID
			std::vector<IXAudio2SourceVoice*> m_SourceVoice;                 //音源池
		public:
			XAudio2VoicePool(IXAudio2* p, IXAudio2MasteringVoice* m);
			~XAudio2VoicePool();
		private:
			int allocID();
			void freeID(int id);
		public:
			IXAudio2SubmixVoice* CreateMixVoice(const std::string& name);
			IXAudio2SubmixVoice* GetMixVoice(const std::string& name);
			int                  CreateSourceVoice(const std::string& mixername, WAVEFORMATEX& format);
			IXAudio2SourceVoice* GetSourceVoice(int id);
			void                 DeleteSourceVoice(int id);
		};
	}
}
