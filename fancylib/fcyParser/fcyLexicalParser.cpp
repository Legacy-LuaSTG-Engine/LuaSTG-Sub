#include "fcyParser/fcyLexicalParser.h"

#include "fcyMisc/fcyStringHelper.h"
#include <Windows.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

fcyLexicalException::fcyLexicalException(fcStr Src, fcStr Desc, uint32_t Line, uint32_t Row)
	: fcyException(Src, Desc), m_Line(Line), m_Row(Row)
{
	m_ExcpDesc += (string("(at Line:") + 
		fcyStringHelper::ToStr((int32_t)Line) + 
		" Row:" +
		fcyStringHelper::ToStr((int32_t)Row) +
		")");
}

fcyLexicalException::~fcyLexicalException(void)
{}

uint32_t fcyLexicalException::GetLine()
{
	return m_Line;
}

uint32_t fcyLexicalException::GetRow()
{
	return m_Row;
}

////////////////////////////////////////////////////////////////////////////////

fcyLexicalReader::fcyLexicalReader(const wstring& SrcText)
	: m_Str(SrcText), m_Pos(0)
{
	if(m_Str.size() && m_Str[0] == (wchar_t)0xFEFF)
	{
		m_Str.erase(0,1);
	}

	m_Len = m_Str.size();
}

fcyLexicalReader::fcyLexicalReader(fcyStream* pStream)
	: m_Pos(0)
{
	if(!pStream)
		throw fcyException("fcyLexicalReader::fcyLexicalReader", "Param 'pStream' is null.");

	m_Str = preprocess(pStream);
	m_Len = m_Str.size();
}

fcyLexicalReader::~fcyLexicalReader(void)
{
}

bool fcyLexicalReader::checkUTF8(fcyStream* pStream)
{
	uint64_t tPos = pStream->GetPosition();

	// 尝试读取BOM头
	uint8_t tBomCache[3];
	if(FCYOK(pStream->ReadBytes(tBomCache, 3, NULL)))
	{
		uint8_t tUTF8[3] = { 0xEF, 0xBB, 0xBF };
		if(memcmp(tBomCache, tUTF8, 3)==0)
			return true;
		else
			pStream->SetPosition(FCYSEEKORIGIN_BEG, tPos);
	}
	else
			pStream->SetPosition(FCYSEEKORIGIN_BEG, tPos);

	return false;
}

bool fcyLexicalReader::checkUTF16LE(fcyStream* pStream)
{
	uint64_t tPos = pStream->GetPosition();

	// 尝试读取BOM头
	uint8_t tBomCache[2];
	if(FCYOK(pStream->ReadBytes(tBomCache, 2, NULL)))
	{
		uint8_t tUTF16LE[2] = { 0xFF, 0xFE };
		if(memcmp(tBomCache, tUTF16LE, 2)==0)
			return true;
		else
			pStream->SetPosition(FCYSEEKORIGIN_BEG, tPos);
	}
	else
		pStream->SetPosition(FCYSEEKORIGIN_BEG, tPos);

	return false;
}

wstring fcyLexicalReader::preprocess(fcyStream* pStream)
{
	if(checkUTF16LE(pStream))
	{
		wstring tRet;
		uint64_t tSize = pStream->GetLength() - pStream->GetPosition();
		tRet.resize((size_t)tSize / sizeof(wchar_t));
		if(tRet.length())
			pStream->ReadBytes((fData)tRet.data(), tSize, NULL);
		return tRet;
	}
	else if(checkUTF8(pStream))
	{
		string tTemp;
		uint64_t tSize = pStream->GetLength() - pStream->GetPosition();
		tTemp.resize((size_t)tSize);
		if(tTemp.length())
			pStream->ReadBytes((fData)tTemp.data(), tSize, NULL);
		return fcyStringHelper::MultiByteToWideChar(tTemp, CP_UTF8);
	}
	else
	{
		string tTemp;
		uint64_t tSize = pStream->GetLength() - pStream->GetPosition();
		tTemp.resize((size_t)tSize);
		if(tTemp.length())
			pStream->ReadBytes((fData)tTemp.data(), tSize, NULL);
		return fcyStringHelper::MultiByteToWideChar(tTemp, CP_OEMCP);
	}
}

