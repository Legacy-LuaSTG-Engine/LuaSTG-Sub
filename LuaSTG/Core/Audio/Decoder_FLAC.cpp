#include "Core/Audio/Decoder_FLAC.hpp"
#include "Core/FileManager.hpp"
#include "utility/encoding.hpp"

namespace LuaSTG::Core::Audio
{
	inline Decoder_FLAC& cast(void* client_data) { return *(Decoder_FLAC*)client_data; }

	// 进用于内存中的 FLAC 文件

	FLAC__StreamDecoderReadStatus Decoder_FLAC::onRead(FLAC__StreamDecoder const*, FLAC__byte buffer[], size_t* bytes, void* client_data)
	{
		assert(bytes);

		Decoder_FLAC& self = cast(client_data);

		// 看看还剩下多少能读的
		size_t const valid_size = self.m_data.size() - (self.m_ptr - self.m_data.data());
		if (valid_size == 0)
		{
			// 寄了
			*bytes = 0;
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		}

		assert(buffer);

		// 复制数据，也可能读取的大小比要求的大小要小
		size_t const read_size = std::min(valid_size, *bytes);
		std::memcpy(buffer, self.m_ptr, read_size);
		self.m_ptr += read_size; // 记得移动指针
		*bytes = read_size;
		return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
	}
	FLAC__StreamDecoderSeekStatus Decoder_FLAC::onSeek(FLAC__StreamDecoder const*, FLAC__uint64 absolute_byte_offset, void* client_data)
	{
		Decoder_FLAC& self = cast(client_data);
		assert(absolute_byte_offset <= UINT32_MAX); // TODO: 谁那么丧心病狂整超过 4GB 的文件呢
		if (absolute_byte_offset > self.m_data.size())
		{
			self.m_ptr = self.m_data.data() + self.m_data.size(); // 限制位置
			return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
		}
		else
		{
			self.m_ptr = self.m_data.data() + absolute_byte_offset;
			return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
		}
	}
	FLAC__StreamDecoderTellStatus Decoder_FLAC::onTell(FLAC__StreamDecoder const*, FLAC__uint64* absolute_byte_offset, void* client_data)
	{
		Decoder_FLAC& self = cast(client_data);
		*absolute_byte_offset = (FLAC__uint64)(self.m_ptr - self.m_data.data());
		return FLAC__STREAM_DECODER_TELL_STATUS_OK;
	}
	FLAC__StreamDecoderLengthStatus Decoder_FLAC::onGetLength(FLAC__StreamDecoder const*, FLAC__uint64* stream_length, void* client_data)
	{
		Decoder_FLAC& self = cast(client_data);
		*stream_length = self.m_data.size();
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	}
	FLAC__bool Decoder_FLAC::onCheckEOF(FLAC__StreamDecoder const*, void* client_data)
	{
		Decoder_FLAC& self = cast(client_data);
		return (self.m_ptr - self.m_data.data()) >= (ptrdiff_t)self.m_data.size();
	}

	// 公共回调

