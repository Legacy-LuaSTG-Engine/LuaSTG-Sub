#pragma once
#include <cassert>
#include <algorithm>
#include <xaudio2redist.h>

namespace core {
	// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
	struct XAudio2VoiceCallbackHelper : IXAudio2VoiceCallback {
		// ReSharper disable CppEnforceOverridingFunctionStyle
		virtual void WINAPI OnVoiceProcessingPassStart(UINT32) noexcept override {}
		virtual void WINAPI OnVoiceProcessingPassEnd() noexcept override {}
		virtual void WINAPI OnStreamEnd() noexcept override {}
		virtual void WINAPI OnBufferStart(void*) noexcept override {}
		virtual void WINAPI OnBufferEnd(void*) noexcept override {}
		virtual void WINAPI OnLoopEnd(void*) noexcept override {}
		virtual void WINAPI OnVoiceError(void*, HRESULT) noexcept override {}
		// ReSharper restore CppEnforceOverridingFunctionStyle
	};

	// https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--pan-a-sound
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ksmedia/ns-ksmedia-ksaudio_channel_config
	inline HRESULT setOutputBalance(IXAudio2Voice* const upstream, IXAudio2Voice* const downstream, float const balance) {
		assert(upstream != nullptr);
		assert(downstream != nullptr);

		XAUDIO2_VOICE_DETAILS upstream_info{};
		upstream->GetVoiceDetails(&upstream_info);

		XAUDIO2_VOICE_DETAILS downstream_info{};
		downstream->GetVoiceDetails(&downstream_info);

		assert(upstream_info.InputChannels == 1 || upstream_info.InputChannels == 2);
		assert(downstream_info.InputChannels == 2);

		float const left = std::clamp(0.5f - balance * 0.5f, 0.0f, 1.0f);
		float const right = std::clamp(0.5f + balance * 0.5f, 0.0f, 1.0f);

		float const mono_to_stereo[2]{
			left, right, // <S> mono  -> <D> left, right
		};
		float const stereo_to_stereo[4]{
			left, 0.0f,  // <S> left  -> <D> left, right
			0.0f, right, // <S> right -> <D> left, right
		};

		float const* matrix{};
		switch (upstream_info.InputChannels) {
		case 1:
			matrix = mono_to_stereo;
			break;
		case 2:
			matrix = stereo_to_stereo;
			break;
		default:
			return E_NOTIMPL;
		}

		return upstream->SetOutputMatrix(downstream, upstream_info.InputChannels, downstream_info.InputChannels, matrix);
	}
}
