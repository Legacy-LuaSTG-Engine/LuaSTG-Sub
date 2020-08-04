#include "fcyCPUID.h"

#include <intrin.h>

#include "../fcyMisc/fcyStringHelper.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////

#ifdef _M_ARM
std::string fcyCPUID::GetCPUString()
{
	return "{Not supported on ARM platform}";
}

std::string fcyCPUID::GetCPUBrand()
{
	return "{Not supported on ARM platform}";
}
#else
std::string fcyCPUID::GetCPUString()
{
	int tCPUInfo[4] = {-1};
	char tCPUString[0x20];

	__cpuid(tCPUInfo, 0);

	memset(tCPUString, 0, sizeof(tCPUString));
    *((int*)tCPUString) = tCPUInfo[1];
    *((int*)(tCPUString + 4)) = tCPUInfo[3];
    *((int*)(tCPUString + 8)) = tCPUInfo[2];

	return fcyStringHelper::Trim(tCPUString);
}

std::string fcyCPUID::GetCPUBrand()
{
	int tCPUInfo[4] = {-1};
	char tCPUBrandString[0x40];

    memset(tCPUBrandString, 0, sizeof(tCPUBrandString));

	__cpuid(tCPUInfo, 0x80000002);
	memcpy(tCPUBrandString, tCPUInfo, sizeof(tCPUInfo));
	__cpuid(tCPUInfo, 0x80000003);
	memcpy(tCPUBrandString + 16, tCPUInfo, sizeof(tCPUInfo));
	__cpuid(tCPUInfo, 0x80000004);
	memcpy(tCPUBrandString + 32, tCPUInfo, sizeof(tCPUInfo));

	return fcyStringHelper::Trim(tCPUBrandString);
}
#endif
