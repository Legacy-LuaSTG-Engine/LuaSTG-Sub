#pragma once
#include "Core/Object.hpp"
#include "Core/Audio/Decoder.hpp"

#include <vorbis/vorbisfile.h>

namespace LuaSTG::Core::Audio
{
	class Decoder_VorbisOGG : public Object<IDecoder>
	{
	public:
		struct OggVorbis_Stream
		{
			uint8_t* data;
			size_t size;
			uint8_t* ptr;

			static size_t read(void* ptr, size_t size, size_t nmemb, void* datasource);
			static int seek(void* datasource, ogg_int64_t offset, int whence);
			static int close(void* datasource);
			static long tell(void* datasource);
		};

	private:
		std::vector<uint8_t> m_data;
		OggVorbis_Stream m_stream;
		OggVorbis_File m_ogg;
		bool m_init;

	public:
		uint16_t getSampleSize() { return 2; } // OGG 永远是 16bits
		uint16_t getChannelCount();
		uint16_t getFrameSize() { return getChannelCount() * getSampleSize(); }
		uint32_t getSampleRate();
		uint32_t getByteRate() { return getSampleRate() * (uint32_t)getFrameSize(); }
		uint32_t getFrameCount(); // TODO: uint32 在 44800Hz 的采样率下，只能表示 24 小时，但是一般也不会有人弄这么鬼长的音频文件吧……

		bool seek(uint32_t pcm_frame);
		bool seekByTime(double sec);
		bool tell(uint32_t* pcm_frame);
		bool tellAsTime(double* sec);
		bool read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame);

	public:
		Decoder_VorbisOGG(StringView path);
		~Decoder_VorbisOGG();
	};
}