uint32_t fcyLexicalReader::GetLine()
{
	uint32_t tLine = 0;

	wchar_t tLast = L'\0';
	for(uint32_t i = 0; i < m_Pos; ++i)
	{
		if(tLast == L'\n')
			tLine++;

		tLast = m_Str[i];
	}

	return tLine;
}

uint32_t fcyLexicalReader::GetRow()
{
	uint32_t tRow = 0;

	wchar_t tLast = L'\0';
	for(uint32_t i = 0; i < m_Pos; ++i)
	{
		if(tLast == L'\n')
			tRow = 0;
		
		tRow++;

		tLast = m_Str[i];
	}

	return tRow;
}

bool fcyLexicalReader::IsEOF()
{
	return (m_Pos >= m_Len);
}

wchar_t fcyLexicalReader::ReadChar()
{
	if(IsEOF())
		throw fcyLexicalException("fcyLexicalReader::ReadChar", "EOF.", GetLine(), GetRow());

	wchar_t tRet = m_Str[m_Pos];
	m_Pos++;

	return tRet;
}

wchar_t fcyLexicalReader::PeekChar()
{
	if(IsEOF())
		throw fcyLexicalException("fcyLexicalReader::PeekChar", "EOF.", GetLine(), GetRow());

	wchar_t tRet = m_Str[m_Pos];

	return tRet;
}

void fcyLexicalReader::Match(wchar_t Char, bool bIgnoreSpace)
{
	if(bIgnoreSpace)
		IgnoreSpace();

	wchar_t tChar = ReadChar();

	if(tChar != Char)
	{
		wchar_t tTextBuffer[1024];
		swprintf_s(tTextBuffer, L"'%c' expected, but found '%c'.", Char, tChar);
		
		throw fcyLexicalException("fcyLexicalReader::Match", 
			fcyStringHelper::WideCharToMultiByte(tTextBuffer).c_str(), 
			GetLine(), GetRow()); 
	}
}

void fcyLexicalReader::Match(fcStrW Str, bool bIgnoreSpace)
{
	if(bIgnoreSpace)
		IgnoreSpace();

	uint32_t tLen = wcslen(Str);

	for(uint32_t i = 0; i<tLen; ++i)
	{
		wchar_t tChar = ReadChar();
		if(tChar != Str[i])
		{
			wchar_t tTextBuffer[1024];
			swprintf_s(tTextBuffer, L"\"%s\" expected, but found '%c'.", Str, tChar);

			throw fcyLexicalException("fcyLexicalReader::Match", 
				fcyStringHelper::WideCharToMultiByte(tTextBuffer).c_str(), 
				GetLine(), GetRow()); 
		}
	}
}

bool fcyLexicalReader::TryMatch(wchar_t Char, bool bIgnoreSpace, bool bMatch)
{
	uint32_t tPos = m_Pos;

	if(bIgnoreSpace)
		IgnoreSpace();

	if(IsEOF())
	{
		m_Pos = tPos;
		return false;
	}

	wchar_t tChar = ReadChar();
	if(tChar != Char)
	{
		// 不匹配，返回假
		m_Pos = tPos;
		return false;
	}
	else
	{
		// 匹配
		if(!bMatch)
		{
			m_Pos = tPos;
		}
		return true;
	}
}

bool fcyLexicalReader::TryMatch(fcStrW Str, bool bIgnoreSpace, bool bMatch)
{
	uint32_t tPos = m_Pos;

	uint32_t tLen = wcslen(Str);

	if(bIgnoreSpace)
		IgnoreSpace();

	if(m_Str.size() - m_Pos < tLen)
	{
		// 长度不足，不能匹配
		m_Pos = tPos;
		return false;
	}

	for(uint32_t i = 0; i<tLen; ++i)
	{
		if(m_Str[m_Pos+i] != Str[i])
		{
			// 不能匹配
			m_Pos = tPos;
			return false;
		}
	}

	if(bMatch)
	{
		m_Pos += tLen;
	}
	else
	{
		m_Pos = tPos;
	}

	return true;
}

void fcyLexicalReader::IgnoreSpace()
{
	while(!IsEOF())
	{
		wchar_t tRet = PeekChar();
		if(iswspace(tRet))
			ReadChar();
		else
			break;
	}
}
