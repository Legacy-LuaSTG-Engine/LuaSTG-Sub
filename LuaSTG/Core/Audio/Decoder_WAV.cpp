#include "Core/Audio/Decoder_WAV.hpp"
#include "Core/FileManager.hpp"
#include "utility/encoding.hpp"

namespace Core::Audio
{
	void Decoder_WAV::destroyResources()
	{
		if (m_init)
		{
			m_init = false;
			drwav_uninit(&m_wav);
		}
		m_data.clear();
	}

	uint32_t Decoder_WAV::getFrameCount()
	{
		drwav_uint64 dr_frame_count = 0;
		drwav_result const result = drwav_get_length_in_pcm_frames(&m_wav, &dr_frame_count);
		assert(DRWAV_SUCCESS == result);
		if (DRWAV_SUCCESS != result) return 0;
		return (uint32_t)dr_frame_count;
	}

	bool Decoder_WAV::seek(uint32_t pcm_frame)
	{
		return DRWAV_TRUE == drwav_seek_to_pcm_frame(&m_wav, pcm_frame);
	}
	bool Decoder_WAV::seekByTime(double sec)
	{
		drwav_uint64 const pcm_frame = drwav_uint64(sec * (double)m_wav.sampleRate);
		return DRWAV_TRUE == drwav_seek_to_pcm_frame(&m_wav, pcm_frame);
	}
	bool Decoder_WAV::tell(uint32_t* pcm_frame)
	{
		drwav_uint64 dr_frame_index = 0;
		drwav_result const result = drwav_get_cursor_in_pcm_frames(&m_wav, &dr_frame_index);
		*pcm_frame = (uint32_t)dr_frame_index;
		return DRWAV_SUCCESS == result;
	}
	bool Decoder_WAV::tellAsTime(double* sec)
	{
		drwav_uint64 dr_frame_index = 0;
		drwav_result const result = drwav_get_cursor_in_pcm_frames(&m_wav, &dr_frame_index);
		*sec = (double)dr_frame_index / (double)m_wav.sampleRate;
		return DRWAV_SUCCESS == result;
	}
	bool Decoder_WAV::read(uint32_t pcm_frame, void* buffer, uint32_t* read_pcm_frame)
	{
		drwav_uint64 const dr_frame_count = drwav_read_pcm_frames_s16(&m_wav, pcm_frame, (drwav_int16*)buffer);
		if (read_pcm_frame)
		{
			*read_pcm_frame = (uint32_t)dr_frame_count;
			return pcm_frame == 0 || dr_frame_count > 0; // 似乎永远不会失败
		}
		else
		{
			assert(pcm_frame == dr_frame_count);
			return pcm_frame == dr_frame_count;
		}
	}

	Decoder_WAV::Decoder_WAV(StringView path)
		: m_wav({})
		, m_init(false)
	{
		if (GFileManager().contain(path))
		{
			// 存在于文件系统，直接以文件的形式打开，一般 wav 都贼 TM 大
			drwav_bool32 const result = drwav_init_file_w(&m_wav, utility::encoding::to_wide(path).c_str(), NULL);
			if (DRWAV_TRUE != result)
			{
				destroyResources();
				throw std::runtime_error("Decoder_WAV::Decoder_WAV (1)");
			}
		}
		else if (GFileManager().containEx(path))
		{
			// 在压缩包里的文件，只能读取到内存了
			if (!GFileManager().loadEx(path, m_data))
			{
				destroyResources();
				throw std::runtime_error("Decoder_WAV::Decoder_WAV (2)");
			}
			drwav_bool32 const result = drwav_init_memory(&m_wav, m_data.data(), m_data.size(), NULL);
			if (DRWAV_TRUE != result)
			{
				destroyResources();
				throw std::runtime_error("Decoder_WAV::Decoder_WAV (3)");
			}
		}
		else
		{
			destroyResources();
			throw std::runtime_error("Decoder_WAV::Decoder_WAV (4)");
		}
		m_init = true; // 标记为需要清理
		// 一些断言
		if ((m_wav.bitsPerSample % 8) != 0 || !(m_wav.channels == 1 || m_wav.channels == 2))
		{
			destroyResources();
			throw std::runtime_error("Decoder_WAV::Decoder_WAV (5)");
		}
		// 先逝一逝
		if (!seek(0))
		{
			destroyResources();
			throw std::runtime_error("Decoder_WAV::Decoder_WAV (6)");
		}
	}
	Decoder_WAV::~Decoder_WAV()
	{
		destroyResources();
	}
}
