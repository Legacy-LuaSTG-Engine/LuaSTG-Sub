#include "fcyIO/fcyBinaryHelper.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

fcyBinaryReader::fcyBinaryReader(fcyStream* pStream)
	: m_pStream(pStream)
{
	if(!m_pStream)
		throw fcyException("fcyBinaryReader::fcyBinaryReader", "Invalid Pointer.");

	m_pStream->AddRef();
}

fcyBinaryReader::~fcyBinaryReader()
{
	FCYSAFEKILL(m_pStream);
}

fcyStream* fcyBinaryReader::GetBaseStream()
{
	return m_pStream;
}

char fcyBinaryReader::ReadChar()
{
	char tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadChar", "ReadBytes Failed.");
	return tRet;
}

void fcyBinaryReader::ReadChars(char* OutBuffer, uint64_t Length)
{
	if(FCYFAILED(m_pStream->ReadBytes((fData)OutBuffer, Length, NULL)))
		throw fcyException("fcyBinaryReader::ReadChars", "ReadBytes Failed.");
}

wchar_t fcyBinaryReader::ReadCharW()
{
	wchar_t tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadCharW", "ReadBytes Failed.");
	return tRet;
}

void fcyBinaryReader::ReadCharsW(fStrW OutBuffer, uint64_t Length)
{
	if(FCYFAILED(m_pStream->ReadBytes((fData)OutBuffer, Length*2, NULL)))
		throw fcyException("fcyBinaryReader::ReadCharsW", "ReadBytes Failed.");
}

uint8_t fcyBinaryReader::ReadByte()
{
	uint8_t tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadByte", "ReadBytes Failed.");
	return tRet;
}

void fcyBinaryReader::ReadBytes(fData OutBuffer, uint64_t Length)
{
	if(FCYFAILED(m_pStream->ReadBytes((fData)OutBuffer, Length, NULL)))
		throw fcyException("fcyBinaryReader::ReadBytes", "ReadBytes Failed.");
}

int16_t fcyBinaryReader::ReadInt16()
{
	int16_t tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadInt16", "ReadBytes Failed.");
	return tRet;
}

uint16_t fcyBinaryReader::ReadUInt16()
{
	uint16_t tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadUInt16", "ReadBytes Failed.");
	return tRet;
}

int32_t fcyBinaryReader::ReadInt32()
{
	int32_t tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadInt32", "ReadBytes Failed.");
	return tRet;
}

uint32_t fcyBinaryReader::ReadUInt32()
{
	uint32_t tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadUInt32", "ReadBytes Failed.");
	return tRet;
}

int64_t fcyBinaryReader::ReadInt64()
{
	int64_t tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadInt64", "ReadBytes Failed.");
	return tRet;
}

uint64_t fcyBinaryReader::ReadUInt64()
{
	uint64_t tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadUInt64", "ReadBytes Failed.");
	return tRet;
}

float fcyBinaryReader::ReadFloat()
{
	float tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadFloat", "ReadBytes Failed.");
	return tRet;
}

double fcyBinaryReader::ReadDouble()
{
	double tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadDouble", "ReadBytes Failed.");
	return tRet;
}

////////////////////////////////////////////////////////////////////////////////

fcyBinaryWriter::fcyBinaryWriter(fcyStream* pStream)
	: m_pStream(pStream)
{
	if(!m_pStream)
		throw fcyException("fcyBinaryWriter::fcyBinaryWriter", "Invalid Pointer.");
	if(!m_pStream->CanWrite())
		throw fcyException("fcyBinaryWriter::fcyBinaryWriter", "Stream is readonly.");

	m_pStream->AddRef();
}

fcyBinaryWriter::~fcyBinaryWriter()
{
	FCYSAFEKILL(m_pStream);
}

fcyStream* fcyBinaryWriter::GetBaseStream()
{
	return m_pStream;
}

void fcyBinaryWriter::Write(char Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fcStr Buffer, uint64_t Length)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)Buffer, Length, NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(wchar_t Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fcStrW Buffer, uint64_t Length)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)Buffer, Length, NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(uint8_t Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fcData Buffer, uint64_t Length)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)Buffer, Length, NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(int16_t Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(uint16_t Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(int32_t Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(uint32_t Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(int64_t Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(uint64_t Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(float Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(double Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}
