#include "backend/AudioDecoderFLAC.hpp"
#include "core/Logger.hpp"

namespace core {
	bool AudioDecodeFLAC::seek(uint32_t const pcm_frame) {
		if (FLAC__STREAM_DECODER_SEEK_ERROR == FLAC__stream_decoder_get_state(m_flac)) {
			if (!FLAC__stream_decoder_flush(m_flac)) {
				return false;
			}
		}
		m_current_pcm_frame = pcm_frame;
		m_flac_frame_data.clear();
		return FLAC__stream_decoder_seek_absolute(m_flac, pcm_frame);
	}
	bool AudioDecodeFLAC::seekByTime(double const sec) {
		if (FLAC__STREAM_DECODER_SEEK_ERROR == FLAC__stream_decoder_get_state(m_flac)) {
			if (!FLAC__stream_decoder_flush(m_flac)) {
				return false;
			}
		}
		FLAC__uint64 const pcm_frame = static_cast<FLAC__uint64>(sec * static_cast<double>(m_info.sample_rate));
		m_current_pcm_frame = static_cast<uint32_t>(pcm_frame);
		m_flac_frame_data.clear();
		return FLAC__stream_decoder_seek_absolute(m_flac, pcm_frame);
	}
	bool AudioDecodeFLAC::tell(uint32_t* pcm_frame) {
		*pcm_frame = m_current_pcm_frame;
		return true;
	}
	bool AudioDecodeFLAC::tellAsTime(double* sec) {
		*sec = static_cast<double>(m_current_pcm_frame) / static_cast<double>(m_info.sample_rate);
		return true;
	}
	bool AudioDecodeFLAC::read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame) {
		uint32_t read_pcm_frame_ = 0;

		while (pcm_frame > 0) {
			// 先康康有没有没处理完的 frame
			if (!m_flac_frame_data.empty()) {
				if (m_current_pcm_frame >= m_flac_frame_data.sample_index && m_current_pcm_frame < (m_flac_frame_data.sample_index + m_flac_frame_data.sample_count)) {
					// 计算参数
					uint32_t const pcm_frame_offset = m_current_pcm_frame - m_flac_frame_data.sample_index;
					uint32_t const valid_pcm_frame = (m_flac_frame_data.sample_index + m_flac_frame_data.sample_count) - m_current_pcm_frame;
					uint32_t const should_read_pcm_frame = std::min(pcm_frame, valid_pcm_frame);
					// 写入
					uint8_t* ptr = (uint8_t*)buffer;
					for (uint32_t idx = 0; idx < should_read_pcm_frame; idx += 1) {
						for (uint16_t chs = 0; chs < m_flac_frame_data.channels; chs += 1) {
							// 拆数据
							int32_t const sample = m_flac_frame_data.data[chs][pcm_frame_offset + idx];
							uint8_t const bytes[4] = {
								(uint8_t)((sample & 0x000000FF)),
								(uint8_t)((sample & 0x0000FF00) >> 8),
								(uint8_t)((sample & 0x00FF0000) >> 16),
								(uint8_t)((sample & 0xFF000000) >> 24),
							};
							// 看位深来写入数据，Windows 一般是小端，如果是大端平台需要把上面的 bytes 数组颠倒
							for (uint16_t bid = 0; bid < (m_flac_frame_data.bits_per_sample / 8); bid += 1) {
								*ptr = bytes[bid];
								ptr += 1;
							}
						}
					}
					// 更新
					buffer = ptr;
					pcm_frame -= should_read_pcm_frame;
					read_pcm_frame_ += should_read_pcm_frame;
					m_current_pcm_frame += should_read_pcm_frame;
					// 如果已经完成了，直接返回
					if (pcm_frame == 0) {
						*read_pcm_frame = read_pcm_frame_;
						return true;
					}
				}
				else {
					m_flac_frame_data.clear(); // 不在范围内，可以清理了
				}
			}

			// 还需要继续读取
			if (!FLAC__stream_decoder_process_single(m_flac)) {
				// 发生错误
				*read_pcm_frame = read_pcm_frame_;
				return false;
			}
		}

		// 没有了
		*read_pcm_frame = read_pcm_frame_;
		return true;
	}

	AudioDecodeFLAC::~AudioDecodeFLAC() {
		close();
	}

	bool AudioDecodeFLAC::open(IData* const data) {
		// read file

		m_data = data;
		m_pointer = static_cast<uint8_t*>(m_data->data());

		// create decoder

		m_flac = FLAC__stream_decoder_new();
		if (nullptr == m_flac) {
			close();
			return false;
		}

		FLAC__stream_decoder_set_metadata_respond(m_flac, FLAC__METADATA_TYPE_STREAMINFO);
		FLAC__stream_decoder_set_md5_checking(m_flac, true);

		// open stream

		FLAC__StreamDecoderInitStatus flac_init = FLAC__STREAM_DECODER_INIT_STATUS_OK;
		flac_init = FLAC__stream_decoder_init_stream(m_flac, &onRead, &onSeek, &onTell, &onGetLength, onCheckEof, &onWrite, &onMetadata, &onError, this);
		if (FLAC__STREAM_DECODER_INIT_STATUS_OK != flac_init) {
			// retry
			m_pointer = static_cast<uint8_t*>(m_data->data()); // reset stream
			flac_init = FLAC__stream_decoder_init_ogg_stream(m_flac, &onRead, &onSeek, &onTell, &onGetLength, onCheckEof, &onWrite, &onMetadata, &onError, this);
		}
		if (FLAC__STREAM_DECODER_INIT_STATUS_OK != flac_init) {
			close();
			return false;
		}
		m_initialized = true;

		// check

		if (!FLAC__stream_decoder_process_until_end_of_metadata(m_flac)) {
			close();
			return false;
		}
		if (!m_has_info) {
			close();
			return false;
		}
		if ((m_info.bits_per_sample % 8) != 0 || !(m_info.channels == 1 || m_info.channels == 2)) {
			close();
			return false;
		}

		// test

		if (!FLAC__stream_decoder_flush(m_flac)) {
			close();
			return false;
		}
		if (!FLAC__stream_decoder_seek_absolute(m_flac, 0)) {
			close();
			return false;
		}

		return true;
	}
	void AudioDecodeFLAC::close() {
		if (m_initialized) {
			if (m_flac != nullptr) {
				FLAC__stream_decoder_finish(m_flac);
			}
			m_initialized = false;
		}
		m_has_info = false;

		if (m_flac != nullptr) {
			FLAC__stream_decoder_delete(m_flac);
			m_flac = nullptr;
		}

		m_data.reset();
		m_pointer = {};
	}

