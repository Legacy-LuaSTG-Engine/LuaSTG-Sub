#pragma once
#include "slow/Type.hpp"
#include "slow/Object.hpp"
#include "slow/ByteArray.hpp"

namespace slow::Graphics
{
	struct IShaderCompiler : public IObject
	{
		virtual b8 compileVertexShader(IByteArray** outObject, u8view data) = 0;
		virtual b8 compilePixelShader(IByteArray** outObject, u8view data) = 0;
		virtual b8 compileVertexShaderFromFile(IByteArray** outObject, c8view path) = 0;
		virtual b8 compilePixelShaderFromFile(IByteArray** outObject, c8view path) = 0;
	};
	
	b8 createShaderCompiler(IShaderCompiler** outObject);
}
