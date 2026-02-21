#include "d3d11/DeviceHelper.hpp"
#include "d3d11/ToStringHelper.hpp"
#include "core/Logger.hpp"

namespace d3d11 {
	using std::string_view_literals::operator ""sv;

	// NOTE: Direct3D 11 does not support D3D_FEATURE_LEVEL_12_2

	bool createDevice(
        IDXGIAdapter1* const adapter,
        ID3D11Device** const device, ID3D11DeviceContext** const device_context, D3D_FEATURE_LEVEL* const feature_level
    ) {
		const auto driver_type = (adapter != nullptr) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;
		UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
	#if (!defined(NDEBUG) && defined(LUASTG_GRAPHICS_DEBUG_LAYER_ENABLE))
		flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
		constexpr D3D_FEATURE_LEVEL feature_levels[] {
			D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1,
		};
		std::vector<HRESULT> results{};
		for (UINT offset = 0; offset <= 3; offset += 1) {
            const auto hr = D3D11CreateDevice(
				adapter, driver_type, nullptr,
				flags, feature_levels + offset, 9u - offset, D3D11_SDK_VERSION,
				device, feature_level, device_context
			);
            if (SUCCEEDED(hr)) {
                return true;
            }
			results.push_back(hr);
        }
		for (const auto hr : results) {
			win32::check_hresult(hr, "D3D11CreateDevice"sv);
		}
		return false;
	}

	bool createSoftwareDevice(
		std::vector<HRESULT>& results, const D3D_DRIVER_TYPE driver_type,
        ID3D11Device** const device, ID3D11DeviceContext** const device_context, D3D_FEATURE_LEVEL* const feature_level
    ) {
		UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
	#if (!defined(NDEBUG) && defined(LUASTG_GRAPHICS_DEBUG_LAYER_ENABLE))
		flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
		constexpr D3D_FEATURE_LEVEL feature_levels[] {
			D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1,
		};
		for (UINT offset = 0; offset <= 3; offset += 1) {
			win32::com_ptr<ID3D11Device> local_devcie;
			win32::com_ptr<ID3D11DeviceContext> local_device_context;
			D3D_FEATURE_LEVEL local_feature_level{};
            const auto hr = D3D11CreateDevice(
				nullptr, driver_type, nullptr,
				flags, feature_levels + offset, 9u - offset, D3D11_SDK_VERSION,
				local_devcie.put(), &local_feature_level, local_device_context.put()
			);
            if (SUCCEEDED(hr) && local_feature_level >= D3D_FEATURE_LEVEL_10_0) {
				if (device) *device = local_devcie.detach();
				if (device_context) *device_context = local_device_context.detach();
				if (feature_level) *feature_level = local_feature_level;
                return true;
            }
			results.push_back(hr);
        }
		return false;
	}
	bool createSoftwareDevice(
        ID3D11Device** const device, ID3D11DeviceContext** const device_context, D3D_FEATURE_LEVEL* const feature_level,
		D3D_DRIVER_TYPE* const driver_type
    ) {
		std::vector<HRESULT> results{};

		results.clear();
		if (createSoftwareDevice(results, D3D_DRIVER_TYPE_WARP, device, device_context, feature_level)) {
			if (driver_type) *driver_type = D3D_DRIVER_TYPE_WARP;
			return true;
		}
		for (const auto hr : results) {
			win32::check_hresult(hr, "D3D11CreateDevice (D3D_DRIVER_TYPE_WARP)"sv);
		}

		results.clear();
		if (createSoftwareDevice(results, D3D_DRIVER_TYPE_SOFTWARE, device, device_context, feature_level)) {
			if (driver_type) *driver_type = D3D_DRIVER_TYPE_SOFTWARE;
			return true;
		}
		for (const auto hr : results) {
			win32::check_hresult(hr, "D3D11CreateDevice (D3D_DRIVER_TYPE_SOFTWARE)"sv);
		}

		results.clear();
		if (createSoftwareDevice(results, D3D_DRIVER_TYPE_REFERENCE, device, device_context, feature_level)) {
			if (driver_type) *driver_type = D3D_DRIVER_TYPE_REFERENCE;
			return true;
		}
		for (const auto hr : results) {
			win32::check_hresult(hr, "D3D11CreateDevice (D3D_DRIVER_TYPE_REFERENCE)"sv);
		}

		return false;
	}

