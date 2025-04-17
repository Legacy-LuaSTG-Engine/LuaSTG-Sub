#pragma once
#include "core/implement/ReferenceCounted.hpp"
#include "Core/Audio/Decoder.hpp"

#include "dr_wav.h"

namespace core::Audio
{
	class Decoder_WAV : public implement::ReferenceCounted<IDecoder>
	{
	private:
		std::vector<uint8_t> m_data;
		drwav m_wav;
		bool m_init;

	private:
		void destroyResources();

	public:
		uint16_t getSampleSize() { return 2; } // 固定为 16bits
		uint16_t getChannelCount() { return m_wav.channels; }
		uint16_t getFrameSize() { return getChannelCount() * getSampleSize(); }
		uint32_t getSampleRate() { return m_wav.sampleRate; }
		uint32_t getByteRate() { return getSampleRate() * (uint32_t)getFrameSize(); }
		uint32_t getFrameCount(); // TODO: uint32 在 44800Hz 的采样率下，只能表示 24 小时，但是一般也不会有人弄这么鬼长的音频文件吧……

		bool seek(uint32_t pcm_frame);
		bool seekByTime(double sec);
		bool tell(uint32_t* pcm_frame);
		bool tellAsTime(double* sec);
		bool read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame);

	public:
		Decoder_WAV(StringView path);
		~Decoder_WAV();
	};
}
