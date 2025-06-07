#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
	//      /------------------stop--------------------\
	//      v                                          |
	// /---------\ <-stop-- /---------\ <-resume-- /--------\
	// | stopped |          | playing |            | paused |
	// \---------/ --play-> \---------/ --pause--> \--------/

	enum class AudioPlayerState : uint8_t {
		stopped = 0,
		playing = 1,
		paused = 2,
	};

	struct CORE_NO_VIRTUAL_TABLE IAudioPlayer : IReferenceCounted {
		virtual bool play(double seconds) = 0;
		virtual bool pause() = 0;
		virtual bool resume() = 0;
		virtual bool stop() = 0;
		virtual AudioPlayerState getState() = 0;

		virtual double getTotalTime() = 0;
		virtual double getTime() = 0;
		virtual bool setLoop(bool enable, double start_pos, double length) = 0;

		virtual float getVolume() = 0;
		virtual bool setVolume(float volume) = 0;
		virtual float getBalance() = 0;
		virtual bool setBalance(float v) = 0;
		virtual float getSpeed() = 0;
		virtual bool setSpeed(float speed) = 0;

		virtual void updateFFT() = 0;
		virtual uint32_t getFFTSize() = 0;
		virtual float const* getFFT() = 0;
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IAudioPlayer
	template<> constexpr InterfaceId getInterfaceId<IAudioPlayer>() { return UUID::parse("fcdf8d18-b862-5de2-8732-2b9bf7d42d88"); }
}
