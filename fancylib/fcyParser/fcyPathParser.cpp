#include "fcyParser/fcyPathParser.h"

#include "fcyMisc/fcyStringHelper.h"

#include <algorithm>

using namespace std;

////////////////////////////////////////////////////////////////////////////////

static char TransCharA(char x)
{
	if(x == '/' || x == '|') 
		return L'\\';
	else 
		return x;
}

static wchar_t TransCharW(wchar_t x)
{
	if(x == L'/' || x == L'|')
		return L'\\';
	else
		return x;
}

string fcyPathParser::GetExtension(const string& Path)
{
	string tRet = GetFileName(Path);

	string::size_type tPos = tRet.find_last_of(L'.');
	if(tPos != string::npos)
		tRet.erase(0, tPos + 1);

	return tRet;
}

string fcyPathParser::GetExtensionLower(const string& Path)
{
	string tRet = GetExtension(Path);
	return fcyStringHelper::ToLower(tRet);
}

string fcyPathParser::GetFileName(const string& Path)
{
	string tRet = Path;
	transform(tRet.begin(), tRet.end(), tRet.begin(), TransCharA);

	string::size_type tPos = Path.find_last_of(L'\\');
	if(tPos != string::npos)
		tRet.erase(0, tPos + 1);

	return tRet;
}

string fcyPathParser::GetFileNameWithoutExt(const string& Path)
{
	string tRet = GetFileName(Path);

	string::size_type tPos = tRet.find_last_of(L'.');
	if(tPos != string::npos)
		tRet.erase(tPos, tRet.length() - tPos);

	return tRet;
}

string fcyPathParser::GetPath(const string& Path)
{
	string tRet = Path;
	transform(tRet.begin(), tRet.end(), tRet.begin(), TransCharA);

	if(tRet.length()>0 && tRet[tRet.length()-1]!='\\')
	{
		string::size_type tPos = tRet.find_last_of(L'\\');
		if(tPos != string::npos)
			tRet.erase(tPos, tRet.length() - tPos);
	}

	if(tRet.length()>0 && tRet[tRet.length()-1]!='\\')
		tRet += '\\';

	return tRet;
}

wstring fcyPathParser::GetExtension(const wstring& Path)
{
	wstring tRet = GetFileName(Path);

	wstring::size_type tPos = tRet.find_last_of(L'.');
	if(tPos != wstring::npos)
		tRet.erase(0, tPos + 1);

	return tRet;
}

wstring fcyPathParser::GetExtensionLower(const wstring& Path)
{
	wstring tRet = GetExtension(Path);
	return fcyStringHelper::ToLower(tRet);
}

wstring fcyPathParser::GetFileName(const wstring& Path)
{
	wstring tRet = Path;
	transform(tRet.begin(), tRet.end(), tRet.begin(), TransCharW);

	wstring::size_type tPos = Path.find_last_of(L'\\');
	if(tPos != wstring::npos)
		tRet.erase(0, tPos + 1);

	return tRet;
}

wstring fcyPathParser::GetFileNameWithoutExt(const wstring& Path)
{
	wstring tRet = GetFileName(Path);

	wstring::size_type tPos = tRet.find_last_of(L'.');
	if(tPos != wstring::npos)
		tRet.erase(tPos, tRet.length() - tPos);

	return tRet;
}

wstring fcyPathParser::GetPath(const wstring& Path)
{
	wstring tRet = Path;
	transform(tRet.begin(), tRet.end(), tRet.begin(), TransCharW);

	if(tRet.length()>0 && tRet[tRet.length()-1]!='\\')
	{
		wstring::size_type tPos = tRet.find_last_of(L'\\');
		if(tPos != wstring::npos)
			tRet.erase(tPos, tRet.length() - tPos);
	}

	if(tRet.length()>0 && tRet[tRet.length()-1]!='\\')
		tRet += '\\';

	return tRet;
}
