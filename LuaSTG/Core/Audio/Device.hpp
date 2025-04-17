#pragma once
#include "Core/Type.hpp"
#include "core/ReferenceCounted.hpp"
#include "Core/Audio/Decoder.hpp"

namespace core::Audio
{
	enum class MixChannel
	{
		Direct,      // 直接输出到设备，不能用于 IDevice::setMixChannelVolume
		SoundEffect, // 属于音效通道
		Music,       // 属于背景音乐通道
		Video,       // TODO: 用于视频

		_COUNT,
	};

	struct IAudioPlayer : public IReferenceCounted
	{
		virtual bool start() = 0;
		virtual bool stop() = 0;
		virtual bool reset() = 0;

		virtual bool isPlaying() = 0;

		virtual double getTotalTime() = 0;
		virtual double getTime() = 0;
		virtual bool setTime(double t) = 0;
		virtual bool setLoop(bool enable, double start_pos, double length) = 0;
		
		virtual float getVolume() = 0;
		virtual bool setVolume(float v) = 0;
		virtual float getBalance() = 0;
		virtual bool setBalance(float v) = 0;
		virtual float getSpeed() = 0;
		virtual bool setSpeed(float v) = 0;

		virtual void updateFFT() = 0;
		virtual uint32_t getFFTSize() = 0;
		virtual float* getFFT() = 0;
	};

	struct IAudioDevice : public IReferenceCounted
	{
		virtual uint32_t getAudioDeviceCount(bool refresh) = 0;
		virtual std::string_view getAudioDeviceName(uint32_t index) const noexcept = 0;
		virtual bool setTargetAudioDevice(std::string_view const audio_device_name) = 0;
		virtual std::string_view getCurrentAudioDeviceName() const noexcept = 0;

		virtual void setVolume(float v) = 0;
		virtual float getVolume() = 0;
		virtual void setMixChannelVolume(MixChannel ch, float v) = 0;
		virtual float getMixChannelVolume(MixChannel ch) = 0;

		virtual bool createAudioPlayer(IDecoder* p_decoder, IAudioPlayer** pp_player) = 0; // 全部解码到内存中
		virtual bool createLoopAudioPlayer(IDecoder* p_decoder, IAudioPlayer** pp_player) = 0; // 全部解码到内存中
		virtual bool createStreamAudioPlayer(IDecoder* p_decoder, IAudioPlayer** pp_player) = 0; // 播放时才逐步解码
	};
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IAudioPlayer
	template<> constexpr InterfaceId getInterfaceId<Audio::IAudioPlayer>() { return UUID::parse("fcdf8d18-b862-5de2-8732-2b9bf7d42d88"); }

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IAudioDevice
	template<> constexpr InterfaceId getInterfaceId<Audio::IAudioDevice>() { return UUID::parse("586a1629-1e5d-5757-a481-7560ef3fca8a"); }
}
