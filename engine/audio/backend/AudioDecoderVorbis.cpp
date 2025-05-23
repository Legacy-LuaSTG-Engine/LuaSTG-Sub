#include "backend/AudioDecoderVorbis.hpp"
#include <algorithm>

namespace core {
	uint16_t AudioDecoderVorbis::getChannelCount() const noexcept {
		auto const info = ov_info(const_cast<OggVorbis_File*>(&m_ogg), -1); // ov_info is read-only
		assert(info != nullptr);
		if (info != nullptr) {
			return static_cast<uint16_t>(info->channels);
		}
		return 0;
	}
	uint32_t AudioDecoderVorbis::getSampleRate() const noexcept {
		auto const info = ov_info(const_cast<OggVorbis_File*>(&m_ogg), -1); // ov_info is read-only
		assert(info != nullptr);
		if (info != nullptr) {
			return static_cast<uint32_t>(info->rate);
		}
		return 0;
	}
	uint32_t AudioDecoderVorbis::getFrameCount() const noexcept {
		auto const pcm_frame_count = ov_pcm_total(const_cast<OggVorbis_File*>(&m_ogg), -1); // ov_pcm_total is read-only
		assert(pcm_frame_count >= 0); // OV_EINVAL
		return (pcm_frame_count > 0) ? static_cast<uint32_t>(pcm_frame_count) : 0;
	}

	bool AudioDecoderVorbis::seek(uint32_t const pcm_frame) {
		return 0 == ov_pcm_seek(&m_ogg, static_cast<ogg_int64_t>(pcm_frame));
	}
	bool AudioDecoderVorbis::seekByTime(double const sec) {
		return 0 == ov_time_seek(&m_ogg, sec);
	}
	bool AudioDecoderVorbis::tell(uint32_t* const pcm_frame) {
		ogg_int64_t const ogg_pcm_frame = ov_pcm_tell(&m_ogg);
		assert(ogg_pcm_frame >= 0); // OV_EINVAL
		*pcm_frame = (ogg_pcm_frame >= 0) ? static_cast<uint32_t>(ogg_pcm_frame) : 0;
		return ogg_pcm_frame >= 0;
	}
	bool AudioDecoderVorbis::tellAsTime(double* const sec) {
		double const time = ov_time_tell(&m_ogg); // return time >= 0 or double(OV_EINVAL)
		assert(time >= 0.0);
		*sec = (time > 0.0) ? time : 0.0;
		return time >= 0.0;
	}
	bool AudioDecoderVorbis::read(uint32_t const pcm_frame, void* const buffer, uint32_t* const read_pcm_frame) {
		uint32_t const target_read_size = pcm_frame * getFrameSize();
		uint32_t read_size = 0;
		char* ptr = (char*)buffer;
		int current_stream = 0;
		while (read_size < target_read_size) {
			long const result = ov_read(&m_ogg, ptr, (int)(target_read_size - read_size), 0, 2, 1, &current_stream);
			if (result < 0) {
				assert(false); return false; // 失败了
			}
			else if (result == 0) {
				break; // 读取到文件尾
			}
			else {
				read_size += result;
				ptr += result;
			}
		}
		if (read_pcm_frame) {
			*read_pcm_frame = read_size / (uint32_t)getFrameSize();
			return true;
		}
		else {
			assert(read_size == target_read_size);
			return read_size == target_read_size;
		}
	}

	AudioDecoderVorbis::~AudioDecoderVorbis() {
		close();
	}

	bool AudioDecoderVorbis::open(IData* const data) {
		m_data = data;
		m_pointer = m_data->data();

		constexpr ov_callbacks callbacks{
			&vorbisRead,
			&vorbisSeek,
			&vorbisClose,
			&vorbisTell,
		};
		if (int const result = ov_open_callbacks(this, &m_ogg, nullptr, 0, callbacks); result != 0) {
			close();
			return false;
		}
		m_initialized = true;

		auto const info = ov_info(&m_ogg, -1);
		if (info == nullptr) {
			close();
			return false;
		}
		if (!(info->channels == 1 || info->channels == 2)) {
			close();
			return false;
		}

		if (!seek(0)) {
			close();
			return false;
		}

		return true;
	}
	void AudioDecoderVorbis::close() {
		if (m_initialized) {
			m_initialized = false;
			ov_clear(&m_ogg);
		}
		m_data.reset();
		m_pointer = {};
	}

#define SELF \
	assert(datasource != nullptr); \
	auto const self = static_cast<AudioDecoderVorbis*>(datasource); \
	[[maybe_unused]] auto const m_data = static_cast<uint8_t*>(self->m_data->data()); \
	[[maybe_unused]] auto const m_size = self->m_data->size(); \
	[[maybe_unused]] auto const m_ptr = static_cast<uint8_t*>(self->m_pointer)

#define MOVE_TO_END self->m_pointer = (m_data + m_size)

#define MOVE_TO_BEGIN self->m_pointer = (m_data)

	size_t AudioDecoderVorbis::vorbisRead(void* const ptr, size_t const size, size_t const n_mem_b, void* const datasource) {
		SELF;
		size_t const right_size = m_size - (m_ptr - m_data);
		size_t const valid_count = right_size / size;
		size_t const count = std::min(n_mem_b, valid_count);
		size_t const should_read_size = count * size;
		std::memcpy(ptr, m_ptr, should_read_size);
		self->m_pointer = m_ptr + should_read_size;
		return count;
	}
	int AudioDecoderVorbis::vorbisSeek(void* const datasource, ogg_int64_t const offset, int const whence) {
		SELF;

		if (whence == SEEK_SET) {
			assert(offset >= 0);
			if (static_cast<size_t>(offset) > m_size) {
				MOVE_TO_END;
				return -1;
			}
			self->m_pointer = m_data + offset;
			return 0;
		}

		if (whence == SEEK_END) {
			assert(offset <= 0);
			if (static_cast<size_t>(-offset) > m_size) {
				MOVE_TO_BEGIN;
				return -1;
			}
			self->m_pointer = m_data + m_size + offset;
			return 0;
		}

		if (whence == SEEK_CUR) {
			if (offset > 0) {
				if (size_t const right_size = m_size - (m_ptr - m_data); static_cast<size_t>(offset) > right_size) {
					MOVE_TO_END;
					return -1;
				}
				self->m_pointer = m_ptr + offset;
				return 0;
			}

			if (offset < 0) {
				if (size_t const left_size = m_ptr - m_data; static_cast<size_t>(-offset) > left_size) {
					MOVE_TO_BEGIN;
					return -1;
				}
				self->m_pointer = m_ptr + offset;
				return 0;
			}

			return 0; // offset == 0
		}

		assert(false);
		return -1;
	}
	int AudioDecoderVorbis::vorbisClose(void* const datasource) {
		SELF;
		MOVE_TO_END;
		return 0;
	}
	long AudioDecoderVorbis::vorbisTell(void* const datasource) {
		SELF;
		return static_cast<long>(m_ptr - m_data);
	}
}
