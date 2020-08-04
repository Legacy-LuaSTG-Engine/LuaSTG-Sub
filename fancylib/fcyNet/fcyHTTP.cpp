#include "fcyHTTP.h"

#include "../fcyIO/fcyDeflate.h"

using namespace fcyNet;
using namespace fcySocket;
using namespace std;

////////////////////////////////////////////////////////////////////////////////

HttpHeader::HttpHeader()
{}

HttpHeader::HttpHeader(Client* pClient)
{
	fChar tChar = 0;
	while(1)
	{
		fBool tEnd = false;
		string tKey;
		string tValue;

		// 读取Key
		while(1)
		{
			// 读取一个字符直到 : 
			tChar = pClient->RecvChar();
			if(tChar == ':')
				break;
			else if(tChar == '\r')
			{
				tChar = pClient->RecvChar();
				if(tChar != '\n')
				{
					tKey.push_back('\r');
					tKey.push_back(tChar);
				}
				else
				{
					tEnd = true;
					break;
				}
			}
			else if(tChar == '\n')
			{
				tEnd = true;
				break;
			}
			else
				tKey.push_back(tChar);
		}

		if(tEnd)
		{
			if(!tKey.empty())
				throw fcyException("HttpHeader::HttpHeader", "illegal http header.");
			break;
		}

		// 读取Value
		do
		{
			// 读取一个字符直到 \r\n
			tChar = pClient->RecvChar();

			if(tValue.empty() && tChar == ' ')
				continue;

			if(tChar == '\r')
			{
				tChar = pClient->RecvChar();
				if(tChar != '\n')
				{
					tValue.push_back('\r');
					tValue.push_back(tChar);
				}
			}
			else if(tChar != '\n')
			{
				tValue.push_back(tChar);
			}
		}
		while(tChar != '\n');
		
		// 加入键值
		m_Header.insert(make_pair(tKey, tValue));
	}
}

HttpHeader::~HttpHeader()
{}

HttpHeader& HttpHeader::operator=(const HttpHeader& right)
{
	m_Header.clear();

	unordered_map<string, string>::const_iterator i = right.m_Header.begin();

	return *this;
}

string& HttpHeader::operator[](const string& Key)
{
	return m_Header[Key];
}

const string& HttpHeader::GetValue(const string& Key)
{
	return m_Header[Key];
}

void HttpHeader::SetValue(const string& Key, const string& Value)
{
	m_Header[Key] = Value;
}

fBool HttpHeader::RemoveKey(const string& Key)
{
	unordered_map<string, string>::iterator i = m_Header.find(Key);

	if(i == m_Header.end())
		return false;
	else
	{
		m_Header.erase(i);
		return true;
	}
}

void HttpHeader::ToString(string& Data)
{
	Data.clear();

	unordered_map<string, string>::iterator i = m_Header.begin();
	for(; i!=m_Header.end(); ++i)
	{
		Data += i->first;
		Data += ": ";
		Data += i->second;
		Data += "\r\n";
	}

	Data += "\r\n";
}

void HttpHeader::Send(Client* pClient)
{
	string tStr;

	ToString(tStr);

	pClient->Send((fcData)&tStr[0], tStr.length());
}

////////////////////////////////////////////////////////////////////////////////

void fcyNet::SplitURL(fcStr URL, string& Host, fuShort& Port, string& Path)
{
	Host.clear();
	Port = 80;
	Path = "/";

	fuInt tPos = 0;
	fChar tChar = 0;
	fuInt tState = 0;

	while(tChar = URL[tPos])
	{
		switch(tState)
		{
		case 0:
			if(tChar == ':')
				tState = 1;
			else if(tChar == '/')
				tState = 2;
			else
				Host += tChar;
			break;
		case 1:
			if(tChar == '/')
				tState = 2;
			else
			{
				Port *= 10;
				Port += tChar - '0';
			}
			break;
		case 2:
			Path += tChar;
			break;
		}

		tPos++;
	}
}

