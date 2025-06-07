#include "backend/AudioDecoderWAV.hpp"
#include <cassert>

namespace core {
	uint32_t AudioDecoderWAV::getFrameCount() const noexcept {
		drwav_uint64 dr_frame_count = 0;
		// drwav_get_length_in_pcm_frames is a read-only function, so we can do const_cast
		if (drwav_result const result = drwav_get_length_in_pcm_frames(const_cast<drwav*>(&m_wav), &dr_frame_count); result != DRWAV_SUCCESS) {
			assert(false);
			return 0;
		}
		return static_cast<uint32_t>(dr_frame_count);
	}

	bool AudioDecoderWAV::seek(uint32_t const pcm_frame) {
		return DRWAV_TRUE == drwav_seek_to_pcm_frame(&m_wav, pcm_frame);
	}
	bool AudioDecoderWAV::seekByTime(double const sec) {
		auto const pcm_frame = static_cast<drwav_uint64>(sec * static_cast<double>(m_wav.sampleRate));
		return DRWAV_TRUE == drwav_seek_to_pcm_frame(&m_wav, pcm_frame);
	}
	bool AudioDecoderWAV::tell(uint32_t* const pcm_frame) {
		if (pcm_frame == nullptr) {
			assert(false);
			return false;
		}
		drwav_uint64 dr_frame_index = 0;
		drwav_result const result = drwav_get_cursor_in_pcm_frames(&m_wav, &dr_frame_index);
		*pcm_frame = static_cast<uint32_t>(dr_frame_index);
		return DRWAV_SUCCESS == result;
	}
	bool AudioDecoderWAV::tellAsTime(double* const sec) {
		if (sec == nullptr) {
			assert(false);
			return false;
		}
		drwav_uint64 dr_frame_index = 0;
		drwav_result const result = drwav_get_cursor_in_pcm_frames(&m_wav, &dr_frame_index);
		*sec = static_cast<double>(dr_frame_index) / static_cast<double>(m_wav.sampleRate);
		return DRWAV_SUCCESS == result;
	}
	bool AudioDecoderWAV::read(uint32_t const pcm_frame, void* const buffer, uint32_t* const read_pcm_frame) {
		if (buffer == nullptr) {
			assert(false);
			return false;
		}
		drwav_uint64 const dr_frame_count = drwav_read_pcm_frames_s16(&m_wav, pcm_frame, static_cast<drwav_int16*>(buffer));
		if (read_pcm_frame) {
			*read_pcm_frame = static_cast<uint32_t>(dr_frame_count);
			return pcm_frame == 0 || dr_frame_count > 0;
		}
		assert(pcm_frame == dr_frame_count);
		return pcm_frame == dr_frame_count;
	}

	AudioDecoderWAV::~AudioDecoderWAV() {
		close();
	}

	bool AudioDecoderWAV::open(IData* const data) {
		m_data = data;
		if (auto const result = drwav_init_memory(&m_wav, m_data->data(), m_data->size(), nullptr); DRWAV_TRUE != result) {
			close();
			return false;
		}
		m_initialized = true;
		if ((m_wav.bitsPerSample % 8) != 0 || !(m_wav.channels == 1 || m_wav.channels == 2)) {
			close();
			return false;
		}
		if (!seek(0)) {
			close();
			return false;
		}
		return true;
	}
	void AudioDecoderWAV::close() {
		if (m_initialized) {
			m_initialized = false;
			drwav_uninit(&m_wav);
		}
		m_data.reset();
	}
}
