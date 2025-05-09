#pragma once
#include "core/implement/ReferenceCounted.hpp"
#include "Core/Graphics/Device.hpp"

// SamplerState
namespace core::Graphics::Direct3D11 {
	class Device;

	class SamplerState final
		: public implement::ReferenceCounted<ISamplerState>
		, public IDeviceEventListener {

	public:
		// IDeviceEventListener

		void onDeviceCreate() override;
		void onDeviceDestroy() override;

		// ISamplerState

		// SamplerState

		SamplerState();
		SamplerState(SamplerState const&) = delete;
		SamplerState(SamplerState&&) = delete;
		SamplerState& operator=(SamplerState const&) = delete;
		SamplerState& operator=(SamplerState&&) = delete;
		~SamplerState();

		[[nodiscard]] ID3D11SamplerState* GetState() const noexcept { return m_sampler.Get(); }

		bool initialize(Device* device, core::Graphics::SamplerState const& info);
		bool createResource();

	private:
		SmartReference<Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
		core::Graphics::SamplerState m_info{};
		bool m_initialized{ false };
	};
}