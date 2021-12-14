#include "slow/Graphics/ShaderCompiler.hpp"
#include "pch.hpp"

namespace slow::Graphics
{
	class ShaderCompiler : public Object<IShaderCompiler>
	{
	private:
		b8 compileShader(u8view data, IByteArray** outObject, c8view target)
		{
			UINT flag1_ = 0;
		#ifdef _DEBUG
			flag1_ |= D3DCOMPILE_DEBUG;
			flag1_ |= D3DCOMPILE_SKIP_OPTIMIZATION;
			flag1_ |= D3DCOMPILE_ENABLE_STRICTNESS;
		#endif
			Microsoft::WRL::ComPtr<ID3DBlob> vs_;
			Microsoft::WRL::ComPtr<ID3DBlob> err_;
			if (!_hr_succeeded(D3DCompile(data.data, data.size, NULL, NULL, NULL, "main", target.data, flag1_, 0, &vs_, &err_)))
			{
				return false;
			}
			object_ptr<IByteArray> barr_;
			if (!createByteArray(~barr_, vs_->GetBufferSize(), 0, false))
				return false;
			CopyMemory(barr_->data(), vs_->GetBufferPointer(), vs_->GetBufferSize());
			*outObject = barr_.getOwnership();
			return true;
		}
	public:
		b8 compileVertexShader(IByteArray** outObject, u8view data)
		{
			return compileShader(data, outObject, "vs_4_0");
		}
		b8 compilePixelShader(IByteArray** outObject, u8view data)
		{
			return compileShader(data, outObject, "ps_4_0");
		}
		b8 compileVertexShaderFromFile(IByteArray** outObject, c8view path)
		{
			object_ptr<IByteArray> data;
			if (!createByteArrayFromFile(~data, path, 0))
				return false;
			return compileVertexShader(outObject, u8view(data->data(), data->size()));
		}
		b8 compilePixelShaderFromFile(IByteArray** outObject, c8view path)
		{
			object_ptr<IByteArray> data;
			if (!createByteArrayFromFile(~data, path, 0))
				return false;
			return compilePixelShader(outObject, u8view(data->data(), data->size()));
		}
	};

	b8 createShaderCompiler(IShaderCompiler** outObject)
	{
		*outObject = dynamic_cast<IShaderCompiler*>(new ShaderCompiler);
		return true;
	}
}
