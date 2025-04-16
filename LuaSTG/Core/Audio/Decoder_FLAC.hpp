#pragma once
#include "Core/Object.hpp"
#include "Core/Audio/Decoder.hpp"

#include "FLAC/stream_decoder.h"

namespace core::Audio
{
	class Decoder_FLAC : public Object<IDecoder>
	{
	private:
		struct Frame
		{
			uint32_t sample_rate{ 0 };
			uint16_t channels{ 0 };
			uint16_t bits_per_sample{ 0 };
			uint32_t sample_index{ 0 };
			uint32_t sample_count{ 0 };
			std::vector<int32_t> data[2]; // 0 mono | 0 left, 1 right

			void clear()
			{
				sample_rate = 0;
				channels = 0;
				bits_per_sample = 0;
				sample_index = 0;
				sample_count = 0;
				//data[0].clear();
				//data[1].clear();
			}
			bool empty()
			{
				return sample_rate == 0
					|| channels == 0
					|| bits_per_sample == 0
					|| sample_count == 0;
			}
		};

	private:
		std::vector<uint8_t> m_data;
		uint8_t* m_ptr{ nullptr };
		FILE* m_file;
		FLAC__StreamDecoder* m_flac;
		FLAC__StreamMetadata_StreamInfo m_info{};
		uint32_t m_current_pcm_frame{ 0 };
		Frame m_flac_frame_data;
		bool m_init{ false };
		bool m_has_info{ false };

	private:
		static FLAC__StreamDecoderReadStatus onRead(FLAC__StreamDecoder const* decoder, FLAC__byte buffer[], size_t* bytes, void* client_data);
		static FLAC__StreamDecoderSeekStatus onSeek(FLAC__StreamDecoder const* decoder, FLAC__uint64 absolute_byte_offset, void* client_data);
		static FLAC__StreamDecoderTellStatus onTell(FLAC__StreamDecoder const* decoder, FLAC__uint64* absolute_byte_offset, void* client_data);
		static FLAC__StreamDecoderLengthStatus onGetLength(FLAC__StreamDecoder const* decoder, FLAC__uint64* stream_length, void* client_data);
		static FLAC__bool onCheckEOF(FLAC__StreamDecoder const* decoder, void* client_data);

	private:
		static FLAC__StreamDecoderWriteStatus onWrite(FLAC__StreamDecoder const* decoder, FLAC__Frame const* frame, const FLAC__int32* const buffer[], void* client_data);
		static void onMetadata(FLAC__StreamDecoder const* decoder, FLAC__StreamMetadata const* metadata, void* client_data);
		static void onError(FLAC__StreamDecoder const* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data);

	private:
		void destroyResources();

	public:
		uint16_t getSampleSize();
		uint16_t getChannelCount();
		uint16_t getFrameSize();
		uint32_t getSampleRate();
		uint32_t getByteRate();
		uint32_t getFrameCount();

		bool seek(uint32_t pcm_frame);
		bool seekByTime(double sec);
		bool tell(uint32_t* pcm_frame);
		bool tellAsTime(double* sec);

		bool read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame);

	public:
		Decoder_FLAC(StringView path);
		~Decoder_FLAC();
	};
}
