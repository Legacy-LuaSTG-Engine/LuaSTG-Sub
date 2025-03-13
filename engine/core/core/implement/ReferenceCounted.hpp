#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/implement/ReferenceCountedDebugger.hpp"
#include <atomic>

namespace core::implement {
	template<typename T>
	class ReferenceCounted : public T {
	public:
		void reference() override {
			m_counter.fetch_add(1);
		}
		void release() override {
			if (auto const last_counter = m_counter.fetch_sub(1); last_counter == 1) {
				delete this;
			}
		}

		ReferenceCounted() : m_counter{ 1 } {
		#ifndef NDEBUG
			ReferenceCountedDebugger::startTracking(this);
		#endif
		}
		ReferenceCounted(ReferenceCounted const&) = delete;
		ReferenceCounted(ReferenceCounted&&) = delete;
		virtual ~ReferenceCounted() {
		#ifndef NDEBUG
			ReferenceCountedDebugger::stopTracking(this);
		#endif
		}

		ReferenceCounted& operator=(ReferenceCounted const&) = delete;
		ReferenceCounted& operator=(ReferenceCounted&&) = delete;

	private:
		std::atomic_int32_t m_counter;
	};
}
