////////////////////////////////////////////////////////////////////////////////
/// @file  fcyHTTP.h
/// @brief fancy HTTP支持
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../fcyType.h"

#include "../fcyIO/fcyStream.h"
#include "../fcyOS/fcySocket.h"

#include <unordered_map>
#include <string>

/// @addtogroup fancy库网络支持
/// @{

/// @brief 网络库
namespace fcyNet
{
	/// @brief HTTP响应类型
	enum HTTPRESPONSE
	{
		HTTPRESPONSE_UNKNWON, // 未知
		HTTPRESPONSE_GET,     // 请求获取Request-URI所标识的资源
		HTTPRESPONSE_POST,    // 在Request-URI所标识的资源后附加新的数据
		HTTPRESPONSE_HEAD,    // 请求获取由Request-URI所标识的资源的响应消息报头
		HTTPRESPONSE_PUT,     // 请求服务器存储一个资源，并用Request-URI作为其标识
		HTTPRESPONSE_DELETE,  // 请求服务器删除Request-URI所标识的资源
		HTTPRESPONSE_TRACE,   // 请求服务器回送收到的请求信息，主要用于测试或诊断
		HTTPRESPONSE_CONNECT, // 保留将来使用
		HTTPRESPONSE_OPTIONS  // 请求查询服务器的性能，或者查询与资源相关的选项和需求
	};

	/// @brief HTTP消息头
	class HttpHeader
	{
	protected:
		std::unordered_map<std::string, std::string> m_Header;
	public:
		/// @brief 复制
		HttpHeader& operator=(const HttpHeader& right);
		/// @brief     获得值
		/// @param[in] Key 键
		std::string& operator[](const std::string& Key);
		/// @brief     获得值
		/// @param[in] Key 键
		const std::string& GetValue(const std::string& Key);
		/// @brief     设置值
		/// @param[in] Key   键
		/// @param[in] Value 值
		void SetValue(const std::string& Key, const std::string& Value);
		/// @brief     移除键
		/// @param[in] Key 键
		fBool RemoveKey(const std::string& Key);
		/// @brief 导出Text
		void ToString(std::string& Data);
		/// @brief 发送数据
		void Send(fcySocket::Client* pClient);
	public:
		/// @brief 建立一个空的Header
		HttpHeader();
		/// @brief 从Client读取Header
		HttpHeader(fcySocket::Client* pClient);
		~HttpHeader();
	};

	/// @brief      分割URL
	/// @param[in]  URL  地址
	/// @param[out] Host 域名
	/// @param[out] Port 端口，默认返回80
	/// @param[out] Path 相对路径
	void SplitURL(fcStr URL, std::string& Host, fuShort& Port, std::string& Path);

	/// @brief      解析回应头
	/// @param[in]  Data        原始数据
	/// @param[out] HTTPVersion HTTP版本号
	/// @param[out] Code        代码
	/// @param[out] Desc        描述
	void ParseRespondHeader(fcStr Data, std::string& HTTPVersion, fuInt& Code, std::string& Desc);

	/// @brief      一直读取直到/r/n
	/// @param[in]  pClient Socket对象
	/// @param[out] Out     输出字符串
	void RecvUntilCRLF(fcySocket::Client* pClient, std::string& Out);

	/// @brief      读取Length长度数据
	/// @param[in]  pClient Socket对象
	/// @param[in]  Length  长度
	/// @param[out] pOut 输出内存的对象
	void RecvData(fcySocket::Client* pClient, fuInt Length, fcyStream** pOut);

	/// @brief      发送一个Get请求
	/// @param[in]  pClient Socket对象
	/// @param[in]  Host    网站
	/// @param[in]  Path    资源路径
	void SendGetRequest(fcySocket::Client* pClient, const std::string& Host, const std::string& Path);

	/// @brief      HTTP读文件
	/// @note       使用Chrome的UA来访问HTTP文件
	/// @param[in]  URL      连接
	/// @param[out] pDataOut 返回数据
	void HTTPReadFile(fcStr URL, fcyStream** pDataOut);
};

/// @}
