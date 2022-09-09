#include "AdapterPolicy.hpp"

// https://developer.download.nvidia.cn/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000000;

// https://gpuopen.com/learn/amdpowerxpressrequesthighperformance/
extern "C" __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000000;

namespace platform
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
