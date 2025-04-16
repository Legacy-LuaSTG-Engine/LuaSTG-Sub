#pragma once
#include "core/WeakReferenceSource.hpp"
#include "core/implement/ReferenceCountedDebugger.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/implement/WeakReference.hpp"

namespace core::implement {
	template<typename T>
	class WeakReferenceSource : public T {
	public:
		int32_t retain() override {
			return m_counters->strong.fetch_add(1) + 1;
		}
		int32_t release() override {
			auto const counters = m_counters;
			auto const last_strong = counters->strong.fetch_sub(1);
			if (last_strong == 1) {
				delete this;
				if (auto const last_weak = counters->weak.fetch_sub(1); last_weak == 1) {
					delete counters;
				}
			}
			return last_strong + 1;
		}
		bool queryInterface(InterfaceId const& uuid, void** const output) override {
			assert(output != nullptr);
			if (uuid == getInterfaceId<IReferenceCounted>()) {
				retain();
				*output = static_cast<IReferenceCounted*>(this);
				return true;
			}
			if (uuid == getInterfaceId<IWeakReferenceSource>()) {
				retain();
				*output = static_cast<IWeakReferenceSource*>(this);
				return true;
			}
			if (uuid == getInterfaceId<T>()) {
				retain();
				*output = static_cast<T*>(this);
				return true;
			}
			*output = nullptr;
			return false;
		}
		void getWeakReference(IWeakReference** const output) override {
			assert(output != nullptr);
			*output = new WeakReference(m_counters, this);
		}

		WeakReferenceSource() : m_counters(new ReferenceCounter()) {
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
		ReferenceCounter* m_counters;
	};
}
