#include "AdapterPolicy.hpp"

using uint32 = unsigned int;

static_assert(sizeof(uint32) == 4);

// https://developer.download.nvidia.cn/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
extern "C" __declspec(dllexport) uint32 NvOptimusEnablement = 0x00000000;

// https://gpuopen.com/learn/amdpowerxpressrequesthighperformance/
extern "C" __declspec(dllexport) uint32 AmdPowerXpressRequestHighPerformance = 0x00000000;

namespace Platform
{
	void AdapterPolicy::setAll(bool active)
	{
		setNVIDIA(active);
		setAMD(active);
	}
	void AdapterPolicy::setNVIDIA(bool active)
	{
		NvOptimusEnablement = active ? 0x00000001 : 0x00000000;
	}
	void AdapterPolicy::setAMD(bool active)
	{
		AmdPowerXpressRequestHighPerformance = active ? 0x00000001 : 0x00000000;
	}
}
