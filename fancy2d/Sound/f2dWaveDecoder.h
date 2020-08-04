////////////////////////////////////////////////////////////////////////////////
/// @file  f2dWaveDecoder.h
/// @brief fancy2D Wave解码器实现
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyRefObj.h"
#include "fcyIO/fcyBinaryHelper.h"

#include "f2dSoundSys.h"

#include <string>
#include <vector>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////
/// @brief Wave解码器
////////////////////////////////////////////////////////////////////////////////
class f2dWaveDecoder :
	public fcyRefObjImpl<f2dSoundDecoder>
{
private:
	static const char* tagRIFF;
	static const char* tagFMT;
	static const char* tagFACT;
	static const char* tagDATA;
private:
	class IChunk
	{
	public:
		char m_ID[5];
		fuInt m_Size;
	public:
		IChunk(fChar* ID, fuInt Size)
			: m_Size(Size)
		{
			memset(m_ID, 0, 5);
			memcpy(m_ID, ID, 4);
		}
		~IChunk() {}
	};
	class CWaveChunk :
		public IChunk
	{
	public:
		char m_Type[4];
	public:
		CWaveChunk(fChar* ID, fuInt Size, fcyBinaryReader* pReader);
		~CWaveChunk();
	};
	class CFormatChunk :
		public IChunk
	{
	public:
		fuShort m_FormatTag;     ///< @brief 编码方式，一般为0x0001
        fuShort m_Channels;      ///< @brief 声道数目 1（单声道）/2（双声道）
		fuInt m_SamplesPerSec;   ///< @brief 采样频率
		fuInt m_AvgBytesPerSec;  ///< @brief 每秒所需字节数
		fuShort m_BlockAlign;    ///< @brief 数据块对齐单位（每个采样需要的字节数)
		fuShort m_BitsPerSample; ///< @brief 每个采样需要的bit数
		fuShort m_Reserved;      ///< @brief 保留
	public:
		CFormatChunk(fChar* ID, fuInt Size, fcyBinaryReader* pReader);
		~CFormatChunk();
	};
	class CFactChunk :
		public IChunk
	{
	public:
		fuInt m_Data;  // 数据
	public:
		CFactChunk(fChar* ID, fuInt Size, fcyBinaryReader* pReader);
		~CFactChunk();
	};
	class CDataChunk :
		public IChunk
	{
	public:
		fLen m_BasePointer;
	public:
		CDataChunk(fChar* ID, fuInt Size, fcyBinaryReader* pReader);
		~CDataChunk();
	};
private:
	std::vector<IChunk*> m_ChunkList;
	std::unordered_map<std::string, IChunk*> m_Chunk;
	fuInt m_cPointer;
	f2dStream* m_pStream;
private:
	void clear();
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
	f2dWaveDecoder(f2dStream* pStream);
	~f2dWaveDecoder();
};
