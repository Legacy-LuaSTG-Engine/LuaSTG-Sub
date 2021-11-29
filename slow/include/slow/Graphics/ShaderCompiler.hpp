#pragma once
#include "slow/Type.hpp"
#include "slow/Object.hpp"
#include "slow/ByteArray.hpp"

namespace slow::Graphics
{
	struct IShaderCompiler : public IObject
	{
		virtual void setDebug(b8 enable) = 0;
		virtual void setOptimization(b8 enable) = 0;
		virtual b8 compileVertexShader(u8view data, IByteArray** outObject) = 0;
		virtual b8 compilePixelShader(u8view data, IByteArray** outObject) = 0;
		virtual b8 compileVertexShaderFromFile(c8view path, IByteArray** outObject) = 0;
		virtual b8 compilePixelShaderFromFile(c8view path, IByteArray** outObject) = 0;
	};
}