void fcyNet::ParseRespondHeader(fcStr Data, string& HTTPVersion, fuInt& Code, string& Desc)
{
	fInt tPos = 0;
	fChar tChar = 0;
	fInt tState = 0;

	HTTPVersion.clear();
	Code = 0;
	Desc.clear();

	while((tChar = Data[tPos]))
	{
		switch(tState)
		{
		case 0:
			if(tChar == ' ')
				tState++;
			else
				HTTPVersion += tChar;
			break;
		case 1:
			if(tChar == ' ')
				tState++;
			else
			{
				Code *= 10;
				Code += tChar - '0';
			}
			break;
		case 2:
			Desc += tChar;
			break;
		}

		tPos++;
	}
}

void fcyNet::RecvUntilCRLF(Client* pClient, string& Out)
{
	fChar tChar = 0;
	Out.clear();

	while(tChar = pClient->RecvChar())
	{
		if(tChar == '\r')
		{
			if((tChar = pClient->RecvChar()) == '\n')
				break;
			else
				Out += '\r';
		}

		Out += tChar;
	}
}

void fcyNet::RecvData(Client* pClient, fuInt Length, fcyStream** pOut)
{
	if(!pOut)
		throw fcyException("fcyNet::RecvData", "Invalid Pointer.");
	*pOut = NULL;

	fData tData = new fByte[Length];
	fuInt tByteReaded = 0;

	try
	{
		while(tByteReaded != Length)
		{
			tByteReaded += pClient->Recv(tData + tByteReaded, Length - tByteReaded);
		}

		*pOut = new fcyMemStream(NULL, 0, true, true);
	}
	catch(...)
	{
		FCYSAFEDELARR(tData);
		throw;
	}

	(*pOut)->WriteBytes(tData, Length, NULL);
	(*pOut)->SetPosition(FCYSEEKORIGIN_BEG, 0);
	FCYSAFEDELARR(tData);
}

