#include "Sound/f2dWaveDecoder.h"

#include "fcyException.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
const char* f2dWaveDecoder::tagRIFF = "RIFF";
const char* f2dWaveDecoder::tagFMT = "fmt ";
const char* f2dWaveDecoder::tagFACT = "fact";
const char* f2dWaveDecoder::tagDATA = "data";

f2dWaveDecoder::CWaveChunk::CWaveChunk(fChar* ID, fuInt Size, fcyBinaryReader* pReader)
	: IChunk(ID, Size)
{
	pReader->ReadChars(m_Type, 4);
}

f2dWaveDecoder::CWaveChunk::~CWaveChunk()
{}

f2dWaveDecoder::CFormatChunk::CFormatChunk(fChar* ID, fuInt Size, fcyBinaryReader* pReader)
	: IChunk(ID, Size)
{
	m_FormatTag = pReader->ReadUInt16();
	m_Channels = pReader->ReadUInt16();
	m_SamplesPerSec = pReader->ReadUInt32();
	m_AvgBytesPerSec = pReader->ReadUInt32();
	m_BlockAlign = pReader->ReadUInt16();
	m_BitsPerSample = pReader->ReadUInt16();
	if(Size > 16)
		m_Reserved = pReader->ReadUInt16();
	else
		m_Reserved = 0;
}

f2dWaveDecoder::CFormatChunk::~CFormatChunk()
{}

f2dWaveDecoder::CFactChunk::CFactChunk(fChar* ID, fuInt Size, fcyBinaryReader* pReader)
	: IChunk(ID, Size)
{
	m_Data = pReader->ReadUInt32();
}

f2dWaveDecoder::CFactChunk::~CFactChunk()
{}

f2dWaveDecoder::CDataChunk::CDataChunk(fChar* ID, fuInt Size, fcyBinaryReader* pReader)
	: IChunk(ID, Size)
{
	m_BasePointer = pReader->GetBaseStream()->GetPosition();
	// 跳过Size
	pReader->GetBaseStream()->SetPosition(FCYSEEKORIGIN_CUR, Size);
}

f2dWaveDecoder::CDataChunk::~CDataChunk()
{}

f2dWaveDecoder::f2dWaveDecoder(f2dStream* pStream)
	: m_pStream(pStream), m_cPointer(0)
{
	if(!m_pStream)
		throw fcyException("f2dWaveDecoder::f2dWaveDecoder", "Invalid Pointer.");
	m_pStream->AddRef();

	// 锁定
	m_pStream->Lock();
	m_pStream->SetPosition(FCYSEEKORIGIN_BEG, 0);

	// 读取所有RIFF块
	try
	{
		fcyBinaryReader tReader(pStream);
		while(pStream->GetPosition() < pStream->GetLength())
		{
			char tID[4];
			fuInt tSize = 0;
			tReader.ReadChars(tID, 4);
			tSize = tReader.ReadUInt32();

			IChunk* pChunk = NULL;
			if(strncmp(tID, tagRIFF, 4) == 0)
				pChunk = new CWaveChunk(tID, tSize, &tReader);
			else if(strncmp(tID, tagFMT, 4) == 0)
				pChunk = new CFormatChunk(tID, tSize, &tReader);
			else if(strncmp(tID, tagFACT, 4) == 0)
				pChunk = new CFactChunk(tID, tSize, &tReader);
			else if(strncmp(tID, tagDATA, 4) == 0)
				pChunk = new CDataChunk(tID, tSize, &tReader);
			else
			{
				// 跳过不支持的chunk
				pStream->SetPosition(FCYSEEKORIGIN_CUR, tSize);
			}

			if(pChunk)
			{
				m_ChunkList.push_back(pChunk);
				m_Chunk[pChunk->m_ID] = pChunk;
			}
		}
	}
	catch(const fcyException& e) // 处理读取异常
	{
		m_pStream->Unlock();
		clear();                // 清除无用数据
		FCYSAFEKILL(m_pStream); // 释放引用
		throw;
	}

	// 解锁
	m_pStream->Unlock();

	// 检查区块完整性
	if(m_Chunk.find(tagRIFF) == m_Chunk.end() || m_Chunk.find(tagFMT) == m_Chunk.end() || m_Chunk.find(tagDATA) == m_Chunk.end())
	{
		clear();
		FCYSAFEKILL(m_pStream);
		throw fcyException("f2dWaveDecoder::f2dWaveDecoder", "Chunk Error.");
	};
}

