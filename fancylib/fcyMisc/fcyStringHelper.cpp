#include "fcyMisc/fcyStringHelper.h"

#include <algorithm>
#include <Windows.h>

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////

fuInt fcyStringHelper::StringSplit(const string& Source, const string& SplitStr, fBool AutoTrim, vector<string>& Out)
{
	vector<string>& ret = Out;
	ret.clear();

	fInt last = 0;
	fInt pos = (fInt)Source.find(SplitStr);
	while(pos != (fInt)string::npos)
	{
		string tstr = Source.substr(last, pos-last);
		if(!(AutoTrim&&tstr.empty()))ret.push_back(tstr);

		last = pos + (fInt)SplitStr.length ();
		pos = (fInt)Source.find(SplitStr, last);
	}
	string tstr = Source.substr(last, pos-last);
	if(!(AutoTrim&&tstr.empty()))ret.push_back(tstr);

	return ret.size();
}

fuInt fcyStringHelper::StringSplit(const wstring& Source, const wstring& SplitStr, fBool AutoTrim, vector<wstring>& Out)
{
	vector<wstring>& ret = Out;
	ret.clear();

	fInt last = 0;
	fInt pos = (fInt)Source.find(SplitStr);
	while(pos != (fInt)wstring::npos)
	{
		wstring tstr = Source.substr(last, pos-last);
		if(!(AutoTrim&&tstr.empty()))ret.push_back(tstr);

		last = pos + (fInt)SplitStr.length ();
		pos = (fInt)Source.find(SplitStr, last);
	}
	wstring tstr = Source.substr(last, pos-last);
	if(!(AutoTrim&&tstr.empty()))ret.push_back(tstr);

	return ret.size();
}

string fcyStringHelper::ToLower(const string& Source)
{
	string tRet = Source;
	transform(tRet.begin(), tRet.end(), tRet.begin(), ::tolower);
	
	return tRet;
}

wstring fcyStringHelper::ToLower(const wstring& Source)
{
	wstring tRet = Source;
	transform(tRet.begin(), tRet.end(), tRet.begin(), ::towlower);
	
	return tRet;
}

string fcyStringHelper::ToStr(fInt Number, fInt Scale)
{
	char tNumStr[16];
	_itoa_s(Number, tNumStr, 16, Scale);
	return string(tNumStr);
}

string fcyStringHelper::ToStr(fDouble Number)
{
	char tNumStr[32];
	sprintf_s(tNumStr, "%lf", Number);
	return string(tNumStr);
}

wstring fcyStringHelper::ToWideStr(fInt Number, fInt Scale)
{
	wchar_t tNumStr[16];
	_itow_s(Number, tNumStr, 16, Scale);
	return wstring(tNumStr);
}

wstring fcyStringHelper::ToWideStr(fDouble Number)
{
	wchar_t tNumStr[32];
	swprintf_s(tNumStr, 32, L"%lf", Number);
	return wstring(tNumStr);
}

std::string fcyStringHelper::TimeToStr(fDouble Seconds)
{
	fuInt tIntPart = (fInt)Seconds;
	fuInt ms = (fuInt)((Seconds - tIntPart) * 1000); // 提出毫秒部分
	fuInt s = tIntPart % 60;    // 提出秒
	tIntPart /= 60;            // 去掉秒
	fuInt m = tIntPart % 60;    // 提出分
	tIntPart /= 60;            // 去掉分

	char tTime[16];
	sprintf_s(tTime, 16, "%02d:%02d:%02d.%03d", (fInt)tIntPart, m, s, ms);

	return string(tTime);
}

std::wstring fcyStringHelper::TimeToWideStr(fDouble Seconds)
{
	fuInt tIntPart = (fInt)Seconds;
	fuInt ms = (fuInt)((Seconds - tIntPart) * 1000); // 提出毫秒部分
	fuInt s = tIntPart % 60;    // 提出秒
	tIntPart /= 60;            // 去掉秒
	fuInt m = tIntPart % 60;    // 提出分
	tIntPart /= 60;            // 去掉分

	wchar_t tTime[16];
	swprintf_s(tTime, 16, L"%02d:%02d:%02d.%03d", (fInt)tIntPart, m, s, ms);

	return wstring(tTime);
}

std::wstring fcyStringHelper::TrimLeft(const std::wstring &Org)
{ 
	wstring tRet = Org;
	wstring::iterator p = find_if_not(tRet.begin(), tRet.end(), ::iswspace); 
	tRet.erase(tRet.begin(), p); 
	return tRet;
}

std::wstring fcyStringHelper::TrimRight(const std::wstring &Org)
{
	wstring tRet = Org;
	wstring::reverse_iterator p = find_if_not(tRet.rbegin(), tRet.rend(), ::iswspace);
	tRet.erase(p.base(), tRet.end()); 
	return tRet; 
}

std::wstring fcyStringHelper::Trim(const std::wstring &Org)
{
	return TrimLeft(TrimRight(Org));
}

std::string fcyStringHelper::TrimLeft(const std::string &Org)
{ 
	string tRet = Org;
	string::const_iterator p = find_if_not(tRet.begin(), tRet.end(), ::isspace); 
	tRet.erase(tRet.begin(), p); 
	return tRet;
}

std::string fcyStringHelper::TrimRight(const std::string &Org)
{
	string tRet = Org;
	string::const_reverse_iterator p = find_if_not(tRet.rbegin(), tRet.rend(), ::isspace); 
	tRet.erase(p.base(), tRet.end()); 
	return tRet; 
}

std::string fcyStringHelper::Trim(const std::string &Org)
{
	return TrimLeft(TrimRight(Org));
}

wstring fcyStringHelper::MultiByteToWideChar(const std::string& Org, fuInt CodePage)
{
	fuInt dwNum = ::MultiByteToWideChar(CodePage, 0, Org.c_str(), -1, NULL, 0); // 获得长度
	wchar_t *pwText = new wchar_t[dwNum];
	::MultiByteToWideChar (CodePage, 0, Org.c_str(), -1, pwText, dwNum);		   // 获得数据
	wstring retStr(pwText);
	delete[] pwText;
	return retStr;
}

string fcyStringHelper::WideCharToMultiByte(const std::wstring& Org, fuInt CodePage)
{
	DWORD tCount = ::WideCharToMultiByte(CodePage, NULL,Org.c_str(), -1, NULL, 0, NULL, FALSE);
	char *tText=NULL;
	tText = new char[tCount];
	::WideCharToMultiByte(CodePage,NULL,Org.c_str(),-1,tText,tCount,NULL,FALSE);
	string tRet = tText;
	delete []tText;
	return tRet;
}
