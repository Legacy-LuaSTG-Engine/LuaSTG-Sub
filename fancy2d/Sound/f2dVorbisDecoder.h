////////////////////////////////////////////////////////////////////////////////
/// @file  f2dVorbisDecoder.h
/// @brief fancy2D OGG解码器实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO/fcyStream.h"

#include "f2dSoundSys.h"

// ogg & vorbis
#include <vorbis/vorbisfile.h>

////////////////////////////////////////////////////////////////////////////////
/// @brief OggVorbis解码器
////////////////////////////////////////////////////////////////////////////////
class f2dVorbisDecoder:
	public fcyRefObjImpl<f2dSoundDecoder>
{
private:
	OggVorbis_File m_OggFile;
	f2dStream* m_pStream;
	int m_CurrentSec;
	fLen m_CurrentPos;
private:
	static size_t streamReadFunc(void *ptr, size_t size, size_t nmemb, void *datasource);
	static int streamSeekFunc(void *datasource, ogg_int64_t offset, int whence);
	static int streamCloseFunc(void *datasource);
	static long streamTellFunc(void *datasource);
public: // 接口实现
	fuInt GetBufferSize();
	fuInt GetAvgBytesPerSec();
	fuShort GetBlockAlign();
	fuShort GetChannelCount();
	fuInt GetSamplesPerSec();
	fuShort GetFormatTag();
	fuShort GetBitsPerSample();

	fLen GetPosition();
	fResult SetPosition(F2DSEEKORIGIN Origin, fInt Offset);
	fResult Read(fData pBuffer, fuInt SizeToRead, fuInt* pSizeRead);
public:
	f2dVorbisDecoder(f2dStream* pStream);
	~f2dVorbisDecoder();
};
