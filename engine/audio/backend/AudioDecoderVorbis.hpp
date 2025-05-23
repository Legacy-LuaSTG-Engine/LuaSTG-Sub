#pragma once
#include "core/AudioDecoder.hpp"
#include "core/SmartReference.hpp"
#include "core/Data.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include <vorbis/vorbisfile.h>

namespace core {
	class AudioDecoderVorbis final : public implement::ReferenceCounted<IAudioDecoder> {
	public:
		// IAudioDecoder

		[[nodiscard]] uint16_t getSampleSize() const noexcept override { return 2; }
		[[nodiscard]] uint16_t getChannelCount() const noexcept override;
		[[nodiscard]] uint16_t getFrameSize() const noexcept override { return getChannelCount() * getSampleSize(); }
		[[nodiscard]] uint32_t getSampleRate() const noexcept override;
		[[nodiscard]] uint32_t getByteRate() const noexcept override { return getSampleRate() * static_cast<uint32_t>(getFrameSize()); }
		[[nodiscard]] uint32_t getFrameCount() const noexcept override;

		[[nodiscard]] bool seek(uint32_t pcm_frame) override;
		[[nodiscard]] bool seekByTime(double sec) override;
		[[nodiscard]] bool tell(uint32_t* pcm_frame) override;
		[[nodiscard]] bool tellAsTime(double* sec) override;
		[[nodiscard]] bool read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame) override;

		// AudioDecoderVorbis

		AudioDecoderVorbis() = default;
		AudioDecoderVorbis(AudioDecoderVorbis const&) = delete;
		AudioDecoderVorbis(AudioDecoderVorbis&&) = delete;
		~AudioDecoderVorbis() override;

		AudioDecoderVorbis& operator=(AudioDecoderVorbis const&) = delete;
		AudioDecoderVorbis& operator=(AudioDecoderVorbis&&) = delete;

		[[nodiscard]] bool open(IData* data);
		void close();

	private:
		static size_t vorbisRead(void* ptr, size_t size, size_t n_mem_b, void* datasource);
		static int vorbisSeek(void* datasource, ogg_int64_t offset, int whence);
		static int vorbisClose(void* datasource);
		static long vorbisTell(void* datasource);

		SmartReference<IData> m_data;
		void* m_pointer{};
		OggVorbis_File m_ogg{};
		bool m_initialized{ false };
	};
}
