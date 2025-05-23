#pragma once
#include "core/AudioDecoder.hpp"
#include "core/SmartReference.hpp"
#include "core/Data.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "FLAC/stream_decoder.h"
#include <vector>

namespace core {
	class AudioDecodeFLAC final : public implement::ReferenceCounted<IAudioDecoder> {
	public:
		// IAudioDecoder

		[[nodiscard]] uint16_t getSampleSize() const noexcept override { return static_cast<uint16_t>(m_info.bits_per_sample) / static_cast<uint16_t>(8); }
		[[nodiscard]] uint16_t getChannelCount() const noexcept override { return static_cast<uint16_t>(m_info.channels); }
		[[nodiscard]] uint16_t getFrameSize() const noexcept override { return getChannelCount() * getSampleSize(); }
		[[nodiscard]] uint32_t getSampleRate() const noexcept override { return m_info.sample_rate; }
		[[nodiscard]] uint32_t getByteRate() const noexcept override { return getSampleRate() * static_cast<uint32_t>(getFrameSize()); }
		[[nodiscard]] uint32_t getFrameCount() const noexcept override { return static_cast<uint32_t>(m_info.total_samples); }

		[[nodiscard]] bool seek(uint32_t pcm_frame) override;
		[[nodiscard]] bool seekByTime(double sec) override;
		[[nodiscard]] bool tell(uint32_t* pcm_frame) override;
		[[nodiscard]] bool tellAsTime(double* sec) override;
		[[nodiscard]] bool read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame) override;

		// AudioDecodeFLAC

		AudioDecodeFLAC() = default;
		AudioDecodeFLAC(AudioDecodeFLAC const&) = delete;
		AudioDecodeFLAC(AudioDecodeFLAC&&) = delete;
		~AudioDecodeFLAC() override;

		AudioDecodeFLAC& operator=(AudioDecodeFLAC const&) = delete;
		AudioDecodeFLAC& operator=(AudioDecodeFLAC&&) = delete;

		[[nodiscard]] bool open(IData* data);
		void close();

	private:
		struct Frame {
			uint32_t sample_rate{ 0 };
			uint16_t channels{ 0 };
			uint16_t bits_per_sample{ 0 };
			uint32_t sample_index{ 0 };
			uint32_t sample_count{ 0 };
			std::vector<int32_t> data[2]; // 0 mono | 0 left, 1 right

			void clear() noexcept {
				sample_rate = 0;
				channels = 0;
				bits_per_sample = 0;
				sample_index = 0;
				sample_count = 0;
				//data[0].clear();
				//data[1].clear();
			}
			[[nodiscard]] bool empty() const noexcept {
				return sample_rate == 0
					|| channels == 0
					|| bits_per_sample == 0
					|| sample_count == 0;
			}
		};

		static FLAC__StreamDecoderReadStatus onRead(FLAC__StreamDecoder const* decoder, FLAC__byte buffer[], size_t* bytes, void* client_data);
		static FLAC__StreamDecoderSeekStatus onSeek(FLAC__StreamDecoder const* decoder, FLAC__uint64 absolute_byte_offset, void* client_data);
		static FLAC__StreamDecoderTellStatus onTell(FLAC__StreamDecoder const* decoder, FLAC__uint64* absolute_byte_offset, void* client_data);
		static FLAC__StreamDecoderLengthStatus onGetLength(FLAC__StreamDecoder const* decoder, FLAC__uint64* stream_length, void* client_data);
		static FLAC__bool onCheckEof(FLAC__StreamDecoder const* decoder, void* client_data);

		static FLAC__StreamDecoderWriteStatus onWrite(FLAC__StreamDecoder const* decoder, FLAC__Frame const* frame, const FLAC__int32* const buffer[], void* client_data);
		static void onMetadata(FLAC__StreamDecoder const* decoder, FLAC__StreamMetadata const* metadata, void* client_data);
		static void onError(FLAC__StreamDecoder const* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data);

		SmartReference<IData> m_data;
		void* m_pointer{};
		FLAC__StreamDecoder* m_flac;
		FLAC__StreamMetadata_StreamInfo m_info{};
		uint32_t m_current_pcm_frame{ 0 };
		Frame m_flac_frame_data;
		bool m_has_info{ false };
		bool m_initialized{ false };
	};
}
