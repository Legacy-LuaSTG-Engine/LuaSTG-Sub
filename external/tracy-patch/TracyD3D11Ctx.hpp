#pragma once
#ifndef D3D11_SDK_VERSION 
#define NOMINMAX
#include <d3d11.h>
#undef NOMINMAX
#endif // !D3D11_SDK_VERSION
#include "TracyD3D11.hpp"

namespace tracy
{
	TracyD3D11Ctx xTracyD3D11Ctx();
	void xTracyD3D11Context(ID3D11Device* device, ID3D11DeviceContext* devicectx);
	void xTracyD3D11Destroy();
}