	FLAC__StreamDecoderWriteStatus Decoder_FLAC::onWrite(FLAC__StreamDecoder const*, FLAC__Frame const* frame, const FLAC__int32* const buffer[], void* client_data)
	{
		Decoder_FLAC& self = cast(client_data);

		// 先康康你发育的正不正常
		assert(frame->header.channels == 1 || frame->header.channels == 2);
		assert((frame->header.bits_per_sample % 8) == 0 && frame->header.bits_per_sample <= 32);
		assert(frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER); // libflac 文档里说为了便于开发者使用， number 类型永远会被转换为样本索引，如果这个 assert 触发了，请找 xiph

		// 记录信息
		auto& cdata = self.m_flac_frame_data;
		cdata.sample_rate = frame->header.sample_rate;
		cdata.channels = (uint16_t)frame->header.channels;
		cdata.bits_per_sample = (uint16_t)frame->header.bits_per_sample;
		cdata.sample_index = (uint32_t)frame->header.number.sample_number;
		cdata.sample_count = frame->header.blocksize;

		// 复制数据
		cdata.data[0].resize(frame->header.blocksize);
		cdata.data[1].resize(frame->header.blocksize);
		for (uint32_t i = 0; i < frame->header.blocksize; i += 1)
		{
			if (frame->header.channels == 1)
			{
				cdata.data[0][i] = buffer[0][i];
			}
			else
			{
				cdata.data[0][i] = buffer[0][i];
				cdata.data[1][i] = buffer[1][i];
			}
		}

		return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	}
	void Decoder_FLAC::onMetadata(FLAC__StreamDecoder const*, FLAC__StreamMetadata const* metadata, void* client_data)
	{
		Decoder_FLAC& self = cast(client_data);

		if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
		{
			self.m_info = metadata->data.stream_info;
			self.m_has_info = true;
		}
	}
	void Decoder_FLAC::onError(FLAC__StreamDecoder const*, FLAC__StreamDecoderErrorStatus status, void*)
	{
		spdlog::error("[core] FLAC stream decoder: {}", FLAC__StreamDecoderErrorStatusString[status]);
	}
}

namespace LuaSTG::Core::Audio
{
	void Decoder_FLAC::destroyResources()
	{
		// 关闭解码器
		if (m_init)
		{
			m_init = false;
			if (m_flac)
			{
				FLAC__stream_decoder_finish(m_flac);
			}
		}
		m_has_info = false;
		// 销毁解码器对象
		if (m_flac)
		{
			FLAC__stream_decoder_delete(m_flac);
			m_flac = NULL;
		}
		// 关闭文件、清理分配的内存
		if (m_file)
		{
			fclose(m_file);
			m_file = NULL;
		}
		m_ptr = nullptr;
		m_data.clear();
	}

	uint16_t Decoder_FLAC::getSampleSize()
	{
		return (uint16_t)m_info.bits_per_sample / uint16_t(8);
	}
	uint16_t Decoder_FLAC::getChannelCount()
	{
		return (uint16_t)m_info.channels;
	}
	uint16_t Decoder_FLAC::getFrameSize()
	{
		return getSampleSize() * getChannelCount();
	}
	uint32_t Decoder_FLAC::getSampleRate()
	{
		return m_info.sample_rate;
	}
	uint32_t Decoder_FLAC::getByteRate()
	{
		return getSampleRate() * getFrameSize();
	}
	uint32_t Decoder_FLAC::getFrameCount()
	{
		return (uint32_t)m_info.total_samples; // TODO: 支持大文件
	}

	bool Decoder_FLAC::seek(uint32_t pcm_frame)
	{
		if (FLAC__STREAM_DECODER_SEEK_ERROR == FLAC__stream_decoder_get_state(m_flac))
		{
			if (!FLAC__stream_decoder_flush(m_flac))
			{
				return false;
			}
		}
		m_current_pcm_frame = pcm_frame;
		m_flac_frame_data.clear();
		return FLAC__stream_decoder_seek_absolute(m_flac, pcm_frame);
	}
	bool Decoder_FLAC::seekByTime(double sec)
	{
		if (FLAC__STREAM_DECODER_SEEK_ERROR == FLAC__stream_decoder_get_state(m_flac))
		{
			if (!FLAC__stream_decoder_flush(m_flac))
			{
				return false;
			}
		}
		FLAC__uint64 const pcm_frame = FLAC__uint64(sec * (double)m_info.sample_rate);
		m_current_pcm_frame = (uint32_t)pcm_frame;
		m_flac_frame_data.clear();
		return FLAC__stream_decoder_seek_absolute(m_flac, pcm_frame);
	}
	bool Decoder_FLAC::tell(uint32_t* pcm_frame)
	{
		*pcm_frame = m_current_pcm_frame;
		return true;
	}
	bool Decoder_FLAC::tellAsTime(double* sec)
	{
		*sec = (double)m_current_pcm_frame / (double)m_info.sample_rate;
		return true;
	}

