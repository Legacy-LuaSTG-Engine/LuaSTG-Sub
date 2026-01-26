#pragma once

namespace Platform
{
	class AdapterPolicy
	{
	public:
		static void setAll(bool active);
		static void setNVIDIA(bool active);
		static void setAMD(bool active);
	};
}
