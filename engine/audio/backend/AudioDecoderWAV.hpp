#pragma once
#include "core/AudioDecoder.hpp"
#include "core/SmartReference.hpp"
#include "core/Data.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "dr_wav.h"

namespace core {
	class AudioDecoderWAV final : public implement::ReferenceCounted<IAudioDecoder> {
	public:
		// IAudioDecoder

		[[nodiscard]] uint16_t getSampleSize() const noexcept override { return 2; } // 固定为 16bits
		[[nodiscard]] uint16_t getChannelCount() const noexcept override { return m_wav.channels; }
		[[nodiscard]] uint16_t getFrameSize() const noexcept override { return getChannelCount() * getSampleSize(); }
		[[nodiscard]] uint32_t getSampleRate() const noexcept override { return m_wav.sampleRate; }
		[[nodiscard]] uint32_t getByteRate() const noexcept override { return getSampleRate() * static_cast<uint32_t>(getFrameSize()); }
		[[nodiscard]] uint32_t getFrameCount() const noexcept override; // TODO: uint32 在 44800Hz 的采样率下，只能表示 24 小时，但是一般也不会有人弄这么鬼长的音频文件吧……

		[[nodiscard]] bool seek(uint32_t pcm_frame) override;
		[[nodiscard]] bool seekByTime(double sec) override;
		[[nodiscard]] bool tell(uint32_t* pcm_frame) override;
		[[nodiscard]] bool tellAsTime(double* sec) override;
		[[nodiscard]] bool read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame) override;

		// AudioDecoderWAV

		AudioDecoderWAV() = default;
		AudioDecoderWAV(AudioDecoderWAV const&) = delete;
		AudioDecoderWAV(AudioDecoderWAV&&) = delete;
		~AudioDecoderWAV() override;

		AudioDecoderWAV& operator=(AudioDecoderWAV const&) = delete;
		AudioDecoderWAV& operator=(AudioDecoderWAV&&) = delete;

		[[nodiscard]] bool open(IData* data);
		void close();

	private:
		SmartReference<IData> m_data;
		drwav m_wav{};
		bool m_initialized{ false };
	};
}