f2dWaveDecoder::~f2dWaveDecoder()
{
	clear();
	FCYSAFEKILL(m_pStream);
}

void f2dWaveDecoder::clear()
{
	vector<IChunk*>::iterator i = m_ChunkList.begin();
	while(i != m_ChunkList.end())
	{
		FCYSAFEDEL(*i);
		++i;
	}
}

fuInt f2dWaveDecoder::GetBufferSize()
{
	CDataChunk* tChunk = (CDataChunk*)(m_Chunk[tagDATA]);
	return tChunk->m_Size;
}

fuInt f2dWaveDecoder::GetAvgBytesPerSec()
{
	CFormatChunk* tChunk = (CFormatChunk*)(m_Chunk[tagFMT]);
	return tChunk->m_AvgBytesPerSec;
}

fuShort f2dWaveDecoder::GetBlockAlign()
{
	CFormatChunk* tChunk = (CFormatChunk*)(m_Chunk[tagFMT]);
	return tChunk->m_BlockAlign;
}

fuShort f2dWaveDecoder::GetChannelCount()
{
	CFormatChunk* tChunk = (CFormatChunk*)(m_Chunk[tagFMT]);
	return tChunk->m_Channels;
}

fuInt f2dWaveDecoder::GetSamplesPerSec()
{
	CFormatChunk* tChunk = (CFormatChunk*)(m_Chunk[tagFMT]);
	return tChunk->m_SamplesPerSec;
}

fuShort f2dWaveDecoder::GetFormatTag()
{
	CFormatChunk* tChunk = (CFormatChunk*)(m_Chunk[tagFMT]);
	return tChunk->m_FormatTag;
}

fuShort f2dWaveDecoder::GetBitsPerSample()
{
	CFormatChunk* tChunk = (CFormatChunk*)(m_Chunk[tagFMT]);
	return tChunk->m_BitsPerSample;
}

fLen f2dWaveDecoder::GetPosition()
{
	return m_cPointer;
}

fResult f2dWaveDecoder::SetPosition(F2DSEEKORIGIN Origin, fInt Offset)
{
	switch(Origin)
	{
	case FCYSEEKORIGIN_CUR:
		break;
	case FCYSEEKORIGIN_BEG:
		m_cPointer = 0;
		break;
	case FCYSEEKORIGIN_END:
		m_cPointer = GetBufferSize();
		break;
	default:
		return FCYERR_INVAILDPARAM;
	}
	if(Offset<0 && ((fuInt)(-Offset))>m_cPointer)
	{
		m_cPointer = 0;
		return FCYERR_OUTOFRANGE;
	}
	else if(Offset>0 && Offset+m_cPointer>=GetBufferSize())
	{
		m_cPointer = GetBufferSize();
		return FCYERR_OUTOFRANGE;
	}
	m_cPointer+=Offset;
	return FCYERR_OK;
}

fResult f2dWaveDecoder::Read(fData pBuffer, fuInt SizeToRead, fuInt* pSizeRead)
{
	// 锁定流
	m_pStream->Lock();

	// 寻址
	CDataChunk* tChunk = (CDataChunk*)(m_Chunk[tagDATA]);
	m_pStream->SetPosition(FCYSEEKORIGIN_BEG, tChunk->m_BasePointer + m_cPointer);

	// 计算读取大小
	fuInt tRest = tChunk->m_Size - m_cPointer;
	if(SizeToRead > tRest)
		SizeToRead = tRest;

	// 从流中读取数据
	fLen tStreamRead = 0;
	m_pStream->ReadBytes(pBuffer, SizeToRead, &tStreamRead);
	if(SizeToRead != tStreamRead)
		SizeToRead = (fuInt)tStreamRead;
	if(pSizeRead)
		*pSizeRead = SizeToRead;

	// 指针后移
	m_cPointer += SizeToRead;
	
	// 解锁流
	m_pStream->Unlock();

	return FCYERR_OK;
}