	bool Decoder_FLAC::read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame)
	{
		uint32_t read_pcm_frame_ = 0;

		while (pcm_frame > 0)
		{
			// 先康康有没有没处理完的 frame
			if (!m_flac_frame_data.empty())
			{
				if (m_current_pcm_frame >= m_flac_frame_data.sample_index && m_current_pcm_frame < (m_flac_frame_data.sample_index + m_flac_frame_data.sample_count))
				{
					// 计算参数
					uint32_t const pcm_frame_offset = m_current_pcm_frame - m_flac_frame_data.sample_index;
					uint32_t const valid_pcm_frame = (m_flac_frame_data.sample_index + m_flac_frame_data.sample_count) - m_current_pcm_frame;
					uint32_t const should_read_pcm_frame = std::min(pcm_frame, valid_pcm_frame);
					// 写入
					uint8_t* ptr = (uint8_t*)buffer;
					for (uint32_t idx = 0; idx < should_read_pcm_frame; idx += 1)
					{
						for (uint16_t chs = 0; chs < m_flac_frame_data.channels; chs += 1)
						{
							// 拆数据
							int32_t const sample = m_flac_frame_data.data[chs][pcm_frame_offset + idx];
							uint8_t const bytes[4] = {
								(uint8_t)((sample & 0x000000FF)      ),
								(uint8_t)((sample & 0x0000FF00) >> 8 ),
								(uint8_t)((sample & 0x00FF0000) >> 16),
								(uint8_t)((sample & 0xFF000000) >> 24),
							};
							// 看位深来写入数据，Windows 一般是小端，如果是大端平台需要把上面的 bytes 数组颠倒
							for (uint16_t bid = 0; bid < (m_flac_frame_data.bits_per_sample / 8); bid += 1)
							{
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
					if (pcm_frame == 0)
					{
						*read_pcm_frame = read_pcm_frame_;
						return true;
					}
				}
				else
				{
					m_flac_frame_data.clear(); // 不在范围内，可以清理了
				}
			}

			// 还需要继续读取
			if (!FLAC__stream_decoder_process_single(m_flac))
			{
				// 发生错误
				*read_pcm_frame = read_pcm_frame_;
				return false;
			}
		}

		// 没有了
		*read_pcm_frame = read_pcm_frame_;
		return true;
	}

	Decoder_FLAC::Decoder_FLAC(StringView path)
		: m_file(NULL)
		, m_flac(NULL)
	{
		// 第一步：打开文件/加载文件
		// WARNING: 记得多测试一下在内存中打开能不能正常读写，行为是不是 libFLAC 期望的

		if (GFileManager().contain(path))
		{
			// 存在于文件系统，直接以文件的形式打开，flac 文件的大小也是有点离谱的
			errno_t const result = _wfopen_s(&m_file, utility::encoding::to_wide(path).c_str(), L"rb");
			if (0 != result)
			{
				destroyResources();
				throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (1.1)");
			}
		}
		else if (GFileManager().containEx(path))
		{
			// 在压缩包里的文件，只能读取到内存了
			if (!GFileManager().loadEx(path, m_data))
			{
				destroyResources();
				throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (1.2)");
			}
			m_ptr = m_data.data(); // 先到文件头
		}
		else
		{
			destroyResources();
			throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (1.3)");
		}

		// 第二步：创建解码器

		m_flac = FLAC__stream_decoder_new();
		if (NULL == m_flac)
		{
			destroyResources();
			throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (2.1)");
		}

		FLAC__stream_decoder_set_md5_checking(m_flac, true); // TODO: 应该开启 md5 验证？还是关掉比较好？

		if (m_file)
		{
			// 以文件的方式解码
			FLAC__StreamDecoderInitStatus flac_init = FLAC__STREAM_DECODER_INIT_STATUS_OK;
			flac_init = FLAC__stream_decoder_init_FILE(m_flac, m_file, &onWrite, NULL, &onError, this);
			if (FLAC__STREAM_DECODER_INIT_STATUS_OK != flac_init)
			{
				// 容器或者格式不对？
				if (0 != _fseeki64(m_file, 0, SEEK_SET)) // 先回到文件头
				{
					// 你怎么也寄了
					destroyResources();
					throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (2.2)");
				}
				flac_init = FLAC__stream_decoder_init_ogg_FILE(m_flac, m_file, &onWrite, NULL, &onError, this);
			}
			if (FLAC__STREAM_DECODER_INIT_STATUS_OK != flac_init)
			{
				// 那大概就是不行了
				destroyResources();
				throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (2.3)");
			}
		}
		else
		{
			// 以内存流的方式打开
			FLAC__StreamDecoderInitStatus flac_init = FLAC__STREAM_DECODER_INIT_STATUS_OK;
			flac_init = FLAC__stream_decoder_init_stream(m_flac, &onRead, &onSeek, &onTell, &onGetLength, onCheckEOF, &onWrite, NULL, &onError, this);
			if (FLAC__STREAM_DECODER_INIT_STATUS_OK != flac_init)
			{
				// 容器或者格式不对？
				m_ptr = m_data.data(); // 先回到文件头
				flac_init = FLAC__stream_decoder_init_ogg_stream(m_flac, &onRead, &onSeek, &onTell, &onGetLength, onCheckEOF, &onWrite, NULL, &onError, this);
			}
			if (FLAC__STREAM_DECODER_INIT_STATUS_OK != flac_init)
			{
				// 那大概就是不行了
				destroyResources();
				throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (2.4)");
			}
		}
		m_init = true; // 标记为需要清理

		// 第三步：读取所有 metadata

		if (!FLAC__stream_decoder_process_until_end_of_metadata(m_flac))
		{
			destroyResources();
			throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (3.1)");
		}
		if (!m_has_info)
		{
			// 暂时不支持没有 info 的格式
			//destroyResources();
			//throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (3.2)");

			// 或者，让我们变通一点，先解码一波
			if (!FLAC__stream_decoder_process_single(m_flac))
			{
				destroyResources();
				throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (3.2.1)");
			}

			if (m_flac_frame_data.empty())
			{
				destroyResources();
				throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (3.2.2)");
			}
			m_info.bits_per_sample = m_flac_frame_data.bits_per_sample;
			m_info.channels = m_flac_frame_data.channels;
			m_info.sample_rate = m_flac_frame_data.sample_rate;
			m_info.total_samples = m_flac_frame_data.sample_count; // 起始

			m_flac_frame_data.clear();
			while (FLAC__stream_decoder_process_single(m_flac))
			{
				if (m_flac_frame_data.empty())
					break;
				if (m_info.bits_per_sample != m_flac_frame_data.bits_per_sample
					|| m_info.channels != m_flac_frame_data.channels
					|| m_info.sample_rate != m_flac_frame_data.sample_rate)
				{
					// 真可惜，你没能一直保持自我
					destroyResources();
					throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (3.2.3)");
				}
				m_info.total_samples += m_flac_frame_data.sample_count; // 增加
				m_flac_frame_data.clear();
			}

			m_has_info = true;
		}
		if ((m_info.bits_per_sample % 8) != 0 || !(m_info.channels == 1 || m_info.channels == 2))
		{
			// 暂时不支持其他格式
			destroyResources();
			throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (3.3)");
		}

		// 第四步：先逝一逝

		if (!FLAC__stream_decoder_flush(m_flac))
		{
			destroyResources();
			throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (4.1)");
		}
		if (!FLAC__stream_decoder_seek_absolute(m_flac, 0))
		{
			destroyResources();
			throw std::runtime_error("Decoder_FLAC::Decoder_FLAC (4.2)");
		}
	}
	Decoder_FLAC::~Decoder_FLAC()
	{
		destroyResources();
	}
}
