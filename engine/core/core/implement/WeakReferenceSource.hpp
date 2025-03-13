#pragma once
#include "core/WeakReferenceSource.hpp"
#include "core/implement/ReferenceCountedDebugger.hpp"
#include <atomic>

namespace core::implement {
	struct ReferenceCounter {
		std::atomic_int32_t strong{ 1 };
		std::atomic_int32_t weak{ 1 };
	};

	template<typename T>
	class WeakReferenceSource : public T {
		template<typename Target>
		friend class WeakReference;
	public:
		void reference() override {
			m_counter->strong.fetch_add(1);
		}
		void release() override {
			auto const counter = m_counter;
			if (auto const last_strong = counter->strong.fetch_sub(1); last_strong == 1) {
				delete this;
				if (auto const last_weak = counter->weak.fetch_sub(1); last_weak == 1) {
					delete counter;
				}
			}
		}

		WeakReferenceSource() : m_counter(new ReferenceCounter()) {
		#ifndef NDEBUG
			ReferenceCountedDebugger::startTracking(this);
		#endif
		}
		WeakReferenceSource(WeakReferenceSource const&) = delete;
		WeakReferenceSource(WeakReferenceSource&&) = delete;
		virtual ~WeakReferenceSource() {
		#ifndef NDEBUG
			ReferenceCountedDebugger::stopTracking(this);
		#endif
		}

		WeakReferenceSource& operator=(WeakReferenceSource const&) = delete;
		WeakReferenceSource& operator=(WeakReferenceSource&&) = delete;

	private:
		ReferenceCounter* m_counter;
	};
}
