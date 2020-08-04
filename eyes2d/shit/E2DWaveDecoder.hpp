#pragma once

#include "E2DGlobal.hpp"
#include "E2DSound.hpp"
#include "fcyIO/fcyStream.h"
#include "E2DAudioDecoder.hpp"

namespace Eyes2D {
	namespace Sound {
		class EYESDLLAPI WaveDecoder : public AudioDecoder {
		private:
			PCM_AudioData m_Data;
		public:
			WaveDecoder();
			~WaveDecoder();
			bool DecodeFromStream(fcyStream* stream);
		public:
			uint16_t GetChannels() { return m_Data.channels; }
			uint32_t GetSamplesPerSec() { return m_Data.samples_per_sec; }
			uint32_t GetAvgBytesPerSec() { return m_Data.bytes_per_sec; }
			uint16_t GetBlockAlign() { return m_Data.block_align; }
			uint16_t GetBitsPerSample() { return m_Data.bits_per_sample; }
			uint8_t* GetDataBuffer() { return m_Data.data; }
			uint32_t GetDataSize() { return m_Data.size; }
		};
	}
}
