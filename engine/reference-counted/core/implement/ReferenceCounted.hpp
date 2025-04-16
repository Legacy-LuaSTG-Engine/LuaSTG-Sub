#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/implement/ReferenceCountedDebugger.hpp"
#include <cassert>
#include <atomic>

namespace core::implement {
	struct ReferenceCounter {
		std::atomic_int32_t strong{ 1 };
		std::atomic_int32_t weak{ 1 };
	};

	template<typename T>
	class ReferenceCounted : public T {
	public:
		int32_t reference() override {
			return m_counter.fetch_add(1) + 1;
		}
		int32_t release() override {
			auto const last_counter = m_counter.fetch_sub(1);
			if (last_counter == 1) {
				delete this;
			}
			return last_counter - 1;
		}
		bool queryInterface(UUID const& uuid, void** const output) override {
			assert(output != nullptr);
			if (uuid == uuid_of<IReferenceCounted>()) {
				reference();
				*output = static_cast<IReferenceCounted*>(this);
				return true;
			}
			if (uuid == uuid_of<T>()) {
				reference();
				*output = static_cast<T*>(this);
				return true;
			}
			*output = nullptr;
			return false;
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