#define SELF \
	auto const self = static_cast<AudioDecodeFLAC*>(client_data); \
	[[maybe_unused]] auto const m_data = static_cast<uint8_t*>(self->m_data->data()); \
	[[maybe_unused]] auto const m_size = self->m_data->size(); \
	[[maybe_unused]] auto const m_ptr = static_cast<uint8_t*>(self->m_pointer)

	FLAC__StreamDecoderReadStatus AudioDecodeFLAC::onRead(FLAC__StreamDecoder const* const, FLAC__byte buffer[], size_t* const bytes, void* const client_data) {
		SELF;
		assert(bytes);

		size_t const valid_size = m_size - (m_ptr - m_data);
		if (valid_size == 0) {
			*bytes = 0;
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		}

		assert(buffer);

		size_t const read_size = std::min(valid_size, *bytes);
		std::memcpy(buffer, m_ptr, read_size);
		self->m_pointer = m_ptr + read_size;
		*bytes = read_size;
		return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
	}
	FLAC__StreamDecoderSeekStatus AudioDecodeFLAC::onSeek(FLAC__StreamDecoder const* const, FLAC__uint64 const absolute_byte_offset, void* const client_data) {
		SELF;
		if (absolute_byte_offset > m_size) {
			self->m_pointer = m_data + m_size;
			return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
		}
		self->m_pointer = m_data + absolute_byte_offset;
		return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
	}
	FLAC__StreamDecoderTellStatus AudioDecodeFLAC::onTell(FLAC__StreamDecoder const* const, FLAC__uint64* const absolute_byte_offset, void* const client_data) {
		SELF;
		*absolute_byte_offset = static_cast<FLAC__uint64>(m_ptr - m_data);
		return FLAC__STREAM_DECODER_TELL_STATUS_OK;
	}
	FLAC__StreamDecoderLengthStatus AudioDecodeFLAC::onGetLength(FLAC__StreamDecoder const* const, FLAC__uint64* const stream_length, void* const client_data) {
		SELF;
		*stream_length = m_size;
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	}
	FLAC__bool AudioDecodeFLAC::onCheckEof(FLAC__StreamDecoder const* const, void* const client_data) {
		SELF;
		return static_cast<size_t>(m_ptr - m_data) >= m_size;
	}

	FLAC__StreamDecoderWriteStatus AudioDecodeFLAC::onWrite(FLAC__StreamDecoder const* const, FLAC__Frame const* const frame, const FLAC__int32* const buffer[], void* const client_data) {
		SELF;

		// check
		assert(frame->header.channels == 1 || frame->header.channels == 2);
		assert((frame->header.bits_per_sample % 8) == 0 && frame->header.bits_per_sample <= 32);
		assert(frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER); // libflac 文档里说为了便于开发者使用， number 类型永远会被转换为样本索引，如果这个 assert 触发了，请找 xiph

		// save frame info
		auto& current = self->m_flac_frame_data;
		current.sample_rate = frame->header.sample_rate;
		current.channels = static_cast<uint16_t>(frame->header.channels);
		current.bits_per_sample = static_cast<uint16_t>(frame->header.bits_per_sample);
		current.sample_index = static_cast<uint32_t>(frame->header.number.sample_number);
		current.sample_count = frame->header.blocksize;

		// copy data
		current.data[0].resize(frame->header.blocksize);
		current.data[1].resize(frame->header.blocksize);
		for (uint32_t i = 0; i < frame->header.blocksize; i += 1) {
			if (frame->header.channels == 1) {
				current.data[0][i] = buffer[0][i];
			}
			else {
				current.data[0][i] = buffer[0][i];
				current.data[1][i] = buffer[1][i];
			}
		}

		return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	}
	void AudioDecodeFLAC::onMetadata(FLAC__StreamDecoder const* const, FLAC__StreamMetadata const* const metadata, void* const client_data) {
		SELF;
		if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
			self->m_info = metadata->data.stream_info;
			self->m_has_info = true;
		}
	}
	void AudioDecodeFLAC::onError(FLAC__StreamDecoder const* const, FLAC__StreamDecoderErrorStatus const status, void* const) {
		Logger::error("[core] FLAC stream decoder: {}", FLAC__StreamDecoderErrorStatusString[status]);
	}
}
