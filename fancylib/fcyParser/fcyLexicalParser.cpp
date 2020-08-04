#include "fcyParser/fcyLexicalParser.h"

#include "fcyMisc/fcyStringHelper.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

fcyLexicalException::fcyLexicalException(fcStr Src, fcStr Desc, fuInt Line, fuInt Row)
	: fcyException(Src, Desc), m_Line(Line), m_Row(Row)
{
	m_ExcpDesc += (string("(at Line:") + 
		fcyStringHelper::ToStr((fInt)Line) + 
		" Row:" +
		fcyStringHelper::ToStr((fInt)Row) +
		")");
}

fcyLexicalException::~fcyLexicalException(void)
{}

fuInt fcyLexicalException::GetLine()
{
	return m_Line;
}

fuInt fcyLexicalException::GetRow()
{
	return m_Row;
}

////////////////////////////////////////////////////////////////////////////////

fcyLexicalReader::fcyLexicalReader(const wstring& SrcText)
	: m_Str(SrcText), m_Pos(0)
{
	if(m_Str.size() && m_Str[0] == (fCharW)0xFEFF)
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

fBool fcyLexicalReader::checkUTF8(fcyStream* pStream)
{
	fLen tPos = pStream->GetPosition();

	// 尝试读取BOM头
	fByte tBomCache[3];
	if(FCYOK(pStream->ReadBytes(tBomCache, 3, NULL)))
	{
		fByte tUTF8[3] = { 0xEF, 0xBB, 0xBF };
		if(memcmp(tBomCache, tUTF8, 3)==0)
			return true;
		else
			pStream->SetPosition(FCYSEEKORIGIN_BEG, tPos);
	}
	else
			pStream->SetPosition(FCYSEEKORIGIN_BEG, tPos);

	return false;
}

fBool fcyLexicalReader::checkUTF16LE(fcyStream* pStream)
{
	fLen tPos = pStream->GetPosition();

	// 尝试读取BOM头
	fByte tBomCache[2];
	if(FCYOK(pStream->ReadBytes(tBomCache, 2, NULL)))
	{
		fByte tUTF16LE[2] = { 0xFF, 0xFE };
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
		fLen tSize = pStream->GetLength() - pStream->GetPosition();
		tRet.resize((size_t)tSize / sizeof(wchar_t));
		if(tRet.length())
			pStream->ReadBytes((fData)tRet.data(), tSize, NULL);
		return tRet;
	}
	else if(checkUTF8(pStream))
	{
		string tTemp;
		fLen tSize = pStream->GetLength() - pStream->GetPosition();
		tTemp.resize((size_t)tSize);
		if(tTemp.length())
			pStream->ReadBytes((fData)tTemp.data(), tSize, NULL);
		return fcyStringHelper::MultiByteToWideChar(tTemp, CP_UTF8);
	}
	else
	{
		string tTemp;
		fLen tSize = pStream->GetLength() - pStream->GetPosition();
		tTemp.resize((size_t)tSize);
		if(tTemp.length())
			pStream->ReadBytes((fData)tTemp.data(), tSize, NULL);
		return fcyStringHelper::MultiByteToWideChar(tTemp, CP_OEMCP);
	}
}

fuInt fcyLexicalReader::GetLine()
{
	fuInt tLine = 0;

	fCharW tLast = L'\0';
	for(fuInt i = 0; i < m_Pos; ++i)
	{
		if(tLast == L'\n')
			tLine++;

		tLast = m_Str[i];
	}

	return tLine;
}

fuInt fcyLexicalReader::GetRow()
{
	fuInt tRow = 0;

	fCharW tLast = L'\0';
	for(fuInt i = 0; i < m_Pos; ++i)
	{
		if(tLast == L'\n')
			tRow = 0;
		
		tRow++;

		tLast = m_Str[i];
	}

	return tRow;
}

fBool fcyLexicalReader::IsEOF()
{
	return (m_Pos >= m_Len);
}

fCharW fcyLexicalReader::ReadChar()
{
	if(IsEOF())
		throw fcyLexicalException("fcyLexicalReader::ReadChar", "EOF.", GetLine(), GetRow());

	fCharW tRet = m_Str[m_Pos];
	m_Pos++;

	return tRet;
}

fCharW fcyLexicalReader::PeekChar()
{
	if(IsEOF())
		throw fcyLexicalException("fcyLexicalReader::PeekChar", "EOF.", GetLine(), GetRow());

	fCharW tRet = m_Str[m_Pos];

	return tRet;
}

void fcyLexicalReader::Match(fCharW Char, fBool bIgnoreSpace)
{
	if(bIgnoreSpace)
		IgnoreSpace();

	fCharW tChar = ReadChar();

	if(tChar != Char)
	{
		fCharW tTextBuffer[1024];
		swprintf_s(tTextBuffer, L"'%c' expected, but found '%c'.", Char, tChar);
		
		throw fcyLexicalException("fcyLexicalReader::Match", 
			fcyStringHelper::WideCharToMultiByte(tTextBuffer).c_str(), 
			GetLine(), GetRow()); 
	}
}

void fcyLexicalReader::Match(fcStrW Str, fBool bIgnoreSpace)
{
	if(bIgnoreSpace)
		IgnoreSpace();

	fuInt tLen = wcslen(Str);

	for(fuInt i = 0; i<tLen; ++i)
	{
		fCharW tChar = ReadChar();
		if(tChar != Str[i])
		{
			fCharW tTextBuffer[1024];
			swprintf_s(tTextBuffer, L"\"%s\" expected, but found '%c'.", Str, tChar);

			throw fcyLexicalException("fcyLexicalReader::Match", 
				fcyStringHelper::WideCharToMultiByte(tTextBuffer).c_str(), 
				GetLine(), GetRow()); 
		}
	}
}

fBool fcyLexicalReader::TryMatch(fCharW Char, fBool bIgnoreSpace, fBool bMatch)
{
	fuInt tPos = m_Pos;

	if(bIgnoreSpace)
		IgnoreSpace();

	if(IsEOF())
	{
		m_Pos = tPos;
		return false;
	}

	fCharW tChar = ReadChar();
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

fBool fcyLexicalReader::TryMatch(fcStrW Str, fBool bIgnoreSpace, fBool bMatch)
{
	fuInt tPos = m_Pos;

	fuInt tLen = wcslen(Str);

	if(bIgnoreSpace)
		IgnoreSpace();

	if(m_Str.size() - m_Pos < tLen)
	{
		// 长度不足，不能匹配
		m_Pos = tPos;
		return false;
	}

	for(fuInt i = 0; i<tLen; ++i)
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
		fCharW tRet = PeekChar();
		if(iswspace(tRet))
			ReadChar();
		else
			break;
	}
}
