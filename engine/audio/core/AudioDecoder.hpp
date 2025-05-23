#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/Data.hpp"

namespace core {
	struct CORE_NO_VIRTUAL_TABLE IAudioDecoder : IReferenceCounted {
		[[nodiscard]] virtual uint16_t getSampleSize() const noexcept = 0;   // 1byte(8bit) 2bytes(16bits) 3byte(24bits) 4byte(32bits)
		[[nodiscard]] virtual uint16_t getChannelCount() const noexcept = 0; // 1channel 2channels
		[[nodiscard]] virtual uint16_t getFrameSize() const noexcept = 0;    // = getChannelCount() * getSampleSize()
		[[nodiscard]] virtual uint32_t getSampleRate() const noexcept = 0;   // 22050Hz 44100Hz 48000Hz
		[[nodiscard]] virtual uint32_t getByteRate() const noexcept = 0;     // = getSampleRate() * getFrameSize()
		[[nodiscard]] virtual uint32_t getFrameCount() const noexcept = 0;   // how many pcm frames?

		[[nodiscard]] virtual bool seek(uint32_t pcm_frame) = 0;
		[[nodiscard]] virtual bool seekByTime(double sec) = 0;
		[[nodiscard]] virtual bool tell(uint32_t* pcm_frame) = 0;
		[[nodiscard]] virtual bool tellAsTime(double* sec) = 0;
		[[nodiscard]] virtual bool read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame) = 0; // s16

		[[nodiscard]] static bool create(IData* data, IAudioDecoder** output_decoder);
		[[nodiscard]] static bool create(std::string_view path, IAudioDecoder** output_decoder);
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IAudioDecoder
	template<> constexpr InterfaceId getInterfaceId<IAudioDecoder>() { return UUID::parse("21bfc418-633d-534a-8e70-a8ff61d94d61"); }
}
