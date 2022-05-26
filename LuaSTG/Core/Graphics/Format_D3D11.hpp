#pragma once
#include "Core/Graphics/Format.hpp"

namespace LuaSTG::Core::Graphics
{
	DXGI_FORMAT convert_Format_to_DXGI_FORMAT(Format format)
	{
		switch (format)
		{
		default:
		case Format::Unknown: return DXGI_FORMAT_UNKNOWN;
		case Format::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case Format::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
		}
	}
	Format convert_DXGI_FORMAT_to_Format(DXGI_FORMAT format)
	{
		switch (format)
		{
		default:
		case DXGI_FORMAT_UNKNOWN: return Format::Unknown;
		case DXGI_FORMAT_R8G8B8A8_UNORM: return Format::R8G8B8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM: return Format::B8G8R8A8_UNORM;
		}
	}
}
