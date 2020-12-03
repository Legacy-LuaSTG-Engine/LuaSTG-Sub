#pragma once

namespace slow {
	struct Mutex {
		virtual void lock() = 0;
		virtual bool trylock() = 0;
		virtual void unlock() = 0;
	};
};
