#include "fcySocket.h"

#include "../fcyException.h"

/*
	这个文件并没有什么卵用
	为了编译通过还是先改一下
*/
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

////////////////////////////////////////////////////////////////////////////////
int g_Socket_Ref = 0;

fResult fcySocket::InitSocket()
{
	if(g_Socket_Ref != 0)
	{
		g_Socket_Ref++;
		return FCYERR_OK;
	}

	WORD wVersionRequested = MAKEWORD( 1, 1 ); // 需求版本号
	WSADATA wsaData;
	
	int tErr = WSAStartup( wVersionRequested, &wsaData );
	if ( tErr != 0 ) 
	{
		return FCYERR_INTERNALERR;
	}
	else
	{
		if ( LOBYTE( wsaData.wVersion ) != 1 ||
			HIBYTE( wsaData.wVersion ) != 1 )
		{
			WSACleanup();
			return FCYERR_INVAILDVERSION;
		}
		else
		{
			g_Socket_Ref++;
			return FCYERR_OK;
		}
	}
}

void fcySocket::HaltSocket()
{
	g_Socket_Ref--;
	if(g_Socket_Ref<=0)
	{
		g_Socket_Ref = 0;
		WSACleanup();
	}
}

string fcySocket::HostToIp(fcStr Host)
{
	HOSTENT* pHostEnt = gethostbyname(Host); // gethostbyname : 该函数返回值分配在静态区域
	sockaddr_in tAddr;

	// 获得第一个IP记录
	if(pHostEnt && pHostEnt->h_addr_list[0])
	{
		// 拷贝IP地址数据
		memcpy(&tAddr.sin_addr.s_addr, pHostEnt->h_addr_list[0], pHostEnt->h_length);
		// 读出IP地址
		return inet_ntoa(tAddr.sin_addr);  // inet_ntoa : 该函数返回值分配在静态区域
	}
	else
		return "";
}

vector<string> fcySocket::HostToAllIp(fcStr Host)
{
	vector<string> tRet;

	HOSTENT* pHostEnt = gethostbyname(Host); // gethostbyname : 该函数返回值分配在静态区域
	sockaddr_in tAddr;
	int nAdapter = 0;

	while(pHostEnt && pHostEnt->h_addr_list[nAdapter])
	{
		// 拷贝IP地址数据
		memcpy(&tAddr.sin_addr.s_addr, pHostEnt->h_addr_list[0], pHostEnt->h_length);
		// 读出IP地址
		tRet.push_back(inet_ntoa(tAddr.sin_addr));  // inet_ntoa : 该函数返回值分配在静态区域

		nAdapter++;
	}

	return tRet;
}

////////////////////////////////////////////////////////////////////////////////

fcySocket::Exception::Exception(fcStr Src, fcStr Desc)
	: m_WSALastError(WSAGetLastError()), fcyException(Src, Desc)
{
	char tNumStr[16] = { 0 };
	_itoa_s((int)m_WSALastError, tNumStr, 16, 10);
	m_ExcpDesc = m_ExcpDesc + " ( WSALastErr = " + tNumStr + " )";
}

fcySocket::Exception::~Exception()
{}

fuInt fcySocket::Exception::GetWSAErrorCode()const
{
	return m_WSALastError;
}

////////////////////////////////////////////////////////////////////////////////

fcySocket::Client::Client(fcStr IP, fuShort Port)
{
	m_Socket = (void*)socket(AF_INET,SOCK_STREAM, 0);

	/*
	int nRecvBuf = 32*1024;  //设置为32K
	setsockopt((SOCKET)m_Socket, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(nRecvBuf));
	*/

	try
	{
		connectTo(IP, Port);
	}
	catch(...)
	{
		closesocket((SOCKET)m_Socket);

		throw;
	}
}

fcySocket::Client::~Client(void)
{
	closesocket((SOCKET)m_Socket);
}

void fcySocket::Client::connectTo(fcStr IP, fuShort Port)
{
	SOCKADDR_IN tAddrSrv;
	tAddrSrv.sin_addr.S_un.S_addr = inet_addr( IP );
	tAddrSrv.sin_family = AF_INET;
	tAddrSrv.sin_port = htons( Port );

	// 连接
	if(0 != connect((SOCKET)m_Socket,(SOCKADDR *)&tAddrSrv,sizeof(SOCKADDR)))
	{
		throw Exception("fcySocket::Client::Connect", "connect failed.");
	}
}

fuInt fcySocket::Client::Recv(fData pBuffer, fLen Length)
{
	int tRet = recv((SOCKET)m_Socket, (char*)pBuffer, (int)Length, 0);
	if(tRet < 0)
		throw Exception("fcySocket::Client::Recv", "recv Failed.");
	else
		return tRet;
}

void fcySocket::Client::Send(fcData pBuffer, fLen Length)
{
	int tRet = send((SOCKET)m_Socket, (const char*)pBuffer, (int)Length, 0);
	if(tRet != Length)
		throw Exception("fcySocket::Client::Send", "send Failed.");
}

fChar fcySocket::Client::RecvChar()
{
	fChar tChar = 0;
	Recv((fData)&tChar, 1);
	return tChar;
}

void fcySocket::Client::SendChar(fChar Data)
{
	Send((fcData)Data, 1);
}
