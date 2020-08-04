#include "ResourceAudio.hpp"

namespace LuaSTGPlus {
	fResult ResMusic::BGMWrapper::Read(fData pBuffer, fuInt SizeToRead, fuInt* pSizeRead)
	{
		fResult tFR;

		// 获得单个采样大小
		fuInt tBlockAlign = GetBlockAlign();

		// 计算需要读取的采样个数
		fuInt tSampleToRead = SizeToRead / tBlockAlign;

		// 填充音频数据
		while (tSampleToRead)
		{
			// 获得当前解码器位置(采样)
			fuInt tCurSample = (fuInt)GetPosition() / tBlockAlign;

			// 检查读取位置是否超出循环节
			if (tCurSample + tSampleToRead > m_pLoopEndSample)
			{
				// 填充尚未填充数据
				if (tCurSample < m_pLoopEndSample)
				{
					fuInt tVaildSample = m_pLoopEndSample - tCurSample;
					fuInt tVaildSize = tVaildSample * tBlockAlign;

					if (FAILED(tFR = m_pDecoder->Read(pBuffer, tVaildSize, pSizeRead)))
						return tFR;

					// 指针后移
					pBuffer += tVaildSize;

					// 减少采样
					tSampleToRead -= tVaildSample;
				}

				// 跳到循环头
				SetPosition(FCYSEEKORIGIN_BEG, m_pLoopStartSample * tBlockAlign);
			}
			else
			{
				// 直接填充数据
				if (FAILED(tFR = m_pDecoder->Read(pBuffer, tSampleToRead * tBlockAlign, pSizeRead)))
					return tFR;

				break;
			}
		}

		if (pSizeRead)
			* pSizeRead = SizeToRead;

		return FCYERR_OK;
	}

	ResMusic::BGMWrapper::BGMWrapper(fcyRefPointer<f2dSoundDecoder> pOrg, fDouble LoopStart, fDouble LoopEnd)
		: m_pDecoder(pOrg)
	{
		LASSERT(pOrg);

		// 计算参数
		m_TotalSample = m_pDecoder->GetBufferSize() / m_pDecoder->GetBlockAlign();

		if (LoopStart < 0)
			LoopStart = 0;
		m_pLoopStartSample = (fuInt)(LoopStart * m_pDecoder->GetSamplesPerSec());

		if (LoopEnd <= 0)
			m_pLoopEndSample = m_TotalSample;
		else
			m_pLoopEndSample = min(m_TotalSample, (fuInt)(LoopEnd * m_pDecoder->GetSamplesPerSec()));

		if (m_pLoopEndSample < m_pLoopStartSample)
			std::swap(m_pLoopStartSample, m_pLoopEndSample);

		if (m_pLoopEndSample == m_pLoopStartSample)
			throw fcyException("ResMusic::BGMWrapper::BGMWrapper", "Invalid loop period.");
	}
}