	bool checkFormatSupport(ID3D11Device* const device, const DXGI_FORMAT format, UINT* const support, UINT* const support2) {
		if (device == nullptr) {
			assert(false); return false;
		}

		D3D11_FEATURE_DATA_FORMAT_SUPPORT info{};
		D3D11_FEATURE_DATA_FORMAT_SUPPORT2 info2{};
		info.InFormat = info2.InFormat = format;
		device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &info, sizeof(info));
		device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT2, &info2, sizeof(info2));

		if (support) *support = info.OutFormatSupport;
		if (support2) *support2 = info2.OutFormatSupport2;
		return true;
	}

	void logDeviceFeatureSupportDetails(ID3D11Device* const device) {
		assert(device != nullptr);

		std::string s;
		s.append("[core] [GraphicsDevice] device feature support details:\n"sv);

		const auto feature_level = device->GetFeatureLevel();
		s.append(std::format("    Direct3D feature level: {}\n"sv, toStringView(feature_level)));

	#define GET_FEATURE(FEATURE) if (D3D11_FEATURE_DATA_##FEATURE info{}; SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_##FEATURE, &info, sizeof(info))))

		GET_FEATURE(THREADING) {
			s.append(std::format(
				"    Threading:\n"sv
				"        Driver concurrent creates: {}\n"sv
				"        Driver command lists: {}\n"sv
				, toStringView(info.DriverConcurrentCreates == TRUE, Semantic::support)
				, toStringView(info.DriverCommandLists == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(DOUBLES) {
			s.append(std::format(
				"    Doubles:\n"sv
				"        Double precision float shader ops: {}\n"sv
				, toStringView(info.DoublePrecisionFloatShaderOps == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(D3D10_X_HARDWARE_OPTIONS) {
			s.append(std::format(
				"    Direct3D 10.x hardware options:\n"sv
				"        ComputeShaders + Raw/StructuredBuffers via shader 4.x: {}\n"sv
				, toStringView(info.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(D3D11_OPTIONS) {
			s.append(std::format(
				"    Direct3D 11 options:\n"sv
				"        Output merger logic op: {}\n"sv
				"        UAV only rendering forced sample count: {}\n"sv
				"        Discard APIs seen by driver: {}\n"sv
				"        Flags for update and copy seen by driver: {}\n"sv
				"        Clear view: {}\n"sv
				"        Copy with overlap: {}\n"sv
				"        ConstantBuffer partial update: {}\n"sv
				"        ConstantBuffer offsetting: {}\n"sv
				"        Map no overwrite on dynamic ConstantBuffer: {}\n"sv
				"        Map no overwrite on dynamic buffer SRV: {}\n"sv
				"        Multisample RTV with forced sample count one: {}\n"sv
				"        SAD4 shader instructions: {}\n"sv
				"        Extended doubles shader instructions: {}\n"sv
				"        Extended resource sharing: {}\n"sv
				, toStringView(info.OutputMergerLogicOp == TRUE, Semantic::support)
				, toStringView(info.UAVOnlyRenderingForcedSampleCount == TRUE, Semantic::support)
				, toStringView(info.DiscardAPIsSeenByDriver == TRUE, Semantic::support)
				, toStringView(info.FlagsForUpdateAndCopySeenByDriver == TRUE, Semantic::support)
				, toStringView(info.ClearView == TRUE, Semantic::support)
				, toStringView(info.CopyWithOverlap == TRUE, Semantic::support)
				, toStringView(info.ConstantBufferPartialUpdate == TRUE, Semantic::support)
				, toStringView(info.ConstantBufferOffsetting == TRUE, Semantic::support)
				, toStringView(info.MapNoOverwriteOnDynamicConstantBuffer == TRUE, Semantic::support)
				, toStringView(info.MapNoOverwriteOnDynamicBufferSRV == TRUE, Semantic::support)
				, toStringView(info.MultisampleRTVWithForcedSampleCountOne == TRUE, Semantic::support)
				, toStringView(info.SAD4ShaderInstructions == TRUE, Semantic::support)
				, toStringView(info.ExtendedDoublesShaderInstructions == TRUE, Semantic::support)
				, toStringView(info.ExtendedResourceSharing == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(ARCHITECTURE_INFO) {
			s.append(std::format(
				"    Architecture info:\n"sv
				"        Tile based deferred renderer: {}\n"sv
				, toStringView(info.TileBasedDeferredRenderer == TRUE, Semantic::yes_no)
			));
		}

		GET_FEATURE(D3D9_OPTIONS) {
			s.append(std::format(
				"    Direct3D 9 options:\n"sv
				"        Full non-pow2 texture support: {}\n"sv
				, toStringView(info.FullNonPow2TextureSupport == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(SHADER_MIN_PRECISION_SUPPORT) {
			s.append(std::format(
				"    Shader min precision support:\n"sv
				"        Pixel shader min precision: {}\n"sv
				"        All other shader stages min precision: {}\n"sv
				, toStringView(static_cast<D3D11_SHADER_MIN_PRECISION_SUPPORT>(info.PixelShaderMinPrecision))
				, toStringView(static_cast<D3D11_SHADER_MIN_PRECISION_SUPPORT>(info.AllOtherShaderStagesMinPrecision))
			));
		}

		GET_FEATURE(D3D9_SHADOW_SUPPORT) {
			s.append(std::format(
				"    Direct3D 9 shadow support:\n"sv
				"        Supports depth as texture with less-equal comparison filter: {}\n"sv
				, toStringView(info.SupportsDepthAsTextureWithLessEqualComparisonFilter == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(D3D11_OPTIONS1) {
			s.append(std::format(
				"    Direct3D 11 options 1:\n"sv
				"        Tiled resources tier: {}\n"sv
				"        Min max filtering: {}\n"sv
				"        Clear view also supports depth only formats: {}\n"sv
				"        Map on default buffers: {}\n"sv
				, toStringView(info.TiledResourcesTier)
				, toStringView(info.MinMaxFiltering == TRUE, Semantic::support)
				, toStringView(info.ClearViewAlsoSupportsDepthOnlyFormats == TRUE, Semantic::support)
				, toStringView(info.MapOnDefaultBuffers == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(D3D9_SIMPLE_INSTANCING_SUPPORT) {
			s.append(std::format(
				"    Direct3D 9 simple instancing support:\n"sv
				"        Simple instancing supported: {}\n"sv
				, toStringView(info.SimpleInstancingSupported == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(MARKER_SUPPORT) {
			s.append(std::format(
				"    Marker support:\n"sv
				"        Profile: {}\n"sv
				, toStringView(info.Profile == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(D3D9_OPTIONS1) {
			s.append(std::format(
				"    Direct3D 9 options 1:\n"sv
				"        Full non-pow2 texture supported: {}\n"sv
				"        Depth as texture with less-equal comparison filter supported: {}\n"sv
				"        Simple instancing supported: {}\n"sv
				"        TextureCube face RenderTarget with non cube DepthStencil supported: {}\n"sv
				, toStringView(info.FullNonPow2TextureSupported == TRUE, Semantic::support)
				, toStringView(info.DepthAsTextureWithLessEqualComparisonFilterSupported == TRUE, Semantic::support)
				, toStringView(info.SimpleInstancingSupported == TRUE, Semantic::support)
				, toStringView(info.TextureCubeFaceRenderTargetWithNonCubeDepthStencilSupported == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(D3D11_OPTIONS2) {
			s.append(std::format(
				"    Direct3D 11 options 1:\n"sv
				"        PS specified stencil ref supported: {}\n"sv
				"        Typed UAV load additional formats: {}\n"sv
				"        ROVs supported: {}\n"sv
				"        Conservative rasterization tier: {}\n"sv
				"        Tiled resources tier: {}\n"sv
				"        Map on default textures: {}\n"sv
				"        Standard swizzle: {}\n"sv
				"        Unified memory architecture: {}\n"sv
				, toStringView(info.PSSpecifiedStencilRefSupported == TRUE, Semantic::support)
				, toStringView(info.TypedUAVLoadAdditionalFormats == TRUE, Semantic::support)
				, toStringView(info.ROVsSupported == TRUE, Semantic::support)
				, toStringView(info.ConservativeRasterizationTier)
				, toStringView(info.TiledResourcesTier)
				, toStringView(info.MapOnDefaultTextures == TRUE, Semantic::support)
				, toStringView(info.StandardSwizzle == TRUE, Semantic::support)
				, toStringView(info.UnifiedMemoryArchitecture == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(D3D11_OPTIONS3) {
			s.append(std::format(
				"    Direct3D 11 options 3:\n"sv
				"        VP and RT array index from any shader feeding rasterizer: {}\n"sv
				, toStringView(info.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(GPU_VIRTUAL_ADDRESS_SUPPORT) {
			s.append(std::format(
				"    GPU virtual address support:\n"sv
				"        Max GPU virtual address bits per resource: {}\n"sv
				"        Max GPU virtual address bits per process: {}\n"sv
				, info.MaxGPUVirtualAddressBitsPerResource
				, info.MaxGPUVirtualAddressBitsPerProcess
			));
		}

		GET_FEATURE(D3D11_OPTIONS4) {
			s.append(std::format(
				"    Direct3D 11 options 4:\n"sv
				"        Extended NV12 shared texture supported: {}\n"sv
				, toStringView(info.ExtendedNV12SharedTextureSupported == TRUE, Semantic::support)
			));
		}

		GET_FEATURE(SHADER_CACHE) {
			s.append(std::format(
				"    Shader cache:\n"sv
				"        Support flags: {}\n"sv
				, toStringView(static_cast<D3D11_SHADER_CACHE_SUPPORT_FLAGS>(info.SupportFlags))
			));
		}

		GET_FEATURE(D3D11_OPTIONS5) {
			s.append(std::format(
				"    Direct3D 11 options 5:\n"sv
				"        Shared resource tier: {}\n"sv
				, toStringView(info.SharedResourceTier)
			));
		}

		GET_FEATURE(DISPLAYABLE) {
			s.append(std::format(
				"    Displayable:\n"sv
				"        Displayable texture: {}\n"sv
				"        Shared resource tier: {}\n"sv
				, toStringView(info.DisplayableTexture == TRUE, Semantic::support)
				, toStringView(info.SharedResourceTier)
			));
		}

		GET_FEATURE(D3D11_OPTIONS6) {
			s.append(std::format(
				"    Direct3D 11 options 6:\n"sv
				"        Shader access restricted resource tier: {}\n"sv
				, toStringView(info.ShaderAccessRestrictedResourceTier)
			));
		}

	#undef GET_FEATURE

		if (s.back() == '\n') {
			s.pop_back();
		}
		core::Logger::info(s);
	}

	void logDeviceFormatSupportDetail(const D3D11_FEATURE_DATA_FORMAT_SUPPORT& info, const D3D11_FEATURE_DATA_FORMAT_SUPPORT2& info2, const std::string_view name, std::string& s) {
		s.append(std::format("    {}:"sv, name));
		if (info.OutFormatSupport != 0) {
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_BUFFER) s.append(" BUFFER"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER) s.append(" IA_VERTEX_BUFFER"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_IA_INDEX_BUFFER) s.append(" IA_INDEX_BUFFER"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_SO_BUFFER) s.append(" SO_BUFFER"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE1D) s.append(" TEXTURE1D"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) s.append(" TEXTURE2D"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE3D) s.append(" TEXTURE3D"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURECUBE) s.append(" TEXTURECUBE"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_LOAD) s.append(" SHADER_LOAD"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) s.append(" SHADER_SAMPLE"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE_COMPARISON) s.append(" SHADER_SAMPLE_COMPARISON"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE_MONO_TEXT) s.append(" SHADER_SAMPLE_MONO_TEXT"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_MIP) s.append(" MIP"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) s.append(" MIP_AUTOGEN"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET) s.append(" RENDER_TARGET"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_BLENDABLE) s.append(" BLENDABLE"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL) s.append(" DEPTH_STENCIL"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_CPU_LOCKABLE) s.append(" CPU_LOCKABLE"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE) s.append(" MULTISAMPLE_RESOLVE"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_DISPLAY) s.append(" DISPLAY"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_CAST_WITHIN_BIT_LAYOUT) s.append(" CAST_WITHIN_BIT_LAYOUT"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET) s.append(" MULTISAMPLE_RENDERTARGET"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_MULTISAMPLE_LOAD) s.append(" MULTISAMPLE_LOAD"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_GATHER) s.append(" SHADER_GATHER"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_BACK_BUFFER_CAST) s.append(" BACK_BUFFER_CAST"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_TYPED_UNORDERED_ACCESS_VIEW) s.append(" TYPED_UNORDERED_ACCESS_VIEW"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_SHADER_GATHER_COMPARISON) s.append(" SHADER_GATHER_COMPARISON"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_DECODER_OUTPUT) s.append(" DECODER_OUTPUT"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_OUTPUT) s.append(" VIDEO_PROCESSOR_OUTPUT"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT) s.append(" VIDEO_PROCESSOR_INPUT"sv);
			if (info.OutFormatSupport & D3D11_FORMAT_SUPPORT_VIDEO_ENCODER) s.append(" VIDEO_ENCODER"sv);
		}
		if (info2.OutFormatSupport2 != 0) {
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_ADD) s.append(" UAV_ATOMIC_ADD"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_BITWISE_OPS) s.append(" UAV_ATOMIC_BITWISE_OPS"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_COMPARE_STORE_OR_COMPARE_EXCHANGE) s.append(" UAV_ATOMIC_COMPARE_STORE_OR_COMPARE_EXCHANGE"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_EXCHANGE) s.append(" UAV_ATOMIC_EXCHANGE"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_SIGNED_MIN_OR_MAX) s.append(" UAV_ATOMIC_SIGNED_MIN_OR_MAX"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_UNSIGNED_MIN_OR_MAX) s.append(" UAV_ATOMIC_UNSIGNED_MIN_OR_MAX"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD) s.append(" UAV_TYPED_LOAD"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_UAV_TYPED_STORE) s.append(" UAV_TYPED_STORE"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_OUTPUT_MERGER_LOGIC_OP) s.append(" OUTPUT_MERGER_LOGIC_OP"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_TILED) s.append(" TILED"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_SHAREABLE) s.append(" SHAREABLE"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_MULTIPLANE_OVERLAY) s.append(" MULTIPLANE_OVERLAY"sv);
			if (info2.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_DISPLAYABLE) s.append(" DISPLAYABLE"sv);
		}
		s.push_back('\n');
	}
	void logDeviceFormatSupportDetails(ID3D11Device* const device) {
		assert(device != nullptr);

		std::string s;
		s.append("[core] [GraphicsDevice] device format support details:\n"sv);

	#define GET_SUPPORT(IN_FORMAT) \
		{\
			D3D11_FEATURE_DATA_FORMAT_SUPPORT info{}; D3D11_FEATURE_DATA_FORMAT_SUPPORT2 info2{}; \
			info.InFormat = info2.InFormat = DXGI_FORMAT_##IN_FORMAT; \
			device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &info, sizeof(info)); \
			device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT2, &info2, sizeof(info2)); \
			logDeviceFormatSupportDetail(info, info2, #IN_FORMAT ""sv, s); \
		}

	GET_SUPPORT(R32G32B32A32_TYPELESS);
	GET_SUPPORT(R32G32B32A32_FLOAT);
	GET_SUPPORT(R32G32B32A32_UINT);
	GET_SUPPORT(R32G32B32A32_SINT);
	GET_SUPPORT(R32G32B32_TYPELESS);
	GET_SUPPORT(R32G32B32_FLOAT);
	GET_SUPPORT(R32G32B32_UINT);
	GET_SUPPORT(R32G32B32_SINT);
	GET_SUPPORT(R16G16B16A16_TYPELESS);
	GET_SUPPORT(R16G16B16A16_FLOAT);
	GET_SUPPORT(R16G16B16A16_UNORM);
	GET_SUPPORT(R16G16B16A16_UINT);
	GET_SUPPORT(R16G16B16A16_SNORM);
	GET_SUPPORT(R16G16B16A16_SINT);
	GET_SUPPORT(R32G32_TYPELESS);
	GET_SUPPORT(R32G32_FLOAT);
	GET_SUPPORT(R32G32_UINT);
	GET_SUPPORT(R32G32_SINT);
	GET_SUPPORT(R32G8X24_TYPELESS);
	GET_SUPPORT(D32_FLOAT_S8X24_UINT);
	GET_SUPPORT(R32_FLOAT_X8X24_TYPELESS);
	GET_SUPPORT(X32_TYPELESS_G8X24_UINT);
	GET_SUPPORT(R10G10B10A2_TYPELESS);
	GET_SUPPORT(R10G10B10A2_UNORM);
	GET_SUPPORT(R10G10B10A2_UINT);
	GET_SUPPORT(R11G11B10_FLOAT);
	GET_SUPPORT(R8G8B8A8_TYPELESS);
	GET_SUPPORT(R8G8B8A8_UNORM);
	GET_SUPPORT(R8G8B8A8_UNORM_SRGB);
	GET_SUPPORT(R8G8B8A8_UINT);
	GET_SUPPORT(R8G8B8A8_SNORM);
	GET_SUPPORT(R8G8B8A8_SINT);
	GET_SUPPORT(R16G16_TYPELESS);
	GET_SUPPORT(R16G16_FLOAT);
	GET_SUPPORT(R16G16_UNORM);
	GET_SUPPORT(R16G16_UINT);
	GET_SUPPORT(R16G16_SNORM);
	GET_SUPPORT(R16G16_SINT);
	GET_SUPPORT(R32_TYPELESS);
	GET_SUPPORT(D32_FLOAT);
	GET_SUPPORT(R32_FLOAT);
	GET_SUPPORT(R32_UINT);
	GET_SUPPORT(R32_SINT);
	GET_SUPPORT(R24G8_TYPELESS);
	GET_SUPPORT(D24_UNORM_S8_UINT);
	GET_SUPPORT(R24_UNORM_X8_TYPELESS);
	GET_SUPPORT(X24_TYPELESS_G8_UINT);
	GET_SUPPORT(R8G8_TYPELESS);
	GET_SUPPORT(R8G8_UNORM);
	GET_SUPPORT(R8G8_UINT);
	GET_SUPPORT(R8G8_SNORM);
	GET_SUPPORT(R8G8_SINT);
	GET_SUPPORT(R16_TYPELESS);
	GET_SUPPORT(R16_FLOAT);
	GET_SUPPORT(D16_UNORM);
	GET_SUPPORT(R16_UNORM);
	GET_SUPPORT(R16_UINT);
	GET_SUPPORT(R16_SNORM);
	GET_SUPPORT(R16_SINT);
	GET_SUPPORT(R8_TYPELESS);
	GET_SUPPORT(R8_UNORM);
	GET_SUPPORT(R8_UINT);
	GET_SUPPORT(R8_SNORM);
	GET_SUPPORT(R8_SINT);
	GET_SUPPORT(A8_UNORM);
	GET_SUPPORT(R1_UNORM);
	GET_SUPPORT(R9G9B9E5_SHAREDEXP);
	GET_SUPPORT(R8G8_B8G8_UNORM);
	GET_SUPPORT(G8R8_G8B8_UNORM);
	GET_SUPPORT(BC1_TYPELESS);
	GET_SUPPORT(BC1_UNORM);
	GET_SUPPORT(BC1_UNORM_SRGB);
	GET_SUPPORT(BC2_TYPELESS);
	GET_SUPPORT(BC2_UNORM);
	GET_SUPPORT(BC2_UNORM_SRGB);
	GET_SUPPORT(BC3_TYPELESS);
	GET_SUPPORT(BC3_UNORM);
	GET_SUPPORT(BC3_UNORM_SRGB);
	GET_SUPPORT(BC4_TYPELESS);
	GET_SUPPORT(BC4_UNORM);
	GET_SUPPORT(BC4_SNORM);
	GET_SUPPORT(BC5_TYPELESS);
	GET_SUPPORT(BC5_UNORM);
	GET_SUPPORT(BC5_SNORM);
	GET_SUPPORT(B5G6R5_UNORM);
	GET_SUPPORT(B5G5R5A1_UNORM);
	GET_SUPPORT(B8G8R8A8_UNORM);
	GET_SUPPORT(B8G8R8X8_UNORM);
	GET_SUPPORT(R10G10B10_XR_BIAS_A2_UNORM);
	GET_SUPPORT(B8G8R8A8_TYPELESS);
	GET_SUPPORT(B8G8R8A8_UNORM_SRGB);
	GET_SUPPORT(B8G8R8X8_TYPELESS);
	GET_SUPPORT(B8G8R8X8_UNORM_SRGB);
	GET_SUPPORT(BC6H_TYPELESS);
	GET_SUPPORT(BC6H_UF16);
	GET_SUPPORT(BC6H_SF16);
	GET_SUPPORT(BC7_TYPELESS);
	GET_SUPPORT(BC7_UNORM);
	GET_SUPPORT(BC7_UNORM_SRGB);
	GET_SUPPORT(AYUV);
	GET_SUPPORT(Y410);
	GET_SUPPORT(Y416);
	GET_SUPPORT(NV12);
	GET_SUPPORT(P010);
	GET_SUPPORT(P016);
	GET_SUPPORT(420_OPAQUE);
	GET_SUPPORT(YUY2);
	GET_SUPPORT(Y210);
	GET_SUPPORT(Y216);
	GET_SUPPORT(NV11);
	GET_SUPPORT(AI44);
	GET_SUPPORT(IA44);
	GET_SUPPORT(P8);
	GET_SUPPORT(A8P8);
	GET_SUPPORT(B4G4R4A4_UNORM);

	GET_SUPPORT(P208);
	GET_SUPPORT(V208);
	GET_SUPPORT(V408);

	GET_SUPPORT(SAMPLER_FEEDBACK_MIN_MIP_OPAQUE);
	GET_SUPPORT(SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE);

	GET_SUPPORT(A4B4G4R4_UNORM);

	#undef GET_SUPPORT

		if (s.back() == '\n') {
			s.pop_back();
		}
		core::Logger::info(s);
	}
}
