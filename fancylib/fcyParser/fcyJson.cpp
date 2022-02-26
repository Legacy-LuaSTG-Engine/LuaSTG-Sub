#include "fcyParser/fcyJson.h"

#include "fcyMisc/fcyStringHelper.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

fcyJsonValue::fcyJsonValue()
	: m_Type(FCYJSONVALUETYPE_NULL), m_ValueNumber(0.)
{
}

fcyJsonValue::fcyJsonValue(fBool Value)
	: m_Type(FCYJSONVALUETYPE_BOOL), m_ValueBool(Value)
{
}

fcyJsonValue::fcyJsonValue(fDouble Value)
	: m_Type(FCYJSONVALUETYPE_NUMBER), m_ValueNumber(Value)
{
}

fcyJsonValue::fcyJsonValue(FCYJSONVALUETYPE Type)
	: m_Type(Type)
{
}

fcyJsonValue::~fcyJsonValue()
{
}

void fcyJsonValue::writeToStr(std::wstring& OutStr)
{
	switch(m_Type)
	{
	case FCYJSONVALUETYPE_NULL:
		OutStr += L"null";
		break;
	case FCYJSONVALUETYPE_BOOL:
		if(m_ValueBool)
			OutStr += L"true";
		else
			OutStr += L"false";
		break;
	case FCYJSONVALUETYPE_NUMBER:
		OutStr += fcyStringHelper::ToWideStr(m_ValueNumber);
		break;
	default:
		throw fcyException("fcyJsonValue::writeToStr", "Internal ERR.");
	}
}

FCYJSONVALUETYPE fcyJsonValue::GetType()
{
	return m_Type;
}

fBool fcyJsonValue::ToNull()
{
	return (m_Type == FCYJSONVALUETYPE_NULL);
}

fBool fcyJsonValue::ToBool()
{
	if(m_Type != FCYJSONVALUETYPE_BOOL)
		return false;
	return m_ValueBool;
}

fDouble fcyJsonValue::ToNumber()
{
	if(m_Type != FCYJSONVALUETYPE_NUMBER)
		return false;
	return m_ValueNumber;
}

fcyJsonString* fcyJsonValue::ToString()
{
	return NULL;
}

fcyJsonList* fcyJsonValue::ToList()
{
	return NULL;
}

fcyJsonDict* fcyJsonValue::ToDict()
{
	return NULL;
}

fBool fcyJsonValue::SetValue()
{
	if(m_Type != FCYJSONVALUETYPE_NULL && 
		m_Type != FCYJSONVALUETYPE_BOOL && 
		m_Type != FCYJSONVALUETYPE_NUMBER)
	{
		return false;
	}

	m_Type = FCYJSONVALUETYPE_NULL;
	return true;
}

fBool fcyJsonValue::SetValue(fBool Value)
{
	if(m_Type != FCYJSONVALUETYPE_NULL && 
		m_Type != FCYJSONVALUETYPE_BOOL && 
		m_Type != FCYJSONVALUETYPE_NUMBER)
	{
		return false;
	}

	m_Type = FCYJSONVALUETYPE_BOOL;
	m_ValueBool = Value;
	return true;
}

