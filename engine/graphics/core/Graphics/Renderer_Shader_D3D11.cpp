#include "core/Graphics/Renderer_D3D11.hpp"
#include "core/Logger.hpp"
#include "core/FileSystem.hpp"
#include "windows/RuntimeLoader/Direct3DCompiler.hpp"

class D3DIncludeImpl : public ID3DInclude
{
public:
	COM_DECLSPEC_NOTHROW HRESULT WINAPI Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		UNREFERENCED_PARAMETER(IncludeType); // 我们不关心它是从哪里包含的
		UNREFERENCED_PARAMETER(pParentData);
		core::SmartReference<core::IData> data;
		if (!core::FileSystemManager::readFile(pFileName, data.put()))
		{
			return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		}
		void* buffer = malloc(data->size());
		if (!buffer) return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
		// TODO: no copy x2
		memcpy(buffer, data->data(), data->size());
		*ppData = buffer;
		*pBytes = static_cast<UINT>(data->size());
		return S_OK;
	}
	COM_DECLSPEC_NOTHROW HRESULT WINAPI Close(LPCVOID pData)
	{
		assert(pData);
		free(const_cast<void*>(pData));
		return S_OK;
	}
};

static D3DIncludeImpl g_include_loader;
static Platform::RuntimeLoader::Direct3DCompiler g_d3dcompiler_loader;

namespace core::Graphics
{
	static bool compileShaderMacro(char const* name, void const* data, size_t size, char const* target, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		UINT flag_ = D3DCOMPILE_ENABLE_STRICTNESS;
	#ifdef _DEBUG
		flag_ |= D3DCOMPILE_DEBUG;
		flag_ |= D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif
		win32::com_ptr<ID3DBlob> errmsg_;
		HRESULT hr = gHR = g_d3dcompiler_loader.Compile(data, size, name, defs, &g_include_loader, "main", target, flag_, 0, ppBlob, errmsg_.put());
		if (FAILED(hr))
		{
			Logger::error("[core] [Renderer] compile shader '{}' failed: {}", name, (char*)errmsg_->GetBufferPointer());
			return false;
		}
		return true;
	}
	[[maybe_unused]] static bool compileVertexShaderMacro11(char const* name, void const* data, size_t size, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		return compileShaderMacro(name, data, size, "vs_4_0", defs, ppBlob);
	}
	static bool compilePixelShaderMacro11(char const* name, void const* data, size_t size, const D3D_SHADER_MACRO* defs, ID3DBlob** ppBlob)
	{
		return compileShaderMacro(name, data, size, "ps_4_0", defs, ppBlob);
	}

	bool PostEffectShader_D3D11::createResources(const bool is_recreating)
	{
		if (!d3d_ps_blob)
		{
			if (is_path)
			{
				SmartReference<IData> src;
				if (!FileSystemManager::readFile(source, src.put()))
					return false;
				if (!compilePixelShaderMacro11(source.c_str(), src->data(), src->size(), nullptr, d3d_ps_blob.put()))
					return false;
			}
			else
			{
				if (!compilePixelShaderMacro11(source.c_str(), source.data(), source.size(), nullptr, d3d_ps_blob.put()))
					return false;
			}
		}
		assert(m_device->GetD3D11Device());
		HRESULT hr = gHR = m_device->GetD3D11Device()->CreatePixelShader(d3d_ps_blob->GetBufferPointer(), d3d_ps_blob->GetBufferSize(), nullptr, d3d11_ps.put());
		if (FAILED(hr))
			return false;
		M_D3D_SET_DEBUG_NAME_SIMPLE(d3d11_ps.get());

		// 着色器反射

		if (!d3d11_ps_reflect)
		{
			hr = gHR = g_d3dcompiler_loader.Reflect(d3d_ps_blob->GetBufferPointer(), d3d_ps_blob->GetBufferSize(), IID_PPV_ARGS(d3d11_ps_reflect.put()));
			if (FAILED(hr)) return false;

			// 获得着色器信息

			D3D11_SHADER_DESC shader_info{};
			hr = gHR = d3d11_ps_reflect->GetDesc(&shader_info);
			if (FAILED(hr)) return false;

			// 常量缓冲区

			m_buffer_map.reserve(shader_info.ConstantBuffers);
			for (UINT i = 0; i < shader_info.ConstantBuffers; i += 1)
			{
				auto* constant_buffer = d3d11_ps_reflect->GetConstantBufferByIndex(i);

				D3D11_SHADER_BUFFER_DESC constant_buffer_info{};
				hr = gHR = constant_buffer->GetDesc(&constant_buffer_info);
				if (FAILED(hr)) return false;

				D3D11_SHADER_INPUT_BIND_DESC bind_info{};
				hr = gHR = d3d11_ps_reflect->GetResourceBindingDescByName(constant_buffer_info.Name, &bind_info);
				if (FAILED(hr)) return false;

				LocalConstantBuffer local_buffer;
				local_buffer.index = bind_info.BindPoint;
				local_buffer.buffer.resize(constant_buffer_info.Size);
				local_buffer.variable.reserve(constant_buffer_info.Variables);

				for (UINT j = 0; j < constant_buffer_info.Variables; j += 1)
				{
					auto* variable = constant_buffer->GetVariableByIndex(j);

					D3D11_SHADER_VARIABLE_DESC variable_info{};
					hr = gHR = variable->GetDesc(&variable_info);
					if (FAILED(hr)) return false;

					auto* variable_type = variable->GetType();

					D3D11_SHADER_TYPE_DESC variable_type_info{};
					hr = gHR = variable_type->GetDesc(&variable_type_info);
					if (FAILED(hr)) return false;

					LocalVariable local_variable;
					local_variable.offset = variable_info.StartOffset;
					local_variable.size = variable_info.Size;

					local_buffer.variable.emplace(variable_info.Name, std::move(local_variable));
				}

				m_buffer_map.emplace(constant_buffer_info.Name, std::move(local_buffer));
			}

			// 纹理

			m_texture2d_map.reserve(shader_info.BoundResources - shader_info.ConstantBuffers);
			for (UINT i = 0; i < shader_info.BoundResources; i += 1)
			{
				D3D11_SHADER_INPUT_BIND_DESC bind_info{};
				hr = gHR = d3d11_ps_reflect->GetResourceBindingDesc(i, &bind_info);
				if (FAILED(hr)) return false;

				if (!(bind_info.Type == D3D_SIT_TEXTURE && bind_info.Dimension == D3D_SRV_DIMENSION_TEXTURE2D))
				{
					continue; // 并非所需
				}

				LocalTexture2D local_texture2d;
				local_texture2d.index = bind_info.BindPoint;

				m_texture2d_map.emplace(bind_info.Name, std::move(local_texture2d));
			}
		}

		// 创建缓冲区

		if (!is_recreating) {
			for (auto& v : m_buffer_map) {
				if (!m_device->createConstantBuffer(
					static_cast<uint32_t>(v.second.buffer.size()),
					v.second.constant_buffer.put()
				)) {
					return false;
				}
			}
		}

		return true;
	}
}
