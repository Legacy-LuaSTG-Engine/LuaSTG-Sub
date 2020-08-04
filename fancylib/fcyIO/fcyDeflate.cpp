#include "fcyIO/fcyDeflate.h"

extern "C"
{
	#include <zlib.h>
}

#define DEFLATE_BUFFERSIZE 512

using namespace std;
using namespace fcyDeflate;

////////////////////////////////////////////////////////////////////////////////

fResult fcyDeflate::CompressStreamToMem(fcyStream* pIn, fcyStream** pOut, fInt Level)
{
	z_stream tStream;
	int tErr;
	fByte tOutBuffer[DEFLATE_BUFFERSIZE], tInBuffer[DEFLATE_BUFFERSIZE];

	if(!pOut || !pIn || Level<-1 || Level>9)
		return FCYERR_INVAILDPARAM;
	
	// 准备缓冲区
	memset(&tStream, 0, sizeof(z_stream));
	tStream.next_in = tInBuffer;   // 原始数据流
	tStream.next_out = tOutBuffer; // 输出的压缩数据流
	
	tStream.avail_out = DEFLATE_BUFFERSIZE;
	
	// 初始化解压流
    tErr = deflateInit(&tStream, Level);
	if (tErr != Z_OK) 
		return FCYERR_INTERNALERR;

	// 创建输出流
	fcyMemStream* pOutStream = new fcyMemStream(NULL, 0, true, true);

	// 压缩循环
	tErr = Z_OK;
	while(tErr != Z_STREAM_END)
	{
		fBool tbEOF = false;
		if(tStream.avail_in == 0)
		{
			// 读取一个缓冲区数据
			fLen tInLen = 0;
			pIn->ReadBytes(tInBuffer, DEFLATE_BUFFERSIZE, &tInLen);

			if(tInLen < DEFLATE_BUFFERSIZE)
				tbEOF = true;

			// 更新next_in和avail_in
			tStream.next_in = tInBuffer;
			tStream.avail_in = (fuInt)tInLen;
		}
		
		tErr = deflate(&tStream, tbEOF ? Z_FINISH : Z_NO_FLUSH);
		if(tErr != Z_OK && tErr != Z_STREAM_END) // 发生错误
		{
			inflateEnd(&tStream);     // 关闭流
			FCYSAFEKILL(pOutStream);  // 销毁解压流
			if(tErr == Z_DATA_ERROR)
				return FCYERR_INVAILDDATA;
			else
				return FCYERR_INTERNALERR;
		}

		if(tStream.avail_out == 0 || tErr == Z_STREAM_END)
		{
			pOutStream->WriteBytes(tOutBuffer, DEFLATE_BUFFERSIZE - tStream.avail_out, NULL);

			tStream.next_out = tOutBuffer;
			tStream.avail_out = DEFLATE_BUFFERSIZE;
		}
	}

	deflateEnd(&tStream);

	*pOut = pOutStream;
	pOutStream->SetPosition(FCYSEEKORIGIN_BEG, 0);

	return FCYERR_OK;
}

