#pragma once
#include "slow/Mutex.hpp"
#include <mutex>

namespace slow {
	template<class T>
	class MutexImpl : public T {
	private:
		std::recursive_mutex _mutex;
	public:
		void lock() {
			_mutex.lock();
		}
		bool trylock() {
			return _mutex.try_lock();
		}
		void unlock() {
			_mutex.unlock();
		}
	public:
		MutexImpl() {}
		virtual ~MutexImpl() {}
	};
};
