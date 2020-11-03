#include "fcyMisc/fcyHash.h"

////////////////////////////////////////////////////////////////////////////////

#define Get16Bits(d) (*((const fuShort *) (d)))

fuInt fcyHash::SuperFastHash(fcData pData, fuInt DataLen)
{
	fuInt tRet = DataLen, t;
	fInt tRem;

	if (DataLen <= 0 || pData == NULL) return 0;

	tRem = DataLen & 3;
	DataLen >>= 2;

	// Hash算法主循环
	for ( ;DataLen > 0; DataLen--) 
	{
		tRet  += Get16Bits(pData);
		t      = ( Get16Bits(pData+2) << 11 ) ^ tRet;
		tRet   = (tRet << 16) ^ t;
		pData += 2 * sizeof(fuShort);
		tRet  += tRet >> 11;
	}

	// 处理结尾
	switch (tRem)
	{
	case 3:
		tRet += Get16Bits(pData);
		tRet ^= tRet << 16;
		tRet ^= pData[ sizeof(fuShort) ] << 18;
		tRet += tRet >> 11;
		break;
	case 2: 
		tRet += Get16Bits(pData);
		tRet ^= tRet << 11;
		tRet += tRet >> 17;
		break;
	case 1: 
		tRet += *pData;
		tRet ^= tRet << 10;
		tRet += tRet >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	tRet ^= tRet << 3;
	tRet += tRet >> 5;
	tRet ^= tRet << 4;
	tRet += tRet >> 17;
	tRet ^= tRet << 25;
	tRet += tRet >> 6;

	return tRet;
}