fResult fcyDeflate::DecompressStreamToMem(fcyStream* pIn, fcyStream** pOut)
{
	z_stream tStream;
	int tErr;
	fByte tOutBuffer[DEFLATE_BUFFERSIZE], tInBuffer[DEFLATE_BUFFERSIZE];

	if(!pOut || !pIn)
		return FCYERR_INVAILDPARAM;

	// 准备缓冲区
	memset(&tStream, 0, sizeof(z_stream));
	tStream.next_in = tInBuffer;   // 压缩数据流
	tStream.next_out = tOutBuffer; // 输出流
	
	tStream.avail_out = DEFLATE_BUFFERSIZE;
	
	// 初始化解压流
    tErr = inflateInit(&tStream);
	if (tErr != Z_OK) 
		return FCYERR_INTERNALERR;

	// 创建输出流
	fcyMemStream* pOutStream = new fcyMemStream(NULL, 0, true, true);

	// 解压缩循环
	tErr = Z_OK;
	while(tErr != Z_STREAM_END)
	{
		if(tStream.avail_in == 0)
		{
			// 读取一个缓冲区数据
			fLen tInLen = 0;
			pIn->ReadBytes(tInBuffer, DEFLATE_BUFFERSIZE, &tInLen);

			// 读完数据了，跳出
			if(tInLen == 0)
				break;

			// 更新next_in和avail_in
			tStream.next_in = tInBuffer;
			tStream.avail_in = (fuInt)tInLen;
		}
		
		tErr = inflate(&tStream, Z_SYNC_FLUSH);
		if(tErr != Z_OK && tErr != Z_STREAM_END) // 发生错误
		{
			if(tErr == Z_DATA_ERROR)
			{
				// 尝试插入一个假头

				static char tFalseHeader[2] =
				{
					0x8 + 0x7 * 0x10,
					(((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
				};
				inflateReset(&tStream);
				tStream.next_in = (Bytef*)tFalseHeader;
				tStream.avail_in = sizeof(tFalseHeader);

				pOutStream->SetLength(0);
				pIn->SetPosition(FCYSEEKORIGIN_BEG, 0);

				tErr = inflate(&tStream, Z_NO_FLUSH);

				continue;
			}
			else
			{
				inflateEnd(&tStream);     // 关闭流
				FCYSAFEKILL(pOutStream);  // 销毁解压流
				if(tErr == Z_DATA_ERROR)
					return FCYERR_INVAILDDATA;
				else
					return FCYERR_INTERNALERR;
			}
		}

		if(tStream.avail_out != DEFLATE_BUFFERSIZE)
		{
			pOutStream->WriteBytes(tOutBuffer, DEFLATE_BUFFERSIZE - tStream.avail_out, NULL);

			tStream.next_out = tOutBuffer;
			tStream.avail_out = DEFLATE_BUFFERSIZE;
		}
	}

	inflateEnd(&tStream);

	*pOut = pOutStream;
	pOutStream->SetPosition(FCYSEEKORIGIN_BEG, 0);

	return FCYERR_OK;
}

fResult fcyDeflate::DecompressStreamToMemEx(fcyStream* pIn, fInt Flag, fcyStream** pOut)
{
	z_stream tStream;
	int tErr;
	fByte tOutBuffer[DEFLATE_BUFFERSIZE], tInBuffer[DEFLATE_BUFFERSIZE];

	if(!pOut || !pIn)
		return FCYERR_INVAILDPARAM;

	// 准备缓冲区
	memset(&tStream, 0, sizeof(z_stream));
	tStream.next_in = tInBuffer;   // 压缩数据流
	tStream.next_out = tOutBuffer; // 输出流
	
	tStream.avail_out = DEFLATE_BUFFERSIZE;
	
	// 初始化解压流
    tErr = inflateInit2(&tStream, Flag);
	if (tErr != Z_OK) 
		return FCYERR_INTERNALERR;

	// 创建输出流
	fcyMemStream* pOutStream = new fcyMemStream(NULL, 0, true, true);

	// 解压缩循环
	tErr = Z_OK;
	while(tErr != Z_STREAM_END)
	{
		if(tStream.avail_in == 0)
		{
			// 读取一个缓冲区数据
			fLen tInLen = 0;
			pIn->ReadBytes(tInBuffer, DEFLATE_BUFFERSIZE, &tInLen);

			// 读完数据了，跳出
			if(tInLen == 0)
				break;

			// 更新next_in和avail_in
			tStream.next_in = tInBuffer;
			tStream.avail_in = (fuInt)tInLen;
		}
		
		tErr = inflate(&tStream, Z_SYNC_FLUSH);
		if(tErr != Z_OK && tErr != Z_STREAM_END) // 发生错误
		{
			inflateEnd(&tStream);     // 关闭流
			FCYSAFEKILL(pOutStream);  // 销毁解压流
			if(tErr == Z_DATA_ERROR)
				return FCYERR_INVAILDDATA;
			else
				return FCYERR_INTERNALERR;
		}

		if(tStream.avail_out != DEFLATE_BUFFERSIZE)
		{
			pOutStream->WriteBytes(tOutBuffer, DEFLATE_BUFFERSIZE - tStream.avail_out, NULL);

			tStream.next_out = tOutBuffer;
			tStream.avail_out = DEFLATE_BUFFERSIZE;
		}
	}

	inflateEnd(&tStream);

	*pOut = pOutStream;
	pOutStream->SetPosition(FCYSEEKORIGIN_BEG, 0);

	return FCYERR_OK;
}