void fcyNet::SendGetRequest(Client* pClient, const string& Host, const string& Path)
{
	// 创建GET请求
	string tRequestGet = "GET ";
	tRequestGet += Path;
	tRequestGet += " HTTP/1.1\r\n";
	
	// 发送GET请求
	pClient->Send((fcData)&tRequestGet[0], tRequestGet.length());

	// 创建头
	HttpHeader tHeader;
	tHeader["Host"] = Host;
	tHeader["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
	tHeader["Accept-Language"] = "zh-cn,zh;q=0.5";
	tHeader["Accept-Encoding"] = "deflate;q=1.0,gzip;q=0.7";
	tHeader["Accept-Charset"] = "GB2312,utf-8;q=0.7,*;q=0.7";
	// tHeader["Connection"] = "keep-alive";
	// tHeader["Keep-Alive"] = "300";
	tHeader["Connection"] = "close";
	tHeader["User-Agent"] = "Mozilla/4.0 (compatible; MSIE 5.5; Windows NT 5.0)"; 
	tHeader["Cache-Control"] = "no-cache";
	tHeader["Pragma"] = "no-cache";

	// 发送头
	tHeader.Send(pClient);
}

void fcyNet::HTTPReadFile(fcStr URL, fcyStream** pDataOut)
{
	if(pDataOut == NULL)
		throw fcyException("fcyNet::HTTPReadFile", "Invalid pointer.");

	*pDataOut = NULL;

	string tHost;
	fuShort tPort;
	string tPath;

	// 分解 URL
	SplitURL(URL, tHost, tPort, tPath);

	// 创建连接
	Client tClient(HostToIp(tHost.c_str()).c_str(), tPort);

	// 发送Get请求
	SendGetRequest(&tClient, tHost, tPath);

	// === 读取数据 ===
	string tRespondHeader;
	string tHTTPVersion;
	fuInt tCode = 0;
	string tDesc;
	
	// 读取响应头
	RecvUntilCRLF(&tClient, tRespondHeader);

	// 解析响应头
	ParseRespondHeader(tRespondHeader.c_str(), tHTTPVersion, tCode, tDesc);

	// 检查版本
	if(!(tHTTPVersion == "HTTP/1.1" || tHTTPVersion == "HTTP/1.0"))
		throw fcyException("fcyNet::HTTPReadFile", "Unknown http protocol version.");
	
	// 解析头部
	HttpHeader tHTTPHeader(&tClient);

	// 获得解压方式
	fBool tDeflate = false;
	fBool tGzip = false;
	if(tHTTPHeader["Content-Encoding"] != "")
	{
		if(tHTTPHeader["Content-Encoding"] == "gzip")
			tGzip = true;
		else if(tHTTPHeader["Content-Encoding"] == "deflate")
			tDeflate = true;
		else
			throw fcyException("fcyNet::HTTPReadFile", "Unknown compress method.");
	}

	// 获得数据长度
	fuInt tDataLen = 0;
	tDataLen = atoi(tHTTPHeader["Content-Length"].c_str());

	// 获得传输编码方式
	fBool tChunked = false;
	if(tHTTPHeader["Transfer-Encoding"] == "chunked")
		tChunked = true;

	if(tChunked)
	{
		fcyStream* tStream = NULL;
		fcyStream *pOut = new fcyMemStream(NULL, 0, true, true);

		try
		{
			fuInt tChunkSize = 0;
			string tChunkData;

			do
			{
				// 读取块大小
				RecvUntilCRLF(&tClient, tChunkData);

				// 转换到10进制
				sscanf(&tChunkData[0], "%x", &tChunkSize);

				// 读取数据
				if(tChunkSize)
				{
					RecvData(&tClient, tChunkSize, &tStream);

					// 拷贝
					fcyStreamHelper::FillStream(tStream, pOut, tStream->GetLength());

					FCYSAFEKILL(tStream);

					// 读取结尾
					RecvUntilCRLF(&tClient, tChunkData);
				}
				else
					// 读取结尾
					RecvUntilCRLF(&tClient, tChunkData);
			}
			while(tChunkSize);
		}
		catch(...)
		{
			FCYSAFEKILL(tStream);
			FCYSAFEKILL(pOut);
			throw;
		}

		// 解压
		if(tDeflate || tGzip)
		{
			fResult tVR;
			if(tGzip)
				tVR = fcyDeflate::DecompressStreamToMemEx(pOut, 47, pDataOut);
			else
				tVR = fcyDeflate::DecompressStreamToMem(pOut, pDataOut);

			FCYSAFEKILL(pOut);
			if(FAILED(tVR))
				throw fcyException("fcyNet::HTTPReadFile", "Uncompress failed.");
		}
		else
		{
			*pDataOut = pOut;
		}
	}
	else
	{
		fcyStream* pStream = NULL;

		if(!tDataLen)
		{
			pStream = new fcyMemStream(NULL, 0, true, true);

			// 只读取第一个包
			try
			{
				fChar tBuffer[128];
				fuInt tReaded;
				while((tReaded = tClient.Recv((fData)tBuffer, 128)))
				{
					pStream->WriteBytes((fData)tBuffer, tReaded, NULL);
				}
			}
			catch(...)
			{
				FCYSAFEKILL(pStream);
				throw;
			}
		}
		else
			RecvData(&tClient, tDataLen, &pStream);

		// 解压
		if(tDeflate || tGzip)
		{
			fResult tVR;
			if(tGzip)
				tVR = fcyDeflate::DecompressStreamToMemEx(pStream, 47, pDataOut);
			else
				tVR = fcyDeflate::DecompressStreamToMem(pStream, pDataOut);

			FCYSAFEKILL(pStream);
			if(FAILED(tVR))
				throw fcyException("fcyNet::HTTPReadFile", "Uncompress failed.");
		}
		else
		{
			*pDataOut = pStream;
		}
	}

	(*pDataOut)->SetPosition(FCYSEEKORIGIN_BEG, 0);
}