fBool fcyJsonValue::SetValue(fDouble Value)
{
	if(m_Type != FCYJSONVALUETYPE_NULL && 
		m_Type != FCYJSONVALUETYPE_BOOL && 
		m_Type != FCYJSONVALUETYPE_NUMBER)
	{
		return false;
	}

	m_Type = FCYJSONVALUETYPE_NUMBER;
	m_ValueNumber = Value;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

fcyJsonString::fcyJsonString(fcStrW Value)
	: fcyJsonValue(FCYJSONVALUETYPE_STRING), m_Str(Value)
{}

fcyJsonString::~fcyJsonString()
{}

void fcyJsonString::writeToStr(std::wstring& OutStr)
{
	OutStr += L"\"";

	for(fuInt i = 0; i<m_Str.size(); ++i)
	{
		switch(m_Str[i])
		{
		case L'"':
			OutStr += L"\"";
			break;
		case L'\\':
			OutStr += L"\\";
			break;
		case L'/':
			OutStr += L"/";
			break;
		case L'\b':
			OutStr += L"\b";
			break;
		case L'\f':
			OutStr += L"\f";
			break;
		case L'\n':
			OutStr += L"\n";
			break;
		case L'\r':
			OutStr += L"\r";
			break;
		case L'\t':
			OutStr += L"\t";
			break;
		default:
			OutStr += m_Str[i];
			break;
		}
	}

	OutStr += L"\"";
}

fcyJsonString* fcyJsonString::ToString()
{
	return this;
}

fcStrW fcyJsonString::GetStr()
{
	return m_Str.c_str();
}

void fcyJsonString::SetStr(fcStrW Value)
{
	if(Value == NULL)
		m_Str = L"";
	else
		m_Str = Value;
}

////////////////////////////////////////////////////////////////////////////////

fcyJsonList::fcyJsonList()
	: fcyJsonValue(FCYJSONVALUETYPE_LIST)
{}

fcyJsonList::~fcyJsonList()
{
	Clear();
}

void fcyJsonList::writeToStr(std::wstring& OutStr)
{
	OutStr += L"[ ";

	for(fuInt i = 0; i<m_ObjList.size(); ++i)
	{
		m_ObjList[i]->writeToStr(OutStr);

		if(i != m_ObjList.size()-1)
			OutStr += L", ";
	}

	OutStr += L" ]";
}

fcyJsonList* fcyJsonList::ToList()
{
	return this;
}

fcyJsonValue* fcyJsonList::GetValue(fuInt Index)
{
	if(Index >= GetCount())
		return NULL;

	return m_ObjList[Index];
}

fResult fcyJsonList::SetValue(fuInt Index, fcyJsonValue* pNew)
{
	if(Index >= GetCount())
		return FCYERR_ILLEGAL;
	
	FCYSAFEDEL(m_ObjList[Index]);
	m_ObjList[Index] = pNew;
	
	return FCYERR_OK;
}

void fcyJsonList::Append(fcyJsonValue* pNew)
{
	if(pNew)
		m_ObjList.push_back(pNew);
}

fResult fcyJsonList::Remove(fuInt Index)
{
	if(Index >= GetCount())
		return FCYERR_ILLEGAL;

	vector<fcyJsonValue*>::iterator i = m_ObjList.begin() + Index;
	FCYSAFEDEL(*i);
	m_ObjList.erase(i);

	return FCYERR_OK;
}

void fcyJsonList::Clear()
{
	vector<fcyJsonValue*>::iterator i = m_ObjList.begin();

	while(i!=m_ObjList.end())
	{
		FCYSAFEDEL(*i);
		++i;
	}

	m_ObjList.clear();
}

fuInt fcyJsonList::GetCount()
{
	return m_ObjList.size();
}

////////////////////////////////////////////////////////////////////////////////
	
fcyJsonDict::fcyJsonDict()
: fcyJsonValue(FCYJSONVALUETYPE_DICT)
{}

fcyJsonDict::~fcyJsonDict()
{
	Clear();
}

void fcyJsonDict::writeToStr(std::wstring& OutStr)
{
	OutStr += L"{ ";

	for(fuInt i = 0; i<m_ObjList.size(); ++i)
	{
		fcyJsonValue* pObj = m_Cache[m_ObjList[i]];

		OutStr += L"\"";
		OutStr += m_ObjList[i];
		OutStr += L"\" : ";
		pObj->writeToStr(OutStr);

		if(i != m_ObjList.size()-1)
			OutStr += L", ";
	}

	OutStr += L" }";
}
	
fcyJsonDict* fcyJsonDict::ToDict()
{
	return this;
}

fcyJsonValue* fcyJsonDict::GetValue(fuInt Index, fcStrW* pKeyOut)
{
	if(Index > GetCount())
		return NULL;
	else
	{
		if (pKeyOut)
			*pKeyOut = m_ObjList[Index].c_str();
		return m_Cache[m_ObjList[Index]];
	}
}

fcyJsonValue* fcyJsonDict::GetValue(fcStrW Name)
{
	if(!Contain(Name))
		return NULL;
	else
		return m_Cache[Name];
}

void fcyJsonDict::SetValue(fcStrW Name, fcyJsonValue* pNew)
{
	if(Contain(Name))
	{
		Remove(Name);
	}

	m_ObjList.push_back(Name);

	m_Cache[Name] = pNew;
}

fBool fcyJsonDict::Contain(fcStrW Name)
{
	return m_Cache.find(Name) != m_Cache.end();
}

fBool fcyJsonDict::Remove(fcStrW Name)
{
	if(!Contain(Name))
		return false;

	vector<wstring>::iterator i = m_ObjList.begin();
	while(i!=m_ObjList.end())
	{
		if((*i) == Name)
		{
			m_ObjList.erase(i);
			break;
		}
		++i;
	}

	unordered_map<wstring, fcyJsonValue*>::iterator j = m_Cache.find(Name);
	FCYSAFEDEL(j->second);
	m_Cache.erase(j);

	return true;
}

void fcyJsonDict::Clear()
{
	unordered_map<wstring, fcyJsonValue*>::iterator j = m_Cache.begin();
	while(j != m_Cache.end())
	{
		FCYSAFEDEL(j->second);
		j++;
	}

	m_Cache.clear();
	m_ObjList.clear();
}

fuInt fcyJsonDict::GetCount()
{
	return m_ObjList.size();
}

//////////////////////////////////////////////////////////////////////////

fcyJson::fcyJson()
	: m_Root(NULL)
{
}

fcyJson::fcyJson(const std::wstring& Str)
	: m_Root(NULL)
{
	fcyLexicalReader tReader(Str);

	m_Root = parseValue(tReader);
}

fcyJson::fcyJson(fcyStream* pStream)
	: m_Root(NULL)
{
	fcyLexicalReader tReader(pStream);

	m_Root = parseValue(tReader);
}

fcyJson::~fcyJson()
{
	FCYSAFEDEL(m_Root);
}

fcyJsonValue* fcyJson::parseValue(fcyLexicalReader& Context)
{
	fcyJsonValue* tRet = NULL;

	Context.IgnoreSpace();
	wchar_t tChar = Context.PeekChar();
	switch(tChar)
	{
	case L'{':
		tRet = parseDict(Context);
		break;
	case L'[':
		tRet = parseList(Context);
		break;
	case L'"':
		tRet = parseString(Context);
		break;
	case L't':
		{
			Context.Match(L"true", false);

			tRet = new fcyJsonValue(true);
		}
		break;
	case L'f':
		{
			Context.Match(L"false", false);

			tRet = new fcyJsonValue(false);
		}
		break;
	case L'n':
		{
			Context.Match(L"null", false);

			tRet = new fcyJsonValue();
		}
		break;
	default:
		if(tChar==L'-' || iswdigit(tChar))
		{
			tRet = parseNumber(Context);
		}
		else
			throw fcyLexicalException("fcyJson::parseValue", "Unexpected char.", Context.GetLine(), Context.GetRow());
		break;
	}
	return tRet;
}

fcyJsonValue* fcyJson::parseNumber(fcyLexicalReader& Context)
{
	wstring tRet;

	Context.IgnoreSpace();

	wchar_t tChar = Context.PeekChar();
	while(iswdigit(tChar) || tChar == L'-' || tChar == L'.' || tChar == L'E' || tChar == L'e') 
	{
		Context.ReadChar();

		tRet += tChar;

		if(Context.IsEOF())
			break;
		else
			tChar = Context.PeekChar();
	}

	return new fcyJsonValue(_wtof(tRet.c_str()));
}

fcyJsonString* fcyJson::parseString(fcyLexicalReader& Context)
{
	wstring tRet;

	Context.Match(L'"', true);
	wchar_t tChar;
	while((tChar = Context.ReadChar()) != L'"') 
	{
		if(iswcntrl(tChar))
			throw fcyLexicalException("fcyJson::parseString", "Unexpected ctrl char.", Context.GetLine(), Context.GetRow());
		if(tChar == L'\\')
		{
			tChar = Context.ReadChar();
			switch(tChar)
			{
			case L'"':
				tRet+=L'"';
				break;
			case L'\\':
				tRet+=L'\\';
				break;
			case L'/':
				tRet+=L'/';
				break;
			case L'b':
				tRet+=L'\b';
				break;
			case L'f':
				tRet+=L'\f';
				break;
			case L'n':
				tRet+=L'\n';
				break;
			case L'r':
				tRet+=L'\r';
				break;
			case L't':
				tRet+=L'\t';
				break;
			case L'u':
				{
					wstring tNum=L"0x";
					tNum += Context.ReadChar();
					tNum += Context.ReadChar();
					tNum += Context.ReadChar();
					tNum += Context.ReadChar();
					wchar_t* tEnd = NULL;
					long tVal = wcstol(tNum.c_str(), &tEnd, 16);
					if((size_t)(tEnd-tNum.c_str())<tNum.size())
						throw fcyLexicalException("fcyJson::parseString", "Unexpected char.", Context.GetLine(), Context.GetRow());
					tNum += (wchar_t)tVal;
					break;
				}
			default:
				{
					throw fcyLexicalException("fcyJson::parseString", "Unexpected escape char.", Context.GetLine(), Context.GetRow());
					break;
				}
			}
		}
		else
			tRet += tChar;
	}
	return new fcyJsonString(tRet.c_str());
}

fcyJsonList* fcyJson::parseList(fcyLexicalReader& Context)
{
	fcyJsonList* tRet = new fcyJsonList();
	
	Context.Match(L'[', true);

	while(1)
	{
		if(Context.TryMatch(L']', true, true))
		{
			break;
		}

		fcyJsonValue* tValue = NULL;
		
		try
		{
			tValue = parseValue(Context);
		}
		catch (fcyLexicalException&)
		{
			delete(tRet);
			throw;
		}

		tRet->Append(tValue);

		if(Context.TryMatch(L',', true, true))
		{
			continue;
		}
	}

	return tRet;
}

fcyJsonDict* fcyJson::parseDict(fcyLexicalReader& Context)
{
	fcyJsonDict* tRet = new fcyJsonDict();

	Context.Match(L'{', true);

	while(1)
	{
		if(Context.TryMatch(L'}', true, true))
		{
			break;
		}

		// 解析键名
		fcyJsonString* tKey = NULL;
		
		try
		{
			tKey = parseString(Context);
		}
		catch (fcyLexicalException&)
		{
			FCYSAFEDEL(tRet);
			throw;
		}

		// match ":"
		Context.Match(L':', true);

		// 解析键值
		fcyJsonValue* tValue = NULL;
		
		try
		{
			tValue = parseValue(Context);
		}
		catch (fcyLexicalException&)
		{
			FCYSAFEDEL(tKey);
			FCYSAFEDEL(tRet);
			throw;
		}

		// 追加
		tRet->SetValue(tKey->GetStr(), tValue);
		FCYSAFEDEL(tKey);

		if(Context.TryMatch(L',', true, true))
		{
			continue;
		}
	}
	return tRet;
}

fcyJsonValue* fcyJson::GetRoot()
{
	return m_Root;
}

void fcyJson::SetRoot(fcyJsonValue* pValue)
{
	FCYSAFEDEL(m_Root);
	m_Root = pValue;
}

void fcyJson::WriteToStr(wstring& pOut)
{
	if(m_Root)
		m_Root->writeToStr(pOut);
}

void fcyJson::WriteToStream(fcyStream* pOut)
{
	wstring tOutStr;

	if(m_Root)
	{
		m_Root->writeToStr(tOutStr);

		// UTF 16 BOM
		fByte tUTF16LE[2] = { 0xFF, 0xFE };
		pOut->SetLength(0);
		pOut->WriteBytes(tUTF16LE, 2, NULL);
		pOut->WriteBytes((fData)&tOutStr[0], tOutStr.length() * sizeof(wchar_t), NULL);
	}
}
