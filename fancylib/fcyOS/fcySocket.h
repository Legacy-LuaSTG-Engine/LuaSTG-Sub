////////////////////////////////////////////////////////////////////////////////
/// @file  fcySocket.h
/// @brief fancy Socket封装
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../fcyType.h"
#include "../fcyException.h"

#include <vector>
#include <string>

/// @addtogroup fancy库底层支持
/// @{

/// @brief Socket封装模块
namespace fcySocket
{
	/// @brief 初始化底层Socket库
	fResult InitSocket();

	/// @brief 关闭Socket库
	void HaltSocket();

	/// @brief 域名转IP
	std::string HostToIp(fcStr Host);

	/// @brief 域名转IP
	std::vector<std::string> HostToAllIp(fcStr Host);

	/// @brief 异常
	class Exception :
		public fcyException
	{
	private:
		fuInt m_WSALastError;          ///< @brief 最后一次错误代码
	public:
		fuInt GetWSAErrorCode()const;  ///< @brief 获得最后一次错误
	public:
		Exception(fcStr Src, fcStr Desc);
		~Exception();
	};

	/// @brief 客户端
	class Client
	{
	protected:
		void* m_Socket;
	protected:
		/// @brief     内部函数，连接到IP
		/// @exception fcySocket::Exception
		void connectTo(fcStr IP, fuShort Port);
	public:
		/// @brief     接收数据到缓冲区
		/// @note      阻塞直到填满缓冲区
		/// @exception fcySocket::Exception
		/// @param[in] pBuffer 数据缓冲区
		/// @param[in] Length  数据长度
		fuInt Recv(fData pBuffer, fLen Length);
		/// @brief     发送数据
		/// @exception fcySocket::Exception
		/// @param[in] pBuffer 数据缓冲区
		/// @param[in] Length  数据长度
		void Send(fcData pBuffer, fLen Length);
		/// @brief     接收一个字节数据
		/// @exception fcySocket::Exception
		fChar RecvChar();
		/// @brief     接收一个字节数据
		/// @exception fcySocket::Exception
		/// @param[in] Data 数据
		void SendChar(fChar Data);
	public:
		/// @brief     构造函数
		/// @exception fcySocket::Exception
		/// @param[in] IP   IP地址
		/// @param[in] Port 端口
		Client(fcStr IP, fuShort Port);
		~Client();
	};
};

/// @}
