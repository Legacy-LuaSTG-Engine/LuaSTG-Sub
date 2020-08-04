////////////////////////////////////////////////////////////////////////////////
/// @file  fcyDeflate.h
/// @brief 对zlib进行封装，以实现对流的压缩和解压操作
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "fcyIO/fcyStream.h"

/// @addtogroup fancy库IO模块
/// @{

/// @brief Deflate算法支持
namespace fcyDeflate
{
	/// @brief      压缩流
	/// @param[in]  pIn   欲被压缩的流
	/// @param[out] pOut  输出的压缩流
	/// @param[in]  Level 压缩等级(-1~9，-1：默认；0：无压缩；9：最高压缩比）
	/// @note       输出的流读取指针会自行回到Begin处
	/// @return     返回操作是否成功
	fResult CompressStreamToMem(fcyStream* pIn, fcyStream** pOut, fInt Level=-1);
	
	/// @brief      解压流
	/// @param[in]  pIn  欲被解压的流
	/// @param[out] pOut 输出的原始数据流
	/// @note       输出的流读取指针会自行回到Begin处
	/// @return     FCYERR_INVAILDDATA：无效数据；FCYERR_INTERNALERR：zlib内部错误；FCYERR_INVAILDPARAM：无效参数
	fResult DecompressStreamToMem(fcyStream* pIn, fcyStream** pOut);

	/// @brief      解压流
	/// @param[in]  pIn  欲被解压的流
	/// @param[in]  Flag 参数
	/// @param[out] pOut 输出的原始数据流
	/// @note       输出的流读取指针会自行回到Begin处
	/// @return     FCYERR_INVAILDDATA：无效数据；FCYERR_INTERNALERR：zlib内部错误；FCYERR_INVAILDPARAM：无效参数
	fResult DecompressStreamToMemEx(fcyStream* pIn, fInt Flag, fcyStream** pOut);
};
/// @}