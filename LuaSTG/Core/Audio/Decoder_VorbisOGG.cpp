#include "Core/Audio/Decoder_VorbisOGG.hpp"
#include "Core/FileManager.hpp"

namespace Core::Audio
{
	inline Decoder_VorbisOGG::OggVorbis_Stream* _cast(void* datasource) { return (Decoder_VorbisOGG::OggVorbis_Stream*)datasource; }

	size_t Decoder_VorbisOGG::OggVorbis_Stream::read(void* ptr, size_t size, size_t nmemb, void* datasource)
	{
		OggVorbis_Stream* self = _cast(datasource);
		size_t const right_size = self->size - (self->ptr - self->data);
		size_t const valid_count = right_size / size;
		size_t const count = (nmemb <= valid_count) ? nmemb : valid_count;
		size_t const should_read_size = count * size;
		std::memcpy(ptr, self->ptr, should_read_size);
		self->ptr += should_read_size;
		return count;
	}
	int Decoder_VorbisOGG::OggVorbis_Stream::seek(void* datasource, ogg_int64_t offset, int whence)
	{
		OggVorbis_Stream* self = _cast(datasource);
		if (whence == SEEK_SET)
		{
			assert(offset >= 0);
			if ((size_t)offset > self->size)
			{
				self->ptr = self->data + self->size;
				return -1;
			}
			else
			{
				self->ptr = self->data + offset;
				return 0;
			}
		}
		else if (whence == SEEK_END)
		{
			assert(offset <= 0);
			if ((size_t)(-offset) > self->size)
			{
				self->ptr = self->data;
				return -1;
			}
			else
			{
				self->ptr = (self->data + self->size) + offset;
				return 0;
			}
		}
		else if (whence == SEEK_CUR)
		{
			if (offset == 0)
			{
				return 0;
			}
			else if (offset > 0)
			{
				size_t const right_size = self->size - (self->ptr - self->data);
				if ((size_t)offset > right_size)
				{
					self->ptr = self->data + self->size;
					return -1;
				}
				else
				{
					self->ptr += offset;
					return 0;
				}
			}
			else // if (offset < 0)
			{
				size_t const left_size = self->ptr - self->data;
				if ((size_t)(-offset) > left_size)
				{
					self->ptr = self->data;
					return -1;
				}
				else
				{
					self->ptr += offset;
					return 0;
				}
			}
		}
		else
		{
			assert(false);
			return -1;
		}
	}
	int Decoder_VorbisOGG::OggVorbis_Stream::close(void*)
	{
		return 0; // 不需要处理
	}
	long Decoder_VorbisOGG::OggVorbis_Stream::tell(void* datasource)
	{
		OggVorbis_Stream* self = _cast(datasource);
		return (long)(self->ptr - self->data);
	}
	
	void Decoder_VorbisOGG::destroyResources()
	{
		if (m_init)
		{
			m_init = false;
			ov_clear(&m_ogg);
		}
		m_data.clear();
	}

	uint16_t Decoder_VorbisOGG::getChannelCount()
	{
		vorbis_info* p_info = ov_info(&m_ogg, -1);
		assert(p_info);
		if (p_info)
		{
			return (uint16_t)p_info->channels;
		}
		else
		{
			return 0;
		}
	}
	uint32_t Decoder_VorbisOGG::getSampleRate()
	{
		vorbis_info* p_info = ov_info(&m_ogg, -1);
		assert(p_info);
		if (p_info)
		{
			return (uint32_t)p_info->rate;
		}
		else
		{
			return 0;
		}
	}
	uint32_t Decoder_VorbisOGG::getFrameCount()
	{
		ogg_int64_t const pcm_frame_count = ov_pcm_total(&m_ogg, -1);
		assert(pcm_frame_count >= 0); // OV_EINVAL
		return (pcm_frame_count > 0) ? (uint32_t)pcm_frame_count : 0;
	}

	bool Decoder_VorbisOGG::seek(uint32_t pcm_frame)
	{
		return 0 == ov_pcm_seek(&m_ogg, (ogg_int64_t)pcm_frame);
	}
	bool Decoder_VorbisOGG::seekByTime(double sec)
	{
		return 0 == ov_time_seek(&m_ogg, sec);
	}
	bool Decoder_VorbisOGG::tell(uint32_t* pcm_frame)
	{
		ogg_int64_t const ogg_pcm_frame = ov_pcm_tell(&m_ogg);
		assert(ogg_pcm_frame >= 0); // OV_EINVAL
		*pcm_frame = (ogg_pcm_frame >= 0) ? (uint32_t)ogg_pcm_frame : 0;
		return ogg_pcm_frame >= 0;
	}
	bool Decoder_VorbisOGG::tellAsTime(double* sec)
	{
		double const time = ov_time_tell(&m_ogg);
		assert(time >= 0.0); // 它失败会返回 double(OV_EINVAL)
		*sec = (time > 0.0) ? time : 0.0;
		return time >= 0.0;
	}
	bool Decoder_VorbisOGG::read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame)
	{
		uint32_t const target_read_size = pcm_frame * getFrameSize();
		uint32_t read_size = 0;
		char* ptr = (char*)buffer;
		int current_stream = 0;
		while (read_size < target_read_size)
		{
			long const result = ov_read(&m_ogg, ptr, (int)(target_read_size - read_size), 0, 2, 1, &current_stream);
			if (result < 0)
			{
				assert(false); return false; // 失败了
			}
			else if (result == 0)
			{
				break; // 读取到文件尾
			}
			else
			{
				read_size += result;
				ptr += result;
			}
		}
		if (read_pcm_frame)
		{
			*read_pcm_frame = read_size / (uint32_t)getFrameSize();
			return true;
		}
		else
		{
			assert(read_size == target_read_size);
			return read_size == target_read_size;
		}
	}

	Decoder_VorbisOGG::Decoder_VorbisOGG(StringView path)
		: m_stream({})
		, m_ogg({})
		, m_init(false)
	{
		// OGG 直接读取进内存，加快解码
		if (!GFileManager().loadEx(path, m_data))
		{
			destroyResources();
			throw std::runtime_error("Decoder_VorbisOGG::Decoder_VorbisOGG (1)");
		}

		m_stream.data = m_data.data();
		m_stream.size = m_data.size();
		m_stream.ptr = m_data.data();

		ov_callbacks callbacks = {
			&OggVorbis_Stream::read,
			&OggVorbis_Stream::seek,
			&OggVorbis_Stream::close,
			&OggVorbis_Stream::tell,
		};
		int const result = ov_open_callbacks(&m_stream, &m_ogg, NULL, 0, callbacks);
		if (result != 0)
		{
			destroyResources();
			throw std::runtime_error("Decoder_VorbisOGG::Decoder_VorbisOGG (2)");
		}
		m_init = true; // 标记为需要清理

		// 一些断言
		vorbis_info* p_info = ov_info(&m_ogg, -1);
		if (!p_info)
		{
			destroyResources();
			throw std::runtime_error("Decoder_VorbisOGG::Decoder_VorbisOGG (3)");
		}
		if (!(p_info->channels == 1 || p_info->channels == 2))
		{
			destroyResources();
			throw std::runtime_error("Decoder_VorbisOGG::Decoder_VorbisOGG (4)");
		}

		// 先逝一下
		if (!seek(0))
		{
			destroyResources();
			throw std::runtime_error("Decoder_VorbisOGG::Decoder_VorbisOGG (5)");
		}
	}
	Decoder_VorbisOGG::~Decoder_VorbisOGG()
	{
		destroyResources();
	}
}
