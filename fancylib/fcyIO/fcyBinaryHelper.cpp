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

fChar fcyBinaryReader::ReadChar()
{
	fChar tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadChar", "ReadBytes Failed.");
	return tRet;
}

void fcyBinaryReader::ReadChars(fStr OutBuffer, fLen Length)
{
	if(FCYFAILED(m_pStream->ReadBytes((fData)OutBuffer, Length, NULL)))
		throw fcyException("fcyBinaryReader::ReadChars", "ReadBytes Failed.");
}

fCharW fcyBinaryReader::ReadCharW()
{
	fCharW tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadCharW", "ReadBytes Failed.");
	return tRet;
}

void fcyBinaryReader::ReadCharsW(fStrW OutBuffer, fLen Length)
{
	if(FCYFAILED(m_pStream->ReadBytes((fData)OutBuffer, Length*2, NULL)))
		throw fcyException("fcyBinaryReader::ReadCharsW", "ReadBytes Failed.");
}

fByte fcyBinaryReader::ReadByte()
{
	fByte tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadByte", "ReadBytes Failed.");
	return tRet;
}

void fcyBinaryReader::ReadBytes(fData OutBuffer, fLen Length)
{
	if(FCYFAILED(m_pStream->ReadBytes((fData)OutBuffer, Length, NULL)))
		throw fcyException("fcyBinaryReader::ReadBytes", "ReadBytes Failed.");
}

fShort fcyBinaryReader::ReadInt16()
{
	fShort tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadInt16", "ReadBytes Failed.");
	return tRet;
}

fuShort fcyBinaryReader::ReadUInt16()
{
	fuShort tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadUInt16", "ReadBytes Failed.");
	return tRet;
}

fInt fcyBinaryReader::ReadInt32()
{
	fInt tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadInt32", "ReadBytes Failed.");
	return tRet;
}

fuInt fcyBinaryReader::ReadUInt32()
{
	fuInt tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadUInt32", "ReadBytes Failed.");
	return tRet;
}

fLong fcyBinaryReader::ReadInt64()
{
	fLong tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadInt64", "ReadBytes Failed.");
	return tRet;
}

fuLong fcyBinaryReader::ReadUInt64()
{
	fuLong tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadUInt64", "ReadBytes Failed.");
	return tRet;
}

fFloat fcyBinaryReader::ReadFloat()
{
	fFloat tRet = 0;
	if(FCYFAILED(m_pStream->ReadBytes((fData)&tRet, sizeof(tRet), NULL)))
		throw fcyException("fcyBinaryReader::ReadFloat", "ReadBytes Failed.");
	return tRet;
}

fDouble fcyBinaryReader::ReadDouble()
{
	fDouble tRet = 0;
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

void fcyBinaryWriter::Write(fChar Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fcStr Buffer, fLen Length)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)Buffer, Length, NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fCharW Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fcStrW Buffer, fLen Length)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)Buffer, Length, NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fByte Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fcData Buffer, fLen Length)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)Buffer, Length, NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fShort Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fuShort Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fInt Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fuInt Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fLong Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fuLong Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fFloat Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}

void fcyBinaryWriter::Write(fDouble Value)
{
	if(FCYFAILED(m_pStream->WriteBytes((fData)&Value, sizeof(Value), NULL)))
		throw fcyException("fcyBinaryWriter::Write", "WriteBytes Failed.");
}
