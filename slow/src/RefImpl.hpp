#pragma once
#include "slow/Ref.hpp"
#include <cstdint>
#include <atomic>

namespace slow {
	template<class T>
	class RefImpl : public T {
	private:
		std::atomic_int32_t _count;
	public:
		int32_t retain() {
			return _count.fetch_add(1) + 1;
		}
		int32_t release() {
			int32_t n = _count.fetch_sub(1);
			if (n <= 1) {
				delete this;
			}
			return n - 1;
		}
	public:
		RefImpl() : _count(1) {}
		virtual ~RefImpl() {}
	};
};
