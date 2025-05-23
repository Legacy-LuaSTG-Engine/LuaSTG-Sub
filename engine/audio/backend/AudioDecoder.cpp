#include "core/AudioDecoder.hpp"
#include "core/SmartReference.hpp"
#include "core/FileSystem.hpp"
#include "backend/AudioDecoderWAV.hpp"
#include "backend/AudioDecoderVorbis.hpp"
#include "backend/AudioDecoderFLAC.hpp"

namespace {
	template<typename T>
	bool createDecoder(core::IData* const data, core::IAudioDecoder** const output_decoder) {
		core::SmartReference<T> decoder;
		decoder = new T;
		if (!decoder->open(data)) {
			return false;
		}
		*output_decoder = decoder.detach();
		return true;
	}
}

namespace core {
	bool IAudioDecoder::create(IData* const data, IAudioDecoder** const output_decoder) {
		if (createDecoder<AudioDecoderWAV>(data, output_decoder)) {
			return true;
		}
		if (createDecoder<AudioDecoderVorbis>(data, output_decoder)) {
			return true;
		}
		if (createDecoder<AudioDecodeFLAC>(data, output_decoder)) {
			return true;
		}
		return false;
	}
	bool IAudioDecoder::create(std::string_view const path, IAudioDecoder** const output_decoder) {
		SmartReference<IData> data;
		if (!FileSystemManager::readFile(path, data.put())) {
			return false;
		}
		return create(data.get(), output_decoder);
	}
}